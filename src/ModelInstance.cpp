#include "ModelInstance.hpp"

#include <filesystem>
#include <fstream>
#include <cassert>
#include <nlohmann/json.hpp>
#include <Live2DCubismCore.h>

#include "AlignedAllocator.hpp"

using json = nlohmann::json;

namespace luna {
	namespace live2d {

		ModelInstance::ModelInstance(Model* model) :
			m_coreModel(model->createCoreModel()),
			m_model(model)
		{
			csmVector2 size;
			csmVector2 origin;
			csmReadCanvasInfo(m_coreModel.get(), &size, &origin, &m_pixelsPerUnit);
			m_canvasSize = glm::vec2(size.X, size.Y) / m_pixelsPerUnit;
			m_canvasOrigin = glm::vec2(origin.X, origin.Y) / m_pixelsPerUnit;

			initializeDrawables();
			initializeParameters();
		}

		Model& ModelInstance::getModel() {
			return *m_model;
		}

		const Model& ModelInstance::getModel() const {
			return *m_model;
		}

		void ModelInstance::update(float deltatime) {
			csmResetDrawableDynamicFlags(m_coreModel.get());
			csmUpdateModel(m_coreModel.get());
		}

		void ModelInstance::setTransform(const Transform& transform) {
			m_transform = transform;
		}

		const Transform& ModelInstance::getTransform() const {
			return m_transform;
		}

		Transform& ModelInstance::getTransform() {
			return m_transform;
		}

		glm::vec2 ModelInstance::getCanvasSize() const {
			return m_canvasSize;
		}

		glm::vec2 ModelInstance::getCanvasOrigin() const {
			return m_canvasOrigin;
		}

		float ModelInstance::getPixelsPerUnit() const {
			return m_pixelsPerUnit;
		}

		size_t ModelInstance::getDrawableCount() const {
			return m_drawables.size();
		}

		Drawable* ModelInstance::getDrawables() {
			return m_drawables.data();
		}

		const Drawable* ModelInstance::getDrawables() const {
			return m_drawables.data();
		}

		const Drawable* ModelInstance::getDrawable(const char* id) const {
			std::hash<std::string> hasher;
			auto it = std::find_if(m_drawables.begin(), m_drawables.end(), [hash = hasher(id)](const Drawable& x) { return x.getIdHash() == hash; });
			return it == m_drawables.end() ? &(*it) : nullptr;
		}

		Drawable* ModelInstance::getDrawable(const char* id) {
			std::hash<std::string> hasher;
			auto it = std::find_if(m_drawables.begin(), m_drawables.end(), [hash = hasher(id)](const Drawable& x) { return x.getIdHash() == hash; });
			return it == m_drawables.end() ? &(*it) : nullptr;
		}

		size_t ModelInstance::getParameterCount() const {
			return m_parameters.size();
		}

		Parameter* ModelInstance::getParameters() {
			return m_parameters.data();
		}

		const Parameter* ModelInstance::getParameters() const {
			return m_parameters.data();
		}

		const Parameter* ModelInstance::getParameter(const char* id) const {
			std::hash<std::string> hasher;
			auto it = std::find_if(m_parameters.begin(), m_parameters.end(), [hash = hasher(id)](const Parameter& x) { return x.getIdHash() == hash; });
			return it == m_parameters.end() ? &(*it) : nullptr;
		}

		Parameter* ModelInstance::getParameter(const char* id) {
			std::hash<std::string> hasher;
			auto it = std::find_if(m_parameters.begin(), m_parameters.end(), [hash = hasher(id)](const Parameter& x) { return x.getIdHash() == hash; });
			return it == m_parameters.end() ? &(*it) : nullptr;
		}

		void ModelInstance::initializeDrawables() {
			assert(m_drawables.empty());

			int drawableCount = csmGetDrawableCount(m_coreModel.get());
			m_drawables.reserve(drawableCount);

			const int* textureIndices = csmGetDrawableTextureIndices(m_coreModel.get());
			const int* vertexCounts = csmGetDrawableVertexCounts(m_coreModel.get());
			const csmVector2** vertexPositions = csmGetDrawableVertexPositions(m_coreModel.get());
			const csmVector2** vertexUvs = csmGetDrawableVertexUvs(m_coreModel.get());
			const int* indexCounts = csmGetDrawableIndexCounts(m_coreModel.get());
			const unsigned short** indices = csmGetDrawableIndices(m_coreModel.get());

			const char** ids = csmGetDrawableIds(m_coreModel.get());
			const float* opacities = csmGetDrawableOpacities(m_coreModel.get());
			const int* drawOrders = csmGetDrawableDrawOrders(m_coreModel.get());
			const int* renderOrders = csmGetDrawableRenderOrders(m_coreModel.get());
			const csmFlags* constantFlags = csmGetDrawableConstantFlags(m_coreModel.get());
			const csmFlags* dynamicFlags = csmGetDrawableDynamicFlags(m_coreModel.get());

			const csmVector4* multiplyColors = csmGetDrawableMultiplyColors(m_coreModel.get());
			const csmVector4* screenColors = csmGetDrawableScreenColors(m_coreModel.get());

			const int* maskCounts = csmGetDrawableMaskCounts(m_coreModel.get());
			const int** masks = csmGetDrawableMasks(m_coreModel.get());

			for (int i = 0; i < drawableCount; ++i) {
				if (textureIndices[i] >= m_model->getMaterialCount()) {
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
					&m_model->getMaterials()[textureIndices[i]],
					&m_model->getTextures()[textureIndices[i]]
				);
			}
		}

		void ModelInstance::initializeParameters() {
			assert(m_parameters.empty());

			size_t paramCount = size_t(csmGetParameterCount(m_coreModel.get()));
			m_parameters.reserve(paramCount);

			const char** ids = csmGetParameterIds(m_coreModel.get());
			const float* minValues = csmGetParameterMinimumValues(m_coreModel.get());
			const float* maxValues = csmGetParameterMaximumValues(m_coreModel.get());
			const float* defaultValues = csmGetParameterDefaultValues(m_coreModel.get());
			float* values = csmGetParameterValues(m_coreModel.get());

			for (size_t i = 0; i < paramCount; ++i)
				m_parameters.push_back(Parameter(ids[i], &minValues[i], &maxValues[i], &defaultValues[i], &values[i]));
		}

	}
}