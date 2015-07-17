#include "GridTiledOperator.h"
#include "GridObject.h"
#include "ChannelObject.h"
#include "glm/vec3.hpp"
#include "glm/gtc/type_precision.hpp"

#include <omp.h>
//#include <algorithm>
#include <cstring>

//#pragma omp declare simd
inline static uint32_t flatten3dCoordinatesto1D(uint32_t x, uint32_t y,
                                            uint32_t z, uint32_t channel,
                                            uint32_t chunkSize)
{

    return ((x+channel) + ((y+channel)*chunkSize) + ((z+channel)*chunkSize*chunkSize));
    //return ((x + channel) + ((y + channel) << 3) + ((z + channel) << 3 << 3));
}

inline static uint32_t flatten3dPaddedCoordinatesto1D(uint32_t x, uint32_t y,
                                                uint32_t z,
                                                uint32_t chunkSize) {
  // return   (        (x + chunkSize * (y + chunkSize * z))  *channel) +
  // (channel*chunkSize*chunkSize*chunkSize);
  return ( (x+1) +
           ( (y+1)*(chunkSize+2) ) +
           ( (z+1)*(chunkSize+2) * (chunkSize+2))
           );
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
GridTiledOperator::GridTiledOperator(GridObject *inGridObject, ETileImportType processType)
{
    gridObjectPtr = inGridObject;
    currentSourceChannelObject = gridObjectPtr->channelObjs[0].get(); // default first one
    currentTargetChannelObject = gridObjectPtr->channelObjs[0].get(); // default channel to write into is
                                           // same (used to be dup but changed)

    refreshSourceAndTargetChannelDetails();
    createChunks = false;
    this->boundingBox.setfmin(0.0f, 0.0f, 0.0f);  // set bounds to 0 so we dont emit anything
    this->boundingBox.setfmax(0.0f, 0.0f, 0.0f);
    chnkSize = (int)gridObjectPtr->chunkSize;

    tileImportType = processType;
    if(tileImportType == ETileImportType::raw)
    {
        inTile.resize(chnkSize*chnkSize*chnkSize);
    }
    else if(tileImportType == ETileImportType::staggered)
    {
        inTile.resize((chnkSize+1)*(chnkSize+1)*(chnkSize+1));
    }
    else if(tileImportType == ETileImportType::finiteDifference)
    {
        inTile.resize((chnkSize+2)*(chnkSize+2)*(chnkSize+2));
    }
    else if(tileImportType == ETileImportType::finiteDifferenceSingleChannel)
    {
        inTile.resize((chnkSize+2)*(chnkSize+2)*(chnkSize+2));
    }

    outTile.resize ((chnkSize)*(chnkSize)*(chnkSize));
    //std::fill(outTile.begin(), outTile.end(), 5);

    // cout << "chnkSize is " << chnkSize << endl;
    // cout <<  channelName << endl;
}

//----------------------------------------------
GridTiledOperator::~GridTiledOperator() {}

void GridTiledOperator::refreshSourceAndTargetChannelDetails()
{
    typeToOperateOn = currentTargetChannelObject->channelInfo.channelType;
    channelName = currentTargetChannelObject->channelInfo.channelName;
    // cout << "channel name" << channelName << endl;
}
//----------------------------------------------
void GridTiledOperator::SetGridObject(GridObject *inGridObject)
{
    gridObjectPtr = inGridObject;
}

//----------------------------------------------
void GridTiledOperator::IterateGrid()
{
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
    if (currentTargetChannelObject->channelInfo.channelType == EChannelType::vector)
    {
        internalChannels = 3;
        // cout << "channels : " << internalChannels << endl;
    }

    // cout << "iterating grid" << endl;
    int fmz = gridObjectPtr->boundingBox.fluidMinZ;
    int fmy = gridObjectPtr->boundingBox.fluidMinY;
    int fmx = gridObjectPtr->boundingBox.fluidMinZ;

    int fMax = gridObjectPtr->boundingBox.fluidMaxX;
    int fMay = gridObjectPtr->boundingBox.fluidMaxY;
    int fMaz = gridObjectPtr->boundingBox.fluidMaxZ;

    //   cout << "<" << fmx << ", " << fmy << ", " << fmz << ">  to  <" << fMax <<
    //   ", " << fMay << ", " << fMaz << "> " << endl;



    // cout << "source channel is " << name << " " <<
    // currentSourceChannelObject->channelInfo.channelName << endl;
    double timeE = omp_get_wtime();
    //SORTING BOUNDS------------------------------------------------------------------------------
    #pragma omp parallel for collapse(3)
    for (int i = fmx; i <= fMax; i++)
    {
        for (int j = fmy; j <= fMay; j++)
        {
            for (int k = fmz; k <= fMaz; k++)
            {
            // int threadid  = omp_get_thread_num();
            // cout << threadid << endl;
            // cout <<" jhjhdjs" <<endl;
            // myString << "iterating in operator " << name <<  " chunk "  <<
            // i<<j<<k<<endl;
            chunkAddresses2 newChunkAddresses;
            newChunkAddresses.chunkSource =
            currentSourceChannelObject->GetChunk(i, j, k);
            newChunkAddresses.chunkTarget = currentTargetChannelObject->GetChunk(i, j, k); // if we are writing into the same grid/channel, then set
            // these the same
            newChunkAddresses.chunkIndex = glm::i32vec3(i, j, k);

            // cout << "adding existing" << endl;
            if (newChunkAddresses.chunkSource != currentSourceChannelObject->dummyChunk)   // if there is a chunk there// then go ahead and store // it
            {
                #pragma omp critical
                {chunks.emplace_back(newChunkAddresses);}
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
      for (int i = this->boundingBox.fluidMinZ;
           i <= this->boundingBox.fluidMaxZ; i++) {
        for (int j = this->boundingBox.fluidMinY;
             j <= this->boundingBox.fluidMaxY; j++) {
          for (int k = this->boundingBox.fluidMinX;
               k <= this->boundingBox.fluidMaxX; k++) {

            chunkAddresses2 newChunkAddresses;
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
    for (int i = 0; i < chunks.size(); i++)
    {
        if (callPreChunkOp) {

        {
          this->PreChunkOp(chunks[i].chunkSource, chunks[i].chunkTarget,
                           chunks[i].chunkIndex);
        }
      }


      // cout << "iterating through chunk vector " << i << endl;
      //used to loop over voxels then channels but here we are doing tiles so we loop over each tile channel by channel.
         //that way each kernel can work with the same coordinate offsets
         for (int a = 0; a < internalChannels; a++) {

             //copy mem into tiles
             //check tile type enum and fill it accordingly
             if(tileImportType == ETileImportType::raw)
             {
                 copyRawInput(chunks[i].chunkIndex, a);
             }
             else if(tileImportType == ETileImportType::staggered)
             {

             }
             else if(tileImportType == ETileImportType::finiteDifference)
             {

                 copyFiniteDifferenceInput(chunks[i].chunkIndex, chunks[i].chunkSource, a);
             }
             else if(tileImportType == ETileImportType::finiteDifferenceSingleChannel)
             {
                 copyFiniteDifferenceInputSingleChannel(chunks[i].chunkIndex, chunks[i].chunkSource);
             }

//#pragma omp simd collapse(3)
             for (int w = startVoxel; w < chnkSize; w += skipAmount)
                 {//for skipping voxels in thr red black gauss seidel update
                     for (int v = startVoxel; v < chnkSize; v += skipAmount)
                         {//can do 1-startvoxel to ping pong between 1 & 0
                             for (int u = startVoxel; u < chnkSize; u += skipAmount)
                                 {//and then do skipAmount = startVoxel+1 in each postgridop



                                 // int threadid  = omp_get_thread_num();
                                 // cout << threadid << endl;
                                 // myString << "in cell " << u+a<<v+a<<w+a<<endl;
//                                 uint32_t chunkDataIndex =
//                                     ((u) + (((v)*chnkSize)) + (((w)*chnkSize * chnkSize))) +
//                                     (a * chnkSize * chnkSize * chnkSize);
                                 // uint32_t chunkDataIndex = (       (u + chnkSize * (v + chnkSize *
                                 // w))    *a) + (a*chnkSize*chnkSize*chnkSize);
                                 // cout << "index " << chunkDataIndex << endl;

                                 int X = ((chunks[i].chunkIndex.x * (int)chnkSize) + u);

                                 int Y = ((chunks[i].chunkIndex.y * (int)chnkSize) + v);

                                 int Z = ((chunks[i].chunkIndex.z * (int)chnkSize) + w);


                                this->Algorithm(X, Y, Z,
                                                 u, v, w);
                             }
                     }
             }
             //copy back mem
             copyTileBack(chunks[i].chunkIndex);

         }

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
    //if(numberOfIterations>1){
    //cout << "end of " << name << " iteration: " << iteration+1 << " " << startVoxel << endl;
//}

#pragma omp barrier

if (callGridOp) {
    this->GridOp();
}
}


//END OF ITERATE-----------------------------------------------------------------------------


double timeB = omp_get_wtime();
cout << "grid operator " << name << " took " << timeB - timeA << " seconds"
   << endl;

// cout << myString.str() << endl;
//    cout << currentSourceChannelObject->numChunks << " chunks created " <<
//    endl;
//    currentSourceChannelObject->printChunks();
// myString.str("");
}

void GridTiledOperator::PostChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                           glm::i32vec3 chunkIdSecondary) {
// chunkOpCounter++;
}

void GridTiledOperator::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                          glm::i32vec3 chunkIdSecondary) {}

void GridTiledOperator::PreGridOp() {
currentSourceChannelObject =
  gridObjectPtr->channelObjs[0].get(); // default first one
currentTargetChannelObject =
  gridObjectPtr->channelObjs[0].get(); // default dup channel to write into
//    cout << currentSourceChannelObject << endl;
//    cout << currentTargetChannelObject << endl;
}


void GridTiledOperator::GridOp() {}

void GridTiledOperator::copyRawInput(glm::i32vec3 chunkCoords, uint32_t channel)
{
    memcpy  (   &inTile[0],
                &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z)->
                    chunkData[ channel*(chnkSize*chnkSize*chnkSize) ],
                sizeof(float)*(chnkSize*chnkSize*chnkSize)
            );

}

void GridTiledOperator::copyFiniteDifferenceInput(glm::i32vec3 chunkCoords, Chunk *main, uint32_t channel)
{


    //copy over rows at a time
    for(int k = 0; k < chnkSize; k++)
    {
        for(int j = 0; j < chnkSize; j++)
        {
            for(int i = 0; i < chnkSize; i++)

            memcpy  (   &inTile[ (i+1) + ((j+1)*chnkSize+2) + ((k+1)*chnkSize+2 * chnkSize+2) ],
                        &main->chunkData[ ((j*chnkSize) + (k*chnkSize * chnkSize)) + (channel*(chnkSize*chnkSize*chnkSize)) ],
                        sizeof(float));

        }
    }


    //-x edge
    float *mXedge = &currentSourceChannelObject->GetChunk(chunkCoords.x-1, chunkCoords.y, chunkCoords.z)->chunkData[0];

    //start at one to skip a voxel in Y and Z
    for(int k = 0; k < chnkSize; k++)
    {
        for(int j = 0; j < chnkSize; j++)
        {
            inTile[ ((j+1*(chnkSize+2)) + (k+1*(chnkSize+2 * chnkSize+2)))  ] =
                    mXedge[chnkSize +  ((j*chnkSize) + (k*chnkSize * chnkSize)) + (channel*(chnkSize*chnkSize*chnkSize)) ];

        }
    }

//    //cout << "passed" << endl;

    //+x edge
    float *pXedge = &currentSourceChannelObject->GetChunk(chunkCoords.x+1, chunkCoords.y, chunkCoords.z)->chunkData[0];

    for(int k = 0; k < chnkSize; k++)
    {
        for(int j = 0; j < chnkSize; j++)
        {
            inTile[ chnkSize + 1 + ((j+1*chnkSize+2) + (k+1*chnkSize+2 * chnkSize+2)) ] =
                    pXedge[0 +  ((j*chnkSize) + (k*chnkSize * chnkSize)) + (channel*(chnkSize*chnkSize*chnkSize)) ];

        }
    }

    //-y edge
    float *mYedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y-1, chunkCoords.z)->chunkData[0];

    for(int k = 0; k < chnkSize; k++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ i + 1 + (k+1*chnkSize+2 * chnkSize+2) ] =
                    mYedge[(i) +  (chnkSize-1*chnkSize) + (k*chnkSize * chnkSize) + (channel*(chnkSize*chnkSize*chnkSize)) ];

        }
    }

    //+y edge
    float *pYedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y+1, chunkCoords.z)->chunkData[0];

    for(int k = 0; k < chnkSize; k++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ i + 1 + (chnkSize+1*chnkSize+2) + (k+1*chnkSize+2 * chnkSize+2) ] =
                    pYedge[i + 0  + (k*chnkSize * chnkSize) + (channel*(chnkSize*chnkSize*chnkSize)) ];

        }
    }

    //-z edge
    float *mZedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z-1)->chunkData[0];

    for(int j = 0; j < chnkSize; j++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ i + 1 + (j+1*chnkSize+2) + 0 ] =
                    mZedge[i +  (j*chnkSize) + ((chnkSize-1) *chnkSize * chnkSize) + (channel*(chnkSize*chnkSize*chnkSize)) ];

        }
    }

    //+z edge
    float *pZedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z+1)->chunkData[0];

    for(int j = 0; j < chnkSize; j++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ i + 1 + (j+1*chnkSize+2) + (chnkSize+1*chnkSize+2 * chnkSize+2) ] =
                    pZedge[i +  (j*chnkSize) + (channel*(chnkSize*chnkSize*chnkSize)) ];

        }
    }

}

void GridTiledOperator::copyFiniteDifferenceInputSingleChannel(glm::i32vec3 chunkCoords, Chunk *main)
{


//    //copy over rows at a time
//    for(int k = 0; k < chnkSize; k++)
//    {
//        for(int j = 0; j < chnkSize; j++)
//        {
//            memcpy  (   &inTile[ 1 + ((j+1*(chnkSize+2)) + (k+1*(chnkSize+2) * (chnkSize+2))) ],
//                        &main->chunkData[ ((j*chnkSize) + (k*chnkSize * chnkSize))],
//                        sizeof(float)*(chnkSize));

//        }
//    }

    //copy over rows at a time
    for(int k = 0; k < chnkSize; k++)
    {
        for(int j = 0; j < chnkSize; j++)
        {
            for(int i = 0; i < chnkSize; i++)
            {

                inTile[ (i+1) + ((j+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2)) ] = main->chunkData[ i + ((j*chnkSize) + (k*chnkSize * chnkSize))  ];
            }

        }
    }



//    //-x edge
//    float *mXedge = &currentSourceChannelObject->GetChunk(chunkCoords.x-1, chunkCoords.y, chunkCoords.z)->chunkData[0];

//    //start at one to skip a voxel in Y and Z
//    for(int k = 0; k < chnkSize; k++)
//    {
//        for(int j = 0; j < chnkSize; j++)
//        {
//            inTile[ ((j+1*(chnkSize+2)) + (k+1*((chnkSize+2) * (chnkSize+2))))  ] =
//                    mXedge[chnkSize +  ((j*chnkSize) + (k*chnkSize * chnkSize))  ];

//        }
//    }

////    //cout << "passed" << endl;

//    //+x edge
//    float *pXedge = &currentSourceChannelObject->GetChunk(chunkCoords.x+1, chunkCoords.y, chunkCoords.z)->chunkData[0];

//    for(int k = 0; k < chnkSize; k++)
//    {
//        for(int j = 0; j < chnkSize; j++)
//        {
//            inTile[ ((chnkSize + 1) + ((j+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2))) ] =
//                    pXedge[0 +  ((j*chnkSize) + (k*chnkSize * chnkSize)) ];

//        }
//    }

//    //-y edge
//    float *mYedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y-1, chunkCoords.z)->chunkData[0];

//    for(int k = 0; k < chnkSize; k++)
//    {
//        for( int i = 0; i < chnkSize; i++)
//        {
//            inTile[ i + 1 + ((k+1)*(chnkSize+2) * (chnkSize+2)) ] =
//                    mYedge[(i) +  ((chnkSize-1)*chnkSize) + (k*chnkSize * chnkSize) ];

//        }
//    }

//    //+y edge
//    float *pYedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y+1, chunkCoords.z)->chunkData[0];

//    for(int k = 0; k < chnkSize; k++)
//    {
//        for( int i = 0; i < chnkSize; i++)
//        {
//            inTile[ i + 1 + ((chnkSize+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2)) ] =
//                    pYedge[i + 0  + (k*chnkSize * chnkSize) ];

//        }
//    }

//    //-z edge
//    float *mZedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z-1)->chunkData[0];

//    for(int j = 0; j < chnkSize; j++)
//    {
//        for( int i = 0; i < chnkSize; i++)
//        {
//            inTile[ i + 1 + (j+1*chnkSize+2) + 0 ] =
//                    mZedge[i +  (j*chnkSize) + ((chnkSize-1) *chnkSize * chnkSize) ];

//        }
//    }

//    //+z edge
//    float *pZedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z+1)->chunkData[0];

//    for(int j = 0; j < chnkSize; j++)
//    {
//        for( int i = 0; i < chnkSize; i++)
//        {
//            inTile[ i + 1 + (j+1*chnkSize+2) + (chnkSize+1*chnkSize+2 * chnkSize+2) ] =
//                    pZedge[i +  (j*chnkSize) ];

//        }
//    }

}

void GridTiledOperator::copyTileBack(glm::i32vec3 chunkCoords)
{
    memcpy  (   &currentTargetChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z)->chunkData[0],
                &outTile[0],
                sizeof(float)*(chnkSize*chnkSize*chnkSize)
            );

}

// void GridTiledOperator::BucketOp(glm::i32vec3 chunkId)
//{
//    //cout << "in base bucket op " <<endl;
//}

//----------------------------------------------
void GridTiledOperator::SetChannelName(std::string nameToSet) {
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

void GridTiledOperator::setNodeName(std::string nameIn) { name = nameIn; }
