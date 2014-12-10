#ifndef GRIDVECTOREMITTER_H
#define GRIDVECTOREMITTER_H

#include <iostream>
#include <string>
using namespace std;


#include "GridOperator.h"
#include "VolumeSource.h"

class Chunk;
class ChannelObject;


class GridVectorEmitter: public GridOperator
{


public:
    GridVectorEmitter(GridObject* inGridObject) : GridOperator(inGridObject) {cout << "in GridEmitter Constructor" <<endl;setupDefaults();};
    virtual ~GridVectorEmitter();
	
    virtual void setupDefaults();
    virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition, Chunk* inChunk, Chunk* outChunk, uint32_t dataIndex, uint32_t channnel);
    //float addPositiveDifference(float inputReference, float amountToAdd);
    //virtual void PreChunkOp(Chunk* inChunk, glm::i32vec3 chunkIdSecondary) override;
    void PreGridOp();
    shared_ptr<VolumeSource> sourceVolume;




}; 


	



#endif
