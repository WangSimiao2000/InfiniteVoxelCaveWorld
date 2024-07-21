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

void mouse_callback(GLFWwindow* window, double xpos, double ypos);//���ص�����
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//���ֻص�����

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 750;

// camera
Camera camera(glm::vec3(0.0f, 22.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -105, -30);//�������������, �����ֱ�Ϊ�������λ��, �����Ϸ���, Yaw��, Pitch��

//Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));//�������������, �����ֱ�Ϊ�������λ��

float lastX = SCR_WIDTH / 2.0f;//����ʼλ��
float lastY = SCR_HEIGHT / 2.0f;//����ʼλ��
bool firstMouse = true;//��һ������ƶ�

// timing
float deltaTime = 0.0f;	// ��ǰ֡����һ֡��ʱ���
float lastFrame = 0.0f;// ��һ֡��ʱ��

// ȫ�ֱ������ڼ�¼��ǰ�Ļ�ͼģʽ
bool isWireframe = false;
bool mouseRightPressed = false;
bool cameraControlEnabled = true;

int chunkSize = 16;//�����С

struct Frustum {
	glm::vec4 planes[6];

	// ������׶��ƽ��
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

	// �����Ƿ�����׶����
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

	// ���AABB�Ƿ�����׶����, �����AABB��ʾһ�������壬����С�������ȷ��
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
	glfwInit();//��ʼ��GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//�������汾��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//���ôΰ汾��
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//����OpenGL�����ļ�

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);//macOSϵͳ��Ҫ����
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "InfiniteVoxelWorld", NULL, NULL);//��������
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

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// ����ƽ̨/��Ⱦ����
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // �ڶ����������Ƿ�׽���, �����YOUR_WINDOW�����GLFW����,Ӧ�øĳ���Ĵ��ڱ�����
	ImGui_ImplOpenGL3_Init();

	// ---- ���غʹ������� - START ---- //
	unsigned int texture;//����ID
	glGenTextures(1, &texture);//�����������
	// ��ο���д��ѭ������, ��Ϊ������Ⱦʱ����ı�����, ������δ���ֻ��Ҫ����Ⱦѭ����ִ��һ�μ���(�������Ҳ��һ��, ����������Ҫ����Ⱦʱ�ı�����, ��ʱ����Ҫ����Ⱦѭ����ִ��)
	glActiveTexture(GL_TEXTURE0);//��������Ԫ, Ĭ�ϼ������GL_TEXTURE0, �������д�����ʵ����ʡ��
	glBindTexture(GL_TEXTURE_2D, texture);//���������
	// ���������Ʒ�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//����S��Ļ��Ʒ�ʽΪGL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//����T��Ļ��Ʒ�ʽΪGL_REPEAT
	// ����������˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//������С���˷�ʽΪGL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//���÷Ŵ���˷�ʽΪGL_LINEAR

	// ��������ͼƬ, ��������, ����Mipmap
	int width, height, nrChannels;//ͼƬ���, �߶�, ��ɫͨ����,�����width, height, nrChannels��ͨ��stbi_load�������ص�
	stbi_set_flip_vertically_on_load(true);//��תͼƬy��, ��ΪOpenGL������ԭ���ڴ������½�, ��ͼƬ������ԭ�������Ͻ�
	//unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);//������������ͼƬ
	unsigned char* data = stbi_load("stone_16.png", &width, &height, &nrChannels, 0);//����minecraftʯͷ����ͼƬ
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);//��������, �����ֱ�Ϊ����Ŀ��, mipmap����, ����洢��ʽ, ��, ��, 0, Դͼ��ʽ, Դͼ��������, ͼ������
		//glGenerateMipmap(GL_TEXTURE_2D);//����Mipmap
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);//�ͷ�ͼ���ڴ�

	// ���ù��շ���
	glm::vec3 lightDirection = glm::vec3(0.35f, 0.6f, 0.10f);
	lightDirection = glm::normalize(lightDirection);

	ourShader.use();//ʹ����ɫ������
	ourShader.setVec3("lightDir", lightDirection);//���ù��շ���
	ourShader.setInt("ourTexture", 0);//��������Ԫ, �����ourTexture��Ӧ������ɫ����������uniform sampler2D ourTexture, 0��ʾʹ������ԪGL_TEXTURE0, ��ֻ��һ������Ԫʱ, �����ڵ�ourTexture����Ӱ����, ��������ж������Ԫʱ, ������־ͺ���Ҫ��
	//glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);//������һ�д��빦��һ��, ��������Ԫ, ����һ�д�����ͨ����ɫ����ĺ�������, ���д�����ֱ��ͨ��glUniform1i��������, ����ע�͵�, �����ο�
	// ---- ���غʹ������� - END ---- //

	ChunkManager chunkManager(chunkSize);//�����������������	

	while (!glfwWindowShouldClose(window))//ѭ����Ⱦ
	{
		float currentFrame = static_cast<float>(glfwGetTime());//��ȡ��ǰʱ��
		deltaTime = currentFrame - lastFrame;//����ʱ���
		float fps = 1.0f / deltaTime;//����֡��
		lastFrame = currentFrame;//������һ֡ʱ��

		// ��ʼImGui֡
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			// ������Ҫ���û����Dear ImGui�����İ�
			if (cameraControlEnabled) {
				io.ConfigFlags |= ImGuiConfigFlags_NoMouse; // ���ñ�־���������
			}
			else {
				io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse; // �����־���������
			}

			ImGui::SetNextWindowPos(ImVec2(5, 5));// ���ô���λ��
			ImGui::SetNextWindowSize(ImVec2(435, 200));
			ImGui::Begin("InfiniteVoxelWorld", nullptr, ImGuiWindowFlags_NoResize);// �������ڲ���ʾ��Ϣ

			// ����

			{
				ImGui::Text("FPS: %.1f", fps);//��ʾ֡��
				ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);//��ʾ���λ��		
				ImGui::Text("Camera Chunk Position: (%.0f, %.0f)", floor(camera.Position.x / chunkSize), floor(camera.Position.z / chunkSize));//��ʾ�����������λ��
			}

			ImGui::Separator(); // ��ӷָ���

			{
				ImGui::BeginChild("Chunk Manage", ImVec2(200, 100), true);//����һ���Ӵ���, 200���Ӵ��ڵĿ��, 0���Ӵ��ڵĸ߶�, true��ʾ�Ӵ����й�����
				// ���ð�ť
				if (ImGui::Button("Reset Chunks and Camera")) {
					chunkManager.clearChunks();
					camera.Position = glm::vec3(0.0f, 22.5f, 0.0f);
				}
				// ֹͣ���ذ�ť
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

			ImGui::SameLine(); // ��ͬһ�м�������

			{
				ImGui::BeginChild("Noise Weight Settings", ImVec2(200, 100), true);//����һ���Ӵ���, 200���Ӵ��ڵĿ��, 0���Ӵ��ڵĸ߶�, true��ʾ�Ӵ����й�����
				static float weight1 = 1.0f;
				float weight2 = 1.0f - weight1;

				// �޸�����Ȩ��
				ImGui::SliderFloat("##", &weight1, 0.0f, 1.0f); // ʹ��##ȥ����������ǩ
				ImGui::Text("OpenSimplex2 Weight: %.2f", weight1); // ��ʾ�ֶ����õ� weight1
				ImGui::Text("Perlin Weight: %.2f", weight2); // ��ʾ�Զ������ weight2
				// Ӧ����������
				if (ImGui::Button("Apply Weight")) {
					chunkManager.setNoiseWeights(weight1, weight2);
				}
				ImGui::EndChild();
			}

			// ��ӡ��ǰimgui���ڵĴ�С
			//ImVec2 windowSize = ImGui::GetWindowSize();
			//ImGui::Text("Window Size: (%.0f, %.0f)", windowSize.x, windowSize.y);

			ImGui::End();
		}		
		
		// �������λ��
		glm::vec3 cameraPosition = camera.Position;
		// �������������
		chunkManager.update(cameraPosition);

		// input
		processInput(window);
		
		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //���������Ļ���õ���ɫΪ����ɫ
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//�����ɫ����

		ourShader.use();//ʹ����ɫ������

		ourShader.setVec3("viewPos", cameraPosition); // �����λ�ô��ݸ���ɫ��

		// ���/�۲����
		glm::mat4 view = camera.GetViewMatrix();//��ȡ�۲����
		ourShader.setMat4("view", view);//���ù۲����
		// ����ͶӰ�������ɫ��
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);//͸��ͶӰ
		ourShader.setMat4("projection", projection);//����ͶӰ����

		glm::mat4 projectionViewMatrix = projection * view;//ͶӰ���� * �۲���� = ͶӰ�۲����
		Frustum frustum;
		frustum.calculateFrustum(projectionViewMatrix);

		//glBindVertexArray(VAO);//��VAO����(ֻ��һ��VAO����ʱ���Ǳ����,�������ǻ��ǰ���,�����ɺ�ϰ��)
				
		// ��Ⱦ��ǰ���ص�����
		for (const auto& chunkPair : chunkManager.getChunks()) {
			const Chunk& chunk = chunkPair.second;// �����.second��ʾmap�е�ֵ, .first��ʾmap�еļ�
			
			// ͨ���ɼ�����Ⱦ����
			std::vector<std::pair<glm::vec3, Face>> visibleFaces = chunk.getVisibleFaces();

			// ͨ���ɼ�������Ⱦ����
			if (frustum.isAABBInFrustum(chunk.getMinBounds(), chunk.getMaxBounds()))
			{
				glm::mat4 model = glm::mat4(1.0f); // ģ�;���
				model = glm::translate(model, chunk.getChunkPosition());
				ourShader.setMat4("model", model); // ����ģ�;���

				const auto& vertices = chunk.getChunkVisibleFacesVertices();

				unsigned int VAO, VBO;
				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);

				glBindVertexArray(VAO);

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

				// ���ö���λ������
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
				glEnableVertexAttribArray(0);

				// ����������������
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
				glEnableVertexAttribArray(1);

				// ���ƶ�������
				glDrawArrays(GL_TRIANGLES, 0, vertices.size());

				// ��� VAO �� VBO
				glBindVertexArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
		}

		// ��ȾGUI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//glBindVertexArray(0);//���VAO����(���Ǳ����)

		// glfw: ��������������ѯIO�¼�(��������, ����ƶ���)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ��ѡ: �ͷ�������Դ
	/*glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);*/

	// ��Ⱦ����������ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// ��������֮ǰ�����GLFW��Դ
	glfwTerminate();
	return 0;
}

// ��������
void processInput(GLFWwindow* window)
{
	//����ESC���رմ���
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	// ǰ�������ƶ�
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

	// �����ƶ�
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		camera.Position.y -= camera.MovementSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.Position.y += camera.MovementSpeed * deltaTime;
	}

	// �����Ҽ��л��������
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


// ���ֻص�����: �����ֹ���ʱ����
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}