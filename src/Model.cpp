#include "Model.hpp"

#include <filesystem>
#include <fstream>
#include <Live2DCubismCore.h>
#include <nlohmann/json.hpp>

#include "AlignedAllocator.hpp"

using json = nlohmann::json;

namespace luna {
	namespace live2d {
		namespace {
			constexpr int csmAlignofMoc = 64;
			constexpr int csmAlignofModel = 16;
		}

		std::unique_ptr<luna::Shader> Model::s_shader;

		Model::Model() : m_moc(nullptr, AlignedAllocator::deallocate) {}

		Model::Model(const char* filepath, LoadFlags flags) : Model() {
			load(filepath, flags);
		}

		void Model::load(const char* filepath, LoadFlags flags) {
			// unload the model
			m_moc.reset();
			m_physicsControllerPrototype.reset();
			m_textures.clear();
			m_materials.clear();

			// open model file
			auto root = std::filesystem::path(filepath).parent_path();
			std::string rootStr = root.string() + "/";

			std::ifstream file(filepath);
			if (file.bad() || file.fail() || file.eof()) {
				log("Could not open file at \"" + std::string(filepath) + "\"", MessageSeverity::Error);
				return;
			}
			json modelFile = json::parse(file);
			auto& fileReferences = modelFile.at("FileReferences");

			// load textures
			auto& textureReferences = fileReferences.at("Textures");
			for (std::string texturePath : textureReferences) {
				m_textures.push_back(luna::Texture::loadFromFile((rootStr + texturePath).c_str()));
				m_textures.back().generateMipmap();
				m_textures.back().enableAnisotropicFiltering(4.0f);
				m_materials.emplace_back(s_shader.get());
				m_materials.back().setMainTexture(&m_textures.back());
			}

			// load physics
			if (!(flags & NoPhysics)) {
				std::string physicsPath = fileReferences.at("Physics");
				m_physicsControllerPrototype = std::make_unique<PhysicsController>((rootStr + physicsPath).c_str());
			}

			// load .moc file
			std::string mocPath = fileReferences.at("Moc");
			loadMoc((rootStr + mocPath).c_str());
		}

		CoreModel Model::createCoreModel() const {
			unsigned int modelSize = csmGetSizeofModel(m_moc.get());
			void* modelMemory = AlignedAllocator::allocate(modelSize, csmAlignofModel);
			return CoreModel(csmInitializeModelInPlace(m_moc.get(), modelMemory, modelSize), AlignedAllocator::deallocate);
		}


		std::unique_ptr<PhysicsController> Model::createPhysicsController() const {
			if (!m_physicsControllerPrototype)
				return nullptr;
			return std::make_unique<PhysicsController>(*m_physicsControllerPrototype);
		}

		bool Model::isValid() const {
			return m_moc.get();
		}

		const CoreMoc& Model::getMoc() const {
			return m_moc;
		}

		CoreMoc& Model::getMoc() {
			return m_moc;
		}

		size_t Model::getTextureCount() const {
			return m_textures.size();
		}

		const luna::Texture* Model::getTextures() const {
			return m_textures.data();
		}

		luna::Texture* Model::getTextures() {
			return m_textures.data();
		}

		size_t Model::getMaterialCount() const {
			return m_materials.size();
		}

		const luna::Material* Model::getMaterials() const {
			return m_materials.data();
		}

		luna::Material* Model::getMaterials() {
			return m_materials.data();
		}
		
		luna::Shader* Model::getShader() {
			return s_shader.get();
		}

		void Model::loadShaders() {
			const char* vertSrc =
				"#version 430 core\n"

				"layout(location = 0) in vec3 Position;"
				"layout(location = 1) in vec2 UV;"
				"layout(location = 2) in vec3 ScreenColor;"
				"layout(location = 3) in vec4 MultColor;"

				"out vec3 screenColor;"
				"out vec4 multiplyColor;"
				"out vec2 uv;"
				"out vec4 clipPos;"

				"uniform vec4 MainColor;"
				"uniform vec4 MainTexture_ST;"

				"layout(std140, binding = 0) uniform CameraMatrices {"
				"	mat4 ProjectionMatrix;"
				"	mat4 ViewMatrix;"
				"};"

				"uniform mat4 ModelMatrix;"

				"void main() {"
				"	screenColor = ScreenColor;"
				"	multiplyColor = MainColor * MultColor;"
				"	uv = UV * MainTexture_ST.xy + MainTexture_ST.zw;"
				"	clipPos = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position, 1.0);"
				"	gl_Position = clipPos;"
				"}";

			const char* fragSrc =
				"#version 430 core\n"

				"in vec3 screenColor;"
				"in vec4 multiplyColor;"
				"in vec2 uv;"
				"in vec4 clipPos;"

				"uniform sampler2D MainTexture;"
				"uniform sampler2D Live2DMaskTexture;"
				"uniform int Live2DMaskTextureInversed;"

				"out vec4 fragColor;"

				"float getMask() {"
				"	float mask = texture(Live2DMaskTexture, (clipPos.xy / clipPos.w) * 0.5 + 0.5).a;"
				"	if (bool(Live2DMaskTextureInversed)) mask = 1.0 - mask;"
				"	return mask;"
				"}"

				"void main() {"
				"	fragColor = texture(MainTexture, uv) * multiplyColor;"
				"	fragColor.rgb = vec3(1.0) - (vec3(1.0) - screenColor) * (vec3(1.0) - fragColor.rgb);"
				"	fragColor.a *= getMask();"

				"	if (fragColor.a == 0.0) discard;"
				"}";

			s_shader = std::make_unique<Shader>(vertSrc, fragSrc);
			s_shader->getProgram().setBlendMode(BlendMode::On);
			s_shader->getProgram().setCullMode(CullMode::Off);
			s_shader->getProgram().setDepthTestMode(DepthTestMode::Off);
		}

		void Model::unloadShaders() {
			s_shader.reset();
		}

		void* Model::readFileAligned(const char* path, unsigned int alignment, size_t& size) {
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (file.fail()) {
				log("File could not be opened (" + std::string(path) + ")", MessageSeverity::Error);
				return nullptr;
			}

			size = file.tellg();
			file.seekg(0, std::ios::beg);
			size -= file.tellg();
			char* data = (char*)AlignedAllocator::allocate(size, alignment);
			file.read(data, size);

			return data;
		}

		void Model::loadMoc(const char* filepath) {
			// read file
			size_t mocSize;
			void* mocMemory = readFileAligned(filepath, csmAlignofMoc, mocSize);
			if (mocMemory == nullptr)
				return;

			// check for malformation
			int consistency = csmHasMocConsistency(mocMemory, unsigned(mocSize));
			if (!consistency) {
				log("Live2D model file is malformed (" + std::string(filepath) + ")", MessageSeverity::Error);
				return;
			}

			// load file and model
			m_moc = CoreMoc(csmReviveMocInPlace(mocMemory, unsigned(mocSize)), AlignedAllocator::deallocate);
		}
	}
}