#include "GridVectorEmitter.h"
#include "Chunk.h"
#include "ChannelObject.h"
#include "GridObject.h"

#include "glm/vec3.hpp"
#include "glm/trigonometric.hpp"
#include "glm/gtc/noise.hpp"

//#include <omp.h>

inline static float map_range(float value, float low1, float high1, float low2,
                              float high2) {
  return low2 + (high2 - low2) * (value - low1) / (high1 - low1);
}

//----------------------------------------------
GridVectorEmitter::~GridVectorEmitter() {}

void GridVectorEmitter::setupDefaults() {}

//----------------------------------------------
void GridVectorEmitter::Algorithm(glm::i32vec3 chunkId,
                                  glm::i32vec3 voxelPosition, Chunk *inChunk,
                                  Chunk *outChunk, uint32_t dataIndex,
                                  uint32_t channel, bool internalAccessible)

{

  float X = ((chunkId.x * static_cast<int>(chnkSize)) + voxelPosition.x);

  float Y = ((chunkId.y * static_cast<int>(chnkSize)) + voxelPosition.y);

  float Z = ((chunkId.z * static_cast<int>(chnkSize)) + voxelPosition.z);

  // float sample = sourceVolume->sampleVolume(glm::vec3(X+0.5, Y+0.5, Z+0.5));

  float strength = 0.001f;
  float frequency = 1.0f / 16.0f;

  Z += 50;
  X += 50;
  Y += 50;

  float time = gridObjectPtr->simTime;

  //            glm::vec3 value =
  //            glm::vec3((glm::simplex(glm::vec2(channel+time,
  //            (X)*frequency))*strength,
  //                                        (glm::simplex(glm::vec2(channel+time,
  //                                        (Y)*frequency))*strength,
  //                                        (glm::simplex(glm::vec2(channel+time,
  //                                        (Z)*frequency))*strength))));

//              glm::vec3 value =
//              glm::vec3((glm::simplex(glm::vec3(X*frequency+time,
//              Y*frequency+time, Z*frequency+time))*strength,
//                                          (glm::simplex(glm::vec3(X*frequency+time,
//                                          Y*frequency+time,
//                                          Z*frequency+time))*strength,
//                                          (glm::simplex(glm::vec3(X*frequency+time,
//                                          Y*frequency+time,
//                                          Z*frequency+time))*strength))));

      glm::vec3 value = glm::vec3((glm::simplex(glm::vec2(channel+time,
      (X)*frequency))+(glm::simplex(glm::vec2(channel+time,
      Y*frequency*2))*0.5)*strength,
                                  (glm::simplex(glm::vec2(channel+time,
                                  (Y)*frequency))+(glm::simplex(glm::vec2(channel+time,
                                  Z*frequency*2))*0.5)*strength,
                                  (glm::simplex(glm::vec2(channel+time,
                                  (Z)*frequency))+(glm::simplex(glm::vec2(channel+time,
                                  X*frequency*2))*0.5)*strength))));
  //glm::vec3 value = glm::vec3(0.0f, 0.5f, 0.0f);
  outChunk->chunkData[dataIndex] = value[channel];
  // inChunk->chunkData[dataIndex] = 1;

  // cout << "glm access test " << value[channel];
}

void GridVectorEmitter::PreGridOp() {
  // SetChannelName(std::string("velocity"));
  uint32_t velTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));

  currentTargetChannelObject =
      gridObjectPtr->channelObjs[velTarget].get(); // want vel channel.
}

// float GridEmitter::addPositiveDifference(float inputReference, float
// amountToAdd)
//{
//    return inputReference+(amountToAdd-inputReference);
//}

// void GridEmitter::PreChunkOp(Chunk *inChunk, glm::i32vec3 chunkIdSecondary)
//{
//    //here we do a presample of emission volume once per bucket. simple way to
//    stop race condition if we were to do it per voxel.

//}
