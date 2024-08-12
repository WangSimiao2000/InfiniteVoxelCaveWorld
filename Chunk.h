#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <glm/glm.hpp>
#include <array>
#include <glad/glad.h>
#include "FastNoiseLite.h"

// 面的枚举类型
// Enum type of faces
enum Face {
    FRONT_FACE,
    BACK_FACE,
    TOP_FACE,
    BOTTOM_FACE,
    LEFT_FACE,
    RIGHT_FACE
};

// 顶点结构体
// Vertex structure
struct Vertex {
	glm::vec3 position;
	glm::vec2 texCoords;
};
class Chunk {
public:
    Chunk(int size, const glm::vec3& position);
    Chunk();
    ~Chunk();

    void initializeChunk(FastNoiseLite& noise1, FastNoiseLite& noise2, float weight1, float weight2, float THRESHOLD);
    std::vector<std::pair<glm::vec3, Face>> getVisibleFaces() const;
    std::vector<glm::vec3> getVoxelWorldPositions() const;
    void addVoxel(const glm::vec3& pos);
    const std::vector<std::vector<std::vector<bool>>>& getChunkBlocks() const;
    const std::vector<glm::vec3>& getVoxelPositions() const;
    void generateVisibleFaces();
	glm::vec3 getMaxBounds() const;
	glm::vec3 getMinBounds() const;
	std::vector<Vertex> getChunkVisibleFacesVertices() const;
	glm::vec3 getChunkPosition() const;

private:
    int chunkWidthSize;
	int chunkHeight = 64;
    std::vector<glm::vec3> voxelPositions;
	glm::vec3 position;// The position of the chunk (world coordinates)
    std::vector<std::vector<std::vector<bool>>> chunkBlocks;
    std::vector<std::pair<glm::vec3, Face>> visibleFaces;
    std::vector<Vertex> chunkVisibleFacesVertices;
    bool isVoxelAt(int x, int y, int z) const;
};

#endif // CHUNK_H
