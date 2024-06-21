#pragma once

#include <luna.hpp>

#include "Drawable.hpp"

namespace luna {
	namespace live2d {

		class ModelInstance;

		class Renderer : public luna::Renderer {
		protected:
			struct Batch {
				std::vector<const Drawable*> drawables;
				luna::Mesh mesh;
				size_t maskId = size_t(-1);
			};

		public:
			explicit Renderer(const ModelInstance* model = nullptr);

			void beginFrame() override;
			void endFrame() override;
			void render(const luna::Camera& camera) override;

		protected:
			/**
			 * @brief Checks if a drawable fits within a batch (and can thus be rendered within a single drawcall). 
			 * This function can be overriden when you need to split the batches up in case you want to render stuff
			 * between specific drawables, like an outline filter for example.
			 * @param batch The batch to check if the drawable fits in.
			 * @param drawable The drawable to check if it fits in the batch.
			 * @param isMask Whether the provided batch and drawable are a mask.
			 * @return True if the drawable fits in the batch. False if it doesn't.
			*/
			virtual bool fitsInBatch(const Batch& batch, const Drawable& drawable, bool isMask);

		private:
			bool checkRebuild();

			void sortDrawables();
			void buildBatches();

			static void buildMeshIndices(Batch& batch);
			static void buildMeshVertices(Batch& batch, bool isMask);

			void drawBatch(const Batch& batch, const glm::mat4& matrix, const luna::Texture* mask = nullptr, bool inverseMask = false);

		private:
			const ModelInstance* m_model;

			luna::Texture m_noMaskTexture;
			std::vector<const Drawable*> m_drawables;

			std::vector<Batch> m_batches;
			std::vector<std::vector<Batch>> m_maskBatches;
		};

	}
}