#include "GridDivergence.h"
#include "ChannelObject.h"
#include "GridObject.h"





void GridDivergence::setupDefaults()
{
    uint32_t divergenceTarget =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("divergence"));
    //set the source channel
    uint32_t velocitySource =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));

    velocitySourceChannelObject = gridObjectPtr->channelObjs[velocitySource].get();

    currentSourceChannelObject =
        gridObjectPtr->channelObjs[divergenceTarget].get(); // default first one
    currentTargetChannelObject =
        gridObjectPtr->channelObjs[divergenceTarget].get(); // want vel channel.

    scale = 1.0f;

}

void GridDivergence::Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition,
                            Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                            uint32_t channel){

  float X = ((chunkId.x * (int)chnkSize) + voxelPosition.x);
  float Y = ((chunkId.y * (int)chnkSize) + voxelPosition.y);
  float Z = ((chunkId.z * (int)chnkSize) + voxelPosition.z);

  //do scale here?

  float uDivergenceM1 = velocitySourceChannelObject->SampleExplicit(X, Y, Z, 0 );
  float uDivergenceP1 = velocitySourceChannelObject->SampleExplicit(X+1, Y, Z, 0 );

  float vDivergenceM1 = velocitySourceChannelObject->SampleExplicit(X, Y, Z, 1 );
  float vDivergenceP1 = velocitySourceChannelObject->SampleExplicit(X, Y+1, Z, 1 );

  float wDivergenceM1 = velocitySourceChannelObject->SampleExplicit(X, Y, Z, 2 );
  float wDivergenceP1 = velocitySourceChannelObject->SampleExplicit(X, Y, Z+1, 2 );

  float Udiff = (uDivergenceP1 - uDivergenceM1);
  float Vdiff = (vDivergenceP1 - vDivergenceM1);
  float Wdiff = (wDivergenceP1 - wDivergenceM1);

  float divergence = (scale * Udiff+Vdiff+Wdiff);

  outChunk->chunkData[dataIndex] = divergence;
}


GridDivergence::~GridDivergence()
{

}

void GridDivergence::PreGridOp()
{
    uint32_t divergenceTarget =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("divergence"));
    //set the source channel
    uint32_t velocitySource =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));

    velocitySourceChannelObject = gridObjectPtr->channelObjs[velocitySource].get();

    currentSourceChannelObject =
        gridObjectPtr->channelObjs[divergenceTarget].get(); // default first one
    currentTargetChannelObject =
        gridObjectPtr->channelObjs[divergenceTarget].get(); // want vel channel.

}

