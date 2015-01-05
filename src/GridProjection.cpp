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

    float P;
    float Pp1;



    if (channel == 0) {
            Pm1 = pressureSource->SampleExplicit(X-1, Y, Z, 0 );

            P  = pressureSource->SampleExplicit(X, Y, Z, 0 );
            //Pp1  = pressureSource->SampleExplicit(X+1, Y, Z, 0 );

//            float grad = (P - Pm1);
//           // float gradalt = (Pm1 - Pmm1);
//            float grad2 = (Pp1 - P);
//            if (X == 2 && Y == 2 && Z == 2)
//                cout << "grad at  2 is " << grad << " " << grad2 << endl;
//            if (X == -3 && Y == 2 && Z == 2)
//                cout << "grad at -3 is " << grad << " " << grad2 << endl;
            //outChunk->chunkData[dataIndex] -= (Pp1 - Pmm1);

            ////outChunk->chunkData[dataIndex] -= (Pp1 - Pm1)*0.000001;
            //cout << "in channel 0 " << Pm1 << " " << Pp1 << endl;
            //break;
    }
    else if (channel == 1){


            Pm1  = pressureSource->SampleExplicit(X, Y-1, Z, 0 );
            P  = pressureSource->SampleExplicit(X, Y, Z, 0 );
            //outChunk->chunkData[dataIndex] -= (Pp1 - Pm1);

    }
            //break;
            //outChunk->chunkData[dataIndex] -= (Pp1 - Pm1)*0.000001;
    else if (channel == 2){


            Pm1  = pressureSource->SampleExplicit(X, Y, Z-1, 0 );
            P  = pressureSource->SampleExplicit(X, Y, Z, 0 );
            //outChunk->chunkData[dataIndex] -= (Pp1 - Pm1);

    }
            //outChunk->chunkData[dataIndex] -= (Pp1 - Pm1)*0.000001;

            //break;
       // default:
         //   Pm1  = 0;
           // Pp1  = 0;
            //break;


    //float diff = (Pp1 - Pm1);
    //float oldVel = outChunk->chunkData[dataIndex];
//    if (diff > 0.001)
//        cout << diff << endl;

    outChunk->chunkData[dataIndex] -= (P - Pm1);




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


