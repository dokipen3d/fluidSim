#include "GridBouyancy.h"
#include "Chunk.h"
#include "ChannelObject.h"
#include "GridObject.h"

void GridBouyancy::setupDefaults() {

  uint32_t velTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
  uint32_t densitySource =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("density"));

  currentSourceChannelObject =
      gridObjectPtr->channelObjs[densitySource].get(); // default first one
  currentTargetChannelObject =
      gridObjectPtr->channelObjs[velTarget].get(); // want vel channel.
  // callPreChunkOp = true;
  // callGridOp = true;
}

void GridBouyancy::Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition,
                             Chunk *inChunk, Chunk *outChunk,
                             uint32_t dataIndex, uint32_t channel) {

  float densSample =
      inChunk
          ->chunkData[dataIndex - (channel * chnkSize * chnkSize * chnkSize)];

  glm::vec3 value = glm::vec3(0.0f, densSample * 0.6, 0.0f);
  outChunk->chunkData[dataIndex] = value[channel];
}

void GridBouyancy::PreGridOp() {
  uint32_t velTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
  uint32_t densitySource =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("density"));

  currentSourceChannelObject =
      gridObjectPtr->channelObjs[densitySource].get(); // default first one
  currentTargetChannelObject =
      gridObjectPtr->channelObjs[velTarget].get(); // want vel channel.
}

void GridBouyancy::GridOp() {
  // gridObjectPtr->SwapChannelPointers(std::string("velocity"));
}
