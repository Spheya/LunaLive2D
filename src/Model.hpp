#pragma once

#include <luna.hpp>

#include "Physics.hpp"

struct csmMoc;
struct csmModel;

namespace luna {
	namespace live2d {
		void initialize();
		void terminate();

		using CoreMoc = std::unique_ptr<csmMoc, void(*)(void*)>;
		using CoreModel = std::unique_ptr<csmModel, void(*)(void*)>;

		/**
		 * @brief A Live2D model, the file it needs is the .model3.json file outputted by Live2D.
		 * This class only imports the file, to actually render the model, see ModelInstance.hpp
		*/
		class Model {
		public:
			enum LoadFlags {
				None = 0x0,
				NoPhysics = 0x1,
			};

			Model();
			/**
			 * @brief Loads in the model from a file
			 * @param filepath The path to the .model3.json file
			*/
			explicit Model(const char* filepath, LoadFlags = None);

			/**
			 * @brief Loads in the model from a file
			 * @param filepath The path to the .model3.json file
			*/
			void load(const char* filepath, LoadFlags = None);

			/**
			 * @brief Removes all the contents of this Model
			*/
			void reset();

			/**
			 * @brief Creates a csmModel based on the internal csmMoc of this class.
			 * @return A new csmModel based on the .moc file this class has loaded.
			*/
			CoreModel createCoreModel() const;

			/**
			 * @brief Creates a PhysicsController based on the loaded model
			 * @return A new PhysicsController based on the .physics3.json file that
			 * this class has loaded.
			*/
			std::unique_ptr<PhysicsController> createPhysicsController() const;

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

		private:
			static void* readFileAligned(const char* path, unsigned int alignment, size_t& size);
			void loadMoc(const char* filepath);

		private:
			CoreMoc m_moc;

			std::unique_ptr<PhysicsController> m_physicsControllerPrototype;

			std::vector<luna::Texture> m_textures;
			std::vector<luna::Material> m_materials;
		};

	}
}