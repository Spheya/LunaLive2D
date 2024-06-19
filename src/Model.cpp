#include "Model.hpp"

#include <filesystem>
#include <fstream>
#include <cassert>
#include <nlohmann/json.hpp>
#include <Live2DCubismCore.h>

#include "AlignedAllocator.hpp"

using json = nlohmann::json;

namespace luna {
	namespace live2d {

		std::unique_ptr<luna::Shader> Model::s_shader;

		Model::Model(Model&& other) noexcept :
			m_transform(other.m_transform),
			m_moc(other.m_moc),
			m_model(other.m_model),
			m_mocMemory(other.m_mocMemory),
			m_modelMemory(other.m_modelMemory),
			m_canvasSize(other.m_canvasSize),
			m_canvasOrigin(other.m_canvasOrigin),
			m_pixelsPerUnit(other.m_pixelsPerUnit),
			m_parameters(std::move(other.m_parameters)),
			m_drawables(std::move(other.m_drawables)),
			m_textures(std::move(other.m_textures)),
			m_materials(std::move(other.m_materials))
		{
			other.m_moc = nullptr;
			other.m_model = nullptr;
			other.m_mocMemory = nullptr;
			other.m_modelMemory = nullptr;
		}

		Model& Model::operator=(Model&& other) noexcept {
			Model::~Model();

			m_transform = other.m_transform;
			m_moc = other.m_moc;
			m_model = other.m_model;
			m_mocMemory = other.m_mocMemory;
			m_modelMemory = other.m_modelMemory;
			m_canvasSize = other.m_canvasSize;
			m_canvasOrigin = other.m_canvasOrigin;
			m_pixelsPerUnit = other.m_pixelsPerUnit;
			m_parameters = std::move(other.m_parameters);
			m_drawables = std::move(other.m_drawables);
			m_textures = std::move(other.m_textures);
			m_materials = std::move(other.m_materials);

			other.m_moc = nullptr;
			other.m_model = nullptr;
			other.m_mocMemory = nullptr;
			other.m_modelMemory = nullptr;

			return *this;
		}

		Model::~Model() {
			if (m_mocMemory)
				AlignedAllocator::deallocate(m_mocMemory);

			if (m_modelMemory)
				AlignedAllocator::deallocate(m_modelMemory);
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

			s_shader = std::make_unique<luna::Shader>(vertSrc, fragSrc);
			auto& program = s_shader->getProgram();
			program.setDepthTestMode(luna::DepthTestMode::Off);
			program.setRenderQueue(luna::RenderQueue::Transparent);
			program.setBlendMode(luna::BlendMode::On);
			program.setCullMode(luna::CullMode::Off);
		}

		void Model::unloadShaders() {
			s_shader.reset();
		}

		void Model::load(const char* filepath) {
			if (m_mocMemory)
				AlignedAllocator::deallocate(m_mocMemory);
			if (m_modelMemory)
				AlignedAllocator::deallocate(m_modelMemory);
			m_drawables.clear();
			m_materials.clear();
			m_textures.clear();
			m_parameters.clear();

			if (!s_shader) {
				log("Shaders have to be loaded before loading a  model. Use Model::loadShaders()", MessageSeverity::Error);
				return;
			}

			auto root = std::filesystem::path(filepath).parent_path();
			std::string rootStr = root.string() + "/";

			std::ifstream file(filepath);
			if (file.bad()) {
				log("Could not open file at \"" + std::string(filepath) + "\"", MessageSeverity::Error);
				return;
			}
			json modelFile = json::parse(file);

			auto& fileReferences = modelFile.at("FileReferences");
			std::string mocPath = fileReferences.at("Moc");
			auto& textureReferences = fileReferences.at("Textures");

			for (auto it = textureReferences.begin(); it != textureReferences.end(); ++it) {
				std::string texturePath = it.value();
				m_textures.push_back(luna::Texture::loadFromFile((rootStr + texturePath).c_str()));
				m_textures.back().generateMipmap();
				m_textures.back().enableAnisotropicFiltering(4.0f);
				m_materials.emplace_back(s_shader.get());
				m_materials.back().setMainTexture(&m_textures.back());
			}

			loadMoc((rootStr + mocPath).c_str());

			log("Live2D model loaded successfully (" + std::string(filepath) + ")", MessageSeverity::Info);
		}

		bool Model::isValid() const {
			return m_moc && m_model;
		}

		void Model::update(float deltatime) {
			csmResetDrawableDynamicFlags(m_model);
			csmUpdateModel(m_model);
		}

		void Model::setTransform(const Transform& transform) {
			m_transform = transform;
		}

		const Transform& Model::getTransform() const {
			return m_transform;
		}

		Transform& Model::getTransform() {
			return m_transform;
		}

		glm::vec2 Model::getCanvasSize() const {
			return m_canvasSize;
		}

		glm::vec2 Model::getCanvasOrigin() const {
			return m_canvasOrigin;
		}

		float Model::getPixelsPerUnit() const {
			return m_pixelsPerUnit;
		}

		size_t Model::getDrawableCount() const {
			return m_drawables.size();
		}

		Drawable* Model::getDrawables() {
			return m_drawables.data();
		}

		const Drawable* Model::getDrawables() const {
			return m_drawables.data();
		}

		const Drawable* Model::getDrawable(const char* id) const {
			std::hash<std::string> hasher;
			auto it = std::find_if(m_drawables.begin(), m_drawables.end(), [hash = hasher(id)](const Drawable& x) { return x.getIdHash() == hash; });
			return it == m_drawables.end() ? &(*it) : nullptr;
		}

		Drawable* Model::getDrawable(const char* id) {
			std::hash<std::string> hasher;
			auto it = std::find_if(m_drawables.begin(), m_drawables.end(), [hash = hasher(id)](const Drawable& x) { return x.getIdHash() == hash; });
			return it == m_drawables.end() ? &(*it) : nullptr;
		}

		size_t Model::getParameterCount() const {
			return m_parameters.size();
		}

		Parameter* Model::getParameters() {
			return m_parameters.data();
		}

		const Parameter* Model::getParameters() const {
			return m_parameters.data();
		}

		const Parameter* Model::getParameter(const char* id) const {
			std::hash<std::string> hasher;
			auto it = std::find_if(m_parameters.begin(), m_parameters.end(), [hash = hasher(id)](const Parameter& x) { return x.getIdHash() == hash; });
			return it == m_parameters.end() ? &(*it) : nullptr;
		}

		Parameter* Model::getParameter(const char* id) {
			std::hash<std::string> hasher;
			auto it = std::find_if(m_parameters.begin(), m_parameters.end(), [hash = hasher(id)](const Parameter& x) { return x.getIdHash() == hash; });
			return it == m_parameters.end() ? &(*it) : nullptr;
		}

		void Model::loadMoc(const char* path) {
			constexpr int csmAlignofMoc = 64;
			constexpr int csmAlignofModel = 16;

			// read file
			size_t mocSize;
			m_mocMemory = readFileAligned(path, csmAlignofMoc, mocSize);
			if (m_mocMemory == nullptr)
				return;

			// check for malformation
			int consistency = csmHasMocConsistency(m_mocMemory, unsigned(mocSize));
			if (!consistency) {
				log("Live2D model file is malformed (" + std::string(path) + ")", MessageSeverity::Error);
				return;
			}

			// load file and model
			m_moc = csmReviveMocInPlace(m_mocMemory, unsigned(mocSize));
			unsigned int modelSize = csmGetSizeofModel(m_moc);
			m_modelMemory = AlignedAllocator::allocate(modelSize, csmAlignofModel);
			m_model = csmInitializeModelInPlace(m_moc, m_modelMemory, modelSize);

			// load canvas info
			csmVector2 size;
			csmVector2 origin;
			csmReadCanvasInfo(m_model, &size, &origin, &m_pixelsPerUnit);
			m_canvasSize = glm::vec2(size.X, size.Y) / m_pixelsPerUnit;
			m_canvasOrigin = glm::vec2(origin.X, origin.Y) / m_pixelsPerUnit;

			// set up values
			initializeDrawables();
			initializeParameters();
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

		void Model::initializeDrawables() {
			assert(m_drawables.empty());

			int drawableCount = csmGetDrawableCount(m_model);
			m_drawables.reserve(drawableCount);

			const int* textureIndices = csmGetDrawableTextureIndices(m_model);
			const int* vertexCounts = csmGetDrawableVertexCounts(m_model);
			const csmVector2** vertexPositions = csmGetDrawableVertexPositions(m_model);
			const csmVector2** vertexUvs = csmGetDrawableVertexUvs(m_model);
			const int* indexCounts = csmGetDrawableIndexCounts(m_model);
			const unsigned short** indices = csmGetDrawableIndices(m_model);

			const char** ids = csmGetDrawableIds(m_model);
			const float* opacities = csmGetDrawableOpacities(m_model);
			const int* drawOrders = csmGetDrawableDrawOrders(m_model);
			const int* renderOrders = csmGetDrawableRenderOrders(m_model);
			const csmFlags* constantFlags = csmGetDrawableConstantFlags(m_model);
			const csmFlags* dynamicFlags = csmGetDrawableDynamicFlags(m_model);

			const csmVector4* multiplyColors = csmGetDrawableMultiplyColors(m_model);
			const csmVector4* screenColors = csmGetDrawableScreenColors(m_model);

			const int* maskCounts = csmGetDrawableMaskCounts(m_model);
			const int** masks = csmGetDrawableMasks(m_model);

			for (int i = 0; i < drawableCount; ++i) {
				if (textureIndices[i] >= m_textures.size()) {
					log("Drawable \"" + std::string(ids[i]) + "\" has an invalid texture index", MessageSeverity::Warning);
					continue;
				}

				m_drawables.emplace_back(
					&vertexCounts[i],
					vertexPositions[i],
					vertexUvs[i],
					&indexCounts[i],
					indices[i],
					ids[i],
					&opacities[i],
					&drawOrders[i],
					&renderOrders[i],
					&constantFlags[i],
					&dynamicFlags[i],
					&multiplyColors[i],
					&screenColors[i],
					&maskCounts[i],
					masks[i],
					&m_materials[textureIndices[i]],
					&m_textures[textureIndices[i]]
				);
			}
		}

		void Model::initializeParameters() {
			assert(m_parameters.empty());

			size_t paramCount = size_t(csmGetParameterCount(m_model));
			m_parameters.reserve(paramCount);

			const char** ids = csmGetParameterIds(m_model);
			const float* minValues = csmGetParameterMinimumValues(m_model);
			const float* maxValues = csmGetParameterMaximumValues(m_model);
			const float* defaultValues = csmGetParameterDefaultValues(m_model);
			float* values = csmGetParameterValues(m_model);

			for (size_t i = 0; i < paramCount; ++i)
				m_parameters.push_back(Parameter(ids[i], &minValues[i], &maxValues[i], &defaultValues[i], &values[i]));
		}

		const csmMoc* Model::getMoc() const {
			return m_moc;
		}

		const csmModel* Model::getModel() const {
			return m_model;
		}

	}
}