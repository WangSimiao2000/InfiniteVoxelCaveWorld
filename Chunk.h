#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <glm/glm.hpp>
#include <array>
#include "FastNoiseLite.h"

// 面的枚举类型
enum Face {
    FRONT_FACE,
    BACK_FACE,
    TOP_FACE,
    BOTTOM_FACE,
    LEFT_FACE,
    RIGHT_FACE
};

class Chunk {
public:
    Chunk(int size, const glm::vec3& position);
    Chunk();

    void initializeChunk(FastNoiseLite& noise1, FastNoiseLite& noise2, float weight1, float weight2, float THRESHOLD);
    std::vector<std::pair<glm::vec3, Face>> getVisibleFaces() const;
    std::vector<glm::vec3> getVoxelWorldPositions() const;
    void addVoxel(const glm::vec3& pos);
    const std::vector<std::vector<std::vector<bool>>>& getChunkBlocks() const;
    const std::vector<glm::vec3>& getVoxelPositions() const;
    void generateVisibleFaces();

private:
    int size;
    std::vector<glm::vec3> voxelPositions;
	glm::vec3 position;// 区块的位置(世界坐标)
    std::vector<std::vector<std::vector<bool>>> chunkBlocks;
    std::vector<std::pair<glm::vec3, Face>> visibleFaces;

    bool isVoxelAt(int x, int y, int z) const;
};

#endif // CHUNK_H
