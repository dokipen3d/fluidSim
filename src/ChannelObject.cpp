#include "ChannelObject.h"
#include "GridObject.h"
#include <omp.h>
#include <atomic>
#include "glm/vec3.hpp"
#include "GlobalTypes.hpp"

#include <iostream>

using namespace std;

inline static uint32_t flatten3dCoordinatesto1D(uint32_t x, uint32_t y,
                                                uint32_t z, uint32_t channel,
                                                uint32_t chunkSize) {
  // return   (        (x + chunkSize * (y + chunkSize * z))  *channel) +
  // (channel*chunkSize*chunkSize*chunkSize);
  return ((x) + (((y)*chunkSize)) + (((z)*chunkSize * chunkSize))) +
         (channel * chunkSize * chunkSize * chunkSize);
  // return ((x+channel) + ((y+channel) << 3 ) +  ((z+channel) << 3 << 3));
//  return ((x) + (((y)<<3)) + (((z)<< 3 << 3))) +
//         (channel << 3 << 3 << 3);

  // look into bit shifting
  // if chunksize is   4 we can just do (y+channel) << 2
  //                  8 we can just do (y+channel) << 3
  //                  16 we can just do (y+channel) << 4
  //                  32 we can just do (y+channel) << 5
  //                  64 we can just do (y+channel) << 6
  //                  128 we can just do (y+channel) << 7
  //                  256 we can just do (y+channel) << 8
  //                  512 we can just do (y+channel) << 9
  //                  1024 we can just do (y+channel) << 10

  // same with encoding integerto32bit but limited to 1024 in each direction
  // (512 +/-)
}

//----------------------------------------------
inline static int integerHash3D(int a, int b, int c) {
  return ((a * 73856093 + b * 19349663 + c * 83492791) & 2094967294) + 1;
  // return ((a*73856093 + b*19349663 + c*83492791)%4294967294)+1;

  // return ((a/73856093 + b*-19349663 - c*83492791)%4294967294)+1;
}

inline static uint32_t integerEncodeTo32BitInt(int x, int y, int z) {
  //return ((x + 813) + ((y + 813) * 1625) + ((z + 813) * 1625 * 1625)) + 1;
   return ((x+812) + ((y+812) << 10 ) + ((z+812) << 10 << 10) )+1;

  // return ((a*73856093 + b*19349663 + c*83492791)%4294967294)+1;

  // return ((a/73856093 + b*-19349663 - c*83492791)%4294967294)+1;
}
//----------------------------------------------
inline static int32_t fastFloor(float x) {

  // return (x > 0 ? (int32_t)x : (int32_t)x - 1);
  return (x > 0 ? static_cast<int32_t>(x) : static_cast<int32_t>(x - 1));
}

inline static void separateFracInt(const float &in, float &floatPart,
                                   u_int32_t &intPart, const float &chunkSize) {
  // float tempIn = in;

  float temp = glm::mod(in, chunkSize);
  float tempFloatyInt;
  floatPart = std::modf(temp, &tempFloatyInt);
  intPart = (int)tempFloatyInt;
  //intPart = static_cast<uint32_t>(tempFloatyInt);
 // if (intPart == 8)
   //   cout <<"in: " << in << " floatpart: " << floatPart << " temp floaty int: " << tempFloatyInt << " mod chnk: " << temp << endl;
}

inline static void separateFracIntFast(const float &in,
                                   u_int32_t &intPart, uint32_t parentChunkSizem1) {
  // float tempIn = in;

  int floatToInt = glm::floor(in);
  intPart = floatToInt & parentChunkSizem1;

}

//----------------------------------------------
ChannelObject::ChannelObject(ChannelInfo inInfo, GridObject *parentGrid) {
  // chunks = std::unique_ptr<LockFreeHashTable>(new
  // LockFreeHashTable(65536));//default size of 2^16, 16-bit
  //chunks = std::make_unique<LockFreeHashTable>(65536);
  //chunks = std::make_unique<LockFreeHashTable>(131072);
    chunks = std::make_unique<LockFreeHashTable>(262144);


  channelInfo = inInfo;
  parentGridObject = parentGrid;
  defaultValue = 0.0f;
  // cout << "channel Object " << inInfo.channelName << " created" << endl;
  SamplerPointerFunc[0] = &ChannelObject::SampleTrilinear;
  SamplerPointerFunc[1] = &ChannelObject::SampleTrilinear;
  SamplerPointerFunc[2] = &ChannelObject::SampleTrilinear;
  parentChunkSize = parentGridObject->chunkSize;
  parentChunkSizeMinus1 = parentChunkSize - 1;

  fParentChunkSize = (float)parentChunkSize;
  currentChannelToSample = 0; // set to a single channel by default. in
                              // overloaded SampleTrilinear, use this to go 0-2.
                              // dont forget to set it back to 0;
  channelAmount = 1;          // for making scalar or cevtor
  if (channelInfo.channelType == ChannelType::vector) {
    channelAmount = 3;
  }

  voxelSize = 1.0f / parentChunkSize;
  // create a dummy chunk so when we sample outside bounds we get a default
  // value
  dummyChunk = new Chunk((parentChunkSize * parentChunkSize * parentChunkSize) *
                         channelAmount);
  // set a pointer in the hash table to where our dummy data is
  chunks->setDummyItem((void *)dummyChunk);
  getRidOfDivCalc = 1.0f / parentChunkSize;

  // chunks->setDummyItem(dynamic_cast<void*>(dummyChunk));

  //    cout << "has of 2,2,2 is " << integerHash3D(2,2,2) << endl;
  //    cout << "has of -2,-2,-2 is " << integerHash3D(-2,-2,-2) << endl;
  //    cout << "has of -2,-2,-2 is " << integerHash3D(1,1,1) << endl;
}

//----------------------------------------------
ChannelObject::~ChannelObject() {

  ClearChunks();
  delete dummyChunk;
  // cout << "channel Object " << channelInfo.channelName << " destroyed" <<
  // endl;
}

//void ChannelObject::separateFracInt(const float &in, float &floatPart,
//                                   u_int32_t &intPart, const float &chunkSize) {
//    float tempIn = in;

//  float temp = glm::mod(tempIn, 8.0f);
//  float tempFloatyInt;
//  floatPart = std::modf(temp, &tempFloatyInt);
//  intPart = (u_int32_t)tempFloatyInt;
//  //intPart = static_cast<uint32_t>(tempFloatyInt);
//  //if (intPart == 8)
//  //    cout <<"in: " << in << " floatpart: " << floatPart << " temp floaty int: " << tempFloatyInt << " mod chnk: " << temp << endl;
//}

//----------------------------------------------
void ChannelObject::ClearChunks() {
  for (int k = parentGridObject->boundingBox.fluidMin.z;
       k <= parentGridObject->boundingBox.fluidMax.z; k++) {
    for (int j = parentGridObject->boundingBox.fluidMin.y;
         j <= parentGridObject->boundingBox.fluidMax.y; j++) {
      for (int i = parentGridObject->boundingBox.fluidMin.x;
           i <= parentGridObject->boundingBox.fluidMax.x; i++) {
        DeleteChunk(i, j, k);
      }
    }
  }
}

//----------------------------------------------
Chunk *ChannelObject::CreateChunk(int32_t x, int32_t y, int32_t z) {

  int vec = 1; // for making scalar or vector
  // int pcs = (int)parentGridObject->chunkSize;
  int pcs = static_cast<int>(parentGridObject->chunkSize);

  // cout << "chunk " << x << ", " << y << ", " << z << endl;
  float minX = x >= 0 ? (x + 1) * pcs : x * pcs;
  float minY = y >= 0 ? (y + 1) * pcs : y * pcs;
  float minZ = z >= 0 ? (z + 1) * pcs : z * pcs;
  float maxX = x * pcs;
  float maxY = y * pcs;
  float maxZ = z * pcs;
  //    float minX = (x*(pcs));
  //    float minY = (y*(pcs));
  //    float minZ = (z*(pcs));
  //    float maxX = (x+1)*pcs;
  //    float maxY = (y+1)*pcs;
  //    float maxZ = (z+1)*pcs;

  if (boundsSet == false) {
    parentGridObject->boundingBox.fluidMin.x = x;
    parentGridObject->boundingBox.fluidMin.y = y;
    parentGridObject->boundingBox.fluidMin.z = z;
    parentGridObject->boundingBox.fluidMax.x = x;
    parentGridObject->boundingBox.fluidMax.y = y;
    parentGridObject->boundingBox.fluidMax.z = z;
    parentGridObject->boundingBox.min.x = minX;
    parentGridObject->boundingBox.min.y = minY;
    parentGridObject->boundingBox.min.z = minZ;
    parentGridObject->boundingBox.max.x = minX;
    parentGridObject->boundingBox.max.y = minY;
    parentGridObject->boundingBox.max.z = minZ;
    // cout << "bounds min is " << parentGridObject->boundingBox.min.x <<
    // "bounds max is "<< parentGridObject->boundingBox.max.x << endl;

    boundsSet = true;

  }

  else {
    parentGridObject->boundingBox.fluidMin.x =
        glm::min(x, parentGridObject->boundingBox.fluidMin.x);
    parentGridObject->boundingBox.fluidMin.y =
        glm::min(y, parentGridObject->boundingBox.fluidMin.y);
    parentGridObject->boundingBox.fluidMin.z =
        glm::min(z, parentGridObject->boundingBox.fluidMin.z);
    parentGridObject->boundingBox.fluidMax.x =
        glm::max(x, parentGridObject->boundingBox.fluidMax.x);
    parentGridObject->boundingBox.fluidMax.y =
        glm::max(y, parentGridObject->boundingBox.fluidMax.y);
    parentGridObject->boundingBox.fluidMax.z =
        glm::max(z, parentGridObject->boundingBox.fluidMax.z);

    parentGridObject->boundingBox.min.x =
        glm::min(minX, parentGridObject->boundingBox.min.x);
    parentGridObject->boundingBox.min.y =
        glm::min(minY, parentGridObject->boundingBox.min.y);
    parentGridObject->boundingBox.min.z =
        glm::min(minZ, parentGridObject->boundingBox.min.z);
    parentGridObject->boundingBox.max.x =
        glm::max(maxX + pcs, parentGridObject->boundingBox.max.x);
    parentGridObject->boundingBox.max.y =
        glm::max(maxY + pcs, parentGridObject->boundingBox.max.y);
    parentGridObject->boundingBox.max.z =
        glm::max(maxZ + pcs, parentGridObject->boundingBox.max.z);
    // cout << "bounds min is " << parentGridObject->boundingBox.min.x <<
    // "bounds max is "<< parentGridObject->boundingBox.max.x << endl;
  }

  if (channelInfo.channelType == ChannelType::vector) {
    vec = 3;
  }

  uint32_t chunkKey = integerEncodeTo32BitInt(x, y, z);
  //uint32_t chunkKey = integerHash3D(x, y, z);

  if (controlChannel) {
    // because we have two channels potentially for advecting/copying into (some
    // channels might write in place) we need to increment by two throught the
    // channls vector
    for (int i = 2; i < parentGridObject->channelObjs.size(); i += 2) {
      // cout << "i is " << i <<endl;
      // parentGridObject->channelObjs.at(i)->CreateChunk(x, y, z);
      Chunk *newChunkb =
          new Chunk((parentChunkSize * parentChunkSize * parentChunkSize) *
                    parentGridObject->channelObjs[i]->channelAmount);

      parentGridObject->channelObjs[i]->chunks->SetItem(chunkKey,
                                                        (uintptr_t)newChunkb);
      // parentGridObject->channelObjs[i]->chunks->SetItem(chunkKey,
      // static_cast<uintptr_t>(newChunkb));
    }
  }

  // for padding node

  //    if (x == -2 && y = -2 && z== -2){

  //        cout << "222 is being created in channel and hash is " << chunkKey
  //        << endl;
  //    }
  Chunk *newChunk = new Chunk(
      (parentChunkSize * parentChunkSize * parentChunkSize) * channelAmount);
  // cout << "created" << endl;
  newChunk->empty = true;
  newChunk->okayToDelete = false;
  newChunk->justCreatedOneFrameAgo = true;
  newChunk->voxelCount = pcs * pcs * pcs;
  newChunk->timeCreated = parentGridObject->simTime;

  chunks->SetItem(chunkKey, (uintptr_t)newChunk);
  // cout << "chunk created" << endl;
  numChunks++;

  return newChunk;
}

//----------------------------------------------
Chunk *ChannelObject::GetChunk(int32_t x, int32_t y, int32_t z) {
  uint32_t chunkKey = integerEncodeTo32BitInt(x, y, z);
  //uint32_t chunkKey = integerHash3D(x, y, z);
  return (Chunk *)chunks->GetItem(chunkKey);
}

//----------------------------------------------
bool ChannelObject::ChunkExists(int32_t x, int32_t y, int32_t z) {
  // cout << "checking exisitnece of " << x << y << z << endl;
  uint32_t chunkKey = integerEncodeTo32BitInt(x, y, z);
   // uint32_t chunkKey = integerHash3D(x, y, z);

  //    if (x == 2 && y ==2 && z==2){

  //        cout << "222 is being checked in channel and hash is " << chunkKey
  //        << endl;
  //    }

  //    if (chunkKey == 2421441){
  //        cout << "key is same as " << x << y << z << endl;
  //    }
  // cout << chunkKey << endl;
  return chunks->KeyExists(chunkKey);
}

//----------------------------------------------
float ChannelObject::SampleChannelAtPosition(float x, float y,
                                             float z) // should only need float
                                                      // as the vector channels
                                                      // behave as 3 floats
                                                      // anyway
{
  // cout << "about to sample" <<endl;
  return (*this.*SamplerPointerFunc[1])(
      x, y, z,
      0); // replace 0 with the enum cast to int depending on mode of channel
}

glm::vec3 ChannelObject::SampleVectorAtPosition(float x, float y, float z) {
  // remember to -0.5 for float channels at they sit on grid faces. +0.5 in our case as we store the -1/2 face at [0] in the vector and then loop round
  glm::vec3 vectorValue{0.0f, 0.0f, 0.0f};

  //#pragma omp critical
  {
    int currentChannelToSampleLocal = 0;
    vectorValue.x =
        SampleTrilinear(x + 0.5f, y, z, currentChannelToSampleLocal);
    currentChannelToSampleLocal++;
    vectorValue.y =
        SampleTrilinear(x, y + 0.5f, z, currentChannelToSampleLocal);
    currentChannelToSampleLocal++;
    vectorValue.z =
        SampleTrilinear(x, y, z + 0.5f, currentChannelToSampleLocal);
  }
  return vectorValue;
}

glm::vec3 ChannelObject::SampleVectorAtPositionVel(float x, float y, float z) {
  // remember to -0.5 for float channels at they sit on grid faces. +0.5 in our case as we store the -1/2 face at [0] in the vector and then loop round
  glm::vec3 vectorValue{0.0f, 0.0f, 0.0f};

  //#pragma omp critical
  {
    int currentChannelToSampleLocal = 0;
    vectorValue.x =
        SampleTrilinear(x, y + 0.5f, z + 0.5f, currentChannelToSampleLocal);
    currentChannelToSampleLocal++;
    vectorValue.y =
        SampleTrilinear(x + 0.5f, y, z + 0.5f, currentChannelToSampleLocal);
    currentChannelToSampleLocal++;
    vectorValue.z =
        SampleTrilinear(x + 0.5f, y + 0.5f, z, currentChannelToSampleLocal);
  }
  return vectorValue;
}

glm::vec3 ChannelObject::SampleVectorAtPositionExplicit(float x, float y,
                                                        float z) {
  // remember to -0.5 for float channels at they sit on grid faces
  glm::vec3 vectorValue;

  currentChannelToSample = 0;
  vectorValue.x = SampleExplicit(x + 0.5f, y, z, currentChannelToSample);
  currentChannelToSample++;
  vectorValue.y = SampleExplicit(y, y + 0.5f, z, currentChannelToSample);
  currentChannelToSample++;
  vectorValue.z = SampleExplicit(z, y, z + 0.5f, currentChannelToSample);
  currentChannelToSample = 0;

  return vectorValue;
}

glm::vec3 ChannelObject::SampleVectorAtCellCentreFast(float x, float y,
                                                      float z) {
  float u;
  float v;
  float w;

  u = (this->SampleExplicit(x, y, z, 0) +
       this->SampleExplicit(x + 1, y, z, 0)) /
      2.0f; // have to do + voxel size for now as the chunk check happens in
            // Sampleexplicit. +1 would mean +1 in absolute distance
  v = (this->SampleExplicit(x, y, z, 1) +
       this->SampleExplicit(x, y + 1, z, 1)) /
      2.0f;
  w = (this->SampleExplicit(x, y, z, 2) +
       this->SampleExplicit(x, y, z + 1, 2)) /
      2.0f;

  return glm::vec3(u, v, w);
}

glm::vec3 ChannelObject::SampleVectorAtCellFaceFast(float x, float y, float z,
                                                    u_int32_t channel) {
  float u;
  float v;
  float w;

  switch (channel) {
  case 0: // U
    u = (this->SampleExplicit(x, y, z, 0));
    v = (this->SampleExplicit(x, y - 1, z, 1) +
         this->SampleExplicit(x, y, z, 1) +
         this->SampleExplicit(x + 1, y - 1 , z, 1) +
         this->SampleExplicit(x + 1, y , z, 1)) /
        4.0f;
    w = (this->SampleExplicit(x, y, z - 1, 2) +
         this->SampleExplicit(x, y, z , 2) +
         this->SampleExplicit(x + 1, y, z - 1, 2) +
         this->SampleExplicit(x + 1, y, z , 2)) /
        4.0f;

    return glm::vec3(u, v, w);

  case 1: // V
    u = (this->SampleExplicit(x - 1, y, z, 0) +
         this->SampleExplicit(x , y, z, 0) +
         this->SampleExplicit(x - 1, y + 1, z, 0) +
         this->SampleExplicit(x, y + 1, z, 0)) /
        4.0f;
    v = (this->SampleExplicit(x, y, z, 1));
    w = (this->SampleExplicit(x, y, z - 1, 2) +
         this->SampleExplicit(x, y, z, 2) +
         this->SampleExplicit(x, y + 1, z - 1, 2) +
         this->SampleExplicit(x, y + 1, z, 2)) /
        4.0f;

    return glm::vec3(u, v, w);

  case 2: // W
    u = (this->SampleExplicit(x - 1, y, z, 0) +
         this->SampleExplicit(x, y, z, 0) +
         this->SampleExplicit(x - 1, y, z + 1, 0) +
         this->SampleExplicit(x , y, z + 1, 0)) /
        4.0f;
    v = (this->SampleExplicit(x, y - 1, z, 1) +
         this->SampleExplicit(x, y, z, 1) +
         this->SampleExplicit(x, y - 1, z + 1, 1) +
         this->SampleExplicit(x, y, z + 1, 1)) /
        4.0f;
    w = (this->SampleExplicit(x, y, z, 2));

    return glm::vec3(u, v, w);

//    switch (channel) {
//    case 0: // U
//      u = (this->SampleExplicit(x, y, z, 0));
//      v = (this->SampleExplicit(x, y + 1, z, 1) +
//           this->SampleExplicit(x, y, z, 1) +
//           this->SampleExplicit(x + 1, y + 1 , z, 1) +
//           this->SampleExplicit(x + 1, y , z, 1)) /
//          4.0f;
//      w = (this->SampleExplicit(x, y, z + 1, 2) +
//           this->SampleExplicit(x, y, z , 2) +
//           this->SampleExplicit(x + 1, y, z + 1, 2) +
//           this->SampleExplicit(x + 1, y, z , 2)) /
//          4.0f;

//      return glm::vec3(u, v, w);

//    case 1: // V
//      u = (this->SampleExplicit(x + 1, y, z, 0) +
//           this->SampleExplicit(x , y, z, 0) +
//           this->SampleExplicit(x + 1, y + 1, z, 0) +
//           this->SampleExplicit(x, y + 1, z, 0)) /
//          4.0f;
//      v = (this->SampleExplicit(x, y, z, 1));
//      w = (this->SampleExplicit(x, y, z + 1, 2) +
//           this->SampleExplicit(x, y, z, 2) +
//           this->SampleExplicit(x, y + 1, z + 1, 2) +
//           this->SampleExplicit(x, y + 1, z, 2)) /
//          4.0f;

//      return glm::vec3(u, v, w);

//    case 2: // W
//      u = (this->SampleExplicit(x + 1, y, z, 0) +
//           this->SampleExplicit(x, y, z, 0) +
//           this->SampleExplicit(x + 1, y, z + 1, 0) +
//           this->SampleExplicit(x , y, z + 1, 0)) /
//          4.0f;
//      v = (this->SampleExplicit(x, y + 1, z, 1) +
//           this->SampleExplicit(x, y, z, 1) +
//           this->SampleExplicit(x, y + 1, z + 1, 1) +
//           this->SampleExplicit(x, y, z + 1, 1)) /
//          4.0f;
//      w = (this->SampleExplicit(x, y, z, 2));

//      return glm::vec3(u, v, w);
    }
}

void ChannelObject::printChunks() {
  cout << "num of chunks in hash table: " << chunks->GetItemCount() << endl;
}
//---------------------------------------------
float ChannelObject::SampleTrilinear2(float x, float y, float z,
                                     uint32_t channel) {

    float getRidOfDivCalc = 1.0f / parentChunkSize;

    int32_t chunkIndexDivX = glm::floor(x * getRidOfDivCalc);
    int32_t chunkIndexDivY = glm::floor(y * getRidOfDivCalc);
    int32_t chunkIndexDivZ = glm::floor(z * getRidOfDivCalc);



    float fracX;
    float fracY;
    float fracZ;



    uint32_t localGridIndexX;
    uint32_t localGridIndexY;
    uint32_t localGridIndexZ;

    separateFracInt(x, fracX, localGridIndexX, fParentChunkSize);
    separateFracInt(y, fracY, localGridIndexY, fParentChunkSize);
    separateFracInt(z, fracZ, localGridIndexZ, fParentChunkSize);

    // cout << localGridIndexX << localGridIndexY << localGridIndexZ <<endl;

    // fracX < 0 ? 0.000001 : fracX;
    // fracY < 0 ? 0.000001 : fracY;
    // fracZ < 0 ? 0.000001 : fracZ;
    // fracX > 1 ? 0.999999 : fracX;
    // fracY < 1 ? 0.999999 : fracY;
    // fracZ < 1 ? 0.999999 : fracZ;

    //    if (chunkIndexDivX == 1 && chunkIndexDivY == 1  && chunkIndexDivZ == 1){
    //        cout << "in box 2 : " << counter << endl;
    //        counter++;
    //    }
  //      assert (fracX <= 1.0 && fracX >= 0);
  //      assert (fracY <= 1.0 && fracY >= 0);
  //      assert (fracZ <= 1.0 && fracZ >= 0);

    Chunk *sampleChunk = GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ);
    //    if (sampleChunk == dummyChunk)
    //        return defaultValue;
    // cout << "got dummy first" << endl;

    float tmp1{0.0f}, tmp2{0.0f}, tmp3{0.0f}, tmp4{0.0f}, tmp5{0.0f}, tmp6{0.0f},
        tmp7{0.0f}, tmp8{0.0f};

    // cout << "sampling " << chunkIndexDivX << " index " << localGridIndexX <<
    // endl;

    // cout << sampleChunk->chunkData[0] <<endl;
    // cout <<
    // flatten3dCoordinatesto1D(localGridIndexX,localGridIndexY,localGridIndexZ,
    // currentChannelToSample,parentChunkSize ) << endl;
    //    double tmp1 = (*this)(i,j,k);

    //       double tmp12 = LERP(tmp1, tmp2, fracty);
    //       double tmp34 = LERP(tmp3, tmp4, fracty);

    //       double tmp56 = LERP(tmp5, tmp6, fracty);
    //       double tmp78 = LERP(tmp7, tmp8, fracty);

    //       double tmp1234 = LERP (tmp12, tmp34, fractx);
    //       double tmp5678 = LERP (tmp56, tmp78, fractx);

    //       double tmp = LERP(tmp1234, tmp5678, fractz);


      localGridIndexX &= parentChunkSizeMinus1;

      localGridIndexY &= parentChunkSizeMinus1;

      localGridIndexZ &= parentChunkSizeMinus1;

  //    localGridIndexX = glm::min(localGridIndexX, parentChunkSizeMinus1);

  //    localGridIndexY = glm::min(localGridIndexY, parentChunkSizeMinus1);

  //    localGridIndexZ = glm::min(localGridIndexZ, parentChunkSizeMinus1);

    tmp1 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY, localGridIndexZ, channel,
        parentChunkSize)];

    if (localGridIndexX < parentChunkSizeMinus1 &&
        localGridIndexY < parentChunkSizeMinus1 &&
        localGridIndexZ < parentChunkSizeMinus1) // all 3
    {

      tmp2 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY + 1, localGridIndexZ, channel,
          parentChunkSize)];

      tmp3 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY, localGridIndexZ, channel,
          parentChunkSize)];
      tmp4 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY + 1, localGridIndexZ, channel,
          parentChunkSize)];

      tmp5 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY, localGridIndexZ + 1, channel,
          parentChunkSize)];
      tmp6 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY + 1, localGridIndexZ + 1, channel,
          parentChunkSize)];

      tmp7 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY, localGridIndexZ + 1, channel,
          parentChunkSize)];
      tmp8 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY + 1, localGridIndexZ + 1, channel,
          parentChunkSize)];
    }

    else if (localGridIndexX == parentChunkSizeMinus1 &&
             localGridIndexY < parentChunkSizeMinus1 &&
             localGridIndexZ < parentChunkSizeMinus1) // only x at edge
    {
      // cout << "edge case" << endl;
      Chunk *sampleChunkXp1 =
          GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ);

      tmp2 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY + 1, localGridIndexZ, channel,
          parentChunkSize)];

      tmp3 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
          localGridIndexZ, channel, parentChunkSize)];
      tmp4 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY + 1,
          localGridIndexZ, channel, parentChunkSize)];

      tmp5 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY, localGridIndexZ + 1, channel,
          parentChunkSize)];
      tmp6 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY + 1, localGridIndexZ + 1, channel,
          parentChunkSize)];

      tmp7 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
          localGridIndexZ + 1, channel, parentChunkSize)];
      tmp8 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY + 1,
          localGridIndexZ + 1, channel, parentChunkSize)];
    }

    else if (localGridIndexX < parentChunkSizeMinus1 &&
             localGridIndexY == parentChunkSizeMinus1 &&
             localGridIndexZ < parentChunkSizeMinus1) // only y at edge
    {
      Chunk *sampleChunkYp1 =
          GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ);

      tmp2 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
          localGridIndexZ, channel, parentChunkSize)];

      tmp3 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY, localGridIndexZ, channel,
          parentChunkSize)];
      tmp4 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, (localGridIndexY + 1) & parentChunkSizeMinus1,
          localGridIndexZ, channel, parentChunkSize)];

      tmp5 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY, localGridIndexZ + 1, channel,
          parentChunkSize)];
      tmp6 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
          localGridIndexZ + 1, channel, parentChunkSize)];

      tmp7 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY, localGridIndexZ + 1, channel,
          parentChunkSize)];
      tmp8 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, (localGridIndexY + 1) & parentChunkSizeMinus1,
          localGridIndexZ + 1, channel, parentChunkSize)];
    }

    else if (localGridIndexX < parentChunkSizeMinus1 &&
             localGridIndexY < parentChunkSizeMinus1 &&
             localGridIndexZ == parentChunkSizeMinus1) // only z at edge
    {
      Chunk *sampleChunkZp1 =
          GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ + 1);
      tmp2 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY + 1, localGridIndexZ, channel,
          parentChunkSize)];

      tmp3 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY, localGridIndexZ, channel,
          parentChunkSize)];
      tmp4 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY + 1, localGridIndexZ, channel,
          parentChunkSize)];

      tmp5 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
      tmp6 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY + 1,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];

      tmp7 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
      tmp8 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY + 1,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    }

    //
    else if (localGridIndexX == parentChunkSizeMinus1 &&
             localGridIndexY == parentChunkSizeMinus1 &&
             localGridIndexZ < parentChunkSizeMinus1) // x&y at edge
    {
      Chunk *sampleChunkXp1 =
          GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ);
      Chunk *sampleChunkYp1 =
          GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ);
      Chunk *sampleChunkXp1Yp1 =
          GetChunk(chunkIndexDivX + 1, chunkIndexDivY + 1, chunkIndexDivZ);

      tmp2 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
          localGridIndexZ, channel, parentChunkSize)];

      tmp3 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
          localGridIndexZ, channel, parentChunkSize)];
      tmp4 = sampleChunkXp1Yp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1,
          (localGridIndexY + 1) & parentChunkSizeMinus1, localGridIndexZ, channel,
          parentChunkSize)];

      tmp5 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY, localGridIndexZ + 1, channel,
          parentChunkSize)];
      tmp6 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
          localGridIndexZ + 1, channel, parentChunkSize)];

      tmp7 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
          localGridIndexZ + 1, channel, parentChunkSize)];
      tmp8 = sampleChunkXp1Yp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1,
          (localGridIndexY + 1) & parentChunkSizeMinus1, localGridIndexZ + 1, channel,
          parentChunkSize)];
    }

    else if ((localGridIndexX < parentChunkSizeMinus1) &&
             (localGridIndexY == parentChunkSizeMinus1) &&
             (localGridIndexZ == parentChunkSizeMinus1)) // y&z at edge
    {

      Chunk *sampleChunkYp1 =
          GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ);
      Chunk *sampleChunkZp1 =
          GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ + 1);
      Chunk *sampleChunkYp1Zp1 =
          GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ + 1);

      tmp2 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
          localGridIndexZ, channel, parentChunkSize)];

      tmp3 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY, localGridIndexZ, channel,
          parentChunkSize)];
      tmp4 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, (localGridIndexY + 1) & parentChunkSizeMinus1,
          localGridIndexZ, channel, parentChunkSize)];

      tmp5 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
      tmp6 = sampleChunkYp1Zp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];

      tmp7 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, localGridIndexY,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
      tmp8 = sampleChunkYp1Zp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX + 1, (localGridIndexY + 1) & parentChunkSizeMinus1,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    }

    else if (localGridIndexX == parentChunkSizeMinus1 &&
             localGridIndexY < parentChunkSizeMinus1 &&
             localGridIndexZ == parentChunkSizeMinus1) // x&z at edge
    {

      Chunk *sampleChunkXp1 =
          GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ);
      Chunk *sampleChunkZp1 =
          GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ + 1);
      Chunk *sampleChunkXp1Zp1 =
          GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ + 1);

      //       double tmp2 = (*this)(i,j+1,k);
      //       double tmp3 = (*this)(i+1,j,k);
      //       double tmp4 = (*this)(i+1,j+1,k);

      //       double tmp5 = (*this)(i,j,k+1);
      //       double tmp6 = (*this)(i,j+1,k+1);
      //       double tmp7 = (*this)(i+1,j,k+1);
      //       double tmp8 = (*this)(i+1,j+1,k+1);111
      tmp2 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY + 1, localGridIndexZ, channel,
          parentChunkSize)];

      tmp3 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
          localGridIndexZ, channel, parentChunkSize)];
      tmp4 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY + 1,
          localGridIndexZ, channel, parentChunkSize)];

      tmp5 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
      tmp6 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY + 1,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];

      tmp7 = sampleChunkXp1Zp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
      tmp8 = sampleChunkXp1Zp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY + 1,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    }

    else if (localGridIndexX == parentChunkSizeMinus1 &&
             localGridIndexY == parentChunkSizeMinus1 &&
             localGridIndexZ == parentChunkSizeMinus1) // x&y&z at edge
    {
      // cout << "err " << endl;

      Chunk *sampleChunkXp1 =
          GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ);
      Chunk *sampleChunkYp1 =
          GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ);
      Chunk *sampleChunkZp1 =
          GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ + 1);

      Chunk *sampleChunkXp1Yp1 =
          GetChunk(chunkIndexDivX + 1, chunkIndexDivY + 1, chunkIndexDivZ);
      Chunk *sampleChunkXp1Zp1 =
          GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ + 1);
      Chunk *sampleChunkYp1Zp1 =
          GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ + 1);

      Chunk *sampleChunkXp1Yp1Zp1 =
          GetChunk(chunkIndexDivX + 1, chunkIndexDivY + 1, chunkIndexDivZ + 1);

      // if (sampleChunkXp1Yp1Zp1 == dummyChunk)
      // cout << "same as dummy chunk" << endl;

      tmp2 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
          localGridIndexZ, channel, parentChunkSize)];

      tmp3 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
          localGridIndexZ, channel, parentChunkSize)];
      tmp4 = sampleChunkXp1Yp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1,
          (localGridIndexY + 1) & parentChunkSizeMinus1, localGridIndexZ, channel,
          parentChunkSize)];

      tmp5 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, localGridIndexY,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
      tmp6 = sampleChunkYp1Zp1->chunkData[flatten3dCoordinatesto1D(
          localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];

      tmp7 = sampleChunkXp1Zp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
      tmp8 = sampleChunkXp1Yp1Zp1->chunkData[flatten3dCoordinatesto1D(
          (localGridIndexX + 1) & parentChunkSizeMinus1,
          (localGridIndexY + 1) & parentChunkSizeMinus1,
          (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    }

    else {
      cout << "something went wrong, voxel doesn't exist. chunk " <<  chunkIndexDivX << " " << chunkIndexDivY << " " << chunkIndexDivZ << " " << localGridIndexX << " " << localGridIndexY << " " << localGridIndexZ <<  "    : " << x << " " << y << " " << z << endl;
      return -1;
    }

    float tmp12 = glm::mix(tmp1, tmp2, fracY);
    float tmp34 = glm::mix(tmp3, tmp4, fracY);
    float tmp56 = glm::mix(tmp5, tmp6, fracY);
    float tmp78 = glm::mix(tmp7, tmp8, fracY);
    // fracYfracX
    float tmp1234 = glm::mix(tmp12, tmp34, fracX);
    float tmp5678 = glm::mix(tmp56, tmp78, fracX);

    float tmp = glm::mix(tmp1234, tmp5678, fracZ);

    // cout <<tmp<<endl;
    // cout << "sampling " << chunkIndexDivX << " index " << localGridIndexX <<
    // "result " << tmp << endl;

    return tmp;

}

//----------------------------------------------
float ChannelObject::SampleTrilinear(float x, float y, float z,
                                     uint32_t channel) {

  // cout << "sampling "
  // int threadid  = omp_get_thread_num();
  // cout << threadid <<endl;
  // work out what chunk to sample. At first work this out for every voxel.
  // Later can do checking in the algorithm to see if it exists in chunk already
  // (ray box intersection).

  //    inline static void separateFracInt(const float& in, float& floatPart,
  //    uint32_t& intPart, const float& chunkSize)
  //    {
  //        float temp = glm::mod(in, chunkSize);

  //        float tempFloatyInt;
  //        floatPart = std::modf(temp, &tempFloatyInt);
  //        intPart = (uint32_t)tempFloatyInt;
  // 23.5
  //    }23.5/8.0 = 2.9375 => floor = 2 so chunk 2
  // setfracint
  // 23.5 mod 8 = 7.5
  // floatpart = 0.5 and 7.0
  // int part = 7

  // 15 %4 = 3, chunk 3
  // 0.0 and 3.0, index 3





  int32_t chunkIndexDivX = glm::floor(x * getRidOfDivCalc);
  int32_t chunkIndexDivY = glm::floor(y * getRidOfDivCalc);
  int32_t chunkIndexDivZ = glm::floor(z * getRidOfDivCalc);

  //    int32_t chunkIndexDivX = fastFloor(x*getRidOfDivCalc);
  //    int32_t chunkIndexDivY = fastFloor(y*getRidOfDivCalc);
  //    int32_t chunkIndexDivZ = fastFloor(z*getRidOfDivCalc);

  // cout << "sampling chunk " << chunkIndexDivX << ", "<< chunkIndexDivY << ",
  // " << chunkIndexDivZ << endl;
  float fracX;
  float fracY;
  float fracZ;



  uint32_t localGridIndexX;
  uint32_t localGridIndexY;
  uint32_t localGridIndexZ;

  separateFracInt(x, fracX, localGridIndexX, fParentChunkSize);
  separateFracInt(y, fracY, localGridIndexY, fParentChunkSize);
  separateFracInt(z, fracZ, localGridIndexZ, fParentChunkSize);

  // cout << localGridIndexX << localGridIndexY << localGridIndexZ <<endl;

  // fracX < 0 ? 0.000001 : fracX;
  // fracY < 0 ? 0.000001 : fracY;
  // fracZ < 0 ? 0.000001 : fracZ;
  // fracX > 1 ? 0.999999 : fracX;
  // fracY < 1 ? 0.999999 : fracY;
  // fracZ < 1 ? 0.999999 : fracZ;

  //    if (chunkIndexDivX == 1 && chunkIndexDivY == 1  && chunkIndexDivZ == 1){
  //        cout << "in box 2 : " << counter << endl;
  //        counter++;
  //    }
//      assert (fracX <= 1.0 && fracX >= 0);
//      assert (fracY <= 1.0 && fracY >= 0);
//      assert (fracZ <= 1.0 && fracZ >= 0);

  Chunk *sampleChunk = GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ);
  //    if (sampleChunk == dummyChunk)
  //        return defaultValue;
  // cout << "got dummy first" << endl;

  float tmp1{0.0f}, tmp2{0.0f}, tmp3{0.0f}, tmp4{0.0f}, tmp5{0.0f}, tmp6{0.0f},
      tmp7{0.0f}, tmp8{0.0f};

  // cout << "sampling " << chunkIndexDivX << " index " << localGridIndexX <<
  // endl;

  // cout << sampleChunk->chunkData[0] <<endl;
  // cout <<
  // flatten3dCoordinatesto1D(localGridIndexX,localGridIndexY,localGridIndexZ,
  // currentChannelToSample,parentChunkSize ) << endl;
  //    double tmp1 = (*this)(i,j,k);

  //       double tmp12 = LERP(tmp1, tmp2, fracty);
  //       double tmp34 = LERP(tmp3, tmp4, fracty);

  //       double tmp56 = LERP(tmp5, tmp6, fracty);
  //       double tmp78 = LERP(tmp7, tmp8, fracty);

  //       double tmp1234 = LERP (tmp12, tmp34, fractx);
  //       double tmp5678 = LERP (tmp56, tmp78, fractx);

  //       double tmp = LERP(tmp1234, tmp5678, fractz);


    localGridIndexX &= parentChunkSizeMinus1;

    localGridIndexY &= parentChunkSizeMinus1;

    localGridIndexZ &= parentChunkSizeMinus1;

//    localGridIndexX = glm::min(localGridIndexX, parentChunkSizeMinus1);

//    localGridIndexY = glm::min(localGridIndexY, parentChunkSizeMinus1);

//    localGridIndexZ = glm::min(localGridIndexZ, parentChunkSizeMinus1);

  tmp1 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
      localGridIndexX, localGridIndexY, localGridIndexZ, channel,
      parentChunkSize)];

  if (localGridIndexX < parentChunkSizeMinus1 &&
      localGridIndexY < parentChunkSizeMinus1 &&
      localGridIndexZ < parentChunkSizeMinus1) // all 3
  {

    tmp2 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY + 1, localGridIndexZ, channel,
        parentChunkSize)];

    tmp3 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY, localGridIndexZ, channel,
        parentChunkSize)];
    tmp4 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY + 1, localGridIndexZ, channel,
        parentChunkSize)];

    tmp5 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY, localGridIndexZ + 1, channel,
        parentChunkSize)];
    tmp6 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY + 1, localGridIndexZ + 1, channel,
        parentChunkSize)];

    tmp7 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY, localGridIndexZ + 1, channel,
        parentChunkSize)];
    tmp8 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY + 1, localGridIndexZ + 1, channel,
        parentChunkSize)];
  }

  else if (localGridIndexX == parentChunkSizeMinus1 &&
           localGridIndexY < parentChunkSizeMinus1 &&
           localGridIndexZ < parentChunkSizeMinus1) // only x at edge
  {
    // cout << "edge case" << endl;
    Chunk *sampleChunkXp1 =
        GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ);

    tmp2 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY + 1, localGridIndexZ, channel,
        parentChunkSize)];

    tmp3 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
        localGridIndexZ, channel, parentChunkSize)];
    tmp4 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY + 1,
        localGridIndexZ, channel, parentChunkSize)];

    tmp5 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY, localGridIndexZ + 1, channel,
        parentChunkSize)];
    tmp6 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY + 1, localGridIndexZ + 1, channel,
        parentChunkSize)];

    tmp7 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
        localGridIndexZ + 1, channel, parentChunkSize)];
    tmp8 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY + 1,
        localGridIndexZ + 1, channel, parentChunkSize)];
  }

  else if (localGridIndexX < parentChunkSizeMinus1 &&
           localGridIndexY == parentChunkSizeMinus1 &&
           localGridIndexZ < parentChunkSizeMinus1) // only y at edge
  {
    Chunk *sampleChunkYp1 =
        GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ);

    tmp2 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
        localGridIndexZ, channel, parentChunkSize)];

    tmp3 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY, localGridIndexZ, channel,
        parentChunkSize)];
    tmp4 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, (localGridIndexY + 1) & parentChunkSizeMinus1,
        localGridIndexZ, channel, parentChunkSize)];

    tmp5 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY, localGridIndexZ + 1, channel,
        parentChunkSize)];
    tmp6 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
        localGridIndexZ + 1, channel, parentChunkSize)];

    tmp7 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY, localGridIndexZ + 1, channel,
        parentChunkSize)];
    tmp8 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, (localGridIndexY + 1) & parentChunkSizeMinus1,
        localGridIndexZ + 1, channel, parentChunkSize)];
  }

  else if (localGridIndexX < parentChunkSizeMinus1 &&
           localGridIndexY < parentChunkSizeMinus1 &&
           localGridIndexZ == parentChunkSizeMinus1) // only z at edge
  {
    Chunk *sampleChunkZp1 =
        GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ + 1);
    tmp2 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY + 1, localGridIndexZ, channel,
        parentChunkSize)];

    tmp3 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY, localGridIndexZ, channel,
        parentChunkSize)];
    tmp4 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY + 1, localGridIndexZ, channel,
        parentChunkSize)];

    tmp5 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    tmp6 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY + 1,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];

    tmp7 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    tmp8 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY + 1,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
  }

  //
  else if (localGridIndexX == parentChunkSizeMinus1 &&
           localGridIndexY == parentChunkSizeMinus1 &&
           localGridIndexZ < parentChunkSizeMinus1) // x&y at edge
  {
    Chunk *sampleChunkXp1 =
        GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ);
    Chunk *sampleChunkYp1 =
        GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ);
    Chunk *sampleChunkXp1Yp1 =
        GetChunk(chunkIndexDivX + 1, chunkIndexDivY + 1, chunkIndexDivZ);

    tmp2 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
        localGridIndexZ, channel, parentChunkSize)];

    tmp3 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
        localGridIndexZ, channel, parentChunkSize)];
    tmp4 = sampleChunkXp1Yp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1,
        (localGridIndexY + 1) & parentChunkSizeMinus1, localGridIndexZ, channel,
        parentChunkSize)];

    tmp5 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY, localGridIndexZ + 1, channel,
        parentChunkSize)];
    tmp6 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
        localGridIndexZ + 1, channel, parentChunkSize)];

    tmp7 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
        localGridIndexZ + 1, channel, parentChunkSize)];
    tmp8 = sampleChunkXp1Yp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1,
        (localGridIndexY + 1) & parentChunkSizeMinus1, localGridIndexZ + 1, channel,
        parentChunkSize)];
  }

  else if ((localGridIndexX < parentChunkSizeMinus1) &&
           (localGridIndexY == parentChunkSizeMinus1) &&
           (localGridIndexZ == parentChunkSizeMinus1)) // y&z at edge
  {

    Chunk *sampleChunkYp1 =
        GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ);
    Chunk *sampleChunkZp1 =
        GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ + 1);
    Chunk *sampleChunkYp1Zp1 =
        GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ + 1);

    tmp2 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
        localGridIndexZ, channel, parentChunkSize)];

    tmp3 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY, localGridIndexZ, channel,
        parentChunkSize)];
    tmp4 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, (localGridIndexY + 1) & parentChunkSizeMinus1,
        localGridIndexZ, channel, parentChunkSize)];

    tmp5 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    tmp6 = sampleChunkYp1Zp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];

    tmp7 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, localGridIndexY,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    tmp8 = sampleChunkYp1Zp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX + 1, (localGridIndexY + 1) & parentChunkSizeMinus1,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
  }

  else if (localGridIndexX == parentChunkSizeMinus1 &&
           localGridIndexY < parentChunkSizeMinus1 &&
           localGridIndexZ == parentChunkSizeMinus1) // x&z at edge
  {

    Chunk *sampleChunkXp1 =
        GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ);
    Chunk *sampleChunkZp1 =
        GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ + 1);
    Chunk *sampleChunkXp1Zp1 =
        GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ + 1);

    //       double tmp2 = (*this)(i,j+1,k);
    //       double tmp3 = (*this)(i+1,j,k);
    //       double tmp4 = (*this)(i+1,j+1,k);

    //       double tmp5 = (*this)(i,j,k+1);
    //       double tmp6 = (*this)(i,j+1,k+1);
    //       double tmp7 = (*this)(i+1,j,k+1);
    //       double tmp8 = (*this)(i+1,j+1,k+1);111
    tmp2 = sampleChunk->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY + 1, localGridIndexZ, channel,
        parentChunkSize)];

    tmp3 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
        localGridIndexZ, channel, parentChunkSize)];
    tmp4 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY + 1,
        localGridIndexZ, channel, parentChunkSize)];

    tmp5 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    tmp6 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY + 1,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];

    tmp7 = sampleChunkXp1Zp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    tmp8 = sampleChunkXp1Zp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY + 1,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
  }

  else if (localGridIndexX == parentChunkSizeMinus1 &&
           localGridIndexY == parentChunkSizeMinus1 &&
           localGridIndexZ == parentChunkSizeMinus1) // x&y&z at edge
  {
    // cout << "err " << endl;

    Chunk *sampleChunkXp1 =
        GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ);
    Chunk *sampleChunkYp1 =
        GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ);
    Chunk *sampleChunkZp1 =
        GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ + 1);

    Chunk *sampleChunkXp1Yp1 =
        GetChunk(chunkIndexDivX + 1, chunkIndexDivY + 1, chunkIndexDivZ);
    Chunk *sampleChunkXp1Zp1 =
        GetChunk(chunkIndexDivX + 1, chunkIndexDivY, chunkIndexDivZ + 1);
    Chunk *sampleChunkYp1Zp1 =
        GetChunk(chunkIndexDivX, chunkIndexDivY + 1, chunkIndexDivZ + 1);

    Chunk *sampleChunkXp1Yp1Zp1 =
        GetChunk(chunkIndexDivX + 1, chunkIndexDivY + 1, chunkIndexDivZ + 1);

    // if (sampleChunkXp1Yp1Zp1 == dummyChunk)
    // cout << "same as dummy chunk" << endl;

    tmp2 = sampleChunkYp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
        localGridIndexZ, channel, parentChunkSize)];

    tmp3 = sampleChunkXp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
        localGridIndexZ, channel, parentChunkSize)];
    tmp4 = sampleChunkXp1Yp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1,
        (localGridIndexY + 1) & parentChunkSizeMinus1, localGridIndexZ, channel,
        parentChunkSize)];

    tmp5 = sampleChunkZp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, localGridIndexY,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    tmp6 = sampleChunkYp1Zp1->chunkData[flatten3dCoordinatesto1D(
        localGridIndexX, (localGridIndexY + 1) & parentChunkSizeMinus1,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];

    tmp7 = sampleChunkXp1Zp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1, localGridIndexY,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
    tmp8 = sampleChunkXp1Yp1Zp1->chunkData[flatten3dCoordinatesto1D(
        (localGridIndexX + 1) & parentChunkSizeMinus1,
        (localGridIndexY + 1) & parentChunkSizeMinus1,
        (localGridIndexZ + 1) & parentChunkSizeMinus1, channel, parentChunkSize)];
  }

  else {
    cout << "something went wrong, voxel doesn't exist. chunk " <<  chunkIndexDivX << " " << chunkIndexDivY << " " << chunkIndexDivZ << " " << localGridIndexX << " " << localGridIndexY << " " << localGridIndexZ <<  "    : " << x << " " << y << " " << z << endl;
    return -1;
  }

  float tmp12 = glm::mix(tmp1, tmp2, fracY);
  float tmp34 = glm::mix(tmp3, tmp4, fracY);
  float tmp56 = glm::mix(tmp5, tmp6, fracY);
  float tmp78 = glm::mix(tmp7, tmp8, fracY);
  // fracYfracX
  float tmp1234 = glm::mix(tmp12, tmp34, fracX);
  float tmp5678 = glm::mix(tmp56, tmp78, fracX);

  float tmp = glm::mix(tmp1234, tmp5678, fracZ);

  // cout <<tmp<<endl;
  // cout << "sampling " << chunkIndexDivX << " index " << localGridIndexX <<
  // "result " << tmp << endl;

  return tmp;
}

float ChannelObject::SampleExplicit(float x, float y, float z,
                                    uint32_t channel) {

  // int32_t parentChunkSizeMinus1 = parentChunkSize - 1; // 7



  //    int32_t chunkIndexDivX = glm::floor((x)/(parentChunkSize));
  //    int32_t chunkIndexDivY = glm::floor((y)/(parentChunkSize));
  //    int32_t chunkIndexDivZ = glm::floor((z)/(parentChunkSize));

  int32_t chunkIndexDivX = glm::floor((x)*getRidOfDivCalc);
  int32_t chunkIndexDivY = glm::floor((y)*getRidOfDivCalc);
  int32_t chunkIndexDivZ = glm::floor((z)*getRidOfDivCalc);


  // 15.5 / 7 = 2.214

  // int32_t chunkIndexDivX = ((int)x / (parentChunkSize));
  // int32_t chunkIndexDivY = ((int)y / (parentChunkSize));
  // int32_t chunkIndexDivZ = ((int)z / (parentChunkSize));



  uint32_t localGridIndexX;
  uint32_t localGridIndexY;
  uint32_t localGridIndexZ;

//  separateFracInt(x, fracX, localGridIndexX, fParentChunkSize);
//  separateFracInt(y, fracY, localGridIndexY, fParentChunkSize);
//  separateFracInt(z, fracZ, localGridIndexZ, fParentChunkSize);

  separateFracIntFast(x, localGridIndexX, parentChunkSizeMinus1);
  separateFracIntFast(y, localGridIndexY, parentChunkSizeMinus1);
  separateFracIntFast(z, localGridIndexZ, parentChunkSizeMinus1);

//  assert(fracX < 1.0 && fracX >= 0);
//  assert(fracY < 1.0 && fracY >= 0);
//  assert(fracZ < 1.0 && fracZ >= 0);

  Chunk *sampleChunk = GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ);
//  float tmp = sampleChunk->chunkData[flatten3dCoordinatesto1D(
//      (localGridIndexX) & parentChunkSizeMinus1, (localGridIndexY) & parentChunkSizeMinus1,
//      (localGridIndexZ) & parentChunkSizeMinus1, channel, parentChunkSize)];


  float tmp = sampleChunk->chunkData[flatten3dCoordinatesto1D(
      (localGridIndexX) , (localGridIndexY),
      (localGridIndexZ), channel, parentChunkSize)];

  //    if ( (x > 15) && (y > 15) && (z > 15)){
  //        cout << "address of chunk is " << sampleChunk << endl;

  //        cout << "chunk index is " << chunkIndexDivX << ", " <<
  //        chunkIndexDivY << ", " << chunkIndexDivZ << ", " << tmp << endl;
  //    }

  return tmp;
}

float ChannelObject::SampleExplicitAlt(float x, float y, float z) {
  int32_t parentChunkSizeMinus1 = parentChunkSize - 1; // 7

  float getRidOfDivCalc = 1.0 / parentChunkSize;

  //    int32_t chunkIndexDivX = glm::floor((x)/(parentChunkSize));
  //    int32_t chunkIndexDivY = glm::floor((y)/(parentChunkSize));
  //    int32_t chunkIndexDivZ = glm::floor((z)/(parentChunkSize));

  int32_t chunkIndexDivX = glm::floor((x)*getRidOfDivCalc);
  int32_t chunkIndexDivY = glm::floor((y)*getRidOfDivCalc);
  int32_t chunkIndexDivZ = glm::floor((z)*getRidOfDivCalc);

  // int32_t chunkIndexDivX = ((int)x / (parentChunkSize));
  // int32_t chunkIndexDivY = ((int)y / (parentChunkSize));
  // int32_t chunkIndexDivZ = ((int)z / (parentChunkSize));

  float fracX;
  float fracY;
  float fracZ;

  uint32_t localGridIndexX;
  uint32_t localGridIndexY;
  uint32_t localGridIndexZ;

  separateFracInt(x, fracX, localGridIndexX, fParentChunkSize);
  separateFracInt(y, fracY, localGridIndexY, fParentChunkSize);
  separateFracInt(z, fracZ, localGridIndexZ, fParentChunkSize);

  assert(fracX < 1.0 && fracX >= 0);
  assert(fracY < 1.0 && fracY >= 0);
  assert(fracZ < 1.0 && fracZ >= 0);

  Chunk *sampleChunk = GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ);
  if (sampleChunk == dummyChunk) {

    return 0;
  } else {
    return 0.5;
  }
}

bool ChannelObject::SampleIsOutsideBounds(float x, float y, float z, uint32_t channel)
{
    float getRidOfDivCalc = 1.0f / parentChunkSize;

    int32_t chunkIndexDivX = glm::floor((x)*getRidOfDivCalc);
    int32_t chunkIndexDivY = glm::floor((y)*getRidOfDivCalc);
    int32_t chunkIndexDivZ = glm::floor((z)*getRidOfDivCalc);

    if(GetChunk(chunkIndexDivX, chunkIndexDivY, chunkIndexDivZ) == dummyChunk)
        return true;
    else
        return false;
}

//----------------------------------------------
void ChannelObject::DeleteChunk(int32_t x, int32_t y, int32_t z) {
  uint32_t chunkKey = integerEncodeTo32BitInt(x, y, z);

  Chunk *chunk_to_delete_ = (Chunk *)chunks->GetItem(chunkKey);
  if (chunk_to_delete_ != dummyChunk) {

    // cout << "chunk destroyed" << endl;
    chunks->ClearKey(chunkKey);
    delete chunk_to_delete_;
    // cout << "key cleared" << endl;
  }

  if (controlChannel) {
    for (int i = 2; i < parentGridObject->channelObjs.size(); i += 2) {

      Chunk *c =
          (Chunk *)parentGridObject->channelObjs[i]->chunks->GetItem(chunkKey);
      if (parentGridObject->channelObjs[i]->chunks->KeyExists(chunkKey)) {
        delete c;
        parentGridObject->channelObjs[i]->chunks->ClearKey(chunkKey);
      }
    }
  }
}
