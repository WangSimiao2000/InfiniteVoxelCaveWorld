#include "Chunk.h"

// 单个体素的顶点数据
// Vertex data for a single voxel
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
// Initialize the size and position of the chunk; Allocate memory for the three-dimensional boolean array, indicating whether each voxel is filled; Initialize the gradient vector array
Chunk::Chunk(int size, const glm::vec3& position)
    : chunkWidthSize(size), position(position)
{
    chunkBlocks = std::vector<std::vector<std::vector<bool>>>(size,
        std::vector<std::vector<bool>>(chunkHeight,
			std::vector<bool>(size, false)));// 初始化三维布尔数组，表示每个体素是否被填充, 默认为false
	voxelPositions.reserve(size * chunkHeight * size);
}

// 默认构造函数，初始化区块大小为16，位置为原点
// Default constructor, initialize the size of the chunk to 16 and the position to the origin
Chunk::Chunk()
    :   Chunk(16, glm::vec3(0.0f)) {}

// 析构函数，释放VAO和VBO, 会在对象销毁时调用, 有参数的构造函数在对象销毁时也会调用
// Destructor, release VAO and VBO, will be called when the object is destroyed, and the constructor with parameters will also be called when the object is destroyed
Chunk::~Chunk()
{
}

// 初始化区块
//void Chunk::initializeChunk(FastNoiseLite& noise1, FastNoiseLite& noise2, float weight1, float weight2, float THRESHOLD) {
//    voxelPositions.clear();
//    for (int x = 0; x < size; ++x) {
//        for (int y = 0; y < size; ++y) {
//            for (int z = 0; z < size; ++z) {
//                float noiseValue1 = noise1.GetNoise(position.x + x, position.y + y, position.z + z);
//                float noiseValue2 = noise2.GetNoise(position.x + x, position.y + y, position.z + z);
//                float combinedNoiseValue = weight1 * noiseValue1 + weight2 * noiseValue2;
//                if (combinedNoiseValue < THRESHOLD)
//                {
//                    chunkBlocks[x][y][z] = true;
//                    voxelPositions.push_back(glm::vec3(x, y, z));
//				}
//            }
//        }
//    }
//    generateVisibleFaces();
//}

void Chunk::initializeChunk(FastNoiseLite& noise1, FastNoiseLite& noise2, float weight1, float weight2, float THRESHOLD) {
    voxelPositions.clear();
    for (int x = 0; x < chunkWidthSize; ++x) {
        for (int z = 0; z < chunkWidthSize; ++z) {
            // 计算地形高度
            // 调整噪声的幅度
            float amplitude = 0.5f;
            float noiseValue1 = noise1.GetNoise(position.x + x, position.z + z) * amplitude;
            float noiseValue2 = noise2.GetNoise(position.x + x, position.z + z) * amplitude;
            float combinedNoiseValue = weight1 * noiseValue1 + weight2 * noiseValue2;
			int terrainHeight = chunkHeight - 4 + static_cast<int>((combinedNoiseValue + 1.0f) * 0.5f * 4);// 这里的4是地形的最小高度

			for (int y = 0; y < terrainHeight - 16; ++y) {//-16是为了避免地形太低，导致地形下方生成洞穴
                // 如果当前位置在地形高度以下
				// If the current position is below the terrain height
                // 判断是否生成洞穴
				// Determine whether to generate a cave
                float frequency = 0.7f;
                float caveNoise1 = noise1.GetNoise((position.x + x) * frequency, (position.y + y) * frequency, (position.z + z) * frequency);
                float caveNoise2 = noise2.GetNoise((position.x + x) * frequency, (position.y + y) * frequency, (position.z + z) * frequency);
                float caveCombinedNoiseValue = weight1 * caveNoise1 + weight2 * caveNoise2;

                if (caveCombinedNoiseValue < THRESHOLD) {
                    chunkBlocks[x][y][z] = true;
                    voxelPositions.push_back(glm::vec3(x, y, z));
                }
            }
        }
    }
    generateVisibleFaces();
}

// 获取体素的世界坐标(世界坐标 = 区块坐标 + 体素坐标)
// Get the world coordinates of the voxel (world coordinates = chunk coordinates + voxel coordinates)
std::vector<glm::vec3> Chunk::getVoxelWorldPositions() const {
    std::vector<glm::vec3> worldPositions;
    for (const auto& pos : voxelPositions) {
        worldPositions.push_back(pos + position);
    }
    return worldPositions;
}

// 将可见面添加到visibleFaces数组中
// Add visible faces to the visibleFaces array
void Chunk::generateVisibleFaces() {
    visibleFaces.clear();
    for (int x = 0; x < chunkWidthSize; ++x) {
        for (int y = 0; y < chunkHeight; ++y) {
            for (int z = 0; z < chunkWidthSize; ++z) {
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
    return position + glm::vec3(chunkWidthSize, chunkHeight, chunkWidthSize);
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
// Determine if there is a voxel at the coordinate (x, y, z)
bool Chunk::isVoxelAt(int x, int y, int z) const {
    if (x >= 0 && x < chunkWidthSize && y >= 0 && y < chunkHeight && z >= 0 && z < chunkWidthSize) {
        return chunkBlocks[x][y][z];
    }
    return false;
}

// 添加体素到区块中,当体素的坐标在区块范围内时，将体素的坐标添加到voxelPositions数组中, 用于后续渲染
// Add voxels to the chunk. When the voxel coordinates are within the chunk range, add the voxel coordinates to the voxelPositions array for subsequent rendering
void Chunk::addVoxel(const glm::vec3& pos) {
    int x = static_cast<int>(pos.x);
    int y = static_cast<int>(pos.y);
    int z = static_cast<int>(pos.z);

    if (x >= 0 && x < chunkWidthSize && y >= 0 && y < chunkHeight && z >= 0 && z < chunkWidthSize) {
		chunkBlocks[x][y][z] = true;
        voxelPositions.push_back(pos);
    }
}

// 获取chunkBlocks: 三维布尔数组，表示每个体素是否被填充
// Get chunkBlocks: a three-dimensional boolean array indicating whether each voxel is filled
const std::vector<std::vector<std::vector<bool>>>& Chunk::getChunkBlocks() const {
    return chunkBlocks;
}

// 获取voxelPositions: 体素的相对坐标的数组
// Get voxelPositions: an array of relative voxel coordinates
const std::vector<glm::vec3>& Chunk::getVoxelPositions() const {
    return voxelPositions;
}

// 获取区块中的可见面
// Get the visible faces in the chunk
std::vector<std::pair<glm::vec3, Face>> Chunk::getVisibleFaces() const {
    return visibleFaces;
}