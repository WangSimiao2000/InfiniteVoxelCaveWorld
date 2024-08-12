#include "ChunkManager.h"

// ���캯������ʼ�������С
// Constructor, initialize chunk size
ChunkManager::ChunkManager(int chunkSize) : chunkSize(chunkSize) {

    std::cout << "Initializing Origin Chunks" << std::endl;

	// �������һ֡�������λ��
	// Save the camera position of the last frame
	lastCameraPosition = glm::vec3(0.0f);

    // ���������chunks�ļ��У��򴴽�
	// Create the chunks folder if it doesn't exist
    if (!std::filesystem::exists("chunks")) {
        std::filesystem::create_directory("chunks");
    }

    // ��������������
	// Configure noise generator
    // ��ʼ������������
	// Initialize noise generator
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
	// Temporary storage for newly loaded chunks
	std::unordered_map<std::string, Chunk> tempChunks;

    // �����������Χ������
    // Traverse the chunks around the camera
    // 3��3�ķ�Χ����8���������������������ڵ�����
	// A range of 3��3, that is, 8 adjacent chunks plus the chunk where the camera is located
    for (int dx = -viewDistance; dx <= viewDistance; ++dx) {
        for (int dz = -viewDistance; dz <= viewDistance; ++dz) {
            glm::vec3 chunkPos = glm::vec3(dx, 0.0f, dz);// �������������λ��
            chunkPos.x *= chunkSize;
            //chunkPos.y *= chunkSize;
            chunkPos.z *= chunkSize;
            std::string key = getChunkKey(chunkPos);// ��������ļ�

            // �����.find()��������unordered_map�в��Ҽ�ֵΪkey��Ԫ�أ�����ҵ��˾ͷ���ָ���Ԫ�صĵ����������򷵻�unordered_map::end()�������صĵ�����
			// This .find() function is used to find the element with the key value key in the unordered_map. If found, it returns an iterator pointing to the element, otherwise it returns the iterator returned by unordered_map::end()
            if (chunks.find(key) == chunks.end()) {
                //auto start = std::chrono::high_resolution_clock::now();// ��ʱ��ʼ
                // �������δ�����أ������loadChunk��������
                loadChunk(chunkPos);
                //auto end = std::chrono::high_resolution_clock::now(); //��¼����ʱ��
                //std::chrono::duration<double> generationTime = end - start;
                //std::cout << "Chunk generated " << key << " in " << generationTime.count() << " seconds." << std::endl;
            }
            //  ��ʱ�洢��Ҫ����������
			// Temporary storage for chunks to be retained
            tempChunks[key] = chunks[key];
        }
    }

    // ����chunks��Ա����
	// Update the chunks member variable
    chunks = tempChunks;
}

// ���������λ������һ��Ψһ���ַ����������ڱ�ʶ����
// Generate a unique string key based on the position of the chunk to identify the chunk
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
// Update the currently loaded chunks
void ChunkManager::update(const glm::vec3& cameraPosition) {

    if (!isLoading) {
        return; // ���ֹͣ���أ��򲻽��и���
    }

	std::unordered_map<std::string, Chunk> tempChunks;// ��ʱ�洢�¼��ص�����
	tempChunks.clear();

	// ������������ڵ�����λ��
	// Calculate the position of the chunk where the camera is located
    glm::vec3 cameraChunkPosition = glm::vec3(
        floor(cameraPosition.x / chunkSize),
        0.0f,
        floor(cameraPosition.z / chunkSize)
    );

    // ��������λ��û�иı�����������Ϊ0������Ҫ��������
	// If the camera position has not changed and the number of chunks is 0, there is no need to update the chunks
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
    // Traverse the chunks around the camera
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
// Load chunks
void ChunkManager::loadChunk(const glm::vec3& position) {
	// ���������λ������Ψһ�ļ�
	// Generate a unique key based on the position of the chunk
    std::string key = getChunkKey(position);
	// ���������ļ���
	// Generate chunk file name
    std::string filename = "chunks/" + key + ".chunk";

    Chunk chunk(chunkSize, position); // �����յ�Chunk����

    std::ifstream inFile(filename, std::ios::binary);// ifstream�������ڶ�ȡ�ļ�, �Զ����Ƹ�ʽ���ļ�
    if (inFile.is_open()) {
        // ��ȡ��������
		// Read chunk data
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
	// Store the loaded chunks in chunks
    {
        std::lock_guard<std::mutex> lock(chunksMutex);
        chunks[key] = chunk;
    }
}

// ���������ݱ��浽�ļ�
// Save chunk data to file
void ChunkManager::saveChunkToFile(const Chunk& chunk, const std::string& filename) {
    // ʹ�ö����Ƹ�ʽ���������е�����λ��
	// Save voxel positions in the chunk in binary format
    std::ofstream outFile(filename, std::ios::binary);
    
    if (outFile.is_open()) {
        // ������������
		// Save chunk data
        // ���������е�ÿ������λ�ã���д���ļ�
		// Traverse each voxel position in the chunk and write it to the file
        for (const auto& pos : chunk.getVoxelPositions()) {
            outFile.write(reinterpret_cast<const char*>(&pos), sizeof(glm::vec3));
        }
        outFile.close();
        //std::cerr << "Saved chunk to file: " << filename << std::endl;
    }
}
