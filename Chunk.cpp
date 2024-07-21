#include "Chunk.h"

// 单个体素的顶点数据
extern std::vector<Vertex> voxelVertices = {
    // Front face
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
    // Back face
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
    // Top face
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
    // Bottom face
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
    // Right face
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
    // Left face
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}}
};

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

// 析构函数，释放VAO和VBO, 会在对象销毁时调用, 有参数的构造函数在对象销毁时也会调用
Chunk::~Chunk()
{
}

// 初始化区块
void Chunk::initializeChunk(FastNoiseLite& noise1, FastNoiseLite& noise2, float weight1, float weight2, float THRESHOLD) {
    voxelPositions.clear();
    for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
            for (int z = 0; z < size; ++z) {
                float noiseValue1 = noise1.GetNoise(position.x + x, position.y + y, position.z + z);
                float noiseValue2 = noise2.GetNoise(position.x + x, position.y + y, position.z + z);
                float combinedNoiseValue = weight1 * noiseValue1 + weight2 * noiseValue2;
                if (combinedNoiseValue < THRESHOLD)
                {
                    chunkBlocks[x][y][z] = true;
                    voxelPositions.push_back(glm::vec3(x, y, z));
				}
				else
				{
					chunkBlocks[x][y][z] = false;
				}
            }
        }
    }
    generateVisibleFaces();
}

// 获取体素的世界坐标(世界坐标 = 区块坐标 + 体素坐标)
std::vector<glm::vec3> Chunk::getVoxelWorldPositions() const {
    std::vector<glm::vec3> worldPositions;
    for (const auto& pos : voxelPositions) {
        worldPositions.push_back(pos + position);
    }
    return worldPositions;
}

// 将可见面添加到visibleFaces数组中
void Chunk::generateVisibleFaces() {
    visibleFaces.clear();
    for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
            for (int z = 0; z < size; ++z) {
                if (chunkBlocks[x][y][z]) {
                    glm::vec3 voxelPosition = glm::vec3(x, y, z);
                    if (!isVoxelAt(x + 1, y, z)) {
                        visibleFaces.emplace_back(voxelPosition, Face::RIGHT_FACE);
                        for (int i = 24; i < 30; ++i) {
                            chunkVisibleFacesVertices.push_back({ voxelVertices[i].position + voxelPosition, voxelVertices[i].texCoords });
                        }
                    }
                    if (!isVoxelAt(x - 1, y, z)) {
                        visibleFaces.emplace_back(voxelPosition, Face::LEFT_FACE);
                        for (int i = 30; i < 36; ++i) {
                            chunkVisibleFacesVertices.push_back({ voxelVertices[i].position + voxelPosition, voxelVertices[i].texCoords });
                        }
                    }
                    if (!isVoxelAt(x, y + 1, z)) {
                        visibleFaces.emplace_back(voxelPosition, Face::TOP_FACE);
                        for (int i = 12; i < 18; ++i) {
                            chunkVisibleFacesVertices.push_back({ voxelVertices[i].position + voxelPosition, voxelVertices[i].texCoords });
                        }
                    }
                    if (!isVoxelAt(x, y - 1, z)) {
                        visibleFaces.emplace_back(voxelPosition, Face::BOTTOM_FACE);
                        for (int i = 18; i < 24; ++i) {
                            chunkVisibleFacesVertices.push_back({ voxelVertices[i].position + voxelPosition, voxelVertices[i].texCoords });
                        }
                    }
                    if (!isVoxelAt(x, y, z + 1)) {
                        visibleFaces.emplace_back(voxelPosition, Face::FRONT_FACE);
                        for (int i = 0; i < 6; ++i) {
                            chunkVisibleFacesVertices.push_back({ voxelVertices[i].position + voxelPosition, voxelVertices[i].texCoords });
                        }
                    }
                    if (!isVoxelAt(x, y, z - 1)) {
                        visibleFaces.emplace_back(voxelPosition, Face::BACK_FACE);
                        for (int i = 6; i < 12; ++i) {
                            chunkVisibleFacesVertices.push_back({ voxelVertices[i].position + voxelPosition, voxelVertices[i].texCoords });
                        }
                    }
                }
            }
        }
    }
}

glm::vec3 Chunk::getMaxBounds() const
{
    return position + glm::vec3(size);
}

glm::vec3 Chunk::getMinBounds() const
{
    return position;
}

std::vector<Vertex> Chunk::getChunkVisibleFacesVertices() const
{
	return chunkVisibleFacesVertices;
}

glm::vec3 Chunk::getChunkPosition() const
{
    return position;
}

// 判断坐标(x, y, z)处是否有体素
bool Chunk::isVoxelAt(int x, int y, int z) const {
    if (x >= 0 && x < size && y >= 0 && y < size && z >= 0 && z < size) {
        return chunkBlocks[x][y][z];
    }
    return false;
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

// 获取区块中的可见面
std::vector<std::pair<glm::vec3, Face>> Chunk::getVisibleFaces() const {
    return visibleFaces;
}