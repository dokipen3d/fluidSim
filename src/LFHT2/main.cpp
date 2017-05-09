#include "LockFreeHashTable.hpp"
#include <future>
#include <iostream>

class Chunk {

    int id = 0;

public:
    Chunk(int id) : id(id) { std::cout << id << "\n"; }
    void printSomething()
    {
        std::cout << id << std::endl;
    }
};

using namespace std;

int main()
{

    LockFreeHashTable<Chunk> ChunkHT(8192);

    auto ft1 = std::async(std::launch::async, &LockFreeHashTable<Chunk>::IfExistsReturnItElseSetIt, &ChunkHT, 32);
    auto ft2 = std::async(std::launch::async, &LockFreeHashTable<Chunk>::IfExistsReturnItElseSetIt, &ChunkHT, 33);
    auto ft3 = std::async(std::launch::async, &LockFreeHashTable<Chunk>::IfExistsReturnItElseSetIt, &ChunkHT, 33);
    auto ft4 = std::async(std::launch::async, &LockFreeHashTable<Chunk>::IfExistsReturnItElseSetIt, &ChunkHT, 35);

    Chunk* newChunk = ft1.get();
    Chunk* newChunk2 = ft2.get();
    Chunk* newChunk3 = ft3.get();
    Chunk* newChunk4 = ft4.get();

    if (newChunk) {
        newChunk->printSomething();
    }
    if (newChunk2) {
        newChunk2->printSomething();
    }
    if (newChunk3) {
        newChunk3->printSomething();
    }
    else{
        cout << "BOO" << endl;
    }
    if (newChunk4) {
        newChunk4->printSomething();
    }
}