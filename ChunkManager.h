#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <mutex>
#include "Chunk.h"

class ChunkManager {
public:
    ChunkManager(int chunkSize);
    void update(const glm::vec3& cameraPosition);
    void clearChunks();// 清空已加载的区块
    void stopLoading();// 停止加载区块（可用于暂停区块的加载）
	void startLoading();// 开始加载区块（可用于恢复区块的加载）    
    void setNoiseWeights(float weight1, float weight2);// 设置噪声权重
	void setViewDistance(int distance);// 设置可视距离
    void clearChunksFolder();// 清空区块文件夹
	bool getIsLoading() const;
	std::unordered_map<std::string, Chunk>& getChunks();

private:
	std::mutex chunksMutex;// 互斥锁
	int chunkSize;// 区块大小
    glm::vec3 lastCameraPosition; // 上一帧的摄像机位置
    FastNoiseLite noise1; // 第一种噪声生成器
    FastNoiseLite noise2; // 第二种噪声生成器
    float weight1 = 1.0f; // 第一种噪声的权重
    float weight2 = 0.0f; // 第二种噪声的权重
	float THRESHOLD = 0.3f; // 阈值
	int SEED = 1234; // 随机种子
    bool isLoading = false; // 标志是否正在加载区块
    std::unordered_map<std::string, Chunk> chunks;
	int viewDistance = 2; // 可视距离, 以区块为单位, 以相机为中心的正方形区域, 边长为2*2+1, 即所在区块加上两个方向各延申2个区块 

    std::string getChunkKey(const glm::vec3& position);
    void loadChunk(const glm::vec3& position);
    void saveChunkToFile(const Chunk& chunk, const std::string& filename);
};

#endif // CHUNK_MANAGER_H
