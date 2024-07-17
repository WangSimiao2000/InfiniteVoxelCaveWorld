#include "Chunk.h"

// ��ʼ������ı߳���λ��; ������ά����������ڴ�, ��ʾÿ�������Ƿ����; ��ʼ���ݶ���������
Chunk::Chunk(int size, const glm::vec3& position)
    : size(size), position(position)
{
    chunkBlocks = std::vector<std::vector<std::vector<bool>>>(size,
        std::vector<std::vector<bool>>(size,
			std::vector<bool>(size, false)));// ��ʼ����ά�������飬��ʾÿ�������Ƿ����, Ĭ��Ϊfalse
	voxelPositions.reserve(size * size * size);
}

// Ĭ�Ϲ��캯������ʼ�������СΪ16��λ��Ϊԭ��
Chunk::Chunk()
    :   Chunk(16, glm::vec3(0.0f)) {}

// ��ʼ������
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

// ��ȡ���ص���������(�������� = �������� + ��������)
std::vector<glm::vec3> Chunk::getVoxelWorldPositions() const {
    std::vector<glm::vec3> worldPositions;
    for (const auto& pos : voxelPositions) {
        worldPositions.push_back(pos + position);
    }
    return worldPositions;
}

// ������ص�������,�����ص����������鷶Χ��ʱ�������ص�������ӵ�voxelPositions������, ���ں�����Ⱦ
void Chunk::addVoxel(const glm::vec3& pos) {
    int x = static_cast<int>(pos.x);
    int y = static_cast<int>(pos.y);
    int z = static_cast<int>(pos.z);

    if (x >= 0 && x < size && y >= 0 && y < size && z >= 0 && z < size) {
		chunkBlocks[x][y][z] = true;
        voxelPositions.push_back(pos);
    }
}

// ��ȡchunkBlocks: ��ά�������飬��ʾÿ�������Ƿ����
const std::vector<std::vector<std::vector<bool>>>& Chunk::getChunkBlocks() const {
    return chunkBlocks;
}

// ��ȡvoxelPositions: ���ص�������������
const std::vector<glm::vec3>& Chunk::getVoxelPositions() const {
    return voxelPositions;
}
