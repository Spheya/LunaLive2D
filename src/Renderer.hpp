#pragma once

#include <luna.hpp>

#include "Drawable.hpp"

namespace luna {
	namespace live2d {

		class Model;

		class Renderer : public luna::Renderer {
		protected:
			struct Batch {
				std::vector<const Drawable*> drawables;
				luna::Mesh mesh;
				size_t maskId = size_t(-1);
			};

		public:
			explicit Renderer(const Model* model);

			void beginFrame() override;
			void endFrame() override;
			void render(const luna::Camera& camera) override;

		protected:
			virtual bool fitsInBatch(const Batch& batch, const Drawable& drawable, bool isMask);

		private:
			bool checkRebuild();

			void sortDrawables();
			void buildBatches();

			static void buildMeshIndices(Batch& batch);
			static void buildMeshVertices(Batch& batch, bool isMask);

			void drawBatch(const Batch& batch, const glm::mat4& matrix, const luna::Texture* mask = nullptr, bool inverseMask = false);

		private:
			const Model* m_model;

			luna::Texture m_noMaskTexture;
			std::vector<const Drawable*> m_drawables;

			std::vector<Batch> m_batches;
			std::vector<std::vector<Batch>> m_maskBatches;
		};

	}
}