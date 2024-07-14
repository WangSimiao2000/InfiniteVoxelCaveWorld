#include "ChunkManager.h"

// ���캯������ʼ�������С
ChunkManager::ChunkManager(int chunkSize) : chunkSize(chunkSize) {

    std::cout << "-------------- Initialize --------------" << std::endl;

	// �������һ֡�������λ��
	lastCameraPosition = glm::vec3(0.0f);

    // ���������chunks�ļ��У��򴴽�
    if (!std::filesystem::exists("chunks")) {
        std::filesystem::create_directory("chunks");
    }

    // ��ʱ�洢�¼��ص�����
	std::unordered_map<std::string, Chunk> tempChunks;

    // �����������Χ������
    // 3��3�ķ�Χ����8���������������������ڵ�����
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dz = -1; dz <= 1; ++dz) {
            glm::vec3 chunkPos = glm::vec3(dx, 0.0f, dz);// �������������λ��
            chunkPos.x *= chunkSize;
            chunkPos.y *= chunkSize;
            chunkPos.z *= chunkSize;
            std::string key = getChunkKey(chunkPos);// ��������ļ�

            //�����.find()��������unordered_map�в��Ҽ�ֵΪkey��Ԫ�أ�����ҵ��˾ͷ���ָ���Ԫ�صĵ����������򷵻�unordered_map::end()�������صĵ�����
            if (chunks.find(key) == chunks.end()) {
                // �������δ�����أ������loadChunk��������
                loadChunk(chunkPos);
            }
            //  ��ʱ�洢��Ҫ����������
            tempChunks[key] = chunks[key];
        }
    }

    // ����chunks��Ա����
    chunks = tempChunks;
}

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
	std::unordered_map<std::string, Chunk> tempChunks;// ��ʱ�洢�¼��ص�����

	// ������������ڵ�����λ��
    glm::vec3 cameraChunkPosition = glm::vec3(
        floor(cameraPosition.x / chunkSize),
        0.0f,
        floor(cameraPosition.z / chunkSize)
    );

    // ��������λ��û�иı䣬����Ҫ��������
    if (cameraChunkPosition == lastCameraPosition) {
        return;
    }
    std::cout << "-------------- Chunk has updated --------------" << std::endl;

    // ��ӡ��������ڵ�����λ��
    std::cout << "Camera chunk position: " << cameraChunkPosition.x << ", " << cameraChunkPosition.y << ", " << cameraChunkPosition.z << std::endl;
    // ��ӡ��ǰ���е���������
    // std::cout << "Number of chunks: " << chunks.size() << std::endl;
    // ������һ֡�������λ��
    lastCameraPosition = cameraChunkPosition;
    
    // �����������Χ������
	// 3��3�ķ�Χ����8���������������������ڵ�����
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dz = -1; dz <= 1; ++dz) {
            glm::vec3 chunkPos = cameraChunkPosition + glm::vec3(dx, 0.0f, dz);// �������������λ��
            chunkPos.x *= chunkSize;
            chunkPos.y *= chunkSize;
            chunkPos.z *= chunkSize;
            std::string key = getChunkKey(chunkPos);// ��������ļ�

			//�����.find()��������unordered_map�в��Ҽ�ֵΪkey��Ԫ�أ�����ҵ��˾ͷ���ָ���Ԫ�صĵ����������򷵻�unordered_map::end()�������صĵ�����
            if (chunks.find(key) == chunks.end()) {
                // �������δ�����أ������loadChunk��������
                loadChunk(chunkPos);
            }
            //  ��ʱ�洢��Ҫ����������
            tempChunks[key] = chunks[key];
        }
    }

    // ��ӡ��ǰ���е���������
    std::cout << "Number of chunks: " << chunks.size() << std::endl;

    // ж�ز�����Ҫ�����飨�����¼��ص����鷶Χ�ڣ�
    for (const auto& chunk : chunks) {
        if (tempChunks.find(chunk.first) == tempChunks.end()) {
			std::cout << "Unloading chunk: " << chunk.first << std::endl;
            unloadChunk(chunk.first);
        }
    }
    // ����chunks��Ա����
    chunks = tempChunks;
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
        std::cout << "Creating new chunk at position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
        chunk = Chunk(chunkSize, position);
        chunk.initializeChunk();
    }

    // �����ص�����洢��chunks��
    chunks[key] = chunk;
}

// ж������
void ChunkManager::unloadChunk(const std::string& key) {
    std::string filename = "chunks/" + key + ".chunk";
	std::cout << "Unloading chunk: " << filename << std::endl;
    // �����鱣�浽�ļ�
    saveChunkToFile(chunks[key], filename);
	// ��chunks���Ƴ�����
	// ���ﲻ��Ҫ��chunks���Ƴ����飬��Ϊ��update�������Ѿ�������chunks��Ա����, �Ḳ�ǵ�֮ǰ������
    // chunks.erase(key);
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

	std::ifstream inFile(filename, std::ios::binary);// ifstream�������ڶ�ȡ�ļ�, �Զ����Ƹ�ʽ���ļ�
    if (inFile.is_open()) {
        // ��ȡ��������
        glm::vec3 pos;
        while (inFile.read(reinterpret_cast<char*>(&pos), sizeof(glm::vec3))) {
			chunk.addVoxel(pos);
        }
        inFile.close();
		std::cout << "Loaded chunk from file: " << filename << std::endl;
    }
    else {
        std::cerr << "Failed to load chunk from file: " << filename << std::endl;
    }
    return chunk;
}