#include "GridTiledPressure.h"
#include "ChannelObject.h"
#include "GridObject.h"

#pragma omp declare simd
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

#pragma omp declare simd
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
    scale = 1.0f;
    scaleSquared = -(scale*scale);
    numberOfIterations = 20;



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
               (inTile[flatten3dCoordinatesPaddedTo1D(indexX++, indexY, indexZ,chnkSize)]+
                inTile[flatten3dCoordinatesPaddedTo1D(indexX--, indexY, indexZ,chnkSize)]+
                inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY++, indexZ,chnkSize)]+
                inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY--, indexZ,chnkSize)]+
                inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY, indexZ++,chnkSize)]+
                inTile[flatten3dCoordinatesPaddedTo1D(indexX, indexY, indexZ--,chnkSize)]+
                    (extraTile.at(flattenCoordinatesTo1D(indexX, indexY, indexZ,chnkSize))*scaleSquared))/6.0f;





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
