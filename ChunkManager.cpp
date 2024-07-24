#include "ChunkManager.h"

// 构造函数，初始化区块大小
ChunkManager::ChunkManager(int chunkSize) : chunkSize(chunkSize) {

    //std::cout << "-------------- Initialize --------------" << std::endl;

	// 保存最后一帧的摄像机位置
	lastCameraPosition = glm::vec3(0.0f);

    // 如果不存在chunks文件夹，则创建
    if (!std::filesystem::exists("chunks")) {
        std::filesystem::create_directory("chunks");
    }

    // 配置噪声生成器
    // 初始化噪声生成器
    noise1.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise1.SetFrequency(0.1f);
    noise1.SetSeed(SEED);

    noise2.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise2.SetFrequency(0.1f);
    noise2.SetSeed(SEED);

	THRESHOLD = 0.3f; // 阈值

    // 临时存储新加载的区块
	std::unordered_map<std::string, Chunk> tempChunks;

    // 遍历摄像机周围的区块
    // 3×3的范围，即8个相邻区块加上摄像机所在的区块
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dz = -1; dz <= 1; ++dz) {
            glm::vec3 chunkPos = glm::vec3(dx, 0.0f, dz);// 计算相邻区块的位置
            chunkPos.x *= chunkSize;
            //chunkPos.y *= chunkSize;
            chunkPos.z *= chunkSize;
            std::string key = getChunkKey(chunkPos);// 生成区块的键

            //这里的.find()函数是在unordered_map中查找键值为key的元素，如果找到了就返回指向该元素的迭代器，否则返回unordered_map::end()函数返回的迭代器
            if (chunks.find(key) == chunks.end()) {
                // 如果区块未被加载，则调用loadChunk加载区块
                loadChunk(chunkPos);
            }
            //  临时存储需要保留的区块
            tempChunks[key] = chunks[key];
        }
    }

    // 更新chunks成员变量
    chunks = tempChunks;
}

// 根据区块的位置生成一个唯一的字符串键，用于标识区块
// - 将区块的坐标除以区块的大小，然后向下取整，确保获取的是区块的整数坐标
// - 将整数坐标转换为字符串，用下划线分隔，作为键
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

bool ChunkManager::getIsLoading() const
{
    return isLoading;
}

std::unordered_map<std::string, Chunk>& ChunkManager::getChunks()
{
	return chunks;
}

// 更新当前加载的区块
void ChunkManager::update(const glm::vec3& cameraPosition) {

    if (!isLoading) {
        return; // 如果停止加载，则不进行更新
    }

	std::unordered_map<std::string, Chunk> tempChunks;// 临时存储新加载的区块
	tempChunks.clear();

	// 计算摄像机所在的区块位置
    glm::vec3 cameraChunkPosition = glm::vec3(
        floor(cameraPosition.x / chunkSize),
        0.0f,
        floor(cameraPosition.z / chunkSize)
    );

    // 如果摄像机位置没有改变，则不需要更新区块
    if (cameraChunkPosition == lastCameraPosition) {
        return;
    }

	// 清除内存中的区块数据
	chunks.clear();
    //std::cout << "-------------- Chunk has updated --------------" << std::endl;

    // 打印摄像机所在的区块位置
    //std::cout << "Camera chunk position: " << cameraChunkPosition.x << ", " << cameraChunkPosition.y << ", " << cameraChunkPosition.z << std::endl;
    // 打印当前已有的区块数量
    // std::cout << "Number of chunks: " << chunks.size() << std::endl;
    // 更新上一帧的摄像机位置
    lastCameraPosition = cameraChunkPosition;
    
    // 遍历摄像机周围的区块
	// 3×3的范围，即8个相邻区块加上摄像机所在的区块
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dz = -1; dz <= 1; ++dz) {
            glm::vec3 chunkPos = cameraChunkPosition + glm::vec3(dx, 0.0f, dz);// 计算相邻区块的位置
            chunkPos.x *= chunkSize;
            //chunkPos.y *= chunkSize;
            chunkPos.z *= chunkSize;
            std::string key = getChunkKey(chunkPos);// 生成区块的键

			//这里的.find()函数是在unordered_map中查找键值为key的元素，如果找到了就返回指向该元素的迭代器，否则返回unordered_map::end()函数返回的迭代器
            if (chunks.find(key) == chunks.end()) {
                // 如果区块未被加载，则调用loadChunk加载区块
                loadChunk(chunkPos);
            }
            //  临时存储需要保留的区块
            tempChunks[key] = chunks[key];
        }
    }
    
    // 更新chunks成员变量
    chunks = tempChunks;
}

void ChunkManager::clearChunks()
{
    chunks.clear(); // 清空已加载的区块
    //std::cout << "Cleared all loaded chunks." << std::endl;
}

void ChunkManager::stopLoading()
{
    isLoading = false; // 停止加载区块
    //std::cout << "Stopped loading chunks." << std::endl;
}

void ChunkManager::startLoading()
{
	isLoading = true; // 开始加载区块
	//std::cout << "Started loading chunks." << std::endl;
}

// 加载区块
void ChunkManager::loadChunk(const glm::vec3& position) {
	// 根据区块的位置生成唯一的键
    std::string key = getChunkKey(position);
	// 生成区块文件名
    std::string filename = "chunks/" + key + ".chunk";

    Chunk chunk(chunkSize, position); // 创建空的Chunk对象

    std::ifstream inFile(filename, std::ios::binary);// ifstream对象用于读取文件, 以二进制格式打开文件
    if (inFile.is_open()) {
        // 读取区块数据
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


    // 将加载的区块存储到chunks中
    chunks[key] = chunk;
}

// 将区块数据保存到文件
void ChunkManager::saveChunkToFile(const Chunk& chunk, const std::string& filename) {
    // 使用二进制格式保存区块中的体素位置
    std::ofstream outFile(filename, std::ios::binary);
    
    if (outFile.is_open()) {
        // 保存区块数据
        // 遍历区块中的每个体素位置，并写入文件
        for (const auto& pos : chunk.getVoxelPositions()) {
            outFile.write(reinterpret_cast<const char*>(&pos), sizeof(glm::vec3));
        }
        outFile.close();
        //std::cerr << "Saved chunk to file: " << filename << std::endl;
    }
    else {
        //std::cerr << "Failed to save chunk to file: " << filename << std::endl;
    }
}
