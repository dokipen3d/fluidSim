#include "GridProjection.h"
#include "ChannelObject.h"
#include "GridObject.h"





void GridProjection::setupDefaults()
{
    //set the source channel
    uint32_t pressureSourceIndex =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("pressure"));
    //set the target channel
    uint32_t velocitySource =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));


    pressureSource = gridObjectPtr->channelObjs[pressureSourceIndex].get(); // default first one

    currentSourceChannelObject =
        gridObjectPtr->channelObjs[velocitySource].get(); // default first one
    currentTargetChannelObject =
        gridObjectPtr->channelObjs[velocitySource].get(); // want vel channel.

    //callGridOp = true;
    //callPreChunkOp = true;
    scale = 1.0f;
    //internalChannels = 3;
    //scaleSquared = (scale*scale);
    //numberOfIterations = 8;

}



void GridProjection::Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition,
                            Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                            uint32_t channel){





  float X = ((chunkId.x * (int)chnkSize) + voxelPosition.x);
  float Y = ((chunkId.y * (int)chnkSize) + voxelPosition.y);
  float Z = ((chunkId.z * (int)chnkSize) + voxelPosition.z);

  //add scale here?

    float Pm1;
    float Pp1;



    if (channel == 0) {

            Pm1  = pressureSource->SampleExplicit(X-1.0f, Y, Z, 0 );
            Pp1  = pressureSource->SampleExplicit(X, Y, Z, 0 );
            //outChunk->chunkData[dataIndex] -= (Pp1 - Pm1)*0.000001;
            //cout << "in channel 0 " << Pm1 << " " << Pp1 << endl;
            //break;
    }
    else if (channel == 1){


            Pm1  = pressureSource->SampleExplicit(X, Y-1.0f, Z, 0 );
            Pp1  = pressureSource->SampleExplicit(X, Y, Z, 0 );
    }
            //break;
            //outChunk->chunkData[dataIndex] -= (Pp1 - Pm1)*0.000001;
    else if (channel == 2){


            Pm1  = pressureSource->SampleExplicit(X, Y, Z-1.0f, 0 );
            Pp1  = pressureSource->SampleExplicit(X, Y, Z, 0 );
    }
            //outChunk->chunkData[dataIndex] -= (Pp1 - Pm1)*0.000001;

            //break;
       // default:
         //   Pm1  = 0;
           // Pp1  = 0;
            //break;


    float diff = (Pp1 - Pm1);
    float oldVel = outChunk->chunkData[dataIndex];
//    if (diff > 0.001)
//        cout << diff << endl;

    outChunk->chunkData[dataIndex] = oldVel - diff;




}


GridProjection::~GridProjection()
{

}


void GridProjection::PreGridOp()
{

    //set the source channel
    uint32_t pressureSourceIndex =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("pressure"));
    //set the target channel
    uint32_t velocitySource =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("velocity"));

    pressureSource = gridObjectPtr->channelObjs[pressureSourceIndex].get(); // default first one

    currentSourceChannelObject =
        gridObjectPtr->channelObjs[velocitySource].get(); // default first one
    currentTargetChannelObject =
        gridObjectPtr->channelObjs[velocitySource].get(); // want vel channel.



}


