// ObjectPool.h
#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include <vector>
#include <memory>
#include <algorithm> // For std::find

template <typename T>
class ObjectPool {
private:
    std::vector<std::unique_ptr<T>> pool;
    std::vector<T*> freeObjects; // To quickly find inactive objects
    size_t maxSize;

public:
    std::vector<T*> activeObjects; // Now public and persistent

    ObjectPool(size_t size) : maxSize(size) {
        pool.reserve(maxSize);
        freeObjects.reserve(maxSize);
        activeObjects.reserve(maxSize);

        for (size_t i = 0; i < maxSize; ++i) {
            pool.push_back(std::make_unique<T>());
            freeObjects.push_back(pool.back().get()); // All objects are free initially
        }
        // std::cout << "ObjectPool constructed. Size: " << maxSize << std::endl;
    }

    T* acquire() {
        if (freeObjects.empty()) {
            // std::cout << "ObjectPool acquire: No free objects!" << std::endl;
            return nullptr; // No free objects available
        }

        T* obj = freeObjects.back();
        freeObjects.pop_back();

        obj->active = true;
        activeObjects.push_back(obj);
        // std::cout << "ObjectPool acquire: " << obj << " (active objects: " << activeObjects.size() << ", free objects: " << freeObjects.size() << ")" << std::endl;
        return obj;
    }

    void release(T* obj) { // This method is not used, releaseAt is.
        obj->active = false;
        // Find and remove from activeObjects using swap-remove
        // This is O(N) in worst case, but average is better
        for (size_t i = 0; i < activeObjects.size(); ++i) {
            if (activeObjects[i] == obj) {
                activeObjects[i] = activeObjects.back();
                activeObjects.pop_back();
                break;
            }
        }
        freeObjects.push_back(obj); // Add back to free list
        // std::cout << "ObjectPool release (by obj): " << obj << " (active objects: " << activeObjects.size() << ", free objects: " << freeObjects.size() << ")" << std::endl;
    }

    // New method for efficient release by index, to be used with iterators in GameState
    void releaseAt(size_t index) {
        if (index >= activeObjects.size()) {
            // std::cout << "ObjectPool releaseAt: ERROR - invalid index " << index << " (size: " << activeObjects.size() << ")" << std::endl;
            return;
        }

        T* objToRelease = activeObjects[index];
        objToRelease->active = false;

        // Swap-remove from activeObjects
        activeObjects[index] = activeObjects.back();
        activeObjects.pop_back();

        freeObjects.push_back(objToRelease); // Add back to free list
        // std::cout << "ObjectPool releaseAt: " << objToRelease << " at index " << index << " (active objects: " << activeObjects.size() << ", free objects: " << freeObjects.size() << ")" << std::endl;
    }
};

#endif