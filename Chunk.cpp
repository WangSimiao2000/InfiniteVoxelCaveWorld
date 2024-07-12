#include <vector>
#include <glm/glm.hpp>
#include <array>

class Chunk {
public:
	// ��ʼ������ı߳���λ��; ������ά����������ڴ�, ��ʾÿ�������Ƿ����; ��ʼ���ݶ���������
    Chunk(int size, const glm::vec3& position) : size(size), position(position) {
        chunkBlocks = std::vector<std::vector<std::vector<bool>>>(size,
            std::vector<std::vector<bool>>(size,
                std::vector<bool>(size, false)));
        gradients = std::array<glm::vec3, 8>{};
        voxelPositions.reserve(size * size * size);
    }

    // ��ʼ�����飬ʹ�������ض�����䣬����¼���ǵ�������ꡣ
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
    
    // ��������ĳ��������ݶ�����
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

    // ���������������������������ص���������
    std::vector<glm::vec3> getVoxelWorldPositions() const {
        std::vector<glm::vec3> worldPositions;
        for (const auto& pos : voxelPositions) {
            worldPositions.push_back(pos + position);
        }
        return worldPositions;
    }

private:
    // size������ı߳�
    int size;
	// position�������λ��
    glm::vec3 position;
    // chunkBlocks: ���ڱ�ʾ������ÿ�������Ƿ�������ά����
    std::vector<std::vector<std::vector<bool>>> chunkBlocks;
    // voxelPositions���洢���б�������ص��������
    std::vector<glm::vec3> voxelPositions;
    // gradients���洢����8��������ݶ�����
    std::array<glm::vec3, 8> gradients;
};
