#include "Chunk.h"

// 初始化区块的边长和位置; 分配三维布尔数组的内存, 表示每个体素是否被填充; 初始化梯度向量数组
Chunk::Chunk(int size, const glm::vec3& position)
    : size(size), position(position)
{
    chunkBlocks = std::vector<std::vector<std::vector<bool>>>(size,
        std::vector<std::vector<bool>>(size,
			std::vector<bool>(size, false)));// 初始化三维布尔数组，表示每个体素是否被填充, 默认为false
	voxelPositions.reserve(size * size * size);
}

// 默认构造函数，初始化区块大小为16，位置为原点
Chunk::Chunk()
    :   Chunk(16, glm::vec3(0.0f)) {}

// 初始化区块
void Chunk::initializeChunk(FastNoiseLite& noise) {
    voxelPositions.clear();
    for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
            for (int z = 0; z < size; ++z) {
				float THRESHOLD = 0.3f;
                float noiseValue = noise.GetNoise(position.x + x, position.y + y, position.z + z);
                if (noiseValue < THRESHOLD)
                {
                    chunkBlocks[x][y][z] = true;
                    voxelPositions.push_back(glm::vec3(x, y, z));
                }
            }
        }
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

// 添加体素到区块中,当体素的坐标在区块范围内时，将体素的坐标添加到voxelPositions数组中, 用于后续渲染
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
