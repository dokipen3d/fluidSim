#include "GridBasicVelAdvect.h"
#include "Chunk.h"
#include "ChannelObject.h"
#include "glm/vec3.hpp"
#include "glm/trigonometric.hpp"
#include "GridObject.h"
#include <stdio.h>
#include <cassert>
#include <omp.h>

//#include <omp.h>

inline static float map_range(float value, float low1, float high1, float low2,
                              float high2) {
  return low2 + (high2 - low2) * (value - low1) / (high1 - low1);
}

//----------------------------------------------
GridBasicVelAdvect::~GridBasicVelAdvect() {}

void GridBasicVelAdvect::setupDefaults() {
  uint32_t velSource =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
  uint32_t velTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));

  currentSourceChannelObject =
      gridObjectPtr->channelObjs[velSource].get(); // default first one
  currentTargetChannelObject =
      gridObjectPtr->channelObjs[velTarget+1]
          .get(); // want dup density channel.will swap after
  callPreChunkOp = true;

  callGridOp = true;
  internalChannels = 3;
}

void GridBasicVelAdvect::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                                 glm::i32vec3 chunkIdSecondary) {

    //printf("Hello from thread %d, nthreads %d\n", omp_get_thread_num(), omp_get_num_threads());

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

void GridBasicVelAdvect::GridOp() {
  gridObjectPtr->SwapChannelPointers(std::string("velocity"));
  cout << "swapped pointers for vel" << endl;
}
//----------------------------------------------
void GridBasicVelAdvect::Algorithm(glm::i32vec3 chunkId,
                                glm::i32vec3 voxelPosition, Chunk *inChunk,
                                Chunk *outChunk, uint32_t dataIndex,
                                uint32_t channel)

{

  float X = ((chunkId.x * static_cast<int>(chnkSize)) + voxelPosition.x);

  float Y = ((chunkId.y * static_cast<int>(chnkSize)) + voxelPosition.y);

  float Z = ((chunkId.z * static_cast<int>(chnkSize)) + voxelPosition.z);

  //    float sample = sourceVolume->sampleVolume(glm::vec3(X+0.5, Y+0.5,
  //    Z+0.5));

  glm::vec3 sampleVelocity;


      sampleVelocity = currentSourceChannelObject->SampleVectorAtCellFaceFast(X, Y, Z, channel);
      //sampleVelocity = velocitySourceChannelObject->SampleVectorAtPosition(X, Y, Z);
      //sampleVelocity = glm::vec3{0.0f};


      switch (channel) {
          case 0: // U
              outChunk->chunkData[dataIndex] = currentSourceChannelObject->SampleTrilinear(
                            X  - sampleVelocity.x, Y - sampleVelocity.y, Z - sampleVelocity.z, channel);
              break;
          case 1: // V
                Y -= 0.5f;
              outChunk->chunkData[dataIndex] = currentSourceChannelObject->SampleTrilinear(
                            X - sampleVelocity.x, Y - sampleVelocity.y, Z - sampleVelocity.z, channel);
              break;

          case 2: // W
              outChunk->chunkData[dataIndex] = currentSourceChannelObject->SampleTrilinear(
                            X - sampleVelocity.x, Y - sampleVelocity.y, Z - sampleVelocity.z, channel);
              break;
          default:
              outChunk->chunkData[dataIndex] = 0.0f;
              break;
        }

//      outChunk->chunkData[dataIndex] = currentSourceChannelObject->SampleTrilinear(
//                                 X - sampleVelocity.x, Y + 0.5f - sampleVelocity.y, Z - sampleVelocity.z, channel);

}

void GridBasicVelAdvect::PreGridOp() {
    uint32_t velSource =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
    uint32_t velTarget =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));

    currentSourceChannelObject =
        gridObjectPtr->channelObjs[velSource].get(); // default first one
    currentTargetChannelObject =
        gridObjectPtr->channelObjs[velTarget+1]
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
