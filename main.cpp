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

#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);//鼠标回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//滚轮回调函数

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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

	// ---- 设置顶点数据和索引数据 - START ---- //
	// 单个体素的顶点数据
	float vertices[] = {
		// 顶点坐标          // 纹理坐标
		// Front face
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f, 0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,  0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,

		// Back face
		0.5f,  0.5f,  -0.5f,  0.0f, 1.0f,
		0.5f,  -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f, 0.5f,  -0.5f,  1.0f, 1.0f,

		// Top face
		0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
		0.5f,  0.5f,  -0.5f,  1.0f, 1.0f,
		-0.5f, 0.5f,  -0.5f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
		-0.5f, 0.5f,  -0.5f,  0.0f, 1.0f,
		-0.5f, 0.5f,  0.5f,   0.0f, 0.0f,

        // Bottom face
		0.5f,  -0.5f,  0.5f,   1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,   0.0f, 1.0f,
		-0.5f, -0.5f,  -0.5f,  0.0f, 0.0f,
		0.5f,  -0.5f,  0.5f,   1.0f, 1.0f,
		-0.5f, -0.5f,  -0.5f,  0.0f, 0.0f,
		0.5f,  -0.5f,  -0.5f,  1.0f, 0.0f,

		// Right face
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 0.0f,

		// Left face
		- 0.5f,	0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	};

	// ---- 设置顶点数据和索引数据 - END ---- //

	// ---- 绑定顶点数据到缓冲对象 - START ---- //
	unsigned int VAO;//VAO对象是一个容器对象，存储了我们之后定义的顶点属性状态
	glGenVertexArrays(1, &VAO);//生成一个带有缓冲ID的VAO对象
	glBindVertexArray(VAO);//绑定VAO对象

	unsigned int VBO;//VBO对象是一个缓冲对象，用于将顶点数据传递到显卡
	glGenBuffers(1, &VBO);//生成一个带有缓冲ID的VBO对象
	glBindBuffer(GL_ARRAY_BUFFER, VBO);//绑定缓冲对象
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//将顶点数据复制到缓冲的内存中

	// 告诉OpenGL该如何解析顶点数据 
	// 顶点属性位置值为0, location = 0, 意味着我们在顶点着色器中使用layout(location = 0)定义的顶点属性
	// 顶点属性大小为3, 数据类型为GL_FLOAT, 
	// 是否标准化为GL_FALSE, 
	// 步长为3 * sizeof(float), 
	// 在缓冲中起始位置为0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// 纹理坐标属性
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);//解绑VBO
	//glBindVertexArray(0);//解绑VAO
	// ---- 绑定顶点数据到缓冲对象 - END ---- //


	// ---- 加载和创建纹理 - START ---- //
	unsigned int texture;//纹理ID
	glGenTextures(1, &texture);//生成纹理对象
	glActiveTexture(GL_TEXTURE0);//激活纹理单元, 默认激活的是GL_TEXTURE0, 所以这行代码其实可以省略
	glBindTexture(GL_TEXTURE_2D, texture);//绑定纹理对象
	// 设置纹理环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//设置S轴的环绕方式为GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//设置T轴的环绕方式为GL_REPEAT
	// 设置纹理过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//设置缩小过滤方式为GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//设置放大过滤方式为GL_LINEAR

	// 加载纹理图片, 创建纹理, 生成Mipmap
	int width, height, nrChannels;//图片宽度, 高度, 颜色通道数,这里的width, height, nrChannels是通过stbi_load函数返回的
	stbi_set_flip_vertically_on_load(true);//翻转图片y轴, 因为OpenGL的坐标原点在窗口左下角, 而图片的坐标原点在左上角
	//unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);//加载箱子纹理图片
	unsigned char* data = stbi_load("stone.png", &width, &height, &nrChannels, 0);//加载minecraft石头纹理图片
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);//生成纹理, 参数分别为纹理目标, mipmap级别, 纹理存储格式, 宽, 高, 0, 源图格式, 源图数据类型, 图像数据
		glGenerateMipmap(GL_TEXTURE_2D);//生成Mipmap
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);//释放图像内存

	glm::vec3 lightDirection = glm::vec3(0.35f, 0.6f, 0.45f);
	lightDirection = glm::normalize(lightDirection);

	ourShader.use();//使用着色器程序
	ourShader.setVec3("lightDir", lightDirection);//设置光照方向
	ourShader.setInt("ourTexture", 0);//设置纹理单元, 这里的ourTexture对应的是着色器里声明的uniform sampler2D ourTexture, 0表示使用纹理单元GL_TEXTURE0, 当只有一个纹理单元时, 引号内的ourTexture不会影响结果, 但是如果有多个纹理单元时, 这个名字就很重要了
	//glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);//和上面一行代码功能一样, 设置纹理单元, 上面一行代码是通过着色器类的函数设置, 这行代码是直接通过glUniform1i函数设置, 这里注释掉, 仅供参考
	// ---- 加载和创建纹理 - END ---- //

	ChunkManager chunkManager(16);//创建区块管理器对象

	while (!glfwWindowShouldClose(window))//循环渲染
	{
		float currentFrame = static_cast<float>(glfwGetTime());//获取当前时间
		deltaTime = currentFrame - lastFrame;//计算时间差
		lastFrame = currentFrame;//更新上一帧时间
		
		// 更新相机位置
		glm::vec3 cameraPosition = camera.Position;
		// 更新区块管理器
		chunkManager.update(cameraPosition);

		// input
		processInput(window);
		
		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //设置清空屏幕所用的颜色为深蓝色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空颜色缓冲

		// 这段可以写在循环外面, 因为我们渲染时不会改变纹理, 所以这段代码只需要在渲染循环外执行一次即可(多个纹理也是一样, 除非我们需要在渲染时改变纹理, 这时就需要在渲染循环内执行)
		//glActiveTexture(GL_TEXTURE0);//激活纹理单元, 默认激活的是GL_TEXTURE0, 所以这行代码其实可以省略
		//glBindTexture(GL_TEXTURE_2D, texture);//绑定纹理对象

		ourShader.use();//使用着色器程序

		ourShader.setVec3("viewPos", cameraPosition); // 将相机位置传递给着色器

		// 相机/观察矩阵
		glm::mat4 view = camera.GetViewMatrix();//获取观察矩阵
		ourShader.setMat4("view", view);//设置观察矩阵
		// 传递投影矩阵给着色器
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);//透视投影
		ourShader.setMat4("projection", projection);//设置投影矩阵

		glBindVertexArray(VAO);//绑定VAO对象(只有一个VAO对象时不是必须的,但是我们还是绑定它,以养成好习惯)
		//float timeValue = glfwGetTime(); // 获取当前时间
				
		// 渲染当前加载的区块
		for (const auto& chunkPair : chunkManager.chunks) {
			const Chunk& chunk = chunkPair.second;// 这里的.second表示map中的值, .first表示map中的键
			std::vector<glm::vec3> worldVoxelPositions = chunk.getVoxelWorldPositions();

			for (unsigned int i = 0; i < worldVoxelPositions.size(); i++) {
				glm::mat4 model = glm::mat4(1.0f); // 模型矩阵
				model = glm::translate(model, worldVoxelPositions[i]);
				ourShader.setMat4("model", model); // 设置模型矩阵

				glDrawArrays(GL_TRIANGLES, 0, 36); // 参数分别为绘制模式, 起始索引, 绘制顶点个数
			}			
		}

		//glBindVertexArray(0);//解绑VAO对象(不是必须的)

		// glfw: 交换缓冲区和轮询IO事件(键盘输入, 鼠标移动等)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 可选: 释放所有资源
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

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