# Procedural Generation of Infinite Voxel Cave Terrain
# 程序化生成体素洞穴地形

## Introduce 简介

This is a Windows-based project for procedurally generating voxel caves. It primarily implements chunk-based data structures, uses noise functions as thresholds for cave and terrain generation, employs OpenGL to render voxel terrain, optimizes performance with voxel face culling and frustum culling, and utilizes multithreading to separate rendering from generation logic, enabling the creation of an infinite voxel cave terrain.

这是一个Windows平台的程序化生成体素洞穴工程, 主要实现了以区块为数据结构, 以噪声为洞穴和地形的生成阈值, 使用OpenGL渲染体素地形, 使用体素相邻面剔除和视锥体剔除技术优化性能, 使用多线程将渲染与生成逻辑分离, 实现生成无限大的体素洞穴地形的程序

## Features 特性

- Voxel Chunk Data Structure: Uses chunks as the basic data structure for managing and rendering the scene.
- Noise-based Cave and Terrain Generation: Defines generation thresholds for caves and terrain using noise functions to simulate natural landscapes.
- OpenGL Rendering: Renders voxel terrain using OpenGL technology.
- Performance Optimization:
  - Voxel Face Culling: Renders only visible voxel faces, reducing unnecessary rendering load.
  - Frustum Culling: Renders only the chunks within the view frustum, improving rendering performance.
  - Multithreading: Separates rendering and generation logic, enabling parallel processing of generation and rendering.

- 体素区块数据结构: 使用区块作为基础数据结构，实现场景的管理和渲染。
- 噪声生成洞穴和地形: 利用噪声函数定义洞穴和地形的生成阈值，模拟自然地形。
- OpenGL渲染: 使用OpenGL技术实现体素地形渲染。
- 性能优化:
  - 体素相邻面剔除: 仅渲染可见的体素面，减少不必要的渲染开销。
  - 视锥体剔除: 仅渲染在视野范围内的区块，提高渲染性能。
  - 多线程处理: 渲染与生成逻辑分离，实现生成和渲染的并行处理。

## 重点功能实现总结

### 1. 噪声地形区块生成

用三维布尔类型的数组储存体素是否被填充:

``` c++
chunkBlocks = vector<vector<vector<bool>>>(size, vector<vector<bool>>(chunkHeight, vector<bool>(size, false)));
```

通过噪声值计算地形的高度, 留出地形最小高度

```c++
int terrainHeight = chunkHeight - 4 + static_cast<int>((noiseValue + 1.0f) * 0.5f * 4);// 这里的4是地形的最小高度
```

通过噪声值判断地表下方一定高度y之下是否为洞穴:

```c++
if (caveNoise < THRESHOLD) {
  chunkBlocks[x][y][z] = true;
  voxelPositions.push_back(glm::vec3(x, y, z));
}
```

### 2. 体素相邻面剔除

通过获遍历数组每一个体素, 通过它六个方向的相邻体素的bool值判断相邻位置是否有体素, 如果没有体素, 则将此面的顶点添加进可以见面顶点数组中

```c++
if (!isVoxelAt(x + 1, y, z)) {
  visibleFaces.emplace_back(voxelPosition, Face::RIGHT_FACE);
  for (int i = 24; i < 30; ++i) {
    chunkVisibleFacesVertices.push_back({ voxelVertices[i].position + voxelPosition, voxelVertices[i].texCoords });
  }
}
```

### 3. 渲染与地形生成的多线程实现

原子

```c++
std::atomic<bool> updateChunks(true);
```

单独用一个线程来控制区块更新

```c++
static void updateChunksThread(ChunkManager& chunkManager, std::atomic<bool>& running) {
	while (running)
	{
		chunkManager.update(camera.Position);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
```

创建线程:

```c++
std::thread chunkUpdateThread(updateChunksThread, std::ref(chunkManager), std::ref(updateChunks));
```

在 ChunkManager 类中使用Mutex锁来控制可能冲突的资源

```c++
std::mutex chunksMutex; // Mutex lock
```

在需要对 chunks 进行修改时, 需要先lock

```c++
std::lock_guard<std::mutex> lock(chunksMutex);
```

### 4. 视锥体剔除

用 vec4 plane[6] 表示视锥体的6个面, 每个面的方程式为ax + by + cz + d = 0, 其中 a, b, c, d 在 vec4 中的位置是 (a, b, c, d)

```c++
planes[0] = glm::vec4(
    projectionViewMatrix[0][3] + projectionViewMatrix[0][0], // Left
    projectionViewMatrix[1][3] + projectionViewMatrix[1][0],
    projectionViewMatrix[2][3] + projectionViewMatrix[2][0],
    projectionViewMatrix[3][3] + projectionViewMatrix[3][0]);

```

传入的参数为区块的两个边界顶点坐标 max, min 分别是坐标 x y z 的最大值点和最小值点

通过方法 isAABBInFrustum , 以法向量(vec4:plane[i]的前三个元素构成的vec3)的x y z 正负判断, 如果 plane[i].x 为正, 则选择体素顶点中x最大的值: max.x, 以此类推, 得到最后结果点 P

通过法向量与 P 点的点成, 计算距离, w 是视锥体的各个平面的偏移量
如果结果小于0 , 则说明这个点在平面外, 返回 false , 如果所有平面都检查通过, 则返回 true

```c++
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
```

### 5. 区块打包VAO/VBO

VAO是顶点数组对象, VBO是顶点缓冲对象, 这里将一个区块的所有顶点绑定到一个VBO上

```c++
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
```

## Installation and Usage 安装与使用

### System Requirements 系统需求

 - Windows
 - OpenGL3.0+
 - Visual Studio 2022

## Installation Steps 安装步骤

Simply clone the repository and double-click the .sln file to open it with Visual Studio.

直接克隆仓库并双击sln使用VisualStudio打开即可

## Screenshot 示例截图

![cave_overview](cave_overview.png)

## Contact Information 联系方式

- Email: mickeymiao2023@163.com
- WeChat: SiMiao1106

- 邮箱: mickeymiao2023@163.com
- 微信: SiMiao1106




