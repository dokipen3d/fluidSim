#include "GridTiledPressure.h"
#include "ChannelObject.h"
#include "GridObject.h"

inline static uint32_t flatten3dCoordinatesPaddedTo1D(uint32_t x, uint32_t y,
                                                uint32_t z,
                                                uint32_t chunkSize) {

      // return   (        (x + chunkSize * (y + chunkSize * z))  *channel) +
      // (channel*chunkSize*chunkSize*chunkSize);
      return ( (x+1) +
               ( (y+1)*(chunkSize+2) ) +
               ( (z+1)*(chunkSize+2) * (chunkSize+2))
               );
}

inline static uint32_t flattenCoordinatesTo1D(uint32_t x, uint32_t y,
                                                uint32_t z,
                                                uint32_t chunkSize) {

 return   (  x +  (y * chunkSize) + (z*chunkSize*chunkSize));
}

void GridTiledPressure::setupDefaults()
{
    //set the source channel
    uint32_t divergenceSourceChannel =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("divergence"));
    //set the target channel
    uint32_t pressureTarget =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("pressure"));

    extraSourceObject = gridObjectPtr->channelObjs[divergenceSourceChannel].get(); // default first one

    currentSourceChannelObject =
        gridObjectPtr->channelObjs[pressureTarget].get(); // default first one
    currentTargetChannelObject =
        gridObjectPtr->channelObjs[pressureTarget+1].get(); // want vel channel.

    callGridOp = true;
    callPreChunkOp = true;
    callPostChunkOp = true;
    copyExtraTile = true;
    scale = 1.0f;
    scaleSquared = -(scale*scale);
    numberOfIterations = 20;
    forceInputBoundsIteration = false;


}

void GridTiledPressure::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                              glm::i32vec3 chunkIdSecondary) {

    if (currentTargetChannelObject->ChunkExists(
            chunkIdSecondary.x, chunkIdSecondary.y, chunkIdSecondary.z)) {
       //cout << "already exists!" << endl;
      // cout << outChunk << endl;

    } else {
      outChunk = currentTargetChannelObject->CreateChunk(
          chunkIdSecondary.x, chunkIdSecondary.y, chunkIdSecondary.z);
       //cout << "outChunk" << endl;
    }

    //std::fill(outChunk->chunkData.begin(),outChunk->chunkData.end(), 0.0f);
    //callPreChunkOp = false;//only call once;
}

void GridTiledPressure::Algorithm(int worldX, int worldY, int worldZ, uint32_t indexX, uint32_t indexY, uint32_t indexZ, const std::vector<float> &inTile, std::vector<float> &outTile, const std::vector<float> &extraTile)
{

     // uint32_t index = flattenCoordinatesTo1D(indexX, indexY, indexZ,8);
   // float divergence = divergenceSource->SampleExplicit(worldX, worldY, worldZ, 0);
//    float Pip1JK = currentSourceChannelObject->SampleExplicit(worldX+1, worldY, worldZ, 0 );
//    float Pim1JK = currentSourceChannelObject->SampleExplicit(worldX-1, worldY, worldZ, 0 );
//    float PIjp1K = currentSourceChannelObject->SampleExplicit(worldX, worldY+1, worldZ, 0 );
//    float PIjm1K = currentSourceChannelObject->SampleExplicit(worldX, worldY-1, worldZ, 0 );
//    float PIJkp1 = currentSourceChannelObject->SampleExplicit(worldX, worldY, worldZ+1, 0 );
//    float PIJkm1 = currentSourceChannelObject->SampleExplicit(worldX, worldY, worldZ-1, 0 );


//    float divergence = extraTile[index];
//    float Pip1JK = inTile[flatten3dCoordinatesPaddedTo1D(indexX+1, indexY, indexZ,8)];
//    float Pim1JK = inTile[flatten3dCoordinatesPaddedTo1D(indexX-1, indexY, indexZ,8)];
//    float PIjp1K = inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY+1, indexZ,8)];
//    float PIjm1K = inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY-1, indexZ,8)];
//    float PIJkp1 = inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY, indexZ+1,8)];
//    float PIJkm1 = inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY, indexZ-1,8)];

//    if (index == 0){
//        currentSourceChannelObject->SampleExplicitChunkAdressDebug(worldX, worldY, worldZ, 0 );

    //cout << "chunkAddress sampled explicit in algo is " << me << endl;
//}

  //  outTile[index] = (Pip1JK + Pim1JK + PIjp1K + PIjm1K + PIJkp1 + PIJkm1 + (divergence*scaleSquared))/6;

//outTile[flattenCoordinatesTo1D(indexX, indexY, indexZ,chnkSize)] =
//           (inTile[flatten3dCoordinatesPaddedTo1D(indexX++, indexY, indexZ,chnkSize)]+
//            inTile[flatten3dCoordinatesPaddedTo1D(indexX--, indexY, indexZ,chnkSize)]+
//            inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY++, indexZ,chnkSize)]+
//            inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY--, indexZ,chnkSize)]+
//            inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY, indexZ++,chnkSize)]+
//            inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY, indexZ--,chnkSize)]+
//                (extraTile.at(flattenCoordinatesTo1D(indexX, indexY, indexZ,chnkSize))*scaleSquared))/6.0f;

    //const uint32_t index =

    outTile[flattenCoordinatesTo1D(indexX, indexY, indexZ,chnkSize)] =
               (inTile[flatten3dCoordinatesPaddedTo1D(indexX+1, indexY, indexZ,chnkSize)]+
                inTile[flatten3dCoordinatesPaddedTo1D(indexX-1, indexY, indexZ,chnkSize)]+
                inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY+1, indexZ,chnkSize)]+
                inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY-1, indexZ,chnkSize)]+
                inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY, indexZ+1,chnkSize)]+
                inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY, indexZ-1,chnkSize)]+
                    (extraTile.at(flattenCoordinatesTo1D(indexX, indexY, indexZ,chnkSize))*scaleSquared))/6.0f;





}

void GridTiledPressure::ProcessTile(const std::vector<float> &inTile, std::vector<float> &outTile, const std::vector<float> &extraTile, glm::i32vec3 chunkId, Chunk *&pointerRefToSource, Chunk *&pointerRefToTarget)
{

//  #pragma omp simd collapse(3)
    for (uint32_t z = 0; z < chnkSize; z++)
    {//for skipping voxels in thr red black gauss seidel update
        for (uint32_t y = 0; y < chnkSize; y++)
        {//can do 1-startvoxel to ping pong between 1 & 0
            #pragma ivdep
            for (uint32_t x = 0; x < chnkSize; x++)
            {//and then do skipAmount = startVoxel+1 in each postgridop


                outTile[flattenCoordinatesTo1D(x, y, z,chnkSize)] =
                           (inTile[flatten3dCoordinatesPaddedTo1D(x+1, y, z,chnkSize)]+
                            inTile[flatten3dCoordinatesPaddedTo1D(x-1, y, z,chnkSize)]+
                            inTile[flatten3dCoordinatesPaddedTo1D(x, y+1, z,chnkSize)]+
                            inTile[flatten3dCoordinatesPaddedTo1D(x, y-1, z,chnkSize)]+
                            inTile[flatten3dCoordinatesPaddedTo1D(x, y, z+1,chnkSize)]+
                            inTile[flatten3dCoordinatesPaddedTo1D(x, y, z-1,chnkSize)]+
                (extraTile.at(flattenCoordinatesTo1D(x, y, z,chnkSize))*scaleSquared))/6.0f;

//                outTile[  x +  (y * 8) + (z*64) ] =
//                           (inTile[ (x+2) + ((y+1)*10) + ((z+1)*100) ]+
//                            inTile[ (x) + ((y+1)*10) + ((z+1)*100) ]+
//                            inTile[ (x+1) + ((y+2)*10) + ((z+1)*100) ]+
//                            inTile[ (x+1) + ( y*10 ) +  ((z+1)*100) ]+
//                            inTile[ (x+1) +  ( (y+1)*10 ) + ((z+2)*100) ]+
//                            inTile[ (x+1) + ( (y+1)*10 ) + ((z)*100) ]+
//                                (extraTile[  x +  (y * 8) + (z*64) ]*scaleSquared))/6.0f;

//                outTile[x+  (y*8) + (z*8*8)] =
//                           (inTile[(y)*(8+2) + ((z+1)*(8+2)*(8+2))]+
//                            inTile[(y+2)*(8+2) + ((z+1)*(8+2)*(8+2))]);


          }
      }
    }

//    for (int z = 0; z < 8; z++)
//    {
//        for (int y = 0; y < 8; y++)
//        {
//            const float* inRow = &inTile[( (y+1)*(chnkSize+2) + ((z+1)*(chnkSize+2)*(chnkSize+2)) )];
//            float* outRow = &outTile[ (y*chnkSize) + (z*chnkSize*chnkSize) ];
//            const float* extraRow = &extraTile[ (y*chnkSize) + (z*chnkSize*chnkSize) ];



//            for (int x = 0; x < 8; x++)
//            {
//                 outRow[x] = inRow[x] + inRow[x+2];
//            }
//        }
//    }

//        for (int z = 0; z < 8; z++)
//        {
//            for (int y = 0; y < 8; y++)
//            {
//                for (int x = 0; x < 8; x++)
//                {
//                                    outTile[flattenCoordinatesTo1D(x, y, z,chnkSize)] =
//                                               (inTile[flatten3dCoordinatesPaddedTo1D(x+1, y, z,chnkSize)]+
//                                                inTile[flatten3dCoordinatesPaddedTo1D(x-1, y, z,chnkSize)]+
//                                                inTile[flatten3dCoordinatesPaddedTo1D(x, y+1, z,chnkSize)]+
//                                                inTile[flatten3dCoordinatesPaddedTo1D(x, y-1, z,chnkSize)]+
//                                                inTile[flatten3dCoordinatesPaddedTo1D(x, y, z+1,chnkSize)]+
//                                                inTile[flatten3dCoordinatesPaddedTo1D(x, y, z-1,chnkSize)]+
//                                                    (extraTile.at(flattenCoordinatesTo1D(x, y, z,chnkSize))*scaleSquared))/6.0f;
//                }
//            }
//        }




}


GridTiledPressure::~GridTiledPressure()
{

}

void GridTiledPressure::PreGridOp()
{

    //set the source channel
    uint32_t divergenceSourceChannel =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("divergence"));
    //set the target channel
    uint32_t pressureTarget =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("pressure"));

    extraSourceObject = gridObjectPtr->channelObjs[divergenceSourceChannel].get(); // default first one

    currentSourceChannelObject =
        gridObjectPtr->channelObjs[pressureTarget].get(); // default first one
    currentTargetChannelObject =
        gridObjectPtr->channelObjs[pressureTarget+1].get(); // want  channel.


}

void GridTiledPressure::GridOp() {
  gridObjectPtr->SwapChannelPointers(std::string("pressure"));
}

//#pragma omp declare simd
float GridTiledPressure::calcPressure(float a, float b, float c, float d, float e, float f, float g, float h){

return (a + b + c + d + e + f + (g)*0.166666666666);
}

void GridTiledPressure::PostChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                         glm::i32vec3 chunkIdSecondary){

    std::swap(inChunk, outChunk);


}
