#include "GridBasicAdvect.h"
#include "Chunk.h"
#include "ChannelObject.h"
#include "glm/vec3.hpp"
#include "glm/trigonometric.hpp"
#include "GridObject.h"

//#include <omp.h>

inline static float map_range(float value, float low1, float high1, float low2,
                              float high2) {
  return low2 + (high2 - low2) * (value - low1) / (high1 - low1);
}

//----------------------------------------------
GridBasicAdvect::~GridBasicAdvect() {}

void GridBasicAdvect::setupDefaults() {
  uint32_t velSource =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
  uint32_t densityTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("density"));

  velocitySourceChannelObject =
      gridObjectPtr->channelObjs[velSource].get(); // default first one
  currentTargetChannelObject =
      gridObjectPtr->channelObjs[densityTarget + 1]
          .get(); // want dup density channel.will swap after
  callPreChunkOp = true;
  callGridOp = true;
}

void GridBasicAdvect::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                                 glm::i32vec3 chunkIdSecondary) {

  // cout << chunkIdSecondary.x<<" "<<chunkIdSecondary.y<< " " <<
  // chunkIdSecondary.z << " " <<
  // outChunk << " " << currentTargetChannelObject->GetChunk(chunkIdSecondary.x,
  // chunkIdSecondary.y, chunkIdSecondary.z) <<
  //" " << currentTargetChannelObject->dummyChunk << endl;
  if (currentTargetChannelObject->ChunkExists(
          chunkIdSecondary.x, chunkIdSecondary.y, chunkIdSecondary.z)) {
    // cout << "already exists!" << endl;
    // cout << outChunk << endl;

  } else {
    outChunk = currentTargetChannelObject->CreateChunk(
        chunkIdSecondary.x, chunkIdSecondary.y, chunkIdSecondary.z);
    // cout << outChunk << endl;
  }

  //    if(outChunk == currentTargetChannelObject->dummyChunk){
  //         //cout << "err that aint right man!" << endl;
  //    }
  //
  //    }
}

// void GridBasicAdvect::PreGridOp()
//{
//   SetChannelName(std::string("velocity"));

//}

void GridBasicAdvect::GridOp() {
  gridObjectPtr->SwapChannelPointers(std::string("density"));
}
//----------------------------------------------
void GridBasicAdvect::Algorithm(glm::i32vec3 chunkId,
                                glm::i32vec3 voxelPosition, Chunk *inChunk,
                                Chunk *outChunk, uint32_t dataIndex,
                                uint32_t channel)

{

  float X = ((chunkId.x * static_cast<int>(chnkSize)) + voxelPosition.x);

  float Y = ((chunkId.y * static_cast<int>(chnkSize)) + voxelPosition.y);

  float Z = ((chunkId.z * static_cast<int>(chnkSize)) + voxelPosition.z);

  //    float sample = sourceVolume->sampleVolume(glm::vec3(X+0.5, Y+0.5,
  //    Z+0.5));

  glm::vec3 sampleVelocity =
      velocitySourceChannelObject->SampleVectorAtPosition(X, Y, Z);
  // glm::vec3 sampleVelocity =
  // glm::vec3(velocitySourceChannelObject->SampleTrilinear(X-0.5, 0.0f, 0.0f),
  // velocitySourceChannelObject->SampleTrilinear(0.0f, Y-0.5, 0.0f),
  // velocitySourceChannelObject->SampleTrilinear(0.0f, 0.0f, Z-0.5));
  //            float sX = voxelPosition.x - sampleVelocity.x;
  //            float sY = voxelPosition.y - sampleVelocity.y;
  //            float sZ = voxelPosition.z - sampleVelocity.z;

  // outChunk->chunkData[dataIndex] =
  // currentSourceChannelObject->SampleTrilinear(X-sampleVelocity.x,Y-sampleVelocity.y,Z-sampleVelocity.z);
  // outChunk->chunkData[dataIndex] = inChunk->chunkData[dataIndex];
  // float density = inChunk->chunkData[dataIndex];
  outChunk->chunkData[dataIndex] = currentSourceChannelObject->SampleTrilinear(
      X - sampleVelocity.x, Y - sampleVelocity.y, Z - sampleVelocity.z, 0.0f);
  // outChunk->chunkData[dataIndex] =
  // currentSourceChannelObject->SampleTrilinear(X-2,Y-2, Z-2);

  // cout << "xyz" << sampleVelocity.x << " " <<sampleVelocity.y << " " <<
  // sampleVelocity.z << endl;
}

void GridBasicAdvect::PreGridOp() {
  uint32_t velSource =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
  uint32_t densityTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("density"));

  velocitySourceChannelObject =
      gridObjectPtr->channelObjs[velSource].get(); // default first one
  currentTargetChannelObject =
      gridObjectPtr->channelObjs[densityTarget + 1]
          .get(); // want dup density channel.will swap after
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
