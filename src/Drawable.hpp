#pragma once

#include <luna.hpp>
#include <Live2DCubismCore.h>

namespace luna {
	namespace live2d {

		class Drawable {
		public:
			Drawable(
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
			);
			Drawable(Drawable&) = delete;
			Drawable& operator=(Drawable&) = delete;
			Drawable(Drawable&&) = default;
			Drawable& operator=(Drawable&&) = default;
			~Drawable() = default;

			const char* getId() const;
			size_t getIdHash() const;

			void setMaterial(const luna::Material* material);
			const luna::Material* getMaterial() const;
			const luna::Texture* getTexture() const;

			size_t getVertexCount() const;
			const glm::vec2* getVertexPositions() const;
			const glm::vec2* getVertexUvs() const;
			size_t getIndexCount() const;
			const unsigned short* getIndices() const;

			float getOpacity() const;
			int getDrawOrder() const;
			int getRenderOrder() const;
			csmFlags getConstantFlags() const;
			csmFlags getDynamicFlags() const;

			luna::Color getMultiplyColor() const;
			luna::Color getScreenColor() const;

			size_t getMaskCount() const;
			const int* getMasks() const;

			bool hasSameMasks(const Drawable& other) const;

		private:
			size_t m_hashId;

			const int* m_textureIndex;
			const int* m_vertexCount;
			const csmVector2* m_vertexPositions;
			const csmVector2* m_vertexUvs;
			const int* m_indexCount;
			const unsigned short* m_indices;

			const char* m_id;
			const float* m_opacity;
			const int* m_drawOrder;
			const int* m_renderOrder;
			const csmFlags* m_constantFlags;
			const csmFlags* m_dynamicFlags;

			const csmVector4* m_multiplyColor;
			const csmVector4* m_screenColor;

			const int* m_maskCount;
			const int* m_masks;

			const luna::Material* m_material;
			const luna::Texture* m_texture;
		};

	}
}
