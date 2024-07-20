#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "Chunk.h"

class ChunkManager {
public:
    ChunkManager(int chunkSize);
    void update(const glm::vec3& cameraPosition);
    std::unordered_map<std::string, Chunk> chunks; 
    void clearChunks();// 清空已加载的区块
    void stopLoading();// 停止加载区块（可用于暂停区块的加载）
	void startLoading();// 开始加载区块（可用于恢复区块的加载）
    bool isLoading = true; // 标志是否正在加载区块
    // 设置噪声类型和权重
    void setNoiseWeights(float weight1, float weight2);


private:
    int chunkSize;
    glm::vec3 lastCameraPosition; // 上一帧的摄像机位置
    FastNoiseLite noise1; // 第一种噪声生成器
    FastNoiseLite noise2; // 第二种噪声生成器
    float weight1 = 1.0f; // 第一种噪声的权重
    float weight2 = 0.0f; // 第二种噪声的权重
	float THRESHOLD = 0.3f; // 阈值
	int SEED = 1234; // 随机种子

    std::string getChunkKey(const glm::vec3& position);
    void loadChunk(const glm::vec3& position);
    void saveChunkToFile(const Chunk& chunk, const std::string& filename);
};

#endif // CHUNK_MANAGER_H
