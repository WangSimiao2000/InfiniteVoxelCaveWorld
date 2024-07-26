#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Frustum {
public:
    // 计算视锥体平面
    void calculateFrustum(const glm::mat4& projectionViewMatrix) {
        planes[0] = glm::vec4(projectionViewMatrix[0][3] + projectionViewMatrix[0][0], // Left
            projectionViewMatrix[1][3] + projectionViewMatrix[1][0],
            projectionViewMatrix[2][3] + projectionViewMatrix[2][0],
            projectionViewMatrix[3][3] + projectionViewMatrix[3][0]);
        planes[1] = glm::vec4(projectionViewMatrix[0][3] - projectionViewMatrix[0][0], // Right
            projectionViewMatrix[1][3] - projectionViewMatrix[1][0],
            projectionViewMatrix[2][3] - projectionViewMatrix[2][0],
            projectionViewMatrix[3][3] - projectionViewMatrix[3][0]);
        planes[2] = glm::vec4(projectionViewMatrix[0][3] + projectionViewMatrix[0][1], // Bottom
            projectionViewMatrix[1][3] + projectionViewMatrix[1][1],
            projectionViewMatrix[2][3] + projectionViewMatrix[2][1],
            projectionViewMatrix[3][3] + projectionViewMatrix[3][1]);
        planes[3] = glm::vec4(projectionViewMatrix[0][3] - projectionViewMatrix[0][1], // Top
            projectionViewMatrix[1][3] - projectionViewMatrix[1][1],
            projectionViewMatrix[2][3] - projectionViewMatrix[2][1],
            projectionViewMatrix[3][3] - projectionViewMatrix[3][1]);
        planes[4] = glm::vec4(projectionViewMatrix[0][3] + projectionViewMatrix[0][2], // Near
            projectionViewMatrix[1][3] + projectionViewMatrix[1][2],
            projectionViewMatrix[2][3] + projectionViewMatrix[2][2],
            projectionViewMatrix[3][3] + projectionViewMatrix[3][2]);
        planes[5] = glm::vec4(projectionViewMatrix[0][3] - projectionViewMatrix[0][2], // Far
            projectionViewMatrix[1][3] - projectionViewMatrix[1][2],
            projectionViewMatrix[2][3] - projectionViewMatrix[2][2],
            projectionViewMatrix[3][3] - projectionViewMatrix[3][2]);

        for (int i = 0; i < 6; i++) {
            float length = glm::length(glm::vec3(planes[i]));
            planes[i] /= length;
        }
    }

    // 检查点是否在视锥体内
    bool isPointInFrustum(const glm::vec3& point) const {
        for (int i = 0; i < 6; i++) {
            if (glm::dot(glm::vec3(planes[i]), point) + planes[i].w <= 0) {
                return false;
            }
        }
        return true;
    }

    bool isVoxelInFrustum(const glm::vec3& position) const {
        glm::vec3 min = position - glm::vec3(0.5f, 0.5f, 0.5f);
        glm::vec3 max = position + glm::vec3(0.5f, 0.5f, 0.5f);
        return isAABBInFrustum(min, max);
    }

    // 检查AABB是否在视锥体内, 这里的AABB表示一个立方体，由最小点和最大点确定
    bool isAABBInFrustum(const glm::vec3& min, const glm::vec3& max) const {
        for (int i = 0; i < 6; i++) {
            glm::vec3 p = min;
            if (planes[i].x >= 0) p.x = max.x;
            if (planes[i].y >= 0) p.y = max.y;
            if (planes[i].z >= 0) p.z = max.z;
            if (glm::dot(glm::vec3(planes[i]), p) + planes[i].w < 0) {
                return false;
            }
        }
        return true;
    }

private:
    glm::vec4 planes[6];
};

#endif // FRUSTUM_H
