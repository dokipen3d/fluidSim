#include "LockFreeHashTable.h"
#include <assert.h>
#include <memory.h>
#include <atomic>
#include <iostream>
#include <stdint.h>
#include <sstream>

using namespace std;

//----------------------------------------------
// from code.google.com/p/smhasher/wiki/MurmurHash3
inline static uint32_t integerHash(uint32_t h) {
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}


//---------------------------------}

// inline static uint32_t integerHash(uint64_t h)
//{
//    (h) ^= (h) >> 23;               \
//    (h) *= 0x2127599bf4325c37ULL;   \
//    (h) ^= (h) >> 47;
//    return h - (h >> 32);

//}

//----------------------------------------------
LockFreeHashTable::LockFreeHashTable(uint32_t arraySize) {
  // Initialize cells
  //assert((arraySize & (arraySize - 1)) == 0); // Must be a power of 2
  m_arraySize = arraySize;
  m_entries = new Entry[arraySize];
  Clear();
}

//----------------------------------------------
LockFreeHashTable::~LockFreeHashTable() {
  // Delete cells
  delete[] m_entries;
}

//----------------------------------------------

//have this return a bool if the item allready exists. can then set a pointer from channel object and alocate after
void LockFreeHashTable::SetItem(uint32_t key, uintptr_t value) {
  //assert(key != 0);
  //assert(value != 0);

  uint32_t idx = integerHash(key);
  // cout << idx << endl;

  while (true) {
    idx &= m_arraySize - 1;

    uint32_t prevKey = m_entries[idx].key.load(std::memory_order_relaxed);

    // if (prevKey != key)
    //  break;

    if ((prevKey == 0) &&
        (m_entries[idx].key.compare_exchange_weak(zero, key)) || (prevKey == key)) {
      itemCount++;
          uintptr_t oldVal = m_entries[idx].value.load(std::memory_order_relaxed);
          if (m_entries[idx].value.compare_exchange_weak(oldVal, value)) {
            // cout << "it worked" << "with val " << value << std::endl;
            //here is where we would return true to channel object and say its okay for init?
            //delete old?
          } else {
            // Transition failed; oldval has changed
            // We can act “as if” our put() worked but
            // was immediately stomped over
          }

      break;
    }
    if (prevKey == key) {
      break;
    }

    // cout << "clash? " << key << endl;

    idx++;
  }


}

//----------------------------------------------
uintptr_t LockFreeHashTable::GetItem(uint32_t key) {
  //assert(key != 0);

  uint32_t idx = integerHash(key);
  // stringstream ss;
  // ss<< idx <<endl;
  // cout << ss.str() <<std::endl;
  while (true) {
    idx &= m_arraySize - 1;

    uint32_t probedKey = m_entries[idx].key.load(std::memory_order_relaxed);
    // uint32_t probedKey = m_entries[idx].key;

    // uint32_t memoizedHash =
    // m_entries[idx].memhash.load(std::memory_order_relaxed);

    // uint32_t probedKey = m_entries[idx].key;

    // if (probedKey == key || (memoizedHash == hash))
    if (probedKey == key)

      // return m_entries[idx].value.load(std::memory_order_relaxed);
      if (m_entries[idx].value) {
        return m_entries[idx].value;
      } else {
        //return (uintptr_t)dummy;
         return reinterpret_cast<uintptr_t>(dummy);
      }

    if (probedKey == 0) {
      // cout << "probed key 0" << std::endl;
      //return (uintptr_t)dummy;
      return reinterpret_cast<uintptr_t>(dummy);

    }
    // return static_cast<uintptr_t>(dummy);

    idx++;
  }
}

void LockFreeHashTable::setDummyItem(
    void *dummyLocation) // make sure its set to what you expect!
{
  dummy = dummyLocation;
}

//-----------------------------------------------
void LockFreeHashTable::ClearKey(uint32_t key) {
  //assert(key != 0);

  uint32_t idx = integerHash(key);

  while (true) {
    idx &= m_arraySize - 1;

    uint32_t probedKey = m_entries[idx].key.load(std::memory_order_relaxed);
    uintptr_t oldVal = m_entries[idx].value.load(std::memory_order_relaxed);

    // && m_entries[idx].value.compare_exchange_strong(oldVal,0)
    if (probedKey == key &&
        m_entries[idx].value.compare_exchange_weak(oldVal, 0)) {
      m_entries[idx].value = 0;
      // m_entries[idx].key.store(0);//set key to 0. make sure item was deleted
      // by owner otherwise data will leak
      break;
    }

    idx++;
  }
}

//-----------------------------------------------
bool LockFreeHashTable::KeyExists(uint32_t key) {
  //assert(key != 0);

  uint32_t idx = integerHash(key);
  // bool searching = true;
  while (true) {
    idx &= m_arraySize - 1;
    // cout << key << " " << idx << endl;

    // uint32_t probedKey = m_entries[idx].key.load(std::memory_order_relaxed);
    uint32_t probedKey = m_entries[idx].key;

    // cout << probedKey << endl;
    //        if (probedKey == 0)
    //        {
    //            return false;

    //        }

    if (probedKey == 0) {
      // searching = false;
      // cout << "nope not here" << endl;
      return false;
    }

    if (probedKey == key) {
      if ((m_entries[idx].value != 0) &&
          m_entries[idx].value != reinterpret_cast<uintptr_t>(dummy))
        return true;
      else
        return false;
    }



    // cout << "searching" << endl;

    idx++;
  }
}

//----------------------------------------------
uint32_t LockFreeHashTable::GetItemCount() {
  // uint32_t itemCount = 0;
  // for (uint32_t idx = 0; idx < m_arraySize; idx++)
  // {
  //     if ((m_entries[idx].key.load(std::memory_order_relaxed) != 0)
  //         && (m_entries[idx].value.load(std::memory_order_relaxed) != 0))
  //         itemCount++;
  // }
  return itemCount;
}

//----------------------------------------------
void LockFreeHashTable::Clear() {
  memset(m_entries, 0, sizeof(Entry) * m_arraySize);
}
