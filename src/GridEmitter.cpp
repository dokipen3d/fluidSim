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
  tileImportType = ETileImportType::raw;


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

  bShouldCreateChunk = 0;
  countCreated = 0;
  callGridOp = true;

}

void GridEmitter::PreGridOp()
{
    countCreated = 0;
    bShouldCreateChunk = 0;

}

void GridEmitter::GridOp()
{
    cout << "count created is " << countCreated << endl;
}

void GridEmitter::Algorithm(int worldX, int worldY, int worldZ, uint32_t indexX, uint32_t indexY, uint32_t indexZ, const std::vector<float> &inTile, std::vector<float> &outTile, const std::vector<float> &extraTile)
{

}

void GridEmitter::ProcessTile(const std::vector<float> &inTile, std::vector<float> &outTile, const std::vector<float> &extraTile, glm::i32vec3 chunkId, Chunk *&pointerRefToSource, Chunk *&pointerRefToTarget)
{

    //std::vector<float> SDFValues(512);
    bool boolShould = false;
    //loop through and store emission values(not actually putting them in grid yet
    for (uint32_t z = 0; z < chnkSize; z++)
    {//for skipping voxels in thr red black gauss seidel update
        for (uint32_t y = 0; y < chnkSize; y++)
        {//can do 1-startvoxel to ping pong between 1 & 0
            for (uint32_t x = 0; x < chnkSize; x++)
            {

                //set chunk as processed so we dont process it twice if it has forceInputBoundsIteration making it in the vector twice;
                //pointerRefToTarget->alreadyProcessedThisIteration = true;


                int X = ((chunkId.x * (int)chnkSize) + x);
                int Y = ((chunkId.y * (int)chnkSize) + y);
                int Z = ((chunkId.z * (int)chnkSize) + z);






                // float sampleA = sourceVolume->sampleVolume(glm::vec3(X, Y, Z));
                float sample = sourceVolume->sampleVolume(glm::vec3(X + 0.5f, Y + 0.5f, Z + 0.5f));//has to be offset by 0.5 because of central differnece ato make sphere sit at 0 (cell centres are at 0.5 on the 'world grid;


                outTile[x + (y*chnkSize) + (z*chnkSize*chnkSize) ] = sample;
                if (sample < bandwidth){
                    boolShould = true;
                }


            }
        }
    }



    //if weve deteced that there should be emission and also if the chunk doesn't exist then go and create it
    if ( boolShould && !currentTargetChannelObject->ChunkExists(chunkId.x, chunkId.y, chunkId.z) )//(glm::sqrt(X + Y + Z)) - radius;
   // if ( !currentTargetChannelObject->ChunkExists(chunkId.x, chunkId.y, chunkId.z) )//(glm::sqrt(X + Y + Z)) - radius;

    {
        //set new target to newly created chunk. this would normally be done in prechunk op but here the creation is dependent on the emission values. since there is no swapping of chunks and we emit directly into same grid, then this isn't an issue
        pointerRefToTarget = currentTargetChannelObject->CreateChunk(chunkId.x, chunkId.y,
                                                           chunkId.z);
        countCreated++;

    }

    //add value
    for (uint32_t z = 0; z < chnkSize; z++)
    {//for skipping voxels in thr red black gauss seidel update
        for (uint32_t y = 0; y < chnkSize; y++)
        {//can do 1-startvoxel to ping pong between 1 & 0
            for (uint32_t x = 0; x < chnkSize; x++)
            {
                //float sdfValue = SDFValues[x + (y*chnkSize) + (z*chnkSize*chnkSize)];
                outTile[x + (y*chnkSize) + (z*chnkSize*chnkSize) ] =

                        inTile[x + (y*chnkSize) + (z*chnkSize*chnkSize) ] + (0.64 * when_less_than(outTile[x + (y*chnkSize) + (z*chnkSize*chnkSize) ], 0.0f));




            }
        }
    }







           // float newval = addPositiveDifference(outChunk->chunkData[dataIndex],3.0f* glm::abs(glm::simplex(glm::vec3(X/19,Y/19-(currentTime*3.4),Z/19)) ) );

}

//----------------------------------------------


float GridEmitter::addPositiveDifference(float inputReference,
                                         float amountToAdd) {
  return glm::max(0.5f,inputReference + (amountToAdd - inputReference));
  //return inputReference + amountToAdd;

}

void GridEmitter::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk, glm::i32vec3 chunkIdSecondary)
{
  bShouldCreateChunk = false;
}

// void GridEmitter::PreChunkOp(Chunk *inChunk, glm::i32vec3 chunkIdSecondary)
//{
//    //here we do a presample of emission volume once per bucket. simple way to
//    stop race condition if we were to do it per voxel.

//}
