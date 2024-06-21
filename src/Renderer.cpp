#include "Renderer.hpp"

#include "ModelInstance.hpp"

namespace luna {
	namespace live2d {

		Renderer::Renderer(const ModelInstance* model) :
			m_model(model),
			m_noMaskTexture(luna::Color::White)
		{
			if (!m_model) return;

			m_drawables.resize(m_model->getDrawableCount());
			for (size_t i = 0; i < m_drawables.size(); ++i)
				m_drawables[i] = &m_model->getDrawables()[i];

			sortDrawables();
		}

		void Renderer::beginFrame() {}

		void Renderer::endFrame() {
			// check if batches need to be rebuild
			if (checkRebuild())
				sortDrawables();

			// only rebuild meshes that had a drawable update
			for (auto& batch : m_batches) {
				for (const auto* drawable : batch.drawables) {
					if (drawable->getDynamicFlags() & 0b1100110) {
						buildMeshVertices(batch, false);
						break;
					}
				}
			}

			// same thing for mask batches
			for (auto& maskBatchVec : m_maskBatches) {
				for (auto& batch : maskBatchVec) {
					for (const auto* drawable : batch.drawables) {
						if (drawable->getDynamicFlags() & 0b100110) {
							buildMeshVertices(batch, true);
							break;
						}
					}
				}
			}
		}

		void Renderer::render(const luna::Camera& camera) {
			if (!camera.getTarget() || !m_model)
				return;

			// setup
			auto maskTexture = luna::getTempRenderTexture(camera.getTarget()->getSize());
			camera.getTarget()->makeActiveTarget();
			luna::uploadCameraMatrices(camera.projection(), camera.getTransform().inverseMatrix());
			glm::mat4 modelMatrix = m_model->getTransform().matrix();

			// rendering
			for (auto& batch : m_batches) {
				if (batch.maskId != size_t(-1)) {
					// batch has mask
					maskTexture->makeActiveTarget();
					luna::RenderTarget::clear(luna::Color::Clear);

					for (auto& maskBatch : m_maskBatches[batch.maskId])
						drawBatch(maskBatch, modelMatrix);

					camera.getTarget()->makeActiveTarget();
					drawBatch(batch, modelMatrix, &*maskTexture, bool(batch.drawables.front()->getConstantFlags() & 0b1000));
				} else {
					// no mask, just render regularly
					drawBatch(batch, modelMatrix);
				}
			}
		}

		bool Renderer::checkRebuild() {
			for (auto& batch : m_batches) {
				for (const auto* drawable : batch.drawables) {
					if (batch.drawables.front()->getMaterial() != drawable->getMaterial())
						return true; // rebuild when material changed

					if (drawable->getDynamicFlags() & 0b1000) 
						return true; // rebuild when render order changed
				}
			}

			return false;
		}

		void Renderer::sortDrawables() {
			std::sort(m_drawables.begin(), m_drawables.end(), [](const Drawable* a, const Drawable* b) {
				return a->getRenderOrder() < b->getRenderOrder();
			});

			buildBatches(); // rebuild required after sorting
		}

		void Renderer::buildBatches() {
			m_batches.clear();
			m_maskBatches.clear();

			if (m_drawables.empty()) return;

			// split the drawables up in batches
			Batch currentBatch;
			for (const auto* drawable : m_drawables) {
				if (!fitsInBatch(currentBatch, *drawable, false)) {
					// new element can't be batched with previous ones, so set up new batch
					m_batches.emplace_back(std::move(currentBatch));
					currentBatch = {};
					if (drawable->getMaskCount() != 0) {
						// new batch has a mask
						currentBatch.maskId = m_maskBatches.size();
						m_maskBatches.emplace_back();

						// set up batches for masking
						Batch currentMaskBatch;
						for (size_t i = 0; i < drawable->getMaskCount(); ++i) {
							int maskDrawableIdx = drawable->getMasks()[i];
							if (maskDrawableIdx == -1) continue;

							const auto& mask = m_model->getDrawables()[maskDrawableIdx];
							if (!fitsInBatch(currentMaskBatch, mask, true)) {
								// new mask element can't be batched with previos ones, so set up new mask batch
								m_maskBatches.back().emplace_back(std::move(currentMaskBatch));
								currentMaskBatch = {};
							}
							currentMaskBatch.drawables.push_back(&mask);
						}

						m_maskBatches.back().emplace_back(std::move(currentMaskBatch));
					}
				}
				currentBatch.drawables.push_back(drawable);
			}
			m_batches.emplace_back(std::move(currentBatch));

			// build meshes for every batch
			for (auto& batch : m_batches) {
				buildMeshIndices(batch);
				buildMeshVertices(batch, false);
			}

			for (auto& maskBatchVec : m_maskBatches) {
				for (auto& batch : maskBatchVec) {
					buildMeshIndices(batch);
					buildMeshVertices(batch, true);
				}
			}
		}

		void Renderer::buildMeshIndices(Batch& batch) {
			std::vector<unsigned int> indices;
			unsigned int indexOffset = 0;
			size_t indexCount = 0;
			size_t index = 0;

			// sum up size of index buffer
			for (const auto* drawable : batch.drawables)
				indexCount += drawable->getIndexCount();
			indices.resize(indexCount);

			// populate the buffer
			for (const auto* drawable : batch.drawables) {
				for (size_t i = 0; i < drawable->getIndexCount(); ++i) {
					indices[index] = drawable->getIndices()[i] + indexOffset;
					++index;
				}
				indexOffset += unsigned int(drawable->getVertexCount());
			}

			batch.mesh.setIndices(indices.data(), indices.size());
		}

		void Renderer::buildMeshVertices(Batch& batch, bool isMask) {
			std::vector<luna::Vertex> vertices;
			size_t vertexCount = 0;
			size_t index = 0;

			// sum up size of vertex buffer
			for (const auto* drawable : batch.drawables)
				vertexCount += drawable->getVertexCount();
			vertices.resize(vertexCount);

			// populate the buffer
			for (const auto* drawable : batch.drawables) {
				uint32_t multCol = isMask ? 0xFFFFFFFF : drawable->getMultiplyColor().compressed();
				glm::vec3 screenCol = isMask ? glm::vec3(0.0f) : drawable->getScreenColor().vec3();

				for (size_t i = 0; i < drawable->getVertexCount(); ++i) {
					auto pos = drawable->getVertexPositions()[i];
					auto uv = drawable->getVertexUvs()[i];

					// passing the screen colour as a normal, its cursed but you gotta spend sauce to make sauce
					vertices[index] = luna::Vertex(glm::vec3(pos, 0.0f), uv, screenCol, multCol);
					++index;
				}
			}

			batch.mesh.setVertices(vertices.data(), vertices.size());
		}

		bool Renderer::fitsInBatch(const Batch& batch, const Drawable& drawable, bool isMask) {
			if (batch.drawables.empty() || (batch.drawables.front()->getMaterial() == drawable.getMaterial() && batch.drawables.front()->getConstantFlags() == drawable.getConstantFlags() && (isMask || batch.drawables.front()->hasSameMasks(drawable))))
				return true;
			return false;
		}

		void Renderer::drawBatch(const Batch& batch, const glm::mat4& modelMatrix, const luna::Texture* mask, bool inverseMask) {
			int texIdx = int(batch.drawables.front()->getMaterial()->getTextureCount());
			batch.mesh.bind();
			batch.drawables.front()->getMaterial()->bind();
			auto& shader = batch.drawables.front()->getMaterial()->getShader()->getProgram();
			shader.uniform(shader.uniformId("ModelMatrix"), modelMatrix);
			shader.uniform(shader.uniformId("Live2DMaskTexture"), texIdx);
			shader.uniform(shader.uniformId("Live2DMaskTextureInversed"), int(inverseMask));
			(mask ? *mask : m_noMaskTexture).bind(texIdx);

			draw(&batch.mesh);
		}

	}
}