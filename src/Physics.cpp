#include "Physics.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

#include "ModelInstance.hpp"

using json = nlohmann::json;

namespace luna {
	namespace live2d {

		namespace {
			float directionToRadian(glm::vec2 from, glm::vec2 to) {
				float q1 = atan2f(to.y, to.x);
				float q2 = atan2f(from.y, from.x);
				float ret = q1 - q2;

				while (ret < -luna::Pi)
					ret += luna::Tau;

				while (ret > luna::Pi)
					ret -= luna::Tau;

				return ret;
			}
		}

		PhysicsGroup::PhysicsGroup(std::string id, NormalizationParams positionNormalizationParams, NormalizationParams angleNormalizationParams, std::vector<PhysicsPendulumNode> nodes, std::vector<PhysicsInput> inputs, std::vector<PhysicsOutput> outputs) :
			m_id(std::move(id)),
			m_positionNormalizationParams(positionNormalizationParams),
			m_angleNormalizationParams(angleNormalizationParams),
			m_nodes(std::move(nodes)),
			m_inputs(std::move(inputs)),
			m_outputs(std::move(outputs)),
			m_prevGravity(glm::vec2(0.0f, 1.0f))
		{}

		void PhysicsGroup::attachTo(ModelInstance* instance) {
			m_inputParams.clear();
			m_outputParams.clear();

			if (!instance) // only deattach when instance is nullptr
				return;

			for (auto& input : m_inputs)
				m_inputParams.push_back(instance->getParameter(input.paramId.c_str()));

			for (auto& output : m_outputs)
				m_outputParams.push_back(instance->getParameter(output.paramId.c_str()));
		}

		void PhysicsGroup::update(float deltatime) {
			constexpr float maxWeight = 100.0f;
			constexpr float airResistance = 5.0f;

			float rotation = 0.0f;
			readCurrentState(rotation, m_nodes[0].position);

			const glm::vec2 gravity = glm::vec2(sin(rotation * luna::DegToRad), cos(rotation * luna::DegToRad));
			
			const float deltaRotation = directionToRadian(m_prevGravity, gravity) / airResistance;
			const float cosDeltaRotation = cos(deltaRotation);
			const float sinDeltaRotation = sin(deltaRotation);
			glm::mat2 deltaRotationMatrix = glm::mat2(cosDeltaRotation, -sinDeltaRotation, sinDeltaRotation, cosDeltaRotation);

			for (size_t i = 1; i < m_nodes.size(); ++i) {
				// get current forces and position
				float dt = m_nodes[i].delay * deltatime * 30.0f; // deltatime put sped up
				glm::vec2 prevPosition = m_nodes[i].position;
				glm::vec2 prevAcceleration = m_prevGravity * m_nodes[i].acceleration;
				glm::vec2 acceleration = gravity * m_nodes[i].acceleration;

				// velocity verlet
				glm::vec2 velocity = m_nodes[i].velocity + 0.25f * (acceleration + prevAcceleration) * dt;
				m_nodes[i].position += velocity * dt;

				// make sure the node maintains its distance from the previous one
				glm::vec2 newDirection = glm::normalize(m_nodes[i].position - m_nodes[i - 1].position);
				m_nodes[i].position = m_nodes[i - 1].position + (newDirection * m_nodes[i].radius);

				// make the pendulum stationary when the forces become too low
				if (abs(m_nodes[i].position.x) < 0.001f * m_positionNormalizationParams.max)
					m_nodes[i].position.x = 0.0f;

				// update velocity
				if (dt != 0.0f)
					m_nodes[i].velocity = m_nodes[i].mobility * (m_nodes[i].position - prevPosition) / dt;
			}

			m_prevGravity = gravity;
			writeCurrentState();
		}

		void PhysicsGroup::stabilize() {
			// todo: implement
		}

		size_t PhysicsGroup::getNodeCount() {
			return m_nodes.size();
		}

		PhysicsPendulumNode* PhysicsGroup::getNodes() {
			return m_nodes.data();
		}

		const PhysicsPendulumNode* PhysicsGroup::getNodes() const {
			return m_nodes.data();
		}

		void PhysicsGroup::readCurrentState(float& rotation, glm::vec2& position) {
			rotation = 0.0f;
			position = glm::vec2(0.0f);

			for (size_t i = 0; i < m_inputs.size(); ++i) {
				auto& inputData = m_inputs[i];
				auto* parameter = m_inputParams[i];

				if (!parameter)
					continue;

				float sign = inputData.reflect ? -1.0f : 1.0f;

				switch (inputData.type) {

				case PhysicsParameterType::Angle:
					rotation += parameter->getNormalizedValue(m_angleNormalizationParams.base, m_angleNormalizationParams.min, m_angleNormalizationParams.max) * sign * inputData.weight / 100.0f;
					break;

				case PhysicsParameterType::X:
					position.x += parameter->getNormalizedValue(m_positionNormalizationParams.base, m_positionNormalizationParams.min, m_positionNormalizationParams.max) * sign * inputData.weight / 100.0f;
					break;

				case PhysicsParameterType::Y:
					position.y += parameter->getNormalizedValue(m_positionNormalizationParams.base, m_positionNormalizationParams.min, m_positionNormalizationParams.max) * sign * inputData.weight / 100.0f;
					break;

				default:
					break;

				}
			}
		}

		void PhysicsGroup::writeCurrentState() {
			for (size_t i = 0; i < m_outputs.size(); ++i) {
				auto& outputData = m_outputs[i];
				auto* parameter = m_outputParams[i];
				int nodeIdx = outputData.pendulumNodeIndex;

				if (!parameter || nodeIdx < 1 || nodeIdx > m_nodes.size())
					continue;

				glm::vec2 translation = m_nodes[nodeIdx].position - m_nodes[nodeIdx - 1].position;

				float value = 0.0f;
				switch (outputData.type) {

				case PhysicsParameterType::Angle:
					glm::vec2 parentGravity;
					if (nodeIdx >= 2) {
						parentGravity = m_nodes[nodeIdx - 1].position - m_nodes[nodeIdx - 2].position;
					} else {
						parentGravity = glm::vec2(0.0f, 1.0f);
					}

					value = -directionToRadian(parentGravity, translation);
					break;

				case PhysicsParameterType::X:
					value = translation.x;
					break;

				case PhysicsParameterType::Y:
					value = translation.y;
					break;

				default:
					break;

				}

				if (outputData.reflect)
					value = -value;

				value *= outputData.scale;
				value *= outputData.weight / 100.0f;
				
				parameter->setValue(value);
			}
		}

		PhysicsController::PhysicsController(const char* filepath) {
			// load file
			std::ifstream file(filepath);
			if (file.bad() || file.fail() || file.eof()) {
				log("Could not open file at \"" + std::string(filepath) + "\"", MessageSeverity::Error);
				return;
			}
			json modelFile = json::parse(file);

			// parse the separate groups
			auto& settings = modelFile.at("PhysicsSettings");
			for (auto& group : settings) {
				std::string id = group.at("Id");
				NormalizationParams positionNormalizationParams;
				NormalizationParams angleNormalizationParams;
				std::vector<PhysicsPendulumNode> nodes;
				std::vector<PhysicsInput> inputs;
				std::vector<PhysicsOutput> outputs;

				// parse inputs
				for (auto& input : group.at("Input")) {
					std::string target = input.at("Source").at("Target");
					if (target == "Parameter") { // idk what it would be if its not a parameter, but it sure isnt supported
						// parse input
						std::string id = input.at("Source").at("Id");
						float weight = input.at("Weight");
						std::string type = input.at("Type");
						bool reflect = input.at("Reflect");

						inputs.emplace_back(
							id,
							type == "X" ? PhysicsParameterType::X : (type == "Y" ? PhysicsParameterType::Y : PhysicsParameterType::Angle),
							weight,
							reflect
						);
					}
				}

				// parse outputs
				for (auto& output : group.at("Output")) {
					std::string target = output.at("Destination").at("Target");
					if (target == "Parameter") { // idk what it would be if its not a parameter, but it sure isn't supported
						// parse output
						std::string id = output.at("Destination").at("Id");
						int nodeIndex = output.at("VertexIndex");
						float scale = output.at("Scale");
						float weight = output.at("Weight");
						std::string type = output.at("Type");
						bool reflect = output.at("Reflect");

						outputs.emplace_back(
							id,
							type == "X" ? PhysicsParameterType::X : (type == "Y" ? PhysicsParameterType::Y : PhysicsParameterType::Angle),
							nodeIndex,
							weight,
							scale,
							reflect
						);
					}
				}

				// parse nodes
				for (auto& vertex : group.at("Vertices")) {
					PhysicsPendulumNode node;
					node.initialPosition.x = vertex.at("Position").at("X");
					node.initialPosition.y = vertex.at("Position").at("Y");
					node.mobility = vertex.at("Mobility");
					node.delay = vertex.at("Delay");
					node.acceleration = vertex.at("Acceleration");
					node.radius = vertex.at("Radius");
					node.position = node.initialPosition;
					node.velocity = glm::vec2(0.0f);
					nodes.push_back(node);
				}

				// parse normalization parameters
				auto& normalization = group.at("Normalization");
				auto& position = normalization.at("Position");
				auto& angle = normalization.at("Angle");

				positionNormalizationParams.min = position.at("Minimum");
				positionNormalizationParams.max = position.at("Maximum");
				positionNormalizationParams.base = position.at("Default");

				angleNormalizationParams.min = angle.at("Minimum");
				angleNormalizationParams.max = angle.at("Maximum");
				angleNormalizationParams.base = angle.at("Default");

				m_groups.emplace_back(std::move(id), positionNormalizationParams, angleNormalizationParams, std::move(nodes), std::move(inputs), std::move(outputs));
			}
		}

		void PhysicsController::attachTo(ModelInstance* instance) {
			for (auto& group : m_groups)
				group.attachTo(instance);
		}

		void PhysicsController::update(float deltatime) {
			for (auto& group : m_groups)
				group.update(deltatime);
		}

		void PhysicsController::stabilize() {
			for (auto& group : m_groups)
				group.stabilize();
		}

		size_t PhysicsController::getGroupCount() {
			return m_groups.size();
		}

		PhysicsGroup* PhysicsController::getGroups() {
			return m_groups.data();
		}

		const PhysicsGroup* PhysicsController::getGroups() const {
			return m_groups.data();
		}

	}
}