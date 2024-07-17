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
    bool isVoxelAt(const glm::vec3& worldPosition);

private:
    int chunkSize;
    glm::vec3 lastCameraPosition; // 上一帧的摄像机位置
    FastNoiseLite noise; // FastNoiseLite 对象

    std::string getChunkKey(const glm::vec3& position);
    void loadChunk(const glm::vec3& position);
    void saveChunkToFile(const Chunk& chunk, const std::string& filename);
};

#endif // CHUNK_MANAGER_H
