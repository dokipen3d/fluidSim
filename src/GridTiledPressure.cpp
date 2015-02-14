#include "GridTiledPressure.h"
#include "ChannelObject.h"
#include "GridObject.h"


inline static uint32_t flatten3dCoordinatesto1D(uint32_t x, uint32_t y,
                                                uint32_t z,
                                                uint32_t chunkSize) {
  // return   (        (x + chunkSize * (y + chunkSize * z))  *channel) +
  // (channel*chunkSize*chunkSize*chunkSize);
  return ((x+1) + (((y+1)*chunkSize+2)) + (((z+1)*chunkSize+2 * chunkSize+2)));
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


void GridTiledPressure::setupDefaults()
{
    //set the source channel
    uint32_t divergenceSourceChannel =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("divergence"));
    //set the target channel
    uint32_t pressureTarget =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("pressure"));

    divergenceSource = gridObjectPtr->channelObjs[divergenceSourceChannel].get(); // default first one

    currentSourceChannelObject =
        gridObjectPtr->channelObjs[pressureTarget].get(); // default first one
    currentTargetChannelObject =
        gridObjectPtr->channelObjs[pressureTarget+1].get(); // want vel channel.

    callGridOp = true;
    callPreChunkOp = true;
    callPostChunkOp = true;
    scale = 1.0f;
    scaleSquared = -(scale*scale);
    numberOfIterations = 30;

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

void GridTiledPressure::Algorithm(int worldX, int worldY, int worldZ, int indexX, int indexY, int indexZ)
{

    outTile[indexX+(indexY*chnkSize)+ (indexZ*chnkSize*chnkSize)] =// inTile[1]+inTile[2]+inTile[4]+inTile[6]+inTile[8]+inTile[10]+inTile[12]+inTile[14]*0.13;


            inTile[flatten3dCoordinatesto1D(indexX-1, indexY, indexZ,8)]+
            inTile[flatten3dCoordinatesto1D(indexX+1, indexY, indexZ,8)]+
            inTile[flatten3dCoordinatesto1D(indexX, indexY-1, indexZ,8)]+
            inTile[flatten3dCoordinatesto1D(indexX, indexY+1, indexZ,8)]+
            inTile[flatten3dCoordinatesto1D(indexX, indexY, indexZ-1,8)]+
            inTile[flatten3dCoordinatesto1D(indexX, indexY, indexZ+1,8)]*scaleSquared/6;

            outTile[indexX+(indexY*chnkSize)+ (indexZ*chnkSize*chnkSize)] = inTile[flatten3dCoordinatesto1D(indexX, indexY, indexZ,8)];


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

    divergenceSource = gridObjectPtr->channelObjs[divergenceSourceChannel].get(); // default first one

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
