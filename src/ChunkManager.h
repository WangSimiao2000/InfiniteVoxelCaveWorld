#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <atomic>
#include <functional>
#include <memory>
#include "Chunk.h"

class ChunkManager {
public:
    ChunkManager(int chunkSize, const std::string& baseDir);
    void update(const glm::vec3& cameraPosition);
    void clearChunks(); // Clear loaded chunks
    void stopLoading(); // Stop loading chunks (useful for pausing chunk loading)
    void startLoading(); // Start loading chunks (useful for resuming chunk loading)    
    void setNoiseWeights(float weight1, float weight2); // Set noise weights
    void setViewDistance(int distance); // Set view distance
    void clearChunksFolder(); // Clear the chunks folder
    bool getIsLoading() const;
    size_t getChunkCount();

    // 在锁内遍历所有区块，回调函数中可安全访问 Chunk
    // Iterate all chunks under lock, callback can safely access Chunk
    void forEachChunk(const std::function<void(const std::string&, Chunk&)>& fn);

private:
    std::mutex chunksMutex; // Mutex lock
    std::string baseDir; // Base directory for chunk files
    int chunkSize; // Size of each chunk
    glm::vec3 lastCameraPosition; // Camera position in the previous frame
    FastNoiseLite noise1; // First noise generator
    FastNoiseLite noise2; // Second noise generator
    float weight1 = 1.0f; // Weight for the first noise
    float weight2 = 0.0f; // Weight for the second noise
    float THRESHOLD = 0.3f; // Threshold value
    int SEED = 1234; // Random seed
    std::atomic<bool> isLoading{false}; // Flag indicating whether chunks are being loaded
    std::unordered_map<std::string, std::shared_ptr<Chunk>> chunks;
    int viewDistance = 2;

    std::string getChunkKey(const glm::vec3& position);
    void loadChunk(const glm::vec3& position);
    void saveChunkToFile(const Chunk& chunk, const std::string& filename);
    bool isVoxelAtWorld(int wx, int wy, int wz);
};

#endif // CHUNK_MANAGER_H
