#include "GridTiledPressure.h"
#include "ChannelObject.h"
#include "GridObject.h"





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
    numberOfIterations = 200;

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

    outTile[0] = GridTiledPressure::calcPressure(outTile[0],outTile[0],outTile[0], outTile[0], outTile[0], outTile[0], outTile[0], outTile[0]);

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
