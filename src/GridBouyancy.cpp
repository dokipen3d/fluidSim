#include "GridBouyancy.h"
#include "Chunk.h"
#include "ChannelObject.h"
#include "GridObject.h"

void GridBouyancy::setupDefaults() {

  //set the target channel
  uint32_t velTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
  //set the source channel
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

//    float X = ((chunkId.x * static_cast<int>(chnkSize)) + voxelPosition.x);

//    float Y = ((chunkId.y * static_cast<int>(chnkSize)) + voxelPosition.y);

//    float Z = ((chunkId.z * static_cast<int>(chnkSize)) + voxelPosition.z);
    //if we are a vel operator and sampling a scalar, we need to do the inverse channel (could do it before calling algo)
  float densSample =
      inChunk
          ->chunkData[dataIndex - (channel * chnkSize * chnkSize * chnkSize)];

  //glm::vec3 vel =  currentTargetChannelObject->SampleVectorAtPosition(X + 0.5f, Y + 0.5f, Z + 0.5f);

 // float yPos = densSample - vel.y;
  glm::vec3 value = glm::vec3(0.0f, densSample* 1.0f, 0.0f);
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
