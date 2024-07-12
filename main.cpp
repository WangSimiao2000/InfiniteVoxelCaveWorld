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

void mouse_callback(GLFWwindow* window, double xpos, double ypos);//���ص�����
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//���ֻص�����

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(12.0f, 22.5f, 25.0f), glm::vec3(0.0f, 1.0f, 0.0f), -105, -30);//�������������, �����ֱ�Ϊ�������λ��, �����Ϸ���, Yaw��, Pitch��

float lastX = SCR_WIDTH / 2.0f;//����ʼλ��
float lastY = SCR_HEIGHT / 2.0f;//����ʼλ��
bool firstMouse = true;//��һ������ƶ�

// timing
float deltaTime = 0.0f;	// ��ǰ֡����һ֡��ʱ���
float lastFrame = 0.0f;// ��һ֡��ʱ��

// ȫ�ֱ������ڼ�¼��ǰ�Ļ�ͼģʽ
bool isWireframe = false;

int main()
{
	glfwInit();//��ʼ��GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//�������汾��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//���ôΰ汾��
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//����OpenGL�����ļ�

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);//macOSϵͳ��Ҫ����
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);//��������
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);//�����ڵ�����������Ϊ��ǰ�̵߳���������
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//���ô��ڴ�С�ı�ʱ�Ļص�����
	glfwSetCursorPosCallback(window, mouse_callback);//�������ص�����
	glfwSetScrollCallback(window, scroll_callback);//���ù��ֻص�����

	// ����GLFW������Ҫ��׽���е��������
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//��ʼ��GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// ������Ȳ���
	glEnable(GL_DEPTH_TEST);

	// �������޳�
	glEnable(GL_CULL_FACE);

	Shader ourShader("VertexShader.vert", "FragmentShader.frag");//������ɫ������

	// ---- ���ö������ݺ��������� - START ---- //
	// �������صĶ�������
	float vertices[] = {
		// ��������          // ��������
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

	Chunk chunk(16, glm::vec3(0.0f, 0.0f, 0.0f));//�����������
	chunk.initializeChunk();//��ʼ������, ʹ�������ض������, ����¼���ǵ��������
	// ����Ⱦѭ����
	std::vector<glm::vec3> worldVoxelPositions = chunk.getVoxelWorldPositions();
	// ---- ���ö������ݺ��������� - END ---- //

	// ---- �󶨶������ݵ�������� - START ---- //
	unsigned int VAO;//VAO������һ���������󣬴洢������֮����Ķ�������״̬
	glGenVertexArrays(1, &VAO);//����һ�����л���ID��VAO����
	glBindVertexArray(VAO);//��VAO����

	unsigned int VBO;//VBO������һ������������ڽ��������ݴ��ݵ��Կ�
	glGenBuffers(1, &VBO);//����һ�����л���ID��VBO����
	glBindBuffer(GL_ARRAY_BUFFER, VBO);//�󶨻������
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//���������ݸ��Ƶ�������ڴ���

	// ����OpenGL����ν����������� 
	// ��������λ��ֵΪ0, location = 0, ��ζ�������ڶ�����ɫ����ʹ��layout(location = 0)����Ķ�������
	// �������Դ�СΪ3, ��������ΪGL_FLOAT, 
	// �Ƿ��׼��ΪGL_FALSE, 
	// ����Ϊ3 * sizeof(float), 
	// �ڻ�������ʼλ��Ϊ0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// ������������
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);//���VBO
	//glBindVertexArray(0);//���VAO
	// ---- �󶨶������ݵ�������� - END ---- //


	// ---- ���غʹ������� - START ---- //
	unsigned int texture;//����ID
	glGenTextures(1, &texture);//�����������
	glActiveTexture(GL_TEXTURE0);//��������Ԫ, Ĭ�ϼ������GL_TEXTURE0, �������д�����ʵ����ʡ��
	glBindTexture(GL_TEXTURE_2D, texture);//���������
	// ���������Ʒ�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//����S��Ļ��Ʒ�ʽΪGL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//����T��Ļ��Ʒ�ʽΪGL_REPEAT
	// ����������˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//������С���˷�ʽΪGL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//���÷Ŵ���˷�ʽΪGL_LINEAR

	// ��������ͼƬ, ��������, ����Mipmap
	int width, height, nrChannels;//ͼƬ���, �߶�, ��ɫͨ����,�����width, height, nrChannels��ͨ��stbi_load�������ص�
	stbi_set_flip_vertically_on_load(true);//��תͼƬy��, ��ΪOpenGL������ԭ���ڴ������½�, ��ͼƬ������ԭ�������Ͻ�
	//unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);//������������ͼƬ
	unsigned char* data = stbi_load("stone.png", &width, &height, &nrChannels, 0);//����minecraftʯͷ����ͼƬ
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);//��������, �����ֱ�Ϊ����Ŀ��, mipmap����, ����洢��ʽ, ��, ��, 0, Դͼ��ʽ, Դͼ��������, ͼ������
		glGenerateMipmap(GL_TEXTURE_2D);//����Mipmap
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);//�ͷ�ͼ���ڴ�

	ourShader.use();//ʹ����ɫ������
	ourShader.setInt("ourTexture", 0);//��������Ԫ, �����ourTexture��Ӧ������ɫ����������uniform sampler2D ourTexture, 0��ʾʹ������ԪGL_TEXTURE0, ��ֻ��һ������Ԫʱ, �����ڵ�ourTexture����Ӱ����, ��������ж������Ԫʱ, ������־ͺ���Ҫ��
	//glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);//������һ�д��빦��һ��, ��������Ԫ, ����һ�д�����ͨ����ɫ����ĺ�������, ���д�����ֱ��ͨ��glUniform1i��������, ����ע�͵�, �����ο�
	// ---- ���غʹ������� - END ---- //


	while (!glfwWindowShouldClose(window))//ѭ����Ⱦ
	{
		float currentFrame = static_cast<float>(glfwGetTime());//��ȡ��ǰʱ��
		deltaTime = currentFrame - lastFrame;//����ʱ���
		lastFrame = currentFrame;//������һ֡ʱ��
		
		// input
		processInput(window);
		
		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //���������Ļ���õ���ɫΪ����ɫ
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//�����ɫ����

		// ��ο���д��ѭ������, ��Ϊ������Ⱦʱ����ı�����, ������δ���ֻ��Ҫ����Ⱦѭ����ִ��һ�μ���(�������Ҳ��һ��, ����������Ҫ����Ⱦʱ�ı�����, ��ʱ����Ҫ����Ⱦѭ����ִ��)
		//glActiveTexture(GL_TEXTURE0);//��������Ԫ, Ĭ�ϼ������GL_TEXTURE0, �������д�����ʵ����ʡ��
		//glBindTexture(GL_TEXTURE_2D, texture);//���������

		ourShader.use();//ʹ����ɫ������

		// ���/�۲����
		glm::mat4 view = camera.GetViewMatrix();//��ȡ�۲����
		ourShader.setMat4("view", view);//���ù۲����
		// ����ͶӰ�������ɫ��
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);//͸��ͶӰ
		ourShader.setMat4("projection", projection);//����ͶӰ����

		glBindVertexArray(VAO);//��VAO����(ֻ��һ��VAO����ʱ���Ǳ����,�������ǻ��ǰ���,�����ɺ�ϰ��)
		//float timeValue = glfwGetTime(); // ��ȡ��ǰʱ��
		for (unsigned int i = 0; i < worldVoxelPositions.size(); i++)
		{
			glm::mat4 model = glm::mat4(1.0f);//ģ�;���
			model = glm::translate(model, worldVoxelPositions[i]);
			//float angle = 20.0f * i; // ÿ��������ĳ�ʼ�Ƕ�
			//float angle = 20.0f * i + timeValue * glm::radians(50.0f); // ÿ��������ĳ�ʼ�Ƕ� + ʱ��仯
			//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));//��ת
			ourShader.setMat4("model", model);//����ģ�;���

			glDrawArrays(GL_TRIANGLES, 0, 36);//�����ֱ�Ϊ����ģʽ, ��ʼ����, ���ƶ������
		}	
		
		//glBindVertexArray(0);//���VAO����(���Ǳ����)

		// glfw: ��������������ѯIO�¼�(��������, ����ƶ���)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ��ѡ: �ͷ�������Դ
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// ��������֮ǰ�����GLFW��Դ
	glfwTerminate();
	return 0;
}

// ��������
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)//����ESC��
	{
		glfwSetWindowShouldClose(window, true);
	}

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

	// ���¿ո���л���ͼģʽ
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) // ���¿ո��
	{
		if (!isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // �л����߿�ģʽ
			isWireframe = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) // �ɿ��ո��
	{
		if (isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // �л������ģʽ
			isWireframe = false;
		}
	}
}


// �����ڴ�С�ı�ʱ���øú���
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);//�����ӿڴ�С
}


// ���ص�����: ����ƶ�ʱ����
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
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


// ���ֻص�����: �����ֹ���ʱ����
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}