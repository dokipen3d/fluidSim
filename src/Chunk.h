#ifndef CHUNK_H
#define CHUNK_H

#include <iostream>
#include <vector>
#include <memory>
#include <atomic>

using namespace std;



class Chunk{

public:
    Chunk(uint32_t initial_size);
	~Chunk();
	vector<float> chunkData;
    atomic<bool> empty;
    bool okayToDelete;
    bool justCreatedOneFrameAgo;
    atomic<int> voxelCount;
    double timeCreated;







private:

	




};





#endif
