#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <glm/glm.hpp>
#include <array>

class Chunk {
public:
    Chunk(int size, const glm::vec3& position);
    Chunk();

    void initializeChunk();
    void setGradient(int index, const glm::vec3& gradient);
    std::vector<glm::vec3> getVoxelWorldPositions() const;
    void addVoxel(const glm::vec3& pos);
    const std::vector<std::vector<std::vector<bool>>>& getChunkBlocks() const;
    const std::vector<glm::vec3>& getVoxelPositions() const;

private:
    int size;
    glm::vec3 position;
    std::vector<std::vector<std::vector<bool>>> chunkBlocks;
    std::vector<glm::vec3> voxelPositions;
    std::array<glm::vec3, 8> gradients;
};

#endif // CHUNK_H
