#pragma once

#include <assert.h>
#include <atomic>
#include <iostream>
#include <memory.h>
#include <sstream>
#include <stdint.h>

//----------------------------------------------
// The World's Simplest Lock-Free HashTable
//
// Maps 32-bit integers to 32-bit integers.
// Uses open addressing with linear probing.
// You can call SetItem and GetItem from several threads simultaneously.
// GetItem is wait-free.
// SetItem is lock-free.
// You can't assign any value to key = 0.
// In the m_cells array, key = 0 is reserved to indicate an unused cell.
// You can't assign value = 0 to any key.
// value = 0 means the key is unused.
// The hash table never grows in size.
// You can't delete individual items from the hash table.
// You can Clear the hash table, but only at a time when there are no other
// calls being made from other threads.
//----------------------------------------------

//----------------------------------------------
// from code.google.com/p/smhasher/wiki/MurmurHash3
inline static uint32_t integerHash(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

template <typename T>
class LockFreeHashTable {
public:
    struct Entry {
        std::atomic<uint32_t> key;
        std::atomic<T*> value;
    };

private:
    Entry* m_entries;
    uint32_t m_arraySize;
    uint32_t zero = 0;
    uint32_t itemCount = 0;
    void* dummy;

public:
    LockFreeHashTable(uint32_t arraySize)
    {
        // Initialize cells
        //assert((arraySize & (arraySize - 1)) == 0); // Must be a power of 2
        m_arraySize = arraySize;
        m_entries = new Entry[arraySize];
        Clear();
    }

    ~LockFreeHashTable() {}

    // Basic operations

    void Clear()
    {
        memset(m_entries, 0, sizeof(Entry) * m_arraySize);
    }

    T* IfExistsReturnItElseSetIt(uint32_t key)
    {
        uint32_t idx = integerHash(key);

        while (true) {
            idx &= m_arraySize - 1;

            uint32_t prevKey = m_entries[idx].key.load(std::memory_order_relaxed);

            

            if ((prevKey == 0) && (m_entries[idx].key.compare_exchange_weak(zero, key))) {
                itemCount++;
                T* oldVal = new T(key);
                m_entries[idx].value = oldVal;
                // if (m_entries[idx].value.compare_exchange_weak(oldVal, value)) {
                //     return oldVal
                // }
                // break;
                return oldVal;
            }
            //1) If it exists return it
            //if (uint32_t prevKey = m_entries[idx].key.fetch_add(1, std::memory_order_relaxed) == key + 1) { // only this thread could have got that!
            if (prevKey == key) {
                std::cout << "HT:" << key << std::endl;
                //return m_entries[idx].value.load();
                return m_entries[idx].value;

            } 
            idx++; //linear probe
        } //end while true
    }
};
