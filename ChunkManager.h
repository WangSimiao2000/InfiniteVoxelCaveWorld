#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <mutex>
#include "Chunk.h"

class ChunkManager {
public:
    ChunkManager(int chunkSize);
    void update(const glm::vec3& cameraPosition);
    void clearChunks(); // Clear loaded chunks
    void stopLoading(); // Stop loading chunks (useful for pausing chunk loading)
    void startLoading(); // Start loading chunks (useful for resuming chunk loading)    
    void setNoiseWeights(float weight1, float weight2); // Set noise weights
    void setViewDistance(int distance); // Set view distance
    void clearChunksFolder(); // Clear the chunks folder
    bool getIsLoading() const;
    std::unordered_map<std::string, Chunk>& getChunks();

private:
    std::mutex chunksMutex; // Mutex lock
    int chunkSize; // Size of each chunk
    glm::vec3 lastCameraPosition; // Camera position in the previous frame
    FastNoiseLite noise1; // First noise generator
    FastNoiseLite noise2; // Second noise generator
    float weight1 = 1.0f; // Weight for the first noise
    float weight2 = 0.0f; // Weight for the second noise
    float THRESHOLD = 0.3f; // Threshold value
    int SEED = 1234; // Random seed
    bool isLoading = false; // Flag indicating whether chunks are being loaded
    std::unordered_map<std::string, Chunk> chunks;
    int viewDistance = 2; // View distance, measured in chunks, with the camera at the center of a square region. The side length is 2*2+1, including the current chunk plus two chunks extending in each direction

    std::string getChunkKey(const glm::vec3& position);
    void loadChunk(const glm::vec3& position);
    void saveChunkToFile(const Chunk& chunk, const std::string& filename);
};

#endif // CHUNK_MANAGER_H
