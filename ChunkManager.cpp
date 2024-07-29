#include "ChunkManager.h"

// ���캯������ʼ�������С
ChunkManager::ChunkManager(int chunkSize) : chunkSize(chunkSize) {

    std::cout << "Initializing Origin Chunks" << std::endl;

	// �������һ֡�������λ��
	lastCameraPosition = glm::vec3(0.0f);

    // ���������chunks�ļ��У��򴴽�
    if (!std::filesystem::exists("chunks")) {
        std::filesystem::create_directory("chunks");
    }

    // ��������������
    // ��ʼ������������
    //noise1.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise1.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    //noise1.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    noise1.SetFrequency(0.1f);
    noise1.SetSeed(SEED);

    //noise2.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    //noise2.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise2.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    noise2.SetFrequency(0.1f);
    noise2.SetSeed(SEED);

	THRESHOLD = 0.3f; // ��ֵ

    // ��ʱ�洢�¼��ص�����
	std::unordered_map<std::string, Chunk> tempChunks;

    // �����������Χ������
    // 3��3�ķ�Χ����8���������������������ڵ�����
    for (int dx = -viewDistance; dx <= viewDistance; ++dx) {
        for (int dz = -viewDistance; dz <= viewDistance; ++dz) {
            glm::vec3 chunkPos = glm::vec3(dx, 0.0f, dz);// �������������λ��
            chunkPos.x *= chunkSize;
            //chunkPos.y *= chunkSize;
            chunkPos.z *= chunkSize;
            std::string key = getChunkKey(chunkPos);// ��������ļ�

            //�����.find()��������unordered_map�в��Ҽ�ֵΪkey��Ԫ�أ�����ҵ��˾ͷ���ָ���Ԫ�صĵ����������򷵻�unordered_map::end()�������صĵ�����
            if (chunks.find(key) == chunks.end()) {
                //auto start = std::chrono::high_resolution_clock::now();// ��ʱ��ʼ
                // �������δ�����أ������loadChunk��������
                loadChunk(chunkPos);
                //auto end = std::chrono::high_resolution_clock::now(); //��¼����ʱ��
                //std::chrono::duration<double> generationTime = end - start;
                //std::cout << "Chunk generated " << key << " in " << generationTime.count() << " seconds." << std::endl;
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
	//int y = static_cast<int>(floor(position.y / chunkSize));
    int z = static_cast<int>(floor(position.z / chunkSize));
    return std::to_string(x) + "_" + std::to_string(z);
    //return std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
}

void ChunkManager::setNoiseWeights(float weight1, float weight2) {
    this->weight1 = weight1;
    this->weight2 = weight2;
}

void ChunkManager::setViewDistance(int distance)
{
	viewDistance = distance;
}

bool ChunkManager::getIsLoading() const
{
    return isLoading;
}

void ChunkManager::clearChunksFolder()
{
	std::filesystem::remove_all("chunks");
	std::filesystem::create_directory("chunks");
	//std::cout << "Cleared all chunks in the folder." << std::endl;
}

std::unordered_map<std::string, Chunk>& ChunkManager::getChunks()
{
    std::lock_guard<std::mutex> lock(chunksMutex);
	return chunks;
}

// ���µ�ǰ���ص�����
void ChunkManager::update(const glm::vec3& cameraPosition) {

    if (!isLoading) {
        return; // ���ֹͣ���أ��򲻽��и���
    }

	std::unordered_map<std::string, Chunk> tempChunks;// ��ʱ�洢�¼��ص�����
	tempChunks.clear();

	// ������������ڵ�����λ��
    glm::vec3 cameraChunkPosition = glm::vec3(
        floor(cameraPosition.x / chunkSize),
        0.0f,
        floor(cameraPosition.z / chunkSize)
    );

    // ��������λ��û�иı�����������Ϊ0������Ҫ��������
	if (cameraChunkPosition == lastCameraPosition && chunks.size() != 0) {
		return;
	}

	// ����ڴ��е���������
	//chunks.clear();
    //std::cout << "-------------- Chunk has updated --------------" << std::endl;

    // ��ӡ��������ڵ�����λ��
    //std::cout << "Camera chunk position: " << cameraChunkPosition.x << ", " << cameraChunkPosition.y << ", " << cameraChunkPosition.z << std::endl;
    // ��ӡ��ǰ���е���������
    // std::cout << "Number of chunks: " << chunks.size() << std::endl;
    // ������һ֡�������λ��
    lastCameraPosition = cameraChunkPosition;
    
    // �����������Χ������
	// 3��3�ķ�Χ����8���������������������ڵ�����
    for (int dx = -viewDistance; dx <= viewDistance; ++dx) {
        for (int dz = -viewDistance; dz <= viewDistance; ++dz) {
            glm::vec3 chunkPos = cameraChunkPosition + glm::vec3(dx, 0.0f, dz);// �������������λ��
            chunkPos.x *= chunkSize;
            chunkPos.z *= chunkSize;
            std::string key = getChunkKey(chunkPos);// ��������ļ�

			//�����.find()��������unordered_map�в��Ҽ�ֵΪkey��Ԫ�أ�����ҵ��˾ͷ���ָ���Ԫ�صĵ����������򷵻�unordered_map::end()�������صĵ�����
            if (chunks.find(key) == chunks.end()) {
                //auto start = std::chrono::high_resolution_clock::now();// ��ʱ��ʼ
                // �������δ�����أ������loadChunk��������
                loadChunk(chunkPos);
                //auto end = std::chrono::high_resolution_clock::now(); //��¼����ʱ��
                //std::chrono::duration<double> generationTime = end - start;
                //std::cout << "Chunk generated " << key <<" in " << generationTime.count() << " seconds." << std::endl;
            }
            //  ��ʱ�洢��Ҫ����������
            tempChunks[key] = chunks[key];
        }
    }
    
    // ����chunks��Ա����
    {
        std::lock_guard<std::mutex> lock(chunksMutex);
        chunks = tempChunks;
    }
}

void ChunkManager::clearChunks()
{
    std::lock_guard<std::mutex> lock(chunksMutex);
    chunks.clear(); // ����Ѽ��ص�����
    //std::cout << "Cleared all loaded chunks." << std::endl;
}

void ChunkManager::stopLoading()
{
    isLoading = false; // ֹͣ��������
    //std::cout << "Stopped loading chunks." << std::endl;
}

void ChunkManager::startLoading()
{
	isLoading = true; // ��ʼ��������
	//std::cout << "Started loading chunks." << std::endl;
}

// ��������
void ChunkManager::loadChunk(const glm::vec3& position) {
	// ���������λ������Ψһ�ļ�
    std::string key = getChunkKey(position);
	// ���������ļ���
    std::string filename = "chunks/" + key + ".chunk";

    Chunk chunk(chunkSize, position); // �����յ�Chunk����

    std::ifstream inFile(filename, std::ios::binary);// ifstream�������ڶ�ȡ�ļ�, �Զ����Ƹ�ʽ���ļ�
    if (inFile.is_open()) {
        // ��ȡ��������
        glm::vec3 pos;
        while (inFile.read(reinterpret_cast<char*>(&pos), sizeof(glm::vec3))) {
            chunk.addVoxel(pos);
        }
		chunk.generateVisibleFaces();
        inFile.close();
        //std::cout << "Loaded chunk from file: " << filename << std::endl;
    }
    else {
        //std::cerr << "Failed to load chunk from file: " << filename << std::endl;
        //std::cout << "Creating new chunk at position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
        chunk.initializeChunk(noise1, noise2, weight1, weight2, THRESHOLD);
        saveChunkToFile(chunk, filename);
    }


    // �����ص�����洢��chunks��
    {
        std::lock_guard<std::mutex> lock(chunksMutex);
        chunks[key] = chunk;
    }
}

// ���������ݱ��浽�ļ�
void ChunkManager::saveChunkToFile(const Chunk& chunk, const std::string& filename) {
    // ʹ�ö����Ƹ�ʽ���������е�����λ��
    std::ofstream outFile(filename, std::ios::binary);
    
    if (outFile.is_open()) {
        // ������������
        // ���������е�ÿ������λ�ã���д���ļ�
        for (const auto& pos : chunk.getVoxelPositions()) {
            outFile.write(reinterpret_cast<const char*>(&pos), sizeof(glm::vec3));
        }
        outFile.close();
        //std::cerr << "Saved chunk to file: " << filename << std::endl;
    }
}
