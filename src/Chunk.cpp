#include "Chunk.h"
#include <iostream>
#include <sstream>






//----------------------------------------------
Chunk::Chunk(uint32_t initial_size)
{
    chunkData.resize(initial_size);
    //chunkData.reserve(initial_size * initial_size * initial_size);

    std::fill(chunkData.begin(), chunkData.end(),0.0f);





}

//----------------------------------------------
Chunk::~Chunk()
{
    //chunkData.clear();
	//cout << "byee" << endl;




}
