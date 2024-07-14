#include "ChunkManager.h"

// 构造函数，初始化区块大小
ChunkManager::ChunkManager(int chunkSize) : chunkSize(chunkSize) {

    std::cout << "-------------- Initialize --------------" << std::endl;

	// 保存最后一帧的摄像机位置
	lastCameraPosition = glm::vec3(0.0f);

    // 如果不存在chunks文件夹，则创建
    if (!std::filesystem::exists("chunks")) {
        std::filesystem::create_directory("chunks");
    }

    // 临时存储新加载的区块
	std::unordered_map<std::string, Chunk> tempChunks;

    // 遍历摄像机周围的区块
    // 3×3的范围，即8个相邻区块加上摄像机所在的区块
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dz = -1; dz <= 1; ++dz) {
            glm::vec3 chunkPos = glm::vec3(dx, 0.0f, dz);// 计算相邻区块的位置
            chunkPos.x *= chunkSize;
            chunkPos.y *= chunkSize;
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
	int y = static_cast<int>(floor(position.y / chunkSize));
    int z = static_cast<int>(floor(position.z / chunkSize));
    return std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
}

// 更新当前加载的区块
void ChunkManager::update(const glm::vec3& cameraPosition) {
	std::unordered_map<std::string, Chunk> tempChunks;// 临时存储新加载的区块

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
    std::cout << "-------------- Chunk has updated --------------" << std::endl;

    // 打印摄像机所在的区块位置
    std::cout << "Camera chunk position: " << cameraChunkPosition.x << ", " << cameraChunkPosition.y << ", " << cameraChunkPosition.z << std::endl;
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
            chunkPos.y *= chunkSize;
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

    // 打印当前已有的区块数量
    std::cout << "Number of chunks: " << chunks.size() << std::endl;

    // 卸载不再需要的区块（不在新加载的区块范围内）
    for (const auto& chunk : chunks) {
        if (tempChunks.find(chunk.first) == tempChunks.end()) {
			std::cout << "Unloading chunk: " << chunk.first << std::endl;
            unloadChunk(chunk.first);
        }
    }
    // 更新chunks成员变量
    chunks = tempChunks;
}

// 加载区块
void ChunkManager::loadChunk(const glm::vec3& position) {
	// 根据区块的位置生成唯一的键
    std::string key = getChunkKey(position);
	// 生成区块文件名
    std::string filename = "chunks/" + key + ".chunk";
    // 从文件加载区块
    Chunk chunk = loadChunkFromFile(filename);

    // 如果文件不存在或读取失败，则初始化一个新的区块
    // 如果从文件加载的区块为空，则初始化一个新的区块
    if (chunk.getVoxelWorldPositions().empty()) {
        std::cout << "Creating new chunk at position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
        chunk = Chunk(chunkSize, position);
        chunk.initializeChunk();
    }

    // 将加载的区块存储到chunks中
    chunks[key] = chunk;
}

// 卸载区块
void ChunkManager::unloadChunk(const std::string& key) {
    std::string filename = "chunks/" + key + ".chunk";
	std::cout << "Unloading chunk: " << filename << std::endl;
    // 将区块保存到文件
    saveChunkToFile(chunks[key], filename);
	// 从chunks中移除区块
	// 这里不需要从chunks中移除区块，因为在update函数中已经更新了chunks成员变量, 会覆盖掉之前的区块
    // chunks.erase(key);
}

// 将区块数据保存到文件
void ChunkManager::saveChunkToFile(const Chunk& chunk, const std::string& filename) {
    //使用二进制格式保存区块中的体素位置
    std::ofstream outFile(filename, std::ios::binary);
    if (outFile.is_open()) {
        // 保存区块数据
        // 遍历区块中的每个体素位置，并写入文件
        for (const auto& pos : chunk.getVoxelWorldPositions()) {
            outFile.write(reinterpret_cast<const char*>(&pos), sizeof(glm::vec3));
        }
        outFile.close();
    }
    else {
        std::cerr << "Failed to save chunk to file: " << filename << std::endl;
    }
}

// 从文件加载区块数据
Chunk ChunkManager::loadChunkFromFile(const std::string& filename) {
    Chunk chunk(chunkSize, glm::vec3(0.0f)); // 创建空的Chunk对象

	std::ifstream inFile(filename, std::ios::binary);// ifstream对象用于读取文件, 以二进制格式打开文件
    if (inFile.is_open()) {
        // 读取区块数据
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