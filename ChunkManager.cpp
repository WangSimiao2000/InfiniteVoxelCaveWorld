#include "ChunkManager.h"

// ���������λ������һ��Ψһ���ַ����������ڱ�ʶ����
// - ������������������Ĵ�С��Ȼ������ȡ����ȷ����ȡ�����������������
// - ����������ת��Ϊ�ַ��������»��߷ָ�����Ϊ��
std::string ChunkManager::getChunkKey(const glm::vec3& position) {
    int x = static_cast<int>(floor(position.x / chunkSize));
    int y = static_cast<int>(floor(position.y / chunkSize));
    int z = static_cast<int>(floor(position.z / chunkSize));
    return std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
}

// ���µ�ǰ���ص�����
void ChunkManager::update(const glm::vec3& cameraPosition) {
    std::unordered_map<std::string, Chunk> newChunks;

	// ������������ڵ�����λ��
    glm::vec3 cameraChunkPosition = glm::vec3(
        floor(cameraPosition.x / chunkSize),
        floor(cameraPosition.y / chunkSize),
        floor(cameraPosition.z / chunkSize)
    );

	// �����������Χ������
	// 3��3��3�ķ�Χ����26���������������������ڵ�����
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
                glm::vec3 chunkPos = cameraChunkPosition + glm::vec3(dx, dy, dz);
                std::string key = getChunkKey(chunkPos);

                if (chunks.find(key) == chunks.end()) {
                    // �������δ�����أ������loadChunk��������
                    loadChunk(chunkPos);
                }
                //  ��ʱ�洢��Ҫ����������
                newChunks[key] = chunks[key];
            }
        }
    }

    // ж�ز�����Ҫ�����飨�����¼��ص����鷶Χ�ڣ�
    for (const auto& chunk : chunks) {
        if (newChunks.find(chunk.first) == newChunks.end()) {
            unloadChunk(chunk.first);
        }
    }

    // ����chunks��Ա����
    chunks = newChunks;
}

// ��������
void ChunkManager::loadChunk(const glm::vec3& position) {
	// ���������λ������Ψһ�ļ�
    std::string key = getChunkKey(position);
	// ���������ļ���
    std::string filename = "chunks/" + key + ".chunk";
    // ���ļ���������
    Chunk chunk = loadChunkFromFile(filename);

    // ����ļ������ڻ��ȡʧ�ܣ����ʼ��һ���µ�����
    // ������ļ����ص�����Ϊ�գ����ʼ��һ���µ�����
    if (chunk.getVoxelWorldPositions().empty()) {
        chunk = Chunk(chunkSize, position);
        chunk.initializeChunk();
    }

    // �����ص�����洢��chunks��
    chunks[key] = chunk;
}

// ж������
void ChunkManager::unloadChunk(const std::string& key) {
    std::string filename = "chunks/" + key + ".chunk";
    // �����鱣�浽�ļ�
    saveChunkToFile(chunks[key], filename);
	// ��chunks���Ƴ�����
    chunks.erase(key);
}

// ���������ݱ��浽�ļ�
void ChunkManager::saveChunkToFile(const Chunk& chunk, const std::string& filename) {
    //ʹ�ö����Ƹ�ʽ���������е�����λ��
    std::ofstream outFile(filename, std::ios::binary);
    if (outFile.is_open()) {
        // ������������
        // ���������е�ÿ������λ�ã���д���ļ�
        for (const auto& pos : chunk.getVoxelWorldPositions()) {
            outFile.write(reinterpret_cast<const char*>(&pos), sizeof(glm::vec3));
        }
        outFile.close();
    }
    else {
        std::cerr << "Failed to save chunk to file: " << filename << std::endl;
    }
}

// ���ļ�������������
Chunk ChunkManager::loadChunkFromFile(const std::string& filename) {
    Chunk chunk(chunkSize, glm::vec3(0.0f)); // �����յ�Chunk����

    std::ifstream inFile(filename, std::ios::binary);
    if (inFile.is_open()) {
        // ��ȡ��������
        glm::vec3 pos;
        while (inFile.read(reinterpret_cast<char*>(&pos), sizeof(glm::vec3))) {
			chunk.addVoxel(pos);
        }
        inFile.close();
    }
    else {
        std::cerr << "Failed to load chunk from file: " << filename << std::endl;
    }
    return chunk;
}