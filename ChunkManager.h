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
    void clearChunks();// ����Ѽ��ص�����
    void stopLoading();// ֹͣ�������飨��������ͣ����ļ��أ�
	void startLoading();// ��ʼ�������飨�����ڻָ�����ļ��أ�
    bool isLoading = true; // ��־�Ƿ����ڼ�������
    // �����������ͺ�Ȩ��
    void setNoiseWeights(float weight1, float weight2);


private:
    int chunkSize;
    glm::vec3 lastCameraPosition; // ��һ֡�������λ��
    FastNoiseLite noise1; // ��һ������������
    FastNoiseLite noise2; // �ڶ�������������
    float weight1 = 1.0f; // ��һ��������Ȩ��
    float weight2 = 0.0f; // �ڶ���������Ȩ��
	float THRESHOLD = 0.3f; // ��ֵ
	int SEED = 1234; // �������

    std::string getChunkKey(const glm::vec3& position);
    void loadChunk(const glm::vec3& position);
    void saveChunkToFile(const Chunk& chunk, const std::string& filename);
};

#endif // CHUNK_MANAGER_H
