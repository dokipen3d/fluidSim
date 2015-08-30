#include "GridEmitter.h"
#include "Chunk.h"
#include "ChannelObject.h"
#include "glm/vec3.hpp"
#include "glm/trigonometric.hpp"
#include "glm/gtc/noise.hpp"


//#include <omp.h>

inline static float map_range(float value, float low1, float high1, float low2,
                              float high2) {
  return low2 + (high2 - low2) * (value - low1) / (high1 - low1);
}

//----------------------------------------------
GridEmitter::~GridEmitter() {}

void GridEmitter::setupDefaults() {
  createChunks = true;
  std::string name = "sphereEmitter";
  forceInputBoundsIteration = true;
  //auto emitterSphere = make_shared<ImplicitTorus>(name);
  auto emitterSphere = make_shared<ImplicitSphere>(name);
  // callPreChunkOp = true;
  sourceVolume = emitterSphere;
  // work out chunks to process based on implicit bounding box
  //    int minX = glm::floor(sourceVolume->boundingBox.min.x/chnkSize);
  //    int minY = glm::floor(sourceVolume->boundingBox.min.y/chnkSize);
  //    int minZ = glm::floor(sourceVolume->boundingBox.min.z/chnkSize);
  //    int maxX = glm::floor(sourceVolume->boundingBox.max.x/chnkSize);
  //    int maxY = glm::floor(sourceVolume->boundingBox.max.y/chnkSize);
  //    int maxZ = glm::floor(sourceVolume->boundingBox.max.z/chnkSize);
  //    int minX =  glm::floor(sourceVolume->boundingBox.min.x/chnkSize);
  //    int minY =  glm::floor(sourceVolume->boundingBox.min.y/chnkSize);
  //    int minZ =  glm::floor(sourceVolume->boundingBox.min.z/chnkSize);
  //    int maxX =  glm::ceil(sourceVolume->boundingBox.max.x/chnkSize);
  //    int maxY =  glm::ceil(sourceVolume->boundingBox.max.y/chnkSize);
  //    int maxZ =  glm::ceil(sourceVolume->boundingBox.max.z/chnkSize);


  this->boundingBox.minX = sourceVolume->boundingBox.minX.load();
  this->boundingBox.minY = sourceVolume->boundingBox.minY.load();

  this->boundingBox.minZ = sourceVolume->boundingBox.minZ.load();

  this->boundingBox.maxX = sourceVolume->boundingBox.maxX.load();
  this->boundingBox.maxY = sourceVolume->boundingBox.maxY.load();

  this->boundingBox.maxZ = sourceVolume->boundingBox.maxZ.load();



  float intPartMinX;
  float intPartMinY;
  float intPartMinZ;
  float intPartMaxX;
  float intPartMaxY;
  float intPartMaxZ;

  // sep sphere bounds into frac and int
  float minX_a = std::modf(this->boundingBox.minX, &intPartMinX);
  float minY_a = std::modf(this->boundingBox.minY, &intPartMinY);
  float minZ_a = std::modf(this->boundingBox.minZ, &intPartMinZ);

  float maxX_a = std::modf(this->boundingBox.maxX, &intPartMaxX);
  float maxY_a = std::modf(this->boundingBox.maxY, &intPartMaxY);
  float maxZ_a = std::modf(this->boundingBox.maxZ, &intPartMaxZ);

  cout << "inpart is " << intPartMinX << " - " << intPartMaxX << endl;
  cout << "floatPart is " << minX_a << " - " << maxX_a << endl;

  int minX = glm::floor(sourceVolume->boundingBox.minX /
                        chnkSize); // only which chunk ids should exists
  int minY = glm::floor(sourceVolume->boundingBox.minY / chnkSize);
  int minZ = glm::floor(sourceVolume->boundingBox.minZ / chnkSize);
  int maxX = glm::floor((sourceVolume->boundingBox.maxX) / chnkSize);
  int maxY = glm::floor((sourceVolume->boundingBox.maxY) / chnkSize);
  int maxZ = glm::floor((sourceVolume->boundingBox.maxZ) / chnkSize);

  this->boundingBox.fluidMinX = minX;
  this->boundingBox.fluidMinY = minY;
  this->boundingBox.fluidMinZ = minZ;

  this->boundingBox.fluidMaxX = maxX;
  this->boundingBox.fluidMaxY = maxY;
  this->boundingBox.fluidMaxZ = maxZ;

  cout << "emitter bounds is" << this->boundingBox.minX << " to "
       << this->boundingBox.maxX << endl;
  cout << "emitter bounds is" << this->boundingBox.minY << " to "
       << this->boundingBox.maxY << endl;
  cout << "emitter bounds is" << this->boundingBox.minZ << " to "
       << this->boundingBox.maxZ << endl;

  cout << "grid emitter bounds set" << this->boundingBox.fluidMinX << " to "
       << this->boundingBox.fluidMaxX << endl;
  cout << "grid emitter bounds set" << this->boundingBox.fluidMinY << " to "
       << this->boundingBox.fluidMaxY << endl;
  cout << "grid emitter bounds set" << this->boundingBox.fluidMinZ << " to "
       << this->boundingBox.fluidMaxZ << endl;
}

//----------------------------------------------
void GridEmitter::Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelWorldPosition,
                            Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                            uint32_t channel, bool internalAccessible)

{
  // myString <<  "and data is " << inChunk->chunkData[chunkDataIndex] << endl;
  // inChunk->chunkData[chunkDataIndex] = 5;
  // myString << " and data is " << inChunk->chunkData[chunkDataIndex] << endl;
  // float sample =
  // currentSourceChannelObject->SampleChannelAtPosition(500.5,0.0,0.0000);
  // myString << "in algo" << endl;
  //    float sample =
  //    sourceVolume->sampleVolume(glm::vec3((chunkId.x&(chnkSize-1))+voxelPosition.x,//this
  //    mod math can be sped up once working with bitwise. because 4 and  are
  //    power of two we could do & instead of %
  //                                                        (chunkId.y&(chnkSize-1))+voxelPosition.y,
  //                                                        (chunkId.z&(chnkSize-1))+voxelPosition.z)
  //                                                        );
  //    float sample =
  //    sourceVolume->sampleVolume(glm::vec3((chunkId.x%(chnkSize))+voxelPosition.x,//this
  //    mod math can be sped up once working with bitwise. because 4 and  are
  //    power of two we could do & instead of %
  //                                                        (chunkId.y%(chnkSize))+voxelPosition.y,
  //                                                        (chunkId.z%(chnkSize))+voxelPosition.z)
  //                                                        );
  // cout << "in chunk algo" << chunkId.x << " " << chunkId.y << " " <<
  // chunkId.z << endl;

  // cout << voxelPosition.x << ", ";
  //    float X = ((chunkId.x*(int)chnkSize)+voxelPosition.x);

  //    float Y = ((chunkId.y*(int)chnkSize)+voxelPosition.y);

  //    float Z = ((chunkId.z*(int)chnkSize)+voxelPosition.z);

    float X = voxelWorldPosition.x;

    float Y = voxelWorldPosition.y;

    float Z = voxelWorldPosition.z;



  // float sampleA = sourceVolume->sampleVolume(glm::vec3(X, Y, Z));
  float sample =
      sourceVolume->sampleVolume(glm::vec3(voxelWorldPosition.x  + 0.5f, voxelWorldPosition.y + 0.5f, voxelWorldPosition.z + 0.5f));//has to be offset by 0.5 because of central differnece ato make sphere sit at 0 (cell centres are at 0.5 on the 'world grid;

  if (sample < bandwidth) {
    if (!currentTargetChannelObject->ChunkExists(chunkId.x, chunkId.y,
                                                 chunkId.z)) {
      // do checks to emit
      // cout << "chunk id is " << chunkId.x <<  " " << chunkId.y << " " <<
      // chunkId.z << endl;

      //            if ( chunkId.x == 2 && chunkId.y == 2 && chunkId.z == 2){
      //                            //cout << "222 is being called "  << endl;
      //                            cout << "222 exists? " << boolalpha <<
      //                            currentSourceChannelObject->ChunkExists(chunkId.x,
      //                            chunkId.y, chunkId.z) << endl;
      //            }

      outChunk = currentTargetChannelObject->CreateChunk(chunkId.x, chunkId.y,
                                                         chunkId.z);
      // inChunk->chunkData[dataIndex] = 0;
      // myString << "empty ptr can creat if I want" << endl;

      //

    }

    else {
      //                if ( chunkId.x == 2 && chunkId.y == 2 && chunkId.z ==
      //                2){
      //                                //cout << "222 is being called "  <<
      //                                endl;
      //                                cout << "222 exists? " << boolalpha <<
      //                                currentSourceChannelObject->ChunkExists(chunkId.x,
      //                                chunkId.y, chunkId.z) << endl;
      //                }
      // myString << "getting chunk" << endl;
      outChunk =
          currentTargetChannelObject->GetChunk(chunkId.x, chunkId.y, chunkId.z);
    }

    // can safely assume chunk exists?
    // if (inChunk == currentSourceChannelObject->dummyChunk)
    // cout << "erm DUMMY!" << endl;
    // myString << "setting valuefor chunk " << chunkId.x << chunkId.y <<
    // chunkId.z << voxelPosition.x << voxelPosition.y << voxelPosition.z
    // <<endl;
    if (sample < 0.0f) {
      // map_range(sample, -16.0f, -1.0f, 1.0f, 0.1f)

      // map_range(sample, -32.0f, 0.0f, value*0.08, 0.0f)
      //float value = glm::max(glm::cos(currentTime*1.2), 0.0);
      if (currentTime < 500) {
        //outChunk->chunkData[dataIndex] += 0.2f;
          float newval = addPositiveDifference(outChunk->chunkData[dataIndex],3.0f* glm::abs(glm::simplex(glm::vec3(X/19,Y/19-(currentTime*2.4),Z/19)) ) );
          //float newval = addPositiveDifference(outChunk->chunkData[dataIndex],3.0f);

        outChunk->chunkData[dataIndex] = newval ;
      }
//      else {
//        outChunk->chunkData[dataIndex] += 0.0f;
//      }
      // inChunk->chunkData[dataIndex] =
      // addPositiveDifference(inChunk->chunkData[dataIndex],0.2  );
    }

    // else
    // inChunk->chunkData[dataIndex] += 0.0f;

    // inChunk->chunkData[dataIndex] = chunkId.x;
  }

  // myString << "running" << endl;

  // write code to emit into grid

  // int threadid  = omp_get_thread_num();
  // myString << voxelPosition.x<< " " << voxelPosition.y << " "
  // <<voxelPosition.z << endl;
  // myString << sample << " thread: " << omp_get_thread_num() << endl;
  // myString << currentSourceChannelObject->channelInfo.channelName << endl;
}

float GridEmitter::addPositiveDifference(float inputReference,
                                         float amountToAdd) {
  return glm::max(0.5f,inputReference + (amountToAdd - inputReference));
  //return inputReference + amountToAdd;

}

// void GridEmitter::PreChunkOp(Chunk *inChunk, glm::i32vec3 chunkIdSecondary)
//{
//    //here we do a presample of emission volume once per bucket. simple way to
//    stop race condition if we were to do it per voxel.

//}
