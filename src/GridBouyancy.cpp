#include "GridBouyancy.h"
#include "Chunk.h"
#include "ChannelObject.h"
#include "GridObject.h"

void GridBouyancy::setupDefaults() {

  //set the target channel
  uint32_t velTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
  //set the source channel
  uint32_t densitySource =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("density"));

  densitySourceChannel = gridObjectPtr->channelObjs[densitySource].get();

  currentSourceChannelObject =
      gridObjectPtr->channelObjs[velTarget].get(); // default first one
  currentTargetChannelObject =
      gridObjectPtr->channelObjs[velTarget].get(); // want vel channel.
  // callPreChunkOp = true;
  // callGridOp = true;
}

void GridBouyancy::Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelWorldPosition,
                             Chunk *inChunk, Chunk *outChunk,
                             uint32_t dataIndex, uint32_t channel, bool internalAccessible) {

    float X = voxelWorldPosition.x;

    float Y = voxelWorldPosition.y;

    float Z = voxelWorldPosition.z;

    //if we are a vel operator and sampling a scalar, we need to do the inverse channel (could do it before calling algo)
//  float densSample =
//      inChunk
//          ->chunkData[dataIndex - (channel * chnkSize * chnkSize * chnkSize)];
  float densSample = densitySourceChannel->SampleExplicit(X, Y-1, Z, 0);
  float densSampleP1 = densitySourceChannel->SampleExplicit(X, Y, Z, 0);



  //float densSampleXP1 = currentSourceChannelObject->SampleExplicit(X+1, Y, Z, 0);



  //glm::vec3 vel =  currentTargetChannelObject->SampleVectorAtPosition(X + 0.5f, Y + 0.5f, Z + 0.5f);

 // float yPos = densSample - vel.y;

  //expensive if we allow all three channels
  //glm::vec3 value = glm::vec3(((densSample+densSampleXP1)/2)*0.02f, ((densSample+densSampleP1)/2)*0.0f, 0.0f);

  glm::vec3 value = glm::vec3(0.0f, ((densSample+densSampleP1)/2)*0.4, 0.0f);



  //glm::vec3 value = glm::vec3(0.0f, 0.15f, 0.0f);


    //if (currentTime < 50000){
        float newVal = outChunk->chunkData[dataIndex]+ value[channel];

//        if (X == 1 && Y == 2 && Z == 2 && channel == 1)
//            cout << "bouyvel at  3 is " << newVal << endl;
//        if (X == -2 && Y == 2 && Z == 2 && channel == 1)
//            cout << "bouyvel at -4 is " << newVal << endl;
        outChunk->chunkData[dataIndex] = newVal;
    //}
}

void GridBouyancy::PreGridOp() {
  uint32_t velTarget =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));
  uint32_t densitySource =
      gridObjectPtr->GetMemoryIndexForChannelName(std::string("density"));

  densitySourceChannel = gridObjectPtr->channelObjs[densitySource].get();

  currentSourceChannelObject =
      gridObjectPtr->channelObjs[velTarget].get(); // default first one
  currentTargetChannelObject =
      gridObjectPtr->channelObjs[velTarget].get(); // want vel channel.
}

void GridBouyancy::GridOp() {
  // gridObjectPtr->SwapChannelPointers(std::string("velocity"));
}
