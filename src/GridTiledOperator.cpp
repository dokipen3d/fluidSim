#include "GridTiledOperator.h"
#include "GridObject.h"
#include "ChannelObject.h"
#include "glm/vec3.hpp"
#include "glm/gtc/type_precision.hpp"

#include <omp.h>
//#include <algorithm>
#include <cstring>
#include <iomanip>

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
GridTiledOperator::GridTiledOperator(GridObject *inGridObject)
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


    tileImportType = ETileImportType::finiteDifference;

//    if(tileImportType == ETileImportType::raw)
//    {
//        inTile.resize(chnkSize*chnkSize*chnkSize);
//    }
//    else if(tileImportType == ETileImportType::staggered)
//    {
//        inTile.resize((chnkSize+1)*(chnkSize+1)*(chnkSize+1));
//    }
//    else if(tileImportType == ETileImportType::finiteDifference)
//    {
//        inTile.resize((chnkSize+2)*(chnkSize+2)*(chnkSize+2));
//    }
//    else if(tileImportType == ETileImportType::finiteDifferenceSingleChannel)
//    {
//        inTile.resize((chnkSize+2)*(chnkSize+2)*(chnkSize+2));
//    }

    //outTile.resize ((chnkSize)*(chnkSize)*(chnkSize));
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
    double timeAlgo = 0;

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
  //  if (!forceInputBoundsIteration){

    #pragma omp parallel for collapse(3)
    for (int i = fmx; i <= fMax; i++)
    {
        for (int j = fmy; j <= fMay; j++)
        {
            for (int k = fmz; k <= fMaz; k++)
            {

                chunkAddresses2 newChunkAddresses;
                newChunkAddresses.chunkSource =
                currentSourceChannelObject->GetChunk(i, j, k);
                newChunkAddresses.chunkTarget = currentTargetChannelObject->GetChunk(i, j, k); // if we are writing into the same grid/channel, then set these the same
                newChunkAddresses.chunkIndex = glm::i32vec3(i, j, k);
                //set it so that it can be detected if it will be processed. some operators might want to prevent double processing on it or control when to stop
                //newChunkAddresses.chunkTarget->alreadyProcessedThisIteration = false;

                // cout << "adding existing" << endl;
                if (newChunkAddresses.chunkSource != currentSourceChannelObject->dummyChunk)   // if there is a chunk there// then go ahead and store // it
                {
                    #pragma omp critical
                    {
                        chunks.emplace_back(newChunkAddresses);
                    }
                }
            }
        }
    }
  //  }
    //END OF SORTING BOUNDS------------------------------------------------------------------------------


    //FORCE SORTING BOUNDS------------------------------------------------------------------------------
    if (forceInputBoundsIteration)
    {

        #pragma omp parallel for collapse(3)
        for (int i = this->boundingBox.fluidMinZ; i <= this->boundingBox.fluidMaxZ; i++)
        {
            for (int j = this->boundingBox.fluidMinY; j <= this->boundingBox.fluidMaxY; j++)
            {
                for (int k = this->boundingBox.fluidMinX; k <= this->boundingBox.fluidMaxX; k++)
                {

                    chunkAddresses2 newChunkAddresses;
                    newChunkAddresses.chunkSource = nullptr;
                    newChunkAddresses.chunkTarget = nullptr;
                    newChunkAddresses.chunkIndex = glm::i32vec3(i, j, k);



                    #pragma omp critical
                    {
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
    //#pragma omp barrier

    //FOR ALL ITERATIONS-----------------------------------------------------------------------------------
    for (iteration = 0; iteration < numberOfIterations; iteration++)
    {
        if (iteration == 0)
        {
            totalChunksToOperateOn = chunks.size();
            cout << "chunks to iterate through: " << totalChunksToOperateOn << " with "
                << totalChunksToOperateOn *chnkSize *chnkSize *chnkSize << " voxels"
                << endl;
        }




        //CALL PRE GRID OP BEFORE WE START OPERATING ON CHUNKS
        this->PreGridOp();

        //cout << "address of A is " << currentSourceChannelObject << " and B is " << currentTargetChannelObject << endl;


        //FOR ALL CHUNKS---------------------------------------------------------------------------------
        #pragma omp parallel for
        for (int i = 0; i < chunks.size(); i++)
        {




            std::vector<float> localInTile((chnkSize+2)*(chnkSize+2)*(chnkSize+2));
            std::vector<float> localOutTile((chnkSize)*(chnkSize)*(chnkSize));
            std::vector<float> localExtraTile((chnkSize)*(chnkSize)*(chnkSize));

            if (callPreChunkOp)
            {
                this->PreChunkOp(chunks[i].chunkSource, chunks[i].chunkTarget, chunks[i].chunkIndex);
            }

            int a = 0;
            //used to loop over voxels then channels but here we are doing tiles so we loop over each tile channel by channel.
            //that way each kernel can work with the same coordinate offsets
           // for (int a = 0; a < internalChannels; a++)
            //if (!forceInputBoundsIteration)
            {
                //copy mem into tiles
                //check tile type enum and fill it accordingly
                if(tileImportType == ETileImportType::raw)
                {
                    copyRawInput(chunks[i].chunkIndex, a, localInTile, false);
                }
                else if(tileImportType == ETileImportType::staggered)
                {

                }
                else if(tileImportType == ETileImportType::finiteDifference)
                {
                    copyFiniteDifferenceInput(chunks[i].chunkIndex, chunks[i].chunkSource, a, localInTile);
                    //localInTile.resize((chnkSize+2)*(chnkSize+2)*(chnkSize+2));
                    //cout << inTile.size() << endl;
                }
                else if(tileImportType == ETileImportType::finiteDifferenceSingleChannel)
                {

                    //int w = 5;
                    copyFiniteDifferenceInputSingleChannel(chunks[i].chunkIndex, chunks[i].chunkSource, localInTile);

                    /*
                     *                     if (iteration == numberOfIterations-1)

                   // for (int w = startVoxel; w < chnkSize+2; w++)
                    //{//for skipping voxels in thr red black gauss seidel update
                        for (int v = startVoxel; v < chnkSize+2; v++)
                        {//can do 1-startvoxel to ping pong between 1 & 0
                            for (int u = startVoxel; u < chnkSize+2; u++)
                            {//and then do skipAmount = startVoxel+1 in each postgridop

                                cout << setfill('0') << setw(2) << inTile[u + (v*(chnkSize+2))+ (w*(chnkSize+2)*(chnkSize+2))] << " ";
                            }
                            cout << endl;
                        }
                        cout << endl;
                    }*/

                }
            }

            if(copyExtraTile == true)
            {

             copyRawInput(chunks[i].chunkIndex, a, localExtraTile, true );



            }
                //#pragma GCC ivdep
                //#pragma omp simd collapse(3)
//                for (uint32_t w = 0; w < chnkSize; w++)
//                {//for skipping voxels in thr red black gauss seidel update
//                    for (uint32_t v = 0; v < chnkSize; v++)
//                    {//can do 1-startvoxel to ping pong between 1 & 0
//                        //#pragma omp simd safelen(8)
//                        for (uint32_t u = 0; u < chnkSize; u++)
//                        {//and then do skipAmount = startVoxel+1 in each postgridop


//                            int X = ((chunks[i].chunkIndex.x * (int)chnkSize) + u);

//                            int Y = ((chunks[i].chunkIndex.y * (int)chnkSize) + v);

//                            int Z = ((chunks[i].chunkIndex.z * (int)chnkSize) + w);


//                            this->Algorithm(X, Y, Z, u, v, w, localInTile, localOutTile, localExtraTile);
//                        }
//                    }
//                }




            {
            ProcessTile(localInTile,localOutTile,localExtraTile, chunks[i].chunkIndex, chunks[i].chunkSource, chunks[i].chunkTarget);
            }


            // END OF FOR ALL CHANNELS

//            if (chunks[i].chunkIndex.x == 0 && chunks[i].chunkIndex.y == 0 && chunks[i].chunkIndex.z ==0)
//            {
//                cout << "address of A is " << chunks[i].chunkSource << " and B is " << chunks[i].chunkTarget << endl;



//            }



            //copy back mem

            copyTileBack(chunks[i].chunkIndex, localOutTile, chunks[i].chunkTarget );
            //#pragma omp critical
            if (callPostChunkOp)
            {
                this->PostChunkOp(chunks[i].chunkSource, chunks[i].chunkTarget, chunks[i].chunkIndex);
            }






        }
        //END FOR ALL CHUNKS---------------------------------------------------------------------------------
        #pragma omp barrier

        if (callGridOp)
        {
           this->GridOp();
        }


    }
    //END OF ITERATE-----------------------------------------------------------------------------



    #pragma omp barrier

    if (callGridOp)
    {
        this->GridOp();
    }


    double timeB = omp_get_wtime();
    cout << "grid operator " << name << " took " << timeB - timeA << " seconds" << " Extra stuff TOOK " << timeF - timeA
        << endl;

}


void GridTiledOperator::PostChunkOp(Chunk *&inChunk, Chunk *&outChunk, glm::i32vec3 chunkIdSecondary)
{
    // chunkOpCounter++;
}

void GridTiledOperator::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                          glm::i32vec3 chunkIdSecondary) {}

void GridTiledOperator::PreGridOp() {
    //cout << "default being called"<<endl;
currentSourceChannelObject =
  gridObjectPtr->channelObjs[0].get(); // default first one
currentTargetChannelObject =
  gridObjectPtr->channelObjs[0].get(); // default dup channel to write into
//    cout << currentSourceChannelObject << endl;
//    cout << currentTargetChannelObject << endl;
}


void GridTiledOperator::GridOp() {}



void GridTiledOperator::copyRawInput(glm::i32vec3 chunkCoords, uint32_t channel, std::vector<float>& inputTile, bool sourceOrExtra)
{

    Chunk* source =
    sourceOrExtra ? extraSourceObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z) :
                    currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z);

    memcpy(&inputTile[0], &source->chunkData[0], sizeof(float)*chnkSize*chnkSize*chnkSize);
//    for(int k = 0; k < chnkSize; k++)
//    {
//        for(int j = 0; j < chnkSize; j++)
//        {
//            for(int i = 0; i < chnkSize; i++)
//            {


//                inputTile[ i + (j*chnkSize) + (k*chnkSize * chnkSize)  ] =
//                        source->chunkData[ i + (j * chnkSize) + (k*(chnkSize*chnkSize))];
//            }

//        }
//    }

}

void GridTiledOperator::copyFiniteDifferenceInput(glm::i32vec3 chunkCoords, Chunk *main, uint32_t channel, std::vector<float> &inTile)
{
    //channel = 0;
   // main = currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z);
//    if (chunkCoords.x == 0 && chunkCoords.y == 0 && chunkCoords.z ==0)
//    {
//        cout << "address of A source in tilecopy is " << main << endl;



//    }



    //copy over rows at a time
    for(int k = 0; k < chnkSize; k++)
    {
        for(int j = 0; j < chnkSize; j++)
        {
            for(int i = 0; i < chnkSize; i++)
            {
//            memcpy  (   &inTile[ (i+1) + ((j+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2)) + (channel * (chnkSize+2) * (chnkSize+2) * (chnkSize+2)) ],
//                        &main->chunkData[ ((i) + (j * chnkSize)) + (k*(chnkSize*chnkSize)) + (channel * chnkSize * chnkSize * chnkSize)],
//                        sizeof(float));
//                inTile[ (i+1) + ((j+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2)) + (channel * (chnkSize+2) * (chnkSize+2) * (chnkSize+2)) ] =
//                        main->chunkData[ ((i) + (j * chnkSize)) + (k*(chnkSize*chnkSize)) + (channel * chnkSize * chnkSize * chnkSize)];

                inTile[ (i+1) + ((j+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2))  ] =
                        main->chunkData[ i + (j * chnkSize) + (k*(chnkSize*chnkSize))];
            }

        }
    }

//    if (chunkCoords.x == 0 && chunkCoords.y == 0 && chunkCoords.z ==0)
//    {
//        cout << "address of A source in tilecopy is " << main << endl;
//        for(int k = 0; k < chnkSize; k++)
//        {
//            for(int j = 0; j < chnkSize; j++)
//            {
//                for(int i = 0; i < chnkSize; i++)


//                   // cout << main->chunkData


//            }
//        }


//    }


    //-x edge
    //float *mXedge = &currentSourceChannelObject->GetChunk(chunkCoords.x-1, chunkCoords.y, chunkCoords.z)->chunkData[0];
    std::vector<float>& mXe = currentSourceChannelObject->GetChunk(chunkCoords.x-1, chunkCoords.y, chunkCoords.z)->chunkData;


    //start at one to skip a voxel in Y and Z
    for(int k = 0; k < chnkSize; k++)
    {
        for(int j = 0; j < chnkSize; j++)
        {
            inTile[ ((j+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2)) ] =
                    mXe[ (chnkSize-1) +  (j*chnkSize) + (k*chnkSize * chnkSize) ];

        }
    }

//    //cout << "passed" << endl;

    //+x edge
   // float *pXedge = &currentSourceChannelObject->GetChunk(chunkCoords.x+1, chunkCoords.y, chunkCoords.z)->chunkData[0];
    std::vector<float>& pXe = currentSourceChannelObject->GetChunk(chunkCoords.x+1, chunkCoords.y, chunkCoords.z)->chunkData;

    for(int k = 0; k < chnkSize; k++)
    {
        for(int j = 0; j < chnkSize; j++)
        {
            inTile[ (chnkSize + 1) + ((j+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2)) ] =
                    pXe[0 +  (j*chnkSize) + (k*chnkSize * chnkSize)];

        }
    }

    //-y edge
    //float *mYedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y-1, chunkCoords.z)->chunkData[0];
    std::vector<float>& mYe = currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y-1, chunkCoords.z)->chunkData;


    for(int k = 0; k < chnkSize; k++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ (i+1) + ((k+1)*(chnkSize+2) * (chnkSize+2)) ] =
                    mYe[(i) +  ((chnkSize-1)*chnkSize) + (k*chnkSize * chnkSize)];

        }
    }

    //+y edge
    //float *pYedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y+1, chunkCoords.z)->chunkData[0];
    std::vector<float>& pYe = currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y+1, chunkCoords.z)->chunkData;

    for(int k = 0; k < chnkSize; k++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ (i + 1) + ((chnkSize+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2)) ] =
                    pYe[i + 0  + (k*chnkSize * chnkSize) ];

        }
    }

    //-z edge
   // float *mZedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z-1)->chunkData[0];
    std::vector<float>& mZe = currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z-1)->chunkData;


    for(int j = 0; j < chnkSize; j++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ (i + 1) + ((j+1)*(chnkSize+2)) + 0 ] =
                    mZe[i +  (j*chnkSize) + ((chnkSize-1) *chnkSize * chnkSize)];

        }
    }

    //+z edge
    //float *pZedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z+1)->chunkData[0];
    std::vector<float>& pZe = currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z+1)->chunkData;


    for(int j = 0; j < chnkSize; j++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ (i + 1) + ((j+1)*(chnkSize+2)) + ((chnkSize+1)*(chnkSize+2) * (chnkSize+2)) ] =
                    pZe[i +  (j*chnkSize)];

        }
    }

}

void GridTiledOperator::copyFiniteDifferenceInputSingleChannel(glm::i32vec3 chunkCoords,  Chunk *main, std::vector<float> &inTile)
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

                inTile[ (i+1) + ((j+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2)) ] = main->chunkData[ i + (j*chnkSize) + (k*chnkSize * chnkSize) ];
            }

        }
    }



    //-x edge
    float *mXedge = &currentSourceChannelObject->GetChunk(chunkCoords.x-1, chunkCoords.y, chunkCoords.z)->chunkData[0];

    //start at one to skip a voxel in Y and Z
    for(int k = 0; k < chnkSize; k++)
    {
        for(int j = 0; j < chnkSize; j++)
        {
            inTile[ ((j+1*(chnkSize+2)) + (k+1*((chnkSize+2) * (chnkSize+2))))  ] =
                    mXedge[chnkSize +  ((j*chnkSize) + (k*chnkSize * chnkSize))  ];

        }
    }

//    //cout << "passed" << endl;

    //+x edge
    float *pXedge = &currentSourceChannelObject->GetChunk(chunkCoords.x+1, chunkCoords.y, chunkCoords.z)->chunkData[0];

    for(int k = 0; k < chnkSize; k++)
    {
        for(int j = 0; j < chnkSize; j++)
        {
            inTile[ ((chnkSize + 1) + ((j+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2))) ] =
                    pXedge[0 +  ((j*chnkSize) + (k*chnkSize * chnkSize)) ];

        }
    }

    //-y edge
    float *mYedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y-1, chunkCoords.z)->chunkData[0];

    for(int k = 0; k < chnkSize; k++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ i + 1 + ((k+1)*(chnkSize+2) * (chnkSize+2)) ] =
                    mYedge[(i) +  ((chnkSize-1)*chnkSize) + (k*chnkSize * chnkSize) ];

        }
    }

    //+y edge
    float *pYedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y+1, chunkCoords.z)->chunkData[0];

    for(int k = 0; k < chnkSize; k++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ i + 1 + ((chnkSize+1)*(chnkSize+2)) + ((k+1)*(chnkSize+2) * (chnkSize+2)) ] =
                    pYedge[i + 0  + (k*chnkSize * chnkSize) ];

        }
    }

    //-z edge
    float *mZedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z-1)->chunkData[0];

    for(int j = 0; j < chnkSize; j++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ i + 1 + (j+1*chnkSize+2) + 0 ] =
                    mZedge[i +  (j*chnkSize) + ((chnkSize-1) *chnkSize * chnkSize) ];

        }
    }

    //+z edge
    float *pZedge = &currentSourceChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z+1)->chunkData[0];

    for(int j = 0; j < chnkSize; j++)
    {
        for( int i = 0; i < chnkSize; i++)
        {
            inTile[ i + 1 + (j+1*chnkSize+2) + (chnkSize+1*chnkSize+2 * chnkSize+2) ] =
                    pZedge[i +  (j*chnkSize) ];

        }
    }

}

void GridTiledOperator::copyTileBack(glm::i32vec3 chunkCoords, std::vector<float>& outTile, Chunk *target)
{
    memcpy  (   &currentTargetChannelObject->GetChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z)->chunkData[0],
                &outTile[0],
                sizeof(float)*(chnkSize*chnkSize*chnkSize)
            );

    //cout << "copying back" << endl;
//    for(int k = 0; k < chnkSize; k++)
//    {
//        for(int j = 0; j < chnkSize; j++)
//        {
//            for(int i = 0; i < chnkSize; i++)
//            {

//                target->chunkData[i + (j*chnkSize) + (k*chnkSize * chnkSize)] =
//                    outTile[i + (j*chnkSize) + (k*chnkSize * chnkSize)];
//            }

//        }
//    }

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
