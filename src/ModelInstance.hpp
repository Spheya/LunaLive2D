#pragma once

#include <luna.hpp>

#include "Model.hpp"
#include "Renderer.hpp"
#include "Parameter.hpp"

struct csmMoc;
struct csmModel;

namespace luna {
	namespace live2d {

		/**
		 * @brief An instance of the Live2D model. This class provides access to the model's
		 * parameters and drawables. A Renderer also requires a ModelInstance to render the
		 * model.
		*/
		class ModelInstance {
		public:
			/**
			 * @param model A pointer to the Model resource. This pointer has to stay valid 
			 * throughout the lifespan of this instance.
			*/
			explicit ModelInstance(Model* model = nullptr);

			/**
			 * @brief Update the physics, parameters, and vertices of this model.
			 * @param deltatime Duration of the previous frame
			*/
			void update(float deltatime);

			Model* getModel();
			const Model* getModel() const;

			PhysicsController* getPhysicsController();
			const PhysicsController* getPhysicsController() const;

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

			std::unique_ptr<PhysicsController> m_physicsController;

			luna::Transform m_transform;

			glm::vec2 m_canvasSize = glm::vec2(0.0f);
			glm::vec2 m_canvasOrigin = glm::vec2(0.0f);
			float m_pixelsPerUnit = 0.0f;

			std::vector<Drawable> m_drawables;
			std::vector<Parameter> m_parameters;
		};

	}
}