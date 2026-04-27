#include "ChunkManager.h"

// 构造函数，初始化区块大小
// Constructor, initialize chunk size
ChunkManager::ChunkManager(int chunkSize, const std::string& baseDir) : chunkSize(chunkSize), baseDir(baseDir) {

    std::cout << "Initializing Origin Chunks" << std::endl;

	// 保存最后一帧的摄像机位置
	// Save the camera position of the last frame
	lastCameraPosition = glm::vec3(0.0f);

    // 如果不存在chunks文件夹，则创建
	// Create the chunks folder if it doesn't exist
    if (!std::filesystem::exists(baseDir + "chunks")) {
        std::filesystem::create_directory(baseDir + "chunks");
    }

    // 配置噪声生成器
	// Configure noise generator
    noise1.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise1.SetFrequency(0.1f);
    noise1.SetSeed(SEED);

    noise2.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    noise2.SetFrequency(0.1f);
    noise2.SetSeed(SEED);

	THRESHOLD = 0.3f; // 阈值

    // 临时存储新加载的区块
	// Temporary storage for newly loaded chunks
	std::unordered_map<std::string, std::shared_ptr<Chunk>> tempChunks;

    // 遍历摄像机周围的区块
    // Traverse the chunks around the camera
    for (int dx = -viewDistance; dx <= viewDistance; ++dx) {
        for (int dz = -viewDistance; dz <= viewDistance; ++dz) {
            glm::vec3 chunkPos = glm::vec3(dx, 0.0f, dz);
            chunkPos.x *= chunkSize;
            chunkPos.z *= chunkSize;
            std::string key = getChunkKey(chunkPos);

            if (chunks.find(key) == chunks.end()) {
                loadChunk(chunkPos);
            }
            tempChunks[key] = chunks.at(key);
        }
    }

    chunks = std::move(tempChunks);
}

// 根据区块的位置生成一个唯一的字符串键，用于标识区块
std::string ChunkManager::getChunkKey(const glm::vec3& position) {
    int x = static_cast<int>(floor(position.x / chunkSize));
    int z = static_cast<int>(floor(position.z / chunkSize));
    return std::to_string(x) + "_" + std::to_string(z);
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
	std::filesystem::remove_all(baseDir + "chunks");
	std::filesystem::create_directory(baseDir + "chunks");
}

void ChunkManager::forEachChunk(const std::function<void(const std::string&, Chunk&)>& fn)
{
    std::lock_guard<std::mutex> lock(chunksMutex);
    for (auto& [key, ptr] : chunks) {
        fn(key, *ptr);
    }
}

size_t ChunkManager::getChunkCount()
{
    std::lock_guard<std::mutex> lock(chunksMutex);
    return chunks.size();
}

// 更新当前加载的区块
// Update the currently loaded chunks
void ChunkManager::update(const glm::vec3& cameraPosition) {

    if (!isLoading) {
        return;
    }

	std::unordered_map<std::string, std::shared_ptr<Chunk>> tempChunks;

    glm::vec3 cameraChunkPosition = glm::vec3(
        floor(cameraPosition.x / chunkSize),
        0.0f,
        floor(cameraPosition.z / chunkSize)
    );

	if (cameraChunkPosition == lastCameraPosition && chunks.size() != 0) {
		return;
	}

    lastCameraPosition = cameraChunkPosition;
    
    for (int dx = -viewDistance; dx <= viewDistance; ++dx) {
        for (int dz = -viewDistance; dz <= viewDistance; ++dz) {
            glm::vec3 chunkPos = cameraChunkPosition + glm::vec3(dx, 0.0f, dz);
            chunkPos.x *= chunkSize;
            chunkPos.z *= chunkSize;
            std::string key = getChunkKey(chunkPos);

            if (chunks.find(key) == chunks.end()) {
                loadChunk(chunkPos);
            }
            tempChunks[key] = chunks.at(key);
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(chunksMutex);
        chunks = std::move(tempChunks);
    }
}

void ChunkManager::clearChunks()
{
    std::lock_guard<std::mutex> lock(chunksMutex);
    chunks.clear();
}

void ChunkManager::stopLoading()
{
    isLoading = false;
}

void ChunkManager::startLoading()
{
	isLoading = true;
}

// 加载区块
// Load chunks
void ChunkManager::loadChunk(const glm::vec3& position) {
    std::string key = getChunkKey(position);
    std::string filename = baseDir + "chunks/" + key + ".chunk";

    auto chunk = std::make_shared<Chunk>(chunkSize, position);

    std::ifstream inFile(filename, std::ios::binary);
    if (inFile.is_open()) {
        glm::vec3 pos;
        while (inFile.read(reinterpret_cast<char*>(&pos), sizeof(glm::vec3))) {
            chunk->addVoxel(pos);
        }
		chunk->generateVisibleFaces();
        inFile.close();
    }
    else {
        chunk->initializeChunk(noise1, noise2, weight1, weight2, THRESHOLD);
        saveChunkToFile(*chunk, filename);
    }

    {
        std::lock_guard<std::mutex> lock(chunksMutex);
        chunks[key] = std::move(chunk);
    }
}

// 将区块数据保存到文件
// Save chunk data to file
void ChunkManager::saveChunkToFile(const Chunk& chunk, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary);
    
    if (outFile.is_open()) {
        for (const auto& pos : chunk.getVoxelPositions()) {
            outFile.write(reinterpret_cast<const char*>(&pos), sizeof(glm::vec3));
        }
        outFile.close();
    }
}
