#include "GridPadCull.h"
#include "Chunk.h"
#include "ChannelObject.h"
#include "GridObject.h"
#include <atomic>
#include "glm/vec3.hpp"
#include "glm/gtc/type_precision.hpp"

#include <omp.h>

void GridPadCull::setupDefaults() {
  // boundingBox = gridObjectPtr->boundingBox;

  callGridOp = true;
  callPostChunkOp = true;
}

void GridPadCull::Algorithm(glm::i32vec3 chunkId,
                            glm::i32vec3 voxelLocalPosition, Chunk *inChunk,
                            Chunk *outChunk, u_int32_t dataIndex,
                            uint32_t channel) {
  bool lValueFalse = false;
  bool lValueTrue = true;

  if (outChunk->chunkData[dataIndex] > 0.008) {

    // if (inChunk->empty.compare_exchange_strong(lValueTrue, false)){
    outChunk->empty = false;
    //{
    // myString << "chunk not empty" << endl;

    //

    //                         }

    //                    }
    //               }
    //}

  } else {
    //#pragma omp critical

    //        while(true){

    //            int store = inChunk->voxelCount;

    //            if(inChunk->voxelCount.compare_exchange_strong(store,
    //            store-1)){
    //                break;
    //            }

    //        }
    outChunk->voxelCount--;
  }
}

void GridPadCull::PostChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                              glm::i32vec3 chunkIdSecondary) {

  bool padded = false;

  // cout << "voxCount = " << inChunk->voxelCount << endl;

  if (outChunk->voxelCount == 0) {
    // cout << "vox empty" << endl;
    outChunk->empty = true;
    //

  } else {
    outChunk->timeCreated = gridObjectPtr->simTime;
    //#pragma omp parrallel for collapse(3)
    //#pragma omp critical
    {
      for (int i = -1 * chunksToPad; i <= chunksToPad; i++) {
        for (int j = -1 * chunksToPad; j <= chunksToPad; j++) {
          for (int k = -1 * chunksToPad; k <= chunksToPad; k++) {

            // Chunk* padChunk =
            // currentSourceChannelObject->GetChunk(chunkIdSecondary.x+i,
            // chunkIdSecondary.y+j, chunkIdSecondary.z+k);
            if (currentTargetChannelObject->ChunkExists(
                    (int)chunkIdSecondary.x + i, (int)chunkIdSecondary.y + j,
                    (int)chunkIdSecondary.z + k)) {
              //#pragma omp critical
              currentTargetChannelObject->GetChunk((int)chunkIdSecondary.x + i,
                                                   (int)chunkIdSecondary.y + j,
                                                   (int)chunkIdSecondary.z + k)
                  ->okayToDelete = false;

              // break;
              // currentSourceChannelObject->CreateChunk(chunkId.x+i,chunkId.y+j,chunkId.z+k);
            } else
            //#pragma omp barrier
            {
              // myString << chunkIdSecondary.z+k << " doesnt exists" << endl;
              currentTargetChannelObject->CreateChunk(
                  (int)chunkIdSecondary.x + i, (int)chunkIdSecondary.y + j,
                  (int)chunkIdSecondary.z + k);

              padded = true;
            }
          }
        }
      }
    }
    //#pragma omp barrier
    //        for (int i = 1; i <= chunksToPad; i++){
    //            for (int j = 1; j <= chunksToPad; j++){
    //                for (int k = 1; k <= chunksToPad; k++){

    //                    //Chunk* padChunk =
    //                    currentSourceChannelObject->GetChunk(chunkIdSecondary.x+i,
    //                    chunkIdSecondary.y+j, chunkIdSecondary.z+k);
    //                    if
    //                    (currentSourceChannelObject->ChunkExists(chunkIdSecondary.x-i,
    //                    chunkIdSecondary.y-j, chunkIdSecondary.z-k)){
    //                        //#pragma omp critical
    //                        currentSourceChannelObject->GetChunk(chunkIdSecondary.x-i,
    //                        chunkIdSecondary.y-j,
    //                        chunkIdSecondary.z-k)->okayToDelete = false;

    //                        //break;
    //                         //currentSourceChannelObject->CreateChunk(chunkId.x+i,chunkId.y+j,chunkId.z+k);
    //                    }
    //                    else{
    //                    //myString << chunkIdSecondary.z+k << " doesnt exists"
    //                    << endl;
    //                    currentSourceChannelObject->CreateChunk(chunkIdSecondary.x-i,chunkIdSecondary.y-j,chunkIdSecondary.z-k);

    //                     padded = true;
    //                    }
    //                 }
    //            }
    //        }
  }

  //    if (inChunk->justCreatedOneFrameAgo){
  //        inChunk->justCreatedOneFrameAgo = false;
  //    }

  // double timeNow = omp_get_wtime();

  if ((gridObjectPtr->simTime - outChunk->timeCreated) > 1.5) {
    outChunk->okayToDelete = true;
  }

  if ((outChunk->empty == true && outChunk->okayToDelete == true)) {

    currentTargetChannelObject->DeleteChunk(
        chunkIdSecondary.x, chunkIdSecondary.y, chunkIdSecondary.z);
    // cout << "deleted chunk " << chunkIdSecondary.x << endl;

  }

  else {
    if (!padded) {
      boundingBox.fluidMin.x =
          glm::min(chunkIdSecondary.x, boundingBox.fluidMin.x);
      boundingBox.fluidMin.y =
          glm::min(chunkIdSecondary.y, boundingBox.fluidMin.y);
      boundingBox.fluidMin.z =
          glm::min(chunkIdSecondary.z, boundingBox.fluidMin.z);
      boundingBox.fluidMax.x =
          glm::max(chunkIdSecondary.x, boundingBox.fluidMax.x);
      boundingBox.fluidMax.y =
          glm::max(chunkIdSecondary.y, boundingBox.fluidMax.y);
      boundingBox.fluidMax.z =
          glm::max(chunkIdSecondary.z, boundingBox.fluidMax.z);
    } else {

      boundingBox.fluidMin.x =
          glm::min(chunkIdSecondary.x - chunksToPad, boundingBox.fluidMin.x);
      boundingBox.fluidMin.y =
          glm::min(chunkIdSecondary.y - chunksToPad, boundingBox.fluidMin.y);
      boundingBox.fluidMin.z =
          glm::min(chunkIdSecondary.z - chunksToPad, boundingBox.fluidMin.z);
      boundingBox.fluidMax.x =
          glm::max(chunkIdSecondary.x + chunksToPad, boundingBox.fluidMax.x);
      boundingBox.fluidMax.y =
          glm::max(chunkIdSecondary.y + chunksToPad, boundingBox.fluidMax.y);
      boundingBox.fluidMax.z =
          glm::max(chunkIdSecondary.z + chunksToPad, boundingBox.fluidMax.z);
    }

    int pcs = gridObjectPtr->chunkSize;
    outChunk->voxelCount = pcs * pcs * pcs;
  }
}

void GridPadCull::GridOp() {

  // myString << " min is " <<boundingBox.fluidMin.x << endl;
  gridObjectPtr->boundingBox = boundingBox;
  gridObjectPtr->unifyBounds();
  boundingBox = BoundingBox();
}
