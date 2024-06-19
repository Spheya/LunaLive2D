#include "Drawable.hpp"

namespace luna {
	namespace live2d {
		Drawable::Drawable(
			const int* vertexCount,
			const csmVector2* vertexPositions,
			const csmVector2* vertexUvs,
			const int* indexCount,
			const unsigned short* indices,

			const char* id,
			const float* opacity,
			const int* drawOrder,
			const int* renderOrder,
			const csmFlags* constantFlags,
			const csmFlags* dynamicFlags,

			const csmVector4* multiplyColor,
			const csmVector4* screenColor,

			const int* maskCount,
			const int* masks,

			const luna::Material* material,
			const luna::Texture* texture
		) :
			m_vertexCount(vertexCount),
			m_vertexPositions(vertexPositions),
			m_vertexUvs(vertexUvs),
			m_indexCount(indexCount),
			m_indices(indices),

			m_id(id),
			m_opacity(opacity),
			m_drawOrder(drawOrder),
			m_renderOrder(renderOrder),
			m_constantFlags(constantFlags),
			m_dynamicFlags(dynamicFlags),

			m_multiplyColor(multiplyColor),
			m_screenColor(screenColor),

			m_maskCount(maskCount),
			m_masks(masks),

			m_material(material),
			m_texture(texture)
		{
			std::hash<std::string> hasher;
			m_hashId = hasher(std::string(m_id));
		}

		const char* Drawable::getId() const {
			return m_id;
		}

		size_t Drawable::getIdHash() const {
			return m_hashId;
		}

		void Drawable::setMaterial(const luna::Material* material) {
			m_material = material;
		}

		const luna::Material* Drawable::getMaterial() const {
			return m_material;
		}

		const luna::Texture* Drawable::getTexture() const {
			return m_texture;
		}

		size_t Drawable::getVertexCount() const {
			return size_t(*m_vertexCount);
		}

		const glm::vec2* Drawable::getVertexPositions() const {
			return reinterpret_cast<const glm::vec2*>(m_vertexPositions);
		}

		const glm::vec2* Drawable::getVertexUvs() const {
			return reinterpret_cast<const glm::vec2*>(m_vertexUvs);
		}

		size_t Drawable::getIndexCount() const {
			return size_t(*m_indexCount);
		}

		const unsigned short* Drawable::getIndices() const {
			return m_indices;
		}

		float Drawable::getOpacity() const {
			return bool(*m_dynamicFlags & 0b1) ? *m_opacity : 0.0f;
		}

		int Drawable::getDrawOrder() const {
			return *m_drawOrder;
		}

		int Drawable::getRenderOrder() const {
			return *m_renderOrder;
		}

		csmFlags Drawable::getConstantFlags() const {
			return *m_constantFlags;
		}

		csmFlags Drawable::getDynamicFlags() const {
			return *m_dynamicFlags;
		}

		luna::Color Drawable::getMultiplyColor() const {
			return luna::Color(glm::vec4(m_multiplyColor->X, m_multiplyColor->Y, m_multiplyColor->Z, m_multiplyColor->W * getOpacity()));
		}

		luna::Color Drawable::getScreenColor() const {
			return luna::Color(glm::vec3(m_screenColor->X, m_screenColor->Y, m_screenColor->Z));
		}

		size_t Drawable::getMaskCount() const {
			return size_t(*m_maskCount);
		}

		const int* Drawable::getMasks() const {
			return m_masks;
		}

		bool Drawable::hasSameMasks(const Drawable& other) const {
			if (*m_maskCount != *other.m_maskCount) return false;

			for (int i = 0; i < *m_maskCount; ++i) {
				bool contains = false;

				for (int j = 0; j < *m_maskCount; ++j) {
					if (m_masks[i] == other.m_masks[j]) {
						contains = true;
						break;
					}
				}

				if (!contains)
					return false;
			}

			return true;
		}
	}
}
