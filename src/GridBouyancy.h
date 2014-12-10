#pragma once

#include "GridOperator.h"
#include <iostream>

using namespace std;

class GridBouyancy : public GridOperator
{
public:
    GridBouyancy(GridObject* inGridObject) : GridOperator(inGridObject){cout << "in GriBouyany Constructor" <<endl;setupDefaults();};

    //custom constructor
    virtual void setupDefaults();
    virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition, Chunk* inChunk, Chunk* outChunk, uint32_t dataIndex, uint32_t channel);
    virtual void PreGridOp();
    virtual void GridOp();



};

//#endif // GRIDDISSIPATOR_H
