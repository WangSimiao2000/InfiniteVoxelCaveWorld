#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"
#include "Chunk.h"
#include "ChunkManager.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);//鼠标回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//滚轮回调函数

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 750;

// camera
Camera camera(glm::vec3(0.0f, 22.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -105, -30);//创建摄像机对象, 参数分别为摄像机的位置, 世界上方向, Yaw角, Pitch角

//Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));//创建摄像机对象, 参数分别为摄像机的位置

float lastX = SCR_WIDTH / 2.0f;//鼠标初始位置
float lastY = SCR_HEIGHT / 2.0f;//鼠标初始位置
bool firstMouse = true;//第一次鼠标移动

// timing
float deltaTime = 0.0f;	// 当前帧与上一帧的时间差
float lastFrame = 0.0f;// 上一帧的时间

// 全局变量用于记录当前的绘图模式
bool isWireframe = false;
bool mouseRightPressed = false;
bool cameraControlEnabled = true;

int chunkSize = 16;//区块大小

struct Frustum {
	glm::vec4 planes[6];

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
};

int main()
{
	glfwInit();//初始化GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设置主版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设置次版本号
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//设置OpenGL配置文件

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);//macOS系统需要设置
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "InfiniteVoxelWorld", NULL, NULL);//创建窗口
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);//将窗口的上下文设置为当前线程的主上下文
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//设置窗口大小改变时的回调函数
	glfwSetCursorPosCallback(window, mouse_callback);//设置鼠标回调函数
	glfwSetScrollCallback(window, scroll_callback);//设置滚轮回调函数

	// 告诉GLFW我们想要捕捉所有的鼠标输入
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//初始化GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 开启深度测试
	glEnable(GL_DEPTH_TEST);

	// 开启面剔除
	glEnable(GL_CULL_FACE);

	Shader ourShader("VertexShader.vert", "FragmentShader.frag");//创建着色器对象

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// 设置平台/渲染器绑定
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // 第二个参数是是否捕捉鼠标, 这里的YOUR_WINDOW是你的GLFW窗口,应该改成你的窗口变量名
	ImGui_ImplOpenGL3_Init();

	// ---- 加载和创建纹理 - START ---- //
	unsigned int texture;//纹理ID
	glGenTextures(1, &texture);//生成纹理对象
	// 这段可以写在循环外面, 因为我们渲染时不会改变纹理, 所以这段代码只需要在渲染循环外执行一次即可(多个纹理也是一样, 除非我们需要在渲染时改变纹理, 这时就需要在渲染循环内执行)
	glActiveTexture(GL_TEXTURE0);//激活纹理单元, 默认激活的是GL_TEXTURE0, 所以这行代码其实可以省略
	glBindTexture(GL_TEXTURE_2D, texture);//绑定纹理对象
	// 设置纹理环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//设置S轴的环绕方式为GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//设置T轴的环绕方式为GL_REPEAT
	// 设置纹理过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//设置缩小过滤方式为GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//设置放大过滤方式为GL_LINEAR

	// 加载纹理图片, 创建纹理, 生成Mipmap
	int width, height, nrChannels;//图片宽度, 高度, 颜色通道数,这里的width, height, nrChannels是通过stbi_load函数返回的
	stbi_set_flip_vertically_on_load(true);//翻转图片y轴, 因为OpenGL的坐标原点在窗口左下角, 而图片的坐标原点在左上角
	//unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);//加载箱子纹理图片
	unsigned char* data = stbi_load("stone_16.png", &width, &height, &nrChannels, 0);//加载minecraft石头纹理图片
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);//生成纹理, 参数分别为纹理目标, mipmap级别, 纹理存储格式, 宽, 高, 0, 源图格式, 源图数据类型, 图像数据
		//glGenerateMipmap(GL_TEXTURE_2D);//生成Mipmap
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);//释放图像内存

	// 设置光照方向
	glm::vec3 lightDirection = glm::vec3(0.35f, 0.6f, 0.10f);
	lightDirection = glm::normalize(lightDirection);

	ourShader.use();//使用着色器程序
	ourShader.setVec3("lightDir", lightDirection);//设置光照方向
	ourShader.setInt("ourTexture", 0);//设置纹理单元, 这里的ourTexture对应的是着色器里声明的uniform sampler2D ourTexture, 0表示使用纹理单元GL_TEXTURE0, 当只有一个纹理单元时, 引号内的ourTexture不会影响结果, 但是如果有多个纹理单元时, 这个名字就很重要了
	//glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);//和上面一行代码功能一样, 设置纹理单元, 上面一行代码是通过着色器类的函数设置, 这行代码是直接通过glUniform1i函数设置, 这里注释掉, 仅供参考
	// ---- 加载和创建纹理 - END ---- //

	ChunkManager chunkManager(chunkSize);//创建区块管理器对象	

	while (!glfwWindowShouldClose(window))//循环渲染
	{
		float currentFrame = static_cast<float>(glfwGetTime());//获取当前时间
		deltaTime = currentFrame - lastFrame;//计算时间差
		float fps = 1.0f / deltaTime;//计算帧率
		lastFrame = currentFrame;//更新上一帧时间

		// 开始ImGui帧
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			// 根据需要启用或禁用Dear ImGui对鼠标的绑定
			if (cameraControlEnabled) {
				io.ConfigFlags |= ImGuiConfigFlags_NoMouse; // 设置标志，禁用鼠标
			}
			else {
				io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse; // 清除标志，启用鼠标
			}

			ImGui::SetNextWindowPos(ImVec2(5, 5));// 设置窗口位置
			ImGui::SetNextWindowSize(ImVec2(435, 200));
			ImGui::Begin("InfiniteVoxelWorld", nullptr, ImGuiWindowFlags_NoResize);// 创建窗口并显示信息

			// 禁用

			{
				ImGui::Text("FPS: %.1f", fps);//显示帧率
				ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);//显示相机位置		
				ImGui::Text("Camera Chunk Position: (%.0f, %.0f)", floor(camera.Position.x / chunkSize), floor(camera.Position.z / chunkSize));//显示相机所在区块位置
			}

			ImGui::Separator(); // 添加分隔线

			{
				ImGui::BeginChild("Chunk Manage", ImVec2(200, 100), true);//创建一个子窗口, 200是子窗口的宽度, 0是子窗口的高度, true表示子窗口有滚动条
				// 重置按钮
				if (ImGui::Button("Reset Chunks and Camera")) {
					chunkManager.clearChunks();
					camera.Position = glm::vec3(0.0f, 22.5f, 0.0f);
				}
				// 停止加载按钮
				if (chunkManager.getIsLoading())
				{
					if (ImGui::Button("Stop Loading Chunks")) {
						chunkManager.stopLoading();
					}
				}
				else
				{
					if (ImGui::Button("Start Loading Chunks")) {
						chunkManager.startLoading();
					}
				}
				ImGui::EndChild();
			}

			ImGui::SameLine(); // 在同一行继续绘制

			{
				ImGui::BeginChild("Noise Weight Settings", ImVec2(200, 100), true);//创建一个子窗口, 200是子窗口的宽度, 0是子窗口的高度, true表示子窗口有滚动条
				static float weight1 = 1.0f;
				float weight2 = 1.0f - weight1;

				// 修改噪声权重
				ImGui::SliderFloat("##", &weight1, 0.0f, 1.0f); // 使用##去掉滑动条标签
				ImGui::Text("OpenSimplex2 Weight: %.2f", weight1); // 显示手动设置的 weight1
				ImGui::Text("Perlin Weight: %.2f", weight2); // 显示自动计算的 weight2
				// 应用噪声设置
				if (ImGui::Button("Apply Weight")) {
					chunkManager.setNoiseWeights(weight1, weight2);
				}
				ImGui::EndChild();
			}

			// 打印当前imgui窗口的大小
			//ImVec2 windowSize = ImGui::GetWindowSize();
			//ImGui::Text("Window Size: (%.0f, %.0f)", windowSize.x, windowSize.y);

			ImGui::End();
		}		
		
		// 更新相机位置
		glm::vec3 cameraPosition = camera.Position;
		// 更新区块管理器
		chunkManager.update(cameraPosition);

		// input
		processInput(window);
		
		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //设置清空屏幕所用的颜色为深蓝色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空颜色缓冲

		ourShader.use();//使用着色器程序

		ourShader.setVec3("viewPos", cameraPosition); // 将相机位置传递给着色器

		// 相机/观察矩阵
		glm::mat4 view = camera.GetViewMatrix();//获取观察矩阵
		ourShader.setMat4("view", view);//设置观察矩阵
		// 传递投影矩阵给着色器
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);//透视投影
		ourShader.setMat4("projection", projection);//设置投影矩阵

		glm::mat4 projectionViewMatrix = projection * view;//投影矩阵 * 观察矩阵 = 投影观察矩阵
		Frustum frustum;
		frustum.calculateFrustum(projectionViewMatrix);

		//glBindVertexArray(VAO);//绑定VAO对象(只有一个VAO对象时不是必须的,但是我们还是绑定它,以养成好习惯)
				
		// 渲染当前加载的区块
		for (const auto& chunkPair : chunkManager.getChunks()) {
			const Chunk& chunk = chunkPair.second;// 这里的.second表示map中的值, .first表示map中的键
			
			// 通过可见面渲染体素
			std::vector<std::pair<glm::vec3, Face>> visibleFaces = chunk.getVisibleFaces();

			// 通过可见区块渲染体素
			if (frustum.isAABBInFrustum(chunk.getMinBounds(), chunk.getMaxBounds()))
			{
				glm::mat4 model = glm::mat4(1.0f); // 模型矩阵
				model = glm::translate(model, chunk.getChunkPosition());
				ourShader.setMat4("model", model); // 设置模型矩阵

				const auto& vertices = chunk.getChunkVisibleFacesVertices();

				unsigned int VAO, VBO;
				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);

				glBindVertexArray(VAO);

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

				// 设置顶点位置属性
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
				glEnableVertexAttribArray(0);

				// 设置纹理坐标属性
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
				glEnableVertexAttribArray(1);

				// 绘制顶点数据
				glDrawArrays(GL_TRIANGLES, 0, vertices.size());

				// 解绑 VAO 和 VBO
				glBindVertexArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
		}

		// 渲染GUI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//glBindVertexArray(0);//解绑VAO对象(不是必须的)

		// glfw: 交换缓冲区和轮询IO事件(键盘输入, 鼠标移动等)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 可选: 释放所有资源
	/*glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);*/

	// 渲染结束后清理ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// 清理所有之前分配的GLFW资源
	glfwTerminate();
	return 0;
}

// 处理输入
void processInput(GLFWwindow* window)
{
	//按下ESC键关闭窗口
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	// 前后左右移动
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{ 
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}		
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);

	}

	// 上下移动
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		camera.Position.y -= camera.MovementSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.Position.y += camera.MovementSpeed * deltaTime;
	}

	// 按下右键切换相机控制
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		if (!mouseRightPressed)
		{
			mouseRightPressed = true;
			cameraControlEnabled = !cameraControlEnabled;
			if (cameraControlEnabled)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				firstMouse = true;
			}
		}
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		mouseRightPressed = false;
	}

	// 按下空格键切换绘图模式
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) // 按下空格键
	{
		if (!isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 切换到线框模式
			isWireframe = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) // 松开空格键
	{
		if (isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 切换回填充模式
			isWireframe = false;
		}
	}
}


// 当窗口大小改变时调用该函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);//设置视口大小
}


// 鼠标回调函数: 鼠标移动时调用
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (cameraControlEnabled) {
		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}


// 滚轮回调函数: 鼠标滚轮滚动时调用
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}