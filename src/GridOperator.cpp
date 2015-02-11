#include "GridOperator.h"
#include "GridObject.h"
#include "ChannelObject.h"
#include "glm/vec3.hpp"
#include "glm/gtc/type_precision.hpp"

#include <omp.h>

inline static uint32_t flatten3dCoordinatesto1D(uint32_t x, uint32_t y,
                                                uint32_t z, uint32_t channel,
                                                uint32_t chunkSize) {

  return ((x+channel) + ((y+channel)*chunkSize) + ((z+channel)*chunkSize*chunkSize));
  //return ((x + channel) + ((y + channel) << 3) + ((z + channel) << 3 << 3));
}



//----------------------------------------------
GridOperator::GridOperator(GridObject *inGridObject) {
  gridObjectPtr = inGridObject;
  currentSourceChannelObject =
      gridObjectPtr->channelObjs[0].get(); // default first one
  currentTargetChannelObject =
      gridObjectPtr->channelObjs[0].get(); // default channel to write into is
                                           // same (used to be dup but changed)

  refreshSourceAndTargetChannelDetails();
  createChunks = false;
  this->boundingBox.fluidMin =
      glm::i32vec3(0.0); // set bounds to 0 so we dont emit anything
  this->boundingBox.fluidMax = glm::i32vec3(0.0);
  chnkSize = (int)gridObjectPtr->chunkSize;
  // cout << "chnkSize is " << chnkSize << endl;
  // cout <<  channelName << endl;
}

//----------------------------------------------
GridOperator::~GridOperator() {}

void GridOperator::refreshSourceAndTargetChannelDetails() {
  typeToOperateOn = currentTargetChannelObject->channelInfo.channelType;
  channelName = currentTargetChannelObject->channelInfo.channelName;
  // cout << "channel name" << channelName << endl;
}
//----------------------------------------------
void GridOperator::SetGridObject(GridObject *inGridObject) {
  gridObjectPtr = inGridObject;
}

//----------------------------------------------
void GridOperator::IterateGrid() {
  double timeA = omp_get_wtime();
   double timeC, timeD;
//ITERATE-----------------------------------------------------------------------------


    chunks.clear();
  chunkOpCounter = 0;
this->PreGridOp();

  currentTime = gridObjectPtr->simTime;

  // too much.... but keep here in case
  // uint32_t internalChannels =
  // static_cast<uint32_t>(currentSourceChannelObject->channelInfo.channelType);//1
  // for scalar and 3 for vector. will have to deal with sdf another way

  internalChannels = 1; // for making scalar or vector
  if (currentTargetChannelObject->channelInfo.channelType ==
      EChannelType::vector) {
    internalChannels = 3;
    // cout << "channels : " << internalChannels << endl;
  }

  // cout << "iterating grid" << endl;
  int fmz = gridObjectPtr->boundingBox.fluidMin.z;
  int fmy = gridObjectPtr->boundingBox.fluidMin.y;
  int fmx = gridObjectPtr->boundingBox.fluidMin.x;

  int fMax = gridObjectPtr->boundingBox.fluidMax.x;
  int fMay = gridObjectPtr->boundingBox.fluidMax.y;
  int fMaz = gridObjectPtr->boundingBox.fluidMax.z;

//   cout << "<" << fmx << ", " << fmy << ", " << fmz << ">  to  <" << fMax <<
//   ", " << fMay << ", " << fMaz << "> " << endl;



// cout << "source channel is " << name << " " <<
// currentSourceChannelObject->channelInfo.channelName << endl;
double timeE = omp_get_wtime();
//SORTING BOUNDS------------------------------------------------------------------------------
#pragma omp parallel for collapse(3)
  for (int i = fmx; i <= fMax; i++) {
    for (int j = fmy; j <= fMay; j++) {
      for (int k = fmz; k <= fMaz; k++) {
        // int threadid  = omp_get_thread_num();
        // cout << threadid << endl;
        // cout <<" jhjhdjs" <<endl;
        // myString << "iterating in operator " << name <<  " chunk "  <<
        // i<<j<<k<<endl;
        chunkAddresses newChunkAddresses;
        newChunkAddresses.chunkSource =
            currentSourceChannelObject->GetChunk(i, j, k);
        newChunkAddresses.chunkTarget = currentTargetChannelObject->GetChunk(
            i, j, k); // if we are writing into the same grid/channel, then set
                      // these the same
        newChunkAddresses.chunkIndex = glm::i32vec3(i, j, k);

        // cout << "adding existing" << endl;
        if (newChunkAddresses.chunkSource !=
            currentSourceChannelObject->dummyChunk) // if there is a chunk there
                                                    // then go ahead and store
                                                    // it
        {
#pragma omp critical
          { chunks.emplace_back(newChunkAddresses); }
        }
        //                else{
        //                    //myString << "dummyChunk " <<  name <<
        //                    i<<j<<k<<endl;
        //                }
      }
    }
  }
//END OF SORTING BOUNDS------------------------------------------------------------------------------


//FORCE SORTING BOUNDS------------------------------------------------------------------------------
  if (forceInputBoundsIteration) {

#pragma omp parallel for collapse(3)
    for (int i = this->boundingBox.fluidMin.z;
         i <= this->boundingBox.fluidMax.z; i++) {
      for (int j = this->boundingBox.fluidMin.y;
           j <= this->boundingBox.fluidMax.y; j++) {
        for (int k = this->boundingBox.fluidMin.x;
             k <= this->boundingBox.fluidMax.x; k++) {

          chunkAddresses newChunkAddresses;
          newChunkAddresses.chunkSource = nullptr;
          newChunkAddresses.chunkTarget = nullptr;
          newChunkAddresses.chunkIndex = glm::i32vec3(i, j, k);

// myString << "iterating in operator implcit bounds section" << name <<  "
// chunk "  << i<<j<<k<<endl;

// myString << "bounds loop " <<  i<< "," << j << "," << k << "   ";

#pragma omp critical
          {
            // myString << "pushing back " << endl;
            chunks.emplace_back(newChunkAddresses);
          }
        }
      }
    }
  }

double timeF = omp_get_wtime();

//END OF FORCE SORTING BOUNDS------------------------------------------------------------------------------

  // loop through this operator bounding box and push chunks to vector (even
  // though they dont exists. we will check in algorithm if they are null


// cout << myString.str() << endl;
// myString.str("");
// double timeA = omp_get_wtime();
//#pragma omp parallel for
// dx =
// currentSourceChannelObject->parentDx/currentSourceChannelObject->parentChunkSize;
#pragma omp barrier

for (iteration = 0; iteration < numberOfIterations; iteration++){
    if (iteration == 0){
    totalChunksToOperateOn = chunks.size();
    cout << "chunks to iterate through: " << totalChunksToOperateOn << " with "
         << totalChunksToOperateOn *chnkSize *chnkSize *chnkSize << " voxels"
         << endl;
      }
this->PreGridOp();

#pragma omp parallel for
  for (int i = 0; i < chunks.size(); i++) {
    if (callPreChunkOp) {

      {
        this->PreChunkOp(chunks[i].chunkSource, chunks[i].chunkTarget,
                         chunks[i].chunkIndex);
      }
    }
    // cout << "iterating through chunk vector " << i << endl;
      timeC = omp_get_wtime();
    //#pragma omp parallel for collapse(3)
    for (int w = startVoxel; w < chnkSize; w += skipAmount) {//for skipping voxels in thr red black gauss seidel update
      for (int v = startVoxel; v < chnkSize; v += skipAmount) {//can do 1-startvoxel to ping pong between 1 & 0
        for (int u = startVoxel; u < chnkSize; u += skipAmount) {//and then do skipAmount = startVoxel+1 in each postgridop

          for (int a = 0; a < internalChannels; a++) {

            // int threadid  = omp_get_thread_num();
            // cout << threadid << endl;
            // myString << "in cell " << u+a<<v+a<<w+a<<endl;
            uint32_t chunkDataIndex =
                ((u) + (((v)*chnkSize)) + (((w)*chnkSize * chnkSize))) +
                (a * chnkSize * chnkSize * chnkSize);
            // uint32_t chunkDataIndex = (       (u + chnkSize * (v + chnkSize *
            // w))    *a) + (a*chnkSize*chnkSize*chnkSize);
            // cout << "index " << chunkDataIndex << endl;

            int X = ((chunks[i].chunkIndex.x * (int)chnkSize) + u);

            int Y = ((chunks[i].chunkIndex.y * (int)chnkSize) + v);

            int Z = ((chunks[i].chunkIndex.z * (int)chnkSize) + w);
            bool accessable = false;


//            if (a == 0){

//            if (u > 0 && u < chnkSize-1){
//                if (v > 0 && v < chnkSize-1){
//                    if (w > 0 && w < chnkSize-1){
//                        accessable = true;}}}}

            this->Algorithm(
                chunks[i].chunkIndex,
                glm::i32vec3(X, Y, Z), // voxel position in local chunk space
                chunks[i].chunkSource, chunks[i].chunkTarget, chunkDataIndex,
                a, accessable);
            // myString.clear()
          }
        }
      }
    }
    timeD = omp_get_wtime();
// cout << "sending through chunk " << chunks[i].chunkIndex.x << " " <<
// chunks[i].chunkIndex.y << " " << chunks[i].chunkIndex.z << endl;
#pragma omp critical
    if (callPostChunkOp) {

      {
        this->PostChunkOp(chunks[i].chunkSource, chunks[i].chunkTarget,
                          chunks[i].chunkIndex);
      }
    }
  }
  if(numberOfIterations>1){
  cout << "end of " << name << " iteration: " << iteration+1 << " " << startVoxel << endl;
}

//#pragma omp barrier


}
if (callGridOp) {
  this->GridOp();
}

//END OF ITERATE-----------------------------------------------------------------------------


  double timeB = omp_get_wtime();
  cout << "grid operator " << name << " took " << timeB - timeA << " seconds"
       << endl;
  cout << "grid operator " << name << " inside loop took " << timeD - timeC << " seconds"
       << endl;
  cout << "grid operator " << name << " sorting took " << timeF - timeE << " seconds"
       << endl;

  // cout << myString.str() << endl;
  //    cout << currentSourceChannelObject->numChunks << " chunks created " <<
  //    endl;
  //    currentSourceChannelObject->printChunks();
  // myString.str("");
}

void GridOperator::PostChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                               glm::i32vec3 chunkIdSecondary) {
  // chunkOpCounter++;
}

void GridOperator::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                              glm::i32vec3 chunkIdSecondary) {}

void GridOperator::PreGridOp() {
  currentSourceChannelObject =
      gridObjectPtr->channelObjs[0].get(); // default first one
  currentTargetChannelObject =
      gridObjectPtr->channelObjs[0].get(); // default dup channel to write into
  //    cout << currentSourceChannelObject << endl;
  //    cout << currentTargetChannelObject << endl;
}


void GridOperator::GridOp() {}

// void GridOperator::BucketOp(glm::i32vec3 chunkId)
//{
//    //cout << "in base bucket op " <<endl;
//}

//----------------------------------------------
void GridOperator::SetChannelName(std::string nameToSet) {
  channelName = nameToSet;
  uint32_t index = gridObjectPtr->GetMemoryIndexForChannelName(channelName);
  currentSourceChannelObject =
      gridObjectPtr->channelObjs[index].get(); // default first one
  currentTargetChannelObject = gridObjectPtr->channelObjs[index]
                                   .get(); // default dup channel to write into
  // cout << "set channel to " << nameToSet << " " << index << endl;
  // cout << "new source channel is " <<
  // currentSourceChannelObject->channelInfo.channelName << endl;
  refreshSourceAndTargetChannelDetails();
}

void GridOperator::setNodeName(std::string nameIn) { name = nameIn; }
