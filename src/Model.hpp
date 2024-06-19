#pragma once

#include <luna.hpp>

#include "Renderer.hpp"
#include "Parameter.hpp"

struct csmMoc;
struct csmModel;

namespace luna {
	namespace live2d {

		class Model {
		public:
			Model() = default;
			Model(Model&) = delete;
			Model& operator=(Model&) = delete;
			Model(Model&& other) noexcept;
			Model& operator=(Model&& other) noexcept;
			~Model();

			static void loadShaders();
			static void unloadShaders();

			void load(const char* filepath);
			bool isValid() const;
			void update(float deltatime);

			void setTransform(const Transform& transform);
			const Transform& getTransform() const;
			Transform& getTransform();

			glm::vec2 getCanvasSize() const;
			glm::vec2 getCanvasOrigin() const;
			float getPixelsPerUnit() const;

			size_t getDrawableCount() const;
			Drawable* getDrawables();
			const Drawable* getDrawables() const;
			const Drawable* getDrawable(const char* id) const;
			Drawable* getDrawable(const char* id);

			size_t getParameterCount() const;
			Parameter* getParameters();
			const Parameter* getParameters() const;
			const Parameter* getParameter(const char* id) const;
			Parameter* getParameter(const char* id);

		private:
			void loadMoc(const char* path);
			static void* readFileAligned(const char* path, unsigned int alignment, size_t& size);

			void initializeDrawables();
			void initializeParameters();

			const csmMoc* getMoc() const;
			const csmModel* getModel() const;

		private:
			static std::unique_ptr<luna::Shader> s_shader;

			luna::Transform m_transform;

			csmMoc* m_moc = nullptr;
			csmModel* m_model = nullptr;
			void* m_mocMemory = nullptr;
			void* m_modelMemory = nullptr;

			glm::vec2 m_canvasSize = glm::vec2(0.0f);
			glm::vec2 m_canvasOrigin = glm::vec2(0.0f);
			float m_pixelsPerUnit = 0.0f;

			std::vector<Parameter> m_parameters;
			std::vector<Drawable> m_drawables;

			// rendering data
			std::vector<luna::Texture> m_textures;
			std::vector<luna::Material> m_materials;
		};

	}
}