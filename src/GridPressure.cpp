 #include "GridPressure.h"
#include "ChannelObject.h"
#include "GridObject.h"





void GridPressure::setupDefaults()
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

void GridPressure::PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
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

void GridPressure::Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelWorldPosition,
                            Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                            uint32_t channel, bool internalAccessible){





//    float X = voxelWorldPosition.x;

//    float Y = voxelWorldPosition.y;

//    float Z = voxelWorldPosition.z;

  //add scale here?

 // float P6 = inChunk->chunkData[dataIndex];
  //float P  = currentSourceChannelObject->SampleExplicit(X, Y, Z, 0 );

    float Pip1JK;
    float Pim1JK;

    float PIjp1K;
    float PIjm1K;

    float PIJkp1;
    float PIJkm1;
    float div;

//  if (internalAccessible){
//        Pip1JK = inChunk->chunkData[dataIndex+1];
//        Pim1JK = inChunk->chunkData[dataIndex-1];

//        PIjp1K = inChunk->chunkData[dataIndex+chnkSize];
//        PIjm1K = inChunk->chunkData[dataIndex-chnkSize];

//        PIJkp1 = inChunk->chunkData[dataIndex+(chnkSize*chnkSize)];
//        PIJkm1 = inChunk->chunkData[dataIndex-(chnkSize*chnkSize)];
//  }

//  else{
//#pragma omp parallel sections
    {

//#pragma omp  section
    {
   Pip1JK = currentSourceChannelObject->SampleExplicit(voxelWorldPosition.x+1, voxelWorldPosition.y, voxelWorldPosition.z, 0 );
        }
  // #pragma omp  section
    {
   Pim1JK = currentSourceChannelObject->SampleExplicit(voxelWorldPosition.x-1, voxelWorldPosition.y, voxelWorldPosition.z, 0 );}
//#pragma omp  section
   {
   PIjp1K = currentSourceChannelObject->SampleExplicit(voxelWorldPosition.x, voxelWorldPosition.y+1, voxelWorldPosition.z, 0 );}
  //      #pragma omp  section
    {
   PIjm1K = currentSourceChannelObject->SampleExplicit(voxelWorldPosition.x, voxelWorldPosition.y-1, voxelWorldPosition.z, 0 );
    }
//#pragma omp  section
    {
   PIJkp1 = currentSourceChannelObject->SampleExplicit(voxelWorldPosition.x, voxelWorldPosition.y, voxelWorldPosition.z+1, 0 );}
  // #pragma omp  section
{
   PIJkm1 = currentSourceChannelObject->SampleExplicit(voxelWorldPosition.x, voxelWorldPosition.y, voxelWorldPosition.z-1, 0 );
    }
   //#pragma omp  section
        {
     div = divergenceSource->SampleExplicit(voxelWorldPosition.x, voxelWorldPosition.y, voxelWorldPosition.z, 0);
        }

}

  //}


  //float pressureVal = ((P*6.0f) - Pip1JK - Pim1JK - PIjp1K - PIjm1K - PIJkp1 - PIJkm1 + div)/scaleSquared;
  //float pressureVal = (Pip1JK + Pim1JK + PIjp1K + PIjm1K + PIJkp1 + PIJkm1 + (div*scaleSquared))/6;

//  float pressureVal = ( (div*scaleSquared) +((Pip1JK - P) - (P - Pim1JK)) +
//                                            ((P - PIjp1K) - (P - PIjm1K)) +
//                                            ((P - PIJkp1) - (P - PIJkm1)) )/6;

//  float pressureVal =   (1.0f/6.0f)*  (
//                        ((P - Pip1JK) - (P - Pim1JK)) +
//                        ((P - PIjp1K) - (P - PIjm1K)) +
//                        ((P - PIJkp1) - (P - PIJkm1)) + (div*scaleSquared)
//                        );

//  float pressureVal =   (1.0f/6.0f)*(// replace the 1/6 with (1 - omega) * P + omega / 6 where omega is SOR value between 1 and 2 (close to 1.7 is best)
//                        (((Pip1JK - P) - (P - Pim1JK))) +                     // (
//                        (((PIjp1K - P) - (P - PIjm1K))) +                     // ((1 - omega) * P) + (omega / 6))
//                        (((PIJkp1 - P) - (P - PIJkm1))) + div)*scaleSquared  // ) is clearer
//                        ;

//  float pressureVal =  ( ((1 - omega) * P) + (omega / 6)) * (// replace the 1/6 with (1 - omega) * P + omega / 6 where omega is SOR value between 1 and 2 (close to 1.7 is best)
//                        ((P - Pip1JK) - (P - Pim1JK)) +                     // (
//                        ((P - PIjp1K) - (P - PIjm1K)) +                     // ((1 - omega) * P) + (omega / 6))
//                        ((P - PIJkp1) - (P - PIJkm1)) + (div*scaleSquared)  // ) is clearer
//                        ) ;

  //outChunk->chunkData[dataIndex] = pressureVal;

  //float pressure = (Pip1JK + Pim1JK + PIjp1K + PIjm1K + PIJkp1 + PIJkm1 + (div*scaleSquared))/6;
    outChunk->chunkData[dataIndex] = (Pip1JK + Pim1JK + PIjp1K + PIjm1K + PIJkp1 + PIJkm1 + (div*scaleSquared))/6;;

  //outChunk->chunkData[dataIndex] = calcPressure(Pip1JK, Pim1JK, PIjp1K, PIjm1K, PIJkp1, PIJkm1, div, 0);

//    if(debug && iteration == numberOfIterations -1){
//        if (X == -5 && Y == 2 && Z == 2 && channel == 0)
//            cout << "pressure at  -5 is " << pressure << endl;
//        if (X == -4 && Y == 2 && Z == 2 && channel == 0)
//            cout << "pressure at  -4 is " << pressure << endl;
//        if (X == -3 && Y == 2 && Z == 2 && channel == 0)
//            cout << "pressure at  -3 is " << pressure << endl;
//        if (X == -2 && Y == 2 && Z == 2 && channel == 0)
//            cout << "pressure at  -2 is " << pressure << endl;
//        if (X == -1 && Y == 2 && Z == 2 && channel == 0)
//            cout << "pressure at  -1 is " << pressure << endl << "--------------------" << endl;
//        if (X ==  0 && Y == 2 && Z == 2 && channel == 0)
//            cout << "pressure at   0 is " << pressure << endl;
//        if (X ==  1 && Y == 2 && Z == 2 && channel == 0)
//            cout << "pressure at   1 is " << pressure << endl;
//        if (X ==  2 && Y == 2 && Z == 2 && channel == 0)
//            cout << "pressure at   2 is " << pressure << endl;
//        if (X ==  3 && Y == 2 && Z == 2 && channel == 0)
//            cout << "pressure at   3 is " << pressure << endl;
//        if (X ==  4 && Y == 2 && Z == 2 && channel == 0)
//            cout << "pressure at   4 is " << pressure << endl;
//  }

}


GridPressure::~GridPressure()
{

}

void GridPressure::PreGridOp()
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

void GridPressure::GridOp() {
  gridObjectPtr->SwapChannelPointers(std::string("pressure"));
}

//#pragma omp declare simd
float GridPressure::calcPressure(float a, float b, float c, float d, float e, float f, float g, float h){

return (a + b + c + d + e + f + (g)*0.166666666666);
}

void GridPressure::PostChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                         glm::i32vec3 chunkIdSecondary){

    std::swap(inChunk, outChunk);


}
