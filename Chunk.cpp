#include "Chunk.h"

// 初始化区块的边长和位置; 分配三维布尔数组的内存, 表示每个体素是否被填充; 初始化梯度向量数组
Chunk::Chunk(int size, const glm::vec3& position)
    : size(size), position(position)
{
    chunkBlocks = std::vector<std::vector<std::vector<bool>>>(size,
        std::vector<std::vector<bool>>(size,
            std::vector<bool>(size, false)));
    gradients = std::array<glm::vec3, 8>{};
    voxelPositions.reserve(size * size * size);
}

// 默认构造函数，初始化区块大小为16，位置为原点
Chunk::Chunk()
    : size(16), position(glm::vec3(0.0f))
{
    chunkBlocks = std::vector<std::vector<std::vector<bool>>>(size,
        std::vector<std::vector<bool>>(size,
            std::vector<bool>(size, false)));
    gradients = std::array<glm::vec3, 8>{};
    voxelPositions.reserve(size * size * size);
}

// 初始化区块，使所有体素都被填充，并记录它们的相对坐标
void Chunk::initializeChunk() {
    voxelPositions.clear();
    for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
            for (int z = 0; z < size; ++z) {
                chunkBlocks[x][y][z] = true;
                voxelPositions.push_back(glm::vec3(x, y, z));
            }
        }
    }
}

// 设置区块某个顶点的梯度向量
void Chunk::setGradient(int index, const glm::vec3& gradient) {
    if (index >= 0 && index < 8) {
        gradients[index] = gradient;
    }
}

// 获取体素的世界坐标(世界坐标 = 区块坐标 + 体素坐标)
std::vector<glm::vec3> Chunk::getVoxelWorldPositions() const {
    std::vector<glm::vec3> worldPositions;
    for (const auto& pos : voxelPositions) {
        worldPositions.push_back(pos + position);
    }
    return worldPositions;
}

// 添加体素到区块中
void Chunk::addVoxel(const glm::vec3& pos) {
    int x = static_cast<int>(pos.x);
    int y = static_cast<int>(pos.y);
    int z = static_cast<int>(pos.z);

    if (x >= 0 && x < size && y >= 0 && y < size && z >= 0 && z < size) {
        chunkBlocks[x][y][z] = true;
        voxelPositions.push_back(pos);
    }
}

// 获取chunkBlocks: 三维布尔数组，表示每个体素是否被填充
const std::vector<std::vector<std::vector<bool>>>& Chunk::getChunkBlocks() const {
    return chunkBlocks;
}

// 获取voxelPositions: 体素的相对坐标的数组
const std::vector<glm::vec3>& Chunk::getVoxelPositions() const {
    return voxelPositions;
}
