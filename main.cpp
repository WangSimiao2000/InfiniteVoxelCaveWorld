#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include <thread> 
#include <atomic> //原子操作

#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "Frustum.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"


std::atomic<bool> updateChunks(true);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);//鼠标回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//滚轮回调函数

unsigned int SCR_WIDTH = 1180;
unsigned int SCR_HEIGHT = 600;

const unsigned int ImGui_Width = 280;

glm::vec3 originLocation = glm::vec3(8.0f, 58.0f, 8.0f); // 原点位置: 在原始的9x9区块中心上方
//glm::vec3 originLocation = glm::vec3(50.0f, 58.0f, 74.0f); // 原点位置: 远距离观察整个初始9x9区块
//glm::vec3 originLocation = glm::vec3(0.0f, 58.0f, 0.0f); // 原点位置: 近距离观察洞穴结构

// camera
Camera camera(originLocation, glm::vec3(0.0f, 1.0f, 0.0f), -135, -27);//创建摄像机对象, 参数分别为摄像机的位置, 世界上方向, Yaw角, Pitch角
//Camera camera(originLocation, glm::vec3(0.0f, 1.0f, 0.0f), -135, -27);//创建摄像机对象, 参数分别为摄像机的位置, 世界上方向, Yaw角, Pitch角
//Camera camera(originLocation, glm::vec3(0.0f, 1.0f, 0.0f), -135, -27);//创建摄像机对象, 参数分别为摄像机的位置, 世界上方向, Yaw角, Pitch角

//Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));//创建摄像机对象, 参数分别为摄像机的位置

float lastX = SCR_WIDTH / 2.0f;//鼠标初始位置
float lastY = SCR_HEIGHT / 2.0f;//鼠标初始位置
bool firstMouse = true;//第一次鼠标移动

// timing
float deltaTime = 0.0f;	// 当前帧与上一帧的时间差
float lastFrame = 0.0f;// 上一帧的时间

// 全局变量用于记录当前的绘图模式
bool isWireframe = false;
bool mouseRightPressed = true;
bool cameraControlEnabled = false;

unsigned int chunkSize = 16;//区块大小
int viewDistance = 2;//视野区块距离

static void updateChunksThread(ChunkManager& chunkManager, std::atomic<bool>& running) {
	// 更新区块管理器
	// Update the chunk manager
	while (running)
	{
		chunkManager.update(camera.Position); // 更新区块管理器
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 添加一个小的延迟以避免占用过多的CPU
	}
}

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
	// Tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//初始化GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 输出OpenGL版本,显卡型号等信息
	// Output OpenGL version, graphics card model and other information
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;//输出OpenGL版本
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;//输出显卡型号
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;//输出显卡厂商	

	// 开启深度测试
	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// 开启面剔除
	// Enable face culling
	glEnable(GL_CULL_FACE);

	Shader ourShader("shaders/VertexShader.vert", "shaders/FragmentShader.frag");//创建着色器对象
	ourShader.use();//使用着色器程序

	// 初始化ImGui
	// Setup Dear ImGui context
	std::cout << "Initializing ImGui" << std::endl;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// 设置平台/渲染器绑定
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // 第二个参数是是否捕捉鼠标, 这里的YOUR_WINDOW是你的GLFW窗口,应该改成你的窗口变量名
	ImGui_ImplOpenGL3_Init();

	// ---- 加载和创建纹理 - START ---- //
	// ---- Load and create a texture - START ---- //
	{
		std::cout << "Loading Texture" << std::endl; 
		
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


		ourShader.setInt("ourTexture", 0);//设置纹理单元, 这里的ourTexture对应的是着色器里声明的uniform sampler2D ourTexture, 0表示使用纹理单元GL_TEXTURE0, 当只有一个纹理单元时, 引号内的ourTexture不会影响结果, 但是如果有多个纹理单元时, 这个名字就很重要了
		//glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);//和上面一行代码功能一样, 设置纹理单元, 上面一行代码是通过着色器类的函数设置, 这行代码是直接通过glUniform1i函数设置, 这里注释掉, 仅供参考
	}
	// ---- 加载和创建纹理 - END ---- //
	// ---- Load and create a texture - END ---- //


	// 设置光照方向
	// Set light direction
	{
		std::cout << "Setting Light Direction" << std::endl;

		glm::vec3 lightDirection = glm::vec3(0.35f, 0.6f, 0.10f);
		lightDirection = glm::normalize(lightDirection);
		ourShader.setVec3("lightDir", lightDirection);//设置光照方向
	}

	ChunkManager chunkManager(chunkSize);//创建区块管理器对象	

	std::thread chunkUpdateThread(updateChunksThread, std::ref(chunkManager), std::ref(updateChunks));//创建一个线程, 用于更新区块管理器
	
	glViewport(ImGui_Width, 0, SCR_WIDTH - ImGui_Width, SCR_HEIGHT); // 右半部分

	while (!glfwWindowShouldClose(window))//循环渲染
	{
		float currentFrame = static_cast<float>(glfwGetTime());//获取当前时间
		deltaTime = currentFrame - lastFrame;//计算时间差
		lastFrame = currentFrame;//更新上一帧时间

		// input
		processInput(window);

		// 开始ImGui帧
		// Start the ImGui frame
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			// 根据需要启用或禁用Dear ImGui对鼠标的绑定
			// Enable or disable Dear ImGui binding to the mouse as needed
			if (cameraControlEnabled) {
				io.ConfigFlags |= ImGuiConfigFlags_NoMouse; // 设置标志，禁用鼠标
			}
			else {
				io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse; // 清除标志，启用鼠标
			}

			ImGui::SetNextWindowPos(ImVec2(0, 0));// 设置窗口位置
			ImGui::SetNextWindowSize(ImVec2(float(ImGui_Width), float(SCR_HEIGHT))); // 设置窗口大小
			ImGui::Begin("InfiniteVoxelWorld", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);// 创建窗口并显示信息

			{
				ImGui::Text("%.1f FPS", io.Framerate);//显示帧率
				ImGui::Text("Delta time %.3fms / frame", io.DeltaTime * 1000.0f);//显示每帧时间
				ImGui::Spacing();
				ImGui::Separator(); // 添加分隔线
				ImGui::Spacing();
			}

			{
				ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);//显示相机位置		
				ImGui::Text("Camera Chunk Position: (%.0f, %.0f)", floor(camera.Position.x / chunkSize), floor(camera.Position.z / chunkSize));//显示相机所在区块位置
				ImGui::Text("Chunk Size: %d x 64 x %d", chunkSize, chunkSize);//显示区块大小
				ImGui::Spacing();
				ImGui::Separator(); // 添加分隔线
				ImGui::Spacing();
			}

			{
				// 修改视野区块距离
				// Modify the view distance
				ImGui::SliderInt("##View Distance", &viewDistance, 1, 3);//滑动条, 用于修改视野区块距离
				ImGui::Spacing();
				ImGui::Text("View Distance: %d", viewDistance);//显示视野区块距离
				ImGui::Text("Chunk Count: ", chunkManager.getChunks().size());//显示可见区块数量
				ImGui::Spacing();
				if (ImGui::Button("Apply View Distance"))//按钮, 用于应用视野区块距离
				{
					chunkManager.stopLoading();
					chunkManager.clearChunks();
					chunkManager.setViewDistance(viewDistance);
					chunkManager.startLoading();
				}
				ImGui::Spacing();
				ImGui::Separator(); // 添加分隔线
				ImGui::Spacing();
			}

			{
				static float weight1 = 1.0f;
				float weight2 = 1.0f - weight1;

				// 修改噪声权重
				// Modify noise weights
				ImGui::SliderFloat("##Noise Weight", &weight1, 0.0f, 1.0f); // 使用##去掉滑动条标签
				ImGui::Spacing();
				ImGui::Text("OpenSimplex2 Weight: %.2f", weight1); // 显示手动设置的 weight1
				ImGui::Text("OpenSimplex2S Weight: %.2f", weight2); // 显示自动计算的 weight2
				ImGui::Spacing();
				// 应用噪声设置
				// Apply noise settings
				if (ImGui::Button("Apply Noise Weight")) {
					chunkManager.stopLoading();
					chunkManager.clearChunks();
					chunkManager.setNoiseWeights(weight1, weight2);
					chunkManager.clearChunksFolder();//清理chunks文件夹
					chunkManager.startLoading();

				}
				ImGui::Spacing();
				ImGui::Separator(); // 添加分隔线
				ImGui::Spacing();
			}

			{
				// 重置相机位置按钮
				// Reset camera position button
				if (ImGui::Button("Reset Camera")) {
					chunkManager.clearChunks();
					camera.Position = originLocation;
				}

				ImGui::Spacing();

				// 停止加载按钮
				// Stop loading button
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
				ImGui::Spacing();
				ImGui::Separator(); // 添加分隔线
				ImGui::Spacing();
			}

			{
				ImGui::Text("Mouse Control: %s", cameraControlEnabled ? "Enabled" : "Disabled");//鼠标状态	
				ImGui::Spacing();
				ImGui::Separator(); // 添加分隔线
				ImGui::Spacing();
			}

			{
				ImGui::Text("Right Mouse Button:Toggle Control");//注明右键切换相机控制
				ImGui::Text("WASD: Move");//WASD控制移动
				ImGui::Text("Q/E: Up/Down");//Q/E控制上下
				ImGui::Text("Space: Toggle Wireframe");//空格键切换线框模式
				ImGui::Text("Mouse Scroll: Zoom");//鼠标滚轮控制缩放
				ImGui::Text("ESC: Close Window");//ESC关闭窗口
				ImGui::Spacing();
				ImGui::Separator(); // 添加分隔线
				ImGui::Spacing();
			}

			// 打印当前imgui窗口的大小
			//ImVec2 windowSize = ImGui::GetWindowSize();
			//ImGui::Text("Window Size: (%.0f, %.0f)", windowSize.x, windowSize.y);

			ImGui::End();
		}
		
		// 更新相机位置
		// Update camera position
		glm::vec3 cameraPosition = camera.Position;
		
		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //设置清空屏幕所用的颜色为深蓝色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空颜色缓冲

		ourShader.use();//使用着色器程序

		ourShader.setVec3("viewPos", cameraPosition); // 将相机位置传递给着色器

		// 相机/观察矩阵
		// Camera/View transformation
		glm::mat4 view = camera.GetViewMatrix();//获取观察矩阵
		ourShader.setMat4("view", view);//设置观察矩阵
		// 传递投影矩阵给着色器
		// Projection matrix
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)(SCR_WIDTH - ImGui_Width)/ (float)SCR_HEIGHT, 0.1f, 100.0f);//透视投影
		ourShader.setMat4("projection", projection);//设置投影矩阵

		glm::mat4 projectionViewMatrix = projection * view;//投影矩阵 * 观察矩阵 = 投影观察矩阵
		Frustum frustum{};
		frustum.calculateFrustum(projectionViewMatrix);

		//glBindVertexArray(VAO);//绑定VAO对象(只有一个VAO对象时不是必须的,但是我们还是绑定它,以养成好习惯)

		// 渲染当前加载的区块
		// Render currently loaded chunks
		for (const auto& chunkPair : chunkManager.getChunks()) {
			const Chunk& chunk = chunkPair.second;// 这里的.second表示map中的值, .first表示map中的键
			
			// 通过可见区块渲染体素
			if (frustum.isAABBInFrustum(chunk.getMinBounds(), chunk.getMaxBounds()))
			//if (true)
			{
				glm::mat4 model = glm::mat4(1.0f); // 模型矩阵
				model = glm::translate(model, chunk.getChunkPosition());
				ourShader.setMat4("model", model); // 设置模型矩阵

				const auto& vertices = chunk.getChunkVisibleFacesVertices();

				unsigned int VAO, VBO;//VAO是顶点数组对象, VBO是顶点缓冲对象
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
				glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

				// 解绑 VAO 和 VBO
				glBindVertexArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				// 删除 VAO 和 VBO, 释放资源, 避免内存泄漏
				glDeleteVertexArrays(1, &VAO);
				glDeleteBuffers(1, &VBO);
			}
		}

		// 渲染GUI
		// Render GUI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: 交换缓冲区和轮询IO事件(键盘输入, 鼠标移动等)
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 终止更新线程
	// Terminate the update thread
	updateChunks = false;
	chunkUpdateThread.join();

	// 可选: 释放所有资源
	/*glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);*/

	// 渲染结束后清理ImGui
	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// 清理所有之前分配的GLFW资源
	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}


// 处理输入
// Process input
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
		camera.Position.y -= camera.MovementSpeed * deltaTime * 2;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.Position.y += camera.MovementSpeed * deltaTime * 2;
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
// Callback function when the window size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	SCR_HEIGHT = height;
	SCR_WIDTH = width;
	glViewport(ImGui_Width , 0, SCR_WIDTH - ImGui_Width, SCR_HEIGHT);//设置视口大小
}


// 鼠标回调函数: 鼠标移动时调用
// Mouse callback function: called when the mouse moves
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
// Scroll callback function: called when the mouse wheel is scrolled
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}