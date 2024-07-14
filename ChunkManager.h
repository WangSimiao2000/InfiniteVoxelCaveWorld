#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "Chunk.h"

class ChunkManager {
public:
    ChunkManager(int chunkSize);
    void update(const glm::vec3& cameraPosition);
    std::unordered_map<std::string, Chunk> chunks;

private:
    int chunkSize;
    glm::vec3 lastCameraPosition; // 上一帧的摄像机位置

    std::string getChunkKey(const glm::vec3& position);
    void loadChunk(const glm::vec3& position);
    void unloadChunk(const std::string& key);
    void saveChunkToFile(const Chunk& chunk, const std::string& filename);
    Chunk loadChunkFromFile(const std::string& filename);
};

#endif // CHUNK_MANAGER_H
