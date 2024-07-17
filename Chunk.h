#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <glm/glm.hpp>
#include <array>
#include "FastNoiseLite.h"

class Chunk {
public:
    Chunk(int size, const glm::vec3& position);
    Chunk();

    void initializeChunk(FastNoiseLite& noise);
    std::vector<glm::vec3> getVoxelWorldPositions() const;
    void addVoxel(const glm::vec3& pos);
    const std::vector<std::vector<std::vector<bool>>>& getChunkBlocks() const;
    const std::vector<glm::vec3>& getVoxelPositions() const;

private:
    int size;
    std::vector<glm::vec3> voxelPositions;
	glm::vec3 position;// 区块的位置(世界坐标)
    std::vector<std::vector<std::vector<bool>>> chunkBlocks;
};

#endif // CHUNK_H
