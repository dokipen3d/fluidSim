#include "GridPressureRBGS.h"
#include "ChannelObject.h"
#include "GridObject.h"





void GridPressureRBGS::setupDefaults()
{
    //set the source channel
    uint32_t divergenceSourceChannel =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("divergence"));
    //set the target channel
    uint32_t pressureTarget =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("pressure"));

    divergenceSource = gridObjectPtr->channelObjs[divergenceSourceChannel].get(); // default first one

//    currentSourceChannelObject =
//        gridObjectPtr->channelObjs[pressureTarget+1].get(); // default first one
    currentTargetChannelObject =
        gridObjectPtr->channelObjs[pressureTarget].get(); // want vel channel.

    callGridOp = true;
    callPreChunkOp = true;
    scale = gridObjectPtr->dx;
    scaleSquared = -(scale*scale);
    numberOfIterations = 80;
    skipAmount =  2;


}

void GridPressureRBGS::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                              glm::i32vec3 chunkIdSecondary) {

    //start with a pressure value of 0;
    std::fill(outChunk->chunkData.begin(),outChunk->chunkData.end(), 0.0f);
    callPreChunkOp = false;//not after first iteration
}

void GridPressureRBGS::Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition,
                            Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                            uint32_t channel){





  float X = ((chunkId.x * (int)chnkSize) + voxelPosition.x);
  float Y = ((chunkId.y * (int)chnkSize) + voxelPosition.y);
  float Z = ((chunkId.z * (int)chnkSize) + voxelPosition.z);

  //add world scale here?

 // float P6 = inChunk->chunkData[dataIndex];
  float P  = currentTargetChannelObject->SampleExplicit(X, Y, Z, 0 );

  float Pip1JK = currentTargetChannelObject->SampleExplicit(X+1, Y, Z, 0 );
  float Pim1JK = currentTargetChannelObject->SampleExplicit(X-1, Y, Z, 0 );

  float PIjp1K = currentTargetChannelObject->SampleExplicit(X, Y+1, Z, 0 );
  float PIjm1K = currentTargetChannelObject->SampleExplicit(X, Y-1, Z, 0 );

  float PIJkp1 = currentTargetChannelObject->SampleExplicit(X, Y, Z+1, 0 );
  float PIJkm1 = currentTargetChannelObject->SampleExplicit(X, Y, Z-1, 0 );

  float div = divergenceSource->SampleExplicit(X, Y, Z, 0);

  //float pressureVal = (P6 - Pip1JK - Pim1JK - PIjp1K - PIjm1K - PIJkp1 - PIJkm1 + div) * scaleSquared;
//  float pressureVal = ( (div*scaleSquared) +((Pip1JK - P) - (P - Pim1JK)) +
//                                            ((P - PIjp1K) - (P - PIjm1K)) +
//                                            ((P - PIJkp1) - (P - PIJkm1)) )/6;

//  float pressureVal =   (1.0f/6.0f)*  (
//                        ((Pip1JK - P) - (P - Pim1JK)) +
//                        ((P - PIjp1K) - (P - PIjm1K)) +
//                        ((P - PIJkp1) - (P - PIJkm1)) + (div*scaleSquared)
//                        );

  float pressureVal =  (1.0f/6.0f) * (// replace the 1/6 with (1 - omega) * P + omega / 6 where omega is SOR value between 1 and 2 (close to 1.7 is best)
                        ((P - Pip1JK) - (P - Pim1JK)) +                     // (
                        ((P - PIjp1K) - (P - PIjm1K)) +                    // ((1 - omega) * P) + (omega / 6))
                        ((P - PIJkp1) - (P - PIJkm1)) + (div*scaleSquared)  // ) is clearer
                        ) ;

//  float pressureVal =  ( ((1 - omega) * P) + (omega / 6)) * (// replace the 1/6 with (1 - omega) * P + omega / 6 where omega is SOR value between 1 and 2 (close to 1.7 is best)
//                        ((P - Pip1JK) - (P - Pim1JK)) +                     // (
//                        ((P - PIjp1K) - (P - PIjm1K)) +                     // ((1 - omega) * P) + (omega / 6))
//                        ((P - PIJkp1) - (P - PIJkm1)) + (div*scaleSquared)  // ) is clearer
//                        ) ;

  outChunk->chunkData[dataIndex] = pressureVal;



}


GridPressureRBGS::~GridPressureRBGS()
{

}

void GridPressureRBGS::PreGridOp()
{

    //set the source channel
    uint32_t divergenceSourceChannel =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("divergence"));
    //set the target channel
    uint32_t pressureTarget =
        gridObjectPtr->GetMemoryIndexForChannelName(std::string("pressure"));

    divergenceSource = gridObjectPtr->channelObjs[divergenceSourceChannel].get(); // default first one

    currentSourceChannelObject =
        gridObjectPtr->channelObjs[pressureTarget+1].get(); // default first one
    currentTargetChannelObject =
        gridObjectPtr->channelObjs[pressureTarget].get(); // want  channel.


}

void GridPressureRBGS::GridOp() {
  //gridObjectPtr->SwapChannelPointers(std::string("pressure")); //dont swap with red black
    startVoxel = 1 - startVoxel;
}

