#pragma once

#include <luna.hpp>

#include "Model.hpp"
#include "Renderer.hpp"
#include "Parameter.hpp"

struct csmMoc;
struct csmModel;

namespace luna {
	namespace live2d {

		class ModelInstance {
		public:
			ModelInstance(Model* model);

			void update(float deltatime);

			Model& getModel();
			const Model& getModel() const;

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
			void initializeDrawables();
			void initializeParameters();

		private:
			CoreModel m_coreModel;
			Model* m_model;

			luna::Transform m_transform;

			glm::vec2 m_canvasSize = glm::vec2(0.0f);
			glm::vec2 m_canvasOrigin = glm::vec2(0.0f);
			float m_pixelsPerUnit = 0.0f;

			std::vector<Drawable> m_drawables;
			std::vector<Parameter> m_parameters;
		};

	}
}