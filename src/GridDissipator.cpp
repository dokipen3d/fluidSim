#include "GridDissipator.h"
#include "Chunk.h"
#include "ChannelObject.h"

void GridDissipator::setupDefaults() { dissipationRate = 0.1; }

void GridDissipator::Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition,
                               Chunk *inChunk, Chunk *outChunk,
                               uint32_t dataIndex, uint32_t channel, bool internalAccessible) {
  // if(!inChunk->empty)
  // inChunk->chunkData[dataIndex] = glm::max(0.0f ,
  // inChunk->chunkData[dataIndex]-0.08f) ;
  if (inChunk->chunkData[dataIndex] > 0)
    inChunk->chunkData[dataIndex] -= 0.2f;
}
