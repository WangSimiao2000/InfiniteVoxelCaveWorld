#include <vector>
#include <glm/glm.hpp>
#include <array>

class Chunk {
public:
	// 初始化区块的边长和位置; 分配三维布尔数组的内存, 表示每个体素是否被填充; 初始化梯度向量数组
    Chunk(int size, const glm::vec3& position) : size(size), position(position) {
        chunkBlocks = std::vector<std::vector<std::vector<bool>>>(size,
            std::vector<std::vector<bool>>(size,
                std::vector<bool>(size, false)));
        gradients = std::array<glm::vec3, 8>{};
        voxelPositions.reserve(size * size * size);
    }

    // 初始化区块，使所有体素都被填充，并记录它们的相对坐标。
    void initializeChunk() {
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
    void setGradient(int index, const glm::vec3& gradient) {
        if (index >= 0 && index < 8) {
            gradients[index] = gradient;
        }
    }

    glm::vec3 getVoxelWorldPosition(int index) const {
        if (index >= 0 && index < voxelPositions.size()) {
            return voxelPositions[index] + position;
        }
        return glm::vec3(0.0f);
    }

    // 根据区块的坐标和相对坐标计算体素的世界坐标
    std::vector<glm::vec3> getVoxelWorldPositions() const {
        std::vector<glm::vec3> worldPositions;
        for (const auto& pos : voxelPositions) {
            worldPositions.push_back(pos + position);
        }
        return worldPositions;
    }

private:
    // size：区块的边长
    int size;
	// position：区块的位置
    glm::vec3 position;
    // chunkBlocks: 用于表示区块中每个体素是否被填充的三维数组
    std::vector<std::vector<std::vector<bool>>> chunkBlocks;
    // voxelPositions：存储所有被填充体素的相对坐标
    std::vector<glm::vec3> voxelPositions;
    // gradients：存储区块8个顶点的梯度向量
    std::array<glm::vec3, 8> gradients;
};
