#include "GridEmitter.h"
#include "Chunk.h"
#include "ChannelObject.h"
#include "glm/vec3.hpp"
#include "glm/trigonometric.hpp"


//#include <omp.h>

inline static float map_range(float value, float low1, float high1, float low2, float high2) {
    return low2 + (high2 - low2) * (value - low1) / (high1 - low1);
}


//----------------------------------------------
GridEmitter::~GridEmitter()
{


}

void GridEmitter::setupDefaults()
{
    createChunks = true;
    std::string name = "sphereEmitter";
    forceInputBoundsIteration = true;
    //auto emitterSphere = make_shared<ImplicitTorus>(name);
    auto emitterSphere = make_shared<ImplicitSphere>(name);
    //callPreChunkOp = true;
    sourceVolume = emitterSphere;
    //work out chunks to process based on implicit bounding box
//    int minX = glm::floor(sourceVolume->boundingBox.min.x/chnkSize);
//    int minY = glm::floor(sourceVolume->boundingBox.min.y/chnkSize);
//    int minZ = glm::floor(sourceVolume->boundingBox.min.z/chnkSize);
//    int maxX = glm::floor(sourceVolume->boundingBox.max.x/chnkSize);
//    int maxY = glm::floor(sourceVolume->boundingBox.max.y/chnkSize);
//    int maxZ = glm::floor(sourceVolume->boundingBox.max.z/chnkSize);
//    int minX =  glm::floor(sourceVolume->boundingBox.min.x/chnkSize);
//    int minY =  glm::floor(sourceVolume->boundingBox.min.y/chnkSize);
//    int minZ =  glm::floor(sourceVolume->boundingBox.min.z/chnkSize);
//    int maxX =  glm::ceil(sourceVolume->boundingBox.max.x/chnkSize);
//    int maxY =  glm::ceil(sourceVolume->boundingBox.max.y/chnkSize);
//    int maxZ =  glm::ceil(sourceVolume->boundingBox.max.z/chnkSize);
    this->boundingBox.min = sourceVolume->boundingBox.min;
    this->boundingBox.max = sourceVolume->boundingBox.max;

    float intPartMinX;
    float intPartMinY;
    float intPartMinZ;
    float intPartMaxX;
    float intPartMaxY;
    float intPartMaxZ;

    //sep sphere bounds into frac and int
    float minX_a = std::modf(this->boundingBox.min.x, &intPartMinX);
    float minY_a = std::modf(this->boundingBox.min.y, &intPartMinY);
    float minZ_a = std::modf(this->boundingBox.min.z, &intPartMinZ);

    float maxX_a = std::modf(this->boundingBox.max.x, &intPartMaxX);
    float maxY_a = std::modf(this->boundingBox.max.y, &intPartMaxY);
    float maxZ_a = std::modf(this->boundingBox.max.z, &intPartMaxZ);

    cout << "inpart is " << intPartMinX << " - " << intPartMaxX <<endl;
    cout << "floatPart is " << minX_a << " - " << maxX_a <<endl;




    int minX =  glm::floor(sourceVolume->boundingBox.min.x/chnkSize);//only which chunk ids should exists
    int minY =  glm::floor(sourceVolume->boundingBox.min.y/chnkSize);
    int minZ =  glm::floor(sourceVolume->boundingBox.min.z/chnkSize);
    int maxX =  glm::floor((sourceVolume->boundingBox.max.x)/chnkSize);
    int maxY =  glm::floor((sourceVolume->boundingBox.max.y)/chnkSize);
    int maxZ =  glm::floor((sourceVolume->boundingBox.max.z)/chnkSize);


    this->boundingBox.fluidMin = glm::i32vec3(minX, minY, minZ);
    this->boundingBox.fluidMax = glm::i32vec3(maxX, maxY, maxZ);
    cout << "emitter bounds is" << this->boundingBox.min.x << " to " << this->boundingBox.max.x << endl;
    cout << "emitter bounds is" << this->boundingBox.min.y << " to " << this->boundingBox.max.y << endl;
    cout << "emitter bounds is" << this->boundingBox.min.z << " to " << this->boundingBox.max.z << endl;

    cout << "grid emitter bounds set" << this->boundingBox.fluidMin.x << " to " << this->boundingBox.fluidMax.x << endl;
    cout << "grid emitter bounds set" << this->boundingBox.fluidMin.y << " to " << this->boundingBox.fluidMax.y << endl;
    cout << "grid emitter bounds set" << this->boundingBox.fluidMin.z << " to " << this->boundingBox.fluidMax.z << endl;


}

//----------------------------------------------
void GridEmitter::Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition, Chunk* inChunk, Chunk* outChunk, uint32_t dataIndex, uint32_t channel)

{
    //myString <<  "and data is " << inChunk->chunkData[chunkDataIndex] << endl;
    //inChunk->chunkData[chunkDataIndex] = 5;
    //myString << " and data is " << inChunk->chunkData[chunkDataIndex] << endl;
    //float sample = currentSourceChannelObject->SampleChannelAtPosition(500.5,0.0,0.0000);
    //myString << "in algo" << endl;
//    float sample = sourceVolume->sampleVolume(glm::vec3((chunkId.x&(chnkSize-1))+voxelPosition.x,//this mod math can be sped up once working with bitwise. because 4 and  are power of two we could do & instead of %
//                                                        (chunkId.y&(chnkSize-1))+voxelPosition.y,
//                                                        (chunkId.z&(chnkSize-1))+voxelPosition.z) );
//    float sample = sourceVolume->sampleVolume(glm::vec3((chunkId.x%(chnkSize))+voxelPosition.x,//this mod math can be sped up once working with bitwise. because 4 and  are power of two we could do & instead of %
//                                                        (chunkId.y%(chnkSize))+voxelPosition.y,
//                                                        (chunkId.z%(chnkSize))+voxelPosition.z) );
    //cout << "in chunk algo" << chunkId.x << " " << chunkId.y << " " << chunkId.z << endl;

    //cout << voxelPosition.x << ", ";
//    float X = ((chunkId.x*(int)chnkSize)+voxelPosition.x);

//    float Y = ((chunkId.y*(int)chnkSize)+voxelPosition.y);

//    float Z = ((chunkId.z*(int)chnkSize)+voxelPosition.z);

    float X = ((chunkId.x*(int)chnkSize)+voxelPosition.x);

    float Y = ((chunkId.y*(int)chnkSize)+voxelPosition.y);

    float Z = ((chunkId.z*(int)chnkSize)+voxelPosition.z);
    if (X < -32){
        //cout << "about to sample" << X << " " << Y << " " << Z  << endl;

    }

    //float sampleA = sourceVolume->sampleVolume(glm::vec3(X, Y, Z));
    float sample = sourceVolume->sampleVolume(glm::vec3(X+0.5f, Y+0.5f, Z+0.5f));




    if(sample < 4.0f){
        if (!currentTargetChannelObject->ChunkExists(chunkId.x, chunkId.y, chunkId.z))
            {
            //do checks to emit
            //cout << "chunk id is " << chunkId.x <<  " " << chunkId.y << " " << chunkId.z << endl;

//            if ( chunkId.x == 2 && chunkId.y == 2 && chunkId.z == 2){
//                            //cout << "222 is being called "  << endl;
//                            cout << "222 exists? " << boolalpha << currentSourceChannelObject->ChunkExists(chunkId.x, chunkId.y, chunkId.z) << endl;
//            }
            outChunk = currentTargetChannelObject->CreateChunk(chunkId.x,chunkId.y,chunkId.z);
            //inChunk->chunkData[dataIndex] = 0;
            //myString << "empty ptr can creat if I want" << endl;

//

            }


        else
        {
//                if ( chunkId.x == 2 && chunkId.y == 2 && chunkId.z == 2){
//                                //cout << "222 is being called "  << endl;
//                                cout << "222 exists? " << boolalpha << currentSourceChannelObject->ChunkExists(chunkId.x, chunkId.y, chunkId.z) << endl;
//                }
            //myString << "getting chunk" << endl;
            outChunk = currentTargetChannelObject->GetChunk(chunkId.x, chunkId.y, chunkId.z);

        }


        //can safely assume chunk exists?
        //if (inChunk == currentSourceChannelObject->dummyChunk)
                //cout << "erm DUMMY!" << endl;
            //myString << "setting valuefor chunk " << chunkId.x << chunkId.y << chunkId.z << voxelPosition.x << voxelPosition.y << voxelPosition.z <<endl;
        if(sample < 0.0f){
        //map_range(sample, -16.0f, -1.0f, 1.0f, 0.1f)

            //map_range(sample, -32.0f, 0.0f, value*0.08, 0.0f)
            //float value = glm::max(glm::cos(currentTime*3), 0.0);
            if (currentTime < 0.08){
                outChunk->chunkData[dataIndex] += 1.0f;
            }
            else{
                outChunk->chunkData[dataIndex] += 0.0f;

            }
            //inChunk->chunkData[dataIndex] = addPositiveDifference(inChunk->chunkData[dataIndex],0.2  );
        }

        //else
            //inChunk->chunkData[dataIndex] += 0.0f;

        //inChunk->chunkData[dataIndex] = chunkId.x;

    }



    //myString << "running" << endl;

    //write code to emit into grid


    //int threadid  = omp_get_thread_num();
    //myString << voxelPosition.x<< " " << voxelPosition.y << " " <<voxelPosition.z << endl;
    //myString << sample << " thread: " << omp_get_thread_num() << endl;
    //myString << currentSourceChannelObject->channelInfo.channelName << endl;


}

float GridEmitter::addPositiveDifference(float inputReference, float amountToAdd)
{
    return inputReference+(amountToAdd-inputReference);
}

//void GridEmitter::PreChunkOp(Chunk *inChunk, glm::i32vec3 chunkIdSecondary)
//{
//    //here we do a presample of emission volume once per bucket. simple way to stop race condition if we were to do it per voxel.


//}


