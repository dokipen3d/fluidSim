#include "GridRK2Advect.h"
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
GridRK2Advect::~GridRK2Advect() {}

void GridRK2Advect::setupDefaults() {
  uint32_t velSource =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
  uint32_t densitySource =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("density"));
  uint32_t densityTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("density"));

  velocitySourceChannelObject =
      gridObjectPtr->channelObjs[velSource].get(); // default first one
  currentSourceChannelObject =
      gridObjectPtr->channelObjs[densityTarget]
          .get(); // want dup density channel.will swap after

  currentTargetChannelObject =
      gridObjectPtr->channelObjs[densityTarget + 1]
          .get(); // want dup density channel.will swap after
  callPreChunkOp = true;
  callGridOp = true;
}

void GridRK2Advect::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
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

void GridRK2Advect::GridOp() {
  gridObjectPtr->SwapChannelPointers(std::string("density"));
}
//----------------------------------------------
void GridRK2Advect::Algorithm(glm::i32vec3 chunkId,
                                glm::i32vec3 voxelWorldPosition, Chunk *inChunk,
                                Chunk *outChunk, uint32_t dataIndex,
                                uint32_t channel, bool internalAccessible)

{

    float X = voxelWorldPosition.x;

    float Y = voxelWorldPosition.y;

    float Z = voxelWorldPosition.z;
  //    Z+0.5));

  glm::vec3 sampleVelocity{0.0};
  glm::vec3 midVelocity{0.0};



      //sampleVelocity = velocitySourceChannelObject->SampleVectorAtCellCentreFast(X, Y, Z);
  sampleVelocity = velocitySourceChannelObject->SampleVectorAtCellCentreFast(X, Y, Z);
  midVelocity  = velocitySourceChannelObject->SampleVectorAtPosition(
              X - (sampleVelocity.x*0.5f), Y - (sampleVelocity.y * 0.5f), Z - (sampleVelocity.z * 0.5f));



  outChunk->chunkData[dataIndex] = currentSourceChannelObject->SampleTrilinear(
      X - midVelocity.x, Y -  midVelocity.y, Z - midVelocity.z, channel);

}

void GridRK2Advect::PreGridOp() {
  uint32_t velSource =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
  uint32_t densityTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("density"));

  velocitySourceChannelObject =
      gridObjectPtr->channelObjs[velSource].get(); // default first one
  currentSourceChannelObject =
      gridObjectPtr->channelObjs[densityTarget].get();
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
