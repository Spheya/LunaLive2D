#pragma once

#include <vector>
#include <string>
#include <functional>
#include <luna.hpp>

#include "Parameter.hpp"

namespace luna {
	namespace live2d {

		class ModelInstance;

		enum class PhysicsParameterType : uint8_t {
			Angle, X, Y
		};

		struct NormalizationParams {
			float min;
			float max;
			float base;
		};

		struct PhysicsInput {
			std::string paramId;
			PhysicsParameterType type;
			float weight;
			bool reflect;
		};

		struct PhysicsOutput {
			std::string paramId;
			PhysicsParameterType type;
			int pendulumNodeIndex;
			float weight;
			float scale;
			bool reflect;
		};

		struct PhysicsPendulumNode {
			glm::vec2 initialPosition;
			float mobility;
			float delay;
			float acceleration;
			float radius;

			glm::vec2 position;
			glm::vec2 velocity;
		};

		/**
		 * @brief Called a Group in the physics settings within the Live2D application. This represents a single pendulum simulation with multiple Parameters as input/output.
		*/
		class PhysicsGroup {
		public:
			PhysicsGroup(std::string id, NormalizationParams positionNormalizationParams, NormalizationParams angleNormalizationParams, std::vector<PhysicsPendulumNode> nodes, std::vector<PhysicsInput> inputs, std::vector<PhysicsOutput> outputs);

			/**
			 * @brief Attaches this PhysicsGroup to a ModelInstance, so when you call update() on this
			 * controller, it will update the ModelInstance's parameters.
			 * @param instance The ModelInstance you want this PhysicsGroup to be attached to
			*/
			void attachTo(ModelInstance* instance);

			/**
			 * @brief Updates the pendulum simulation and writes to the values of all the connected parameters
			*/
			void update(float deltatime);

			/**
			 * @brief Sets the simulated pendulum in a state where all forces on it cancel out, so there
			 * will be no movement  until one of the input parameters changes. It might make sense to call
			 * this after changing the Parameters in a non-continuous way to avoid a sudden jerk in the
			 * physics.
			*/
			void stabilize();

			size_t getNodeCount();
			PhysicsPendulumNode* getNodes();
			const PhysicsPendulumNode* getNodes() const;

		private:
			void readCurrentState(float& rotation, glm::vec2& position);
			void writeCurrentState();

		private:
			std::string m_id;
			std::vector<PhysicsPendulumNode> m_nodes;
			std::vector<PhysicsInput> m_inputs;
			std::vector<PhysicsOutput> m_outputs;
			std::vector<Parameter*> m_inputParams;
			std::vector<Parameter*> m_outputParams;
			NormalizationParams m_positionNormalizationParams;
			NormalizationParams m_angleNormalizationParams;
			glm::vec2 m_prevGravity;
		};

		/**
		 * @brief Controls the physics of a ModelInstance
		*/
		class PhysicsController {
		public:
			PhysicsController(const char* filepath);

			/**
			 * @brief Attaches this PhysicsController to a ModelInstance, so when you call update() on this
			 * controller, it will update the ModelInstance's parameters.
			 * @param instance The ModelInstance you want this PhysicsController to be attached to
			*/
			void attachTo(ModelInstance* instance);

			/**
			 * @brief Updates the physics of the model and writes to the values of all the connected parameters
			*/
			void update(float deltatime);

			/**
			 * @brief Stabilizes the PhysicsController, that is putting it in a state where it is stable and all
			 * forces cancel out so there will be no movement coming from the physics until one of the input
			 * parameters changes. It might make sense to call this after changing the model's parameters in a
			 * non-continuous way to avoid a sudden jerk in the physics.
			*/
			void stabilize();

			size_t getGroupCount();
			PhysicsGroup* getGroups();
			const PhysicsGroup* getGroups() const;

		private:
			std::vector<PhysicsGroup> m_groups;
		};

	}
}