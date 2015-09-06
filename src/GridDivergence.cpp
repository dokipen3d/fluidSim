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

void GridDivergence::Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelWorldPosition,
                            Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                            uint32_t channel, bool internalAccessible){

    float X = voxelWorldPosition.x;

    float Y = voxelWorldPosition.y;

    float Z = voxelWorldPosition.z;

  //do scale here?
  float uDivergenceP1;
  float vDivergenceP1;
  float wDivergenceP1;

  float uDivergenceM1 = velocitySourceChannelObject->SampleExplicit(X, Y, Z, 0 );
  if (velocitySourceChannelObject->SampleIsOutsideBounds(X+1, Y, Z, 0 ))
  {
    uDivergenceP1 = uDivergenceM1;
  }
  else
  {
    uDivergenceP1 = velocitySourceChannelObject->SampleExplicit(X+1, Y, Z, 0 );
  }

  float vDivergenceM1 = velocitySourceChannelObject->SampleExplicit(X, Y, Z, 1 );

  if (velocitySourceChannelObject->SampleIsOutsideBounds(X, Y+1, Z, 1 ))
  {
    vDivergenceP1 = vDivergenceM1;
  }
  else
  {
    vDivergenceP1 = velocitySourceChannelObject->SampleExplicit(X, Y+1, Z, 1);
  }

  float wDivergenceM1 = velocitySourceChannelObject->SampleExplicit(X, Y, Z, 2 );

  if(velocitySourceChannelObject->SampleIsOutsideBounds(X, Y, Z+1, 2 ))
  {
      wDivergenceP1 = wDivergenceM1;
  }
  else
  {
    wDivergenceP1 = velocitySourceChannelObject->SampleExplicit(X, Y, Z+1, 2 );
  }

//  float Udiff = (uDivergenceP1 - uDivergenceM1);
//  float Vdiff = (vDivergenceP1 - vDivergenceM1);
//  float Wdiff = (wDivergenceP1 - wDivergenceM1);

    float Udiff = (uDivergenceP1 - uDivergenceM1);
    float Vdiff = (vDivergenceP1 - vDivergenceM1);
    float Wdiff = (wDivergenceP1 - wDivergenceM1);

  float divergence = (scale * Udiff+Vdiff+Wdiff);

//  if (debug){
//      if (X == -16 && Y == 2 && Z == 2)
//          cout << "div at -16 is " << divergence << endl;
//      if (X == -15 && Y == 2&& Z == 2)
//          cout << "div at -15 is " << divergence << endl;
//      if (X ==  15 && Y == 2 && Z == 2)
//          cout << "div at  15 is " << divergence << endl;
//      if (X ==  14 && Y == 2&& Z == 2)
//          cout << "div at  14 is " << divergence << endl;

//      }

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

