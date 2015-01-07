#include "GridMacCormackAdvect.h"
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
GridMacCormackAdvect::~GridMacCormackAdvect() {}

void GridMacCormackAdvect::setupDefaults() {
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

void GridMacCormackAdvect::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
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

void GridMacCormackAdvect::GridOp() {
  gridObjectPtr->SwapChannelPointers(std::string("density"));
}
//----------------------------------------------
void GridMacCormackAdvect::Algorithm(glm::i32vec3 chunkId,
                                glm::i32vec3 voxelWorldPosition, Chunk *inChunk,
                                Chunk *outChunk, uint32_t dataIndex,
                                uint32_t channel, bool internalAccessible)

{

    float X = voxelWorldPosition.x;

    float Y = voxelWorldPosition.y;

    float Z = voxelWorldPosition.z;

  //    float sample = sourceVolume->sampleVolume(glm::vec3(X+0.5, Y+0.5,
  //    Z+0.5));

  glm::vec3 sampleVelocity{0.0};

  sampleVelocity = velocitySourceChannelObject->SampleVectorAtCellCentreFast(X, Y, Z);

  float A = currentSourceChannelObject->SampleExplicit(X-sampleVelocity.x,Y+1-sampleVelocity.y,Z-sampleVelocity.z,0);
  float B = currentSourceChannelObject->SampleExplicit(X-sampleVelocity.x+1,Y-sampleVelocity.y,Z-sampleVelocity.z,0);
  float C = currentSourceChannelObject->SampleExplicit(X-sampleVelocity.x,Y-sampleVelocity.y,Z-sampleVelocity.z+1,0);
  float D = currentSourceChannelObject->SampleExplicit(X-sampleVelocity.x+1,Y-sampleVelocity.y+1,Z-sampleVelocity.z+1,0);
  float E = currentSourceChannelObject->SampleExplicit(X-sampleVelocity.x+1,Y-sampleVelocity.y+1,Z-sampleVelocity.z,0);

  float F = currentSourceChannelObject->SampleExplicit(X-sampleVelocity.x,Y-sampleVelocity.y+1,Z-sampleVelocity.z+1,0);
  float G = currentSourceChannelObject->SampleExplicit(X-sampleVelocity.x+1,Y-sampleVelocity.y,Z-sampleVelocity.z+1,0);



  //phi_n
  float phi_n = inChunk->chunkData[dataIndex];

  float min =   glm::min(glm::min(glm::min(glm::min(glm::min(glm::min(
                    A, B), C), D), E), F), G);

  float max =   glm::max(glm::max(glm::max(glm::max(glm::max(glm::max(
                    A, B), C), D), E), F), G);


      //sampleVelocity = velocitySourceChannelObject->SampleVectorAtCellCentreFast(X, Y, Z);

  //phi_np1_hat into target
  float phi_np1_hat = currentSourceChannelObject->SampleTrilinear(
              X - sampleVelocity.x, Y - sampleVelocity.y, Z - sampleVelocity.z, 0);
  outChunk->chunkData[dataIndex] = phi_np1_hat;






  //phi_n_hat backwards step
#pragma omp barrier
  float phi_n_hat = currentTargetChannelObject->SampleTrilinear(
              X + sampleVelocity.x, Y +  sampleVelocity.y, Z + sampleVelocity.z, 0);


  //phi_n
#pragma omp barrier

    float finalVal = phi_np1_hat +
            ( (phi_n - phi_n_hat)/2);

    finalVal =  glm::max(glm::min(finalVal, max), min);
    outChunk->chunkData[dataIndex] =finalVal;





}

void GridMacCormackAdvect::PreGridOp() {
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
