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
    void clearChunks();// ����Ѽ��ص�����
    void stopLoading();// ֹͣ�������飨��������ͣ����ļ��أ�
	void startLoading();// ��ʼ�������飨�����ڻָ�����ļ��أ�    
    void setNoiseWeights(float weight1, float weight2);// ��������Ȩ��
	bool getIsLoading() const;
	std::unordered_map<std::string, Chunk>& getChunks();

private:
	std::mutex chunksMutex;
    int chunkSize;
    glm::vec3 lastCameraPosition; // ��һ֡�������λ��
    FastNoiseLite noise1; // ��һ������������
    FastNoiseLite noise2; // �ڶ�������������
    float weight1 = 1.0f; // ��һ��������Ȩ��
    float weight2 = 0.0f; // �ڶ���������Ȩ��
	float THRESHOLD = 0.3f; // ��ֵ
	int SEED = 1234; // �������
    bool isLoading = false; // ��־�Ƿ����ڼ�������
    std::unordered_map<std::string, Chunk> chunks;

    std::string getChunkKey(const glm::vec3& position);
    void loadChunk(const glm::vec3& position);
    void saveChunkToFile(const Chunk& chunk, const std::string& filename);
};

#endif // CHUNK_MANAGER_H
