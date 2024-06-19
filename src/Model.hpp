#pragma once

#include <luna.hpp>

struct csmMoc;
struct csmModel;

namespace luna {
	namespace live2d {
		using CoreMoc = std::unique_ptr<csmMoc, void(*)(void*)>;
		using CoreModel = std::unique_ptr<csmModel, void(*)(void*)>;

		/**
		 * @brief A Live2D model, the file it needs is the .model3.json file outputted by Live2D.
		 * This class only imports the file, to actually render the model, see ModelInstance.hpp
		*/
		class Model {
		public:
			Model();
			/**
			 * @brief Loads in the model from a file
			 * @param filepath The path to the .model3.json file
			*/
			explicit Model(const char* filepath);

			/**
			 * @brief Loads in the model from a file
			 * @param filepath The path to the .model3.json file
			*/
			void load(const char* filepath);

			/**
			 * @brief Creates a csmModel based on the internal csmMoc of this class.
			 * @return A new csmModel based on the .moc file this class has loaded.
			*/
			CoreModel createCoreModel() const;

			bool isValid() const;
			const CoreMoc& getMoc() const;
			CoreMoc& getMoc();

			size_t getTextureCount() const;
			const luna::Texture* getTextures() const;
			luna::Texture* getTextures();

			size_t getMaterialCount() const;
			const luna::Material* getMaterials() const;
			luna::Material* getMaterials();

			static luna::Shader* getShader();

			static void loadShaders();
			static void unloadShaders();

		private:
			static void* readFileAligned(const char* path, unsigned int alignment, size_t& size);
			void loadMoc(const char* filepath);

		private:
			static std::unique_ptr<luna::Shader> s_shader;

			CoreMoc m_moc;

			std::vector<luna::Texture> m_textures;
			std::vector<luna::Material> m_materials;
		};

	}
}