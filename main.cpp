#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include <thread> 
#include <atomic> //ԭ�Ӳ���

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

void mouse_callback(GLFWwindow* window, double xpos, double ypos);//���ص�����
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//���ֻص�����

unsigned int SCR_WIDTH = 1200;
unsigned int SCR_HEIGHT = 600;

const unsigned int ImGui_Width = 300;

glm::vec3 originLocation = glm::vec3(8.0f, 58.0f, 8.0f); // ԭ��λ��: ��ԭʼ��9x9���������Ϸ�
//glm::vec3 originLocation = glm::vec3(50.0f, 58.0f, 74.0f); // ԭ��λ��: Զ����۲�������ʼ9x9����
//glm::vec3 originLocation = glm::vec3(0.0f, 58.0f, 0.0f); // ԭ��λ��: ������۲춴Ѩ�ṹ

// camera
Camera camera(originLocation, glm::vec3(0.0f, 1.0f, 0.0f), -135, -27);//�������������, �����ֱ�Ϊ�������λ��, �����Ϸ���, Yaw��, Pitch��
//Camera camera(originLocation, glm::vec3(0.0f, 1.0f, 0.0f), -135, -27);//�������������, �����ֱ�Ϊ�������λ��, �����Ϸ���, Yaw��, Pitch��
//Camera camera(originLocation, glm::vec3(0.0f, 1.0f, 0.0f), -135, -27);//�������������, �����ֱ�Ϊ�������λ��, �����Ϸ���, Yaw��, Pitch��

//Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));//�������������, �����ֱ�Ϊ�������λ��

float lastX = SCR_WIDTH / 2.0f;//����ʼλ��
float lastY = SCR_HEIGHT / 2.0f;//����ʼλ��
bool firstMouse = true;//��һ������ƶ�

// timing
float deltaTime = 0.0f;	// ��ǰ֡����һ֡��ʱ���
float lastFrame = 0.0f;// ��һ֡��ʱ��

// ȫ�ֱ������ڼ�¼��ǰ�Ļ�ͼģʽ
bool isWireframe = false;
bool mouseRightPressed = true;
bool cameraControlEnabled = false;

unsigned int chunkSize = 16;//�����С
int viewDistance = 2;//��Ұ�������

static void updateChunksThread(ChunkManager& chunkManager, std::atomic<bool>& running) {
	// �������������
	while (running)
	{
		chunkManager.update(camera.Position); // �������������
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // ���һ��С���ӳ��Ա���ռ�ù����CPU
	}
}

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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//��ʼ��GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// ���OpenGL�汾,�Կ��ͺŵ���Ϣ
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;//���OpenGL�汾
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;//����Կ��ͺ�
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;//����Կ�����	

	// ������Ȳ���
	glEnable(GL_DEPTH_TEST);

	// �������޳�
	glEnable(GL_CULL_FACE);

	Shader ourShader("shaders/VertexShader.vert", "shaders/FragmentShader.frag");//������ɫ������
	ourShader.use();//ʹ����ɫ������

	// ��ʼ��ImGui
	std::cout << "Initializing ImGui" << std::endl;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// ����ƽ̨/��Ⱦ����
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // �ڶ����������Ƿ�׽���, �����YOUR_WINDOW�����GLFW����,Ӧ�øĳ���Ĵ��ڱ�����
	ImGui_ImplOpenGL3_Init();

	// ---- ���غʹ������� - START ---- //
	{
		std::cout << "Loading Texture" << std::endl; 
		
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


		ourShader.setInt("ourTexture", 0);//��������Ԫ, �����ourTexture��Ӧ������ɫ����������uniform sampler2D ourTexture, 0��ʾʹ������ԪGL_TEXTURE0, ��ֻ��һ������Ԫʱ, �����ڵ�ourTexture����Ӱ����, ��������ж������Ԫʱ, ������־ͺ���Ҫ��
		//glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);//������һ�д��빦��һ��, ��������Ԫ, ����һ�д�����ͨ����ɫ����ĺ�������, ���д�����ֱ��ͨ��glUniform1i��������, ����ע�͵�, �����ο�
	}
	// ---- ���غʹ������� - END ---- //


	// ���ù��շ���
	{
		std::cout << "Setting Light Direction" << std::endl;

		glm::vec3 lightDirection = glm::vec3(0.35f, 0.6f, 0.10f);
		lightDirection = glm::normalize(lightDirection);
		ourShader.setVec3("lightDir", lightDirection);//���ù��շ���
	}

	ChunkManager chunkManager(chunkSize);//�����������������	

	std::thread chunkUpdateThread(updateChunksThread, std::ref(chunkManager), std::ref(updateChunks));//����һ���߳�, ���ڸ������������
	
	glViewport(ImGui_Width, 0, SCR_WIDTH - ImGui_Width, SCR_HEIGHT); // �Ұ벿��

	while (!glfwWindowShouldClose(window))//ѭ����Ⱦ
	{
		float currentFrame = static_cast<float>(glfwGetTime());//��ȡ��ǰʱ��
		deltaTime = currentFrame - lastFrame;//����ʱ���
		lastFrame = currentFrame;//������һ֡ʱ��

		// input
		processInput(window);

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

			ImGui::Begin("InfiniteVoxelWorld", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);// �������ڲ���ʾ��Ϣ
			ImGui::SetNextWindowPos(ImVec2(0, 0));// ���ô���λ��
			ImGui::SetNextWindowSize(ImVec2(float(ImGui_Width),float(SCR_HEIGHT)));

			{
				ImGui::Text("%.1f FPS", io.Framerate);//��ʾ֡��
				ImGui::Text("Delta time %.3fms / frame", io.DeltaTime * 1000.0f);//��ʾÿ֡ʱ��
				ImGui::Spacing();
				ImGui::Separator(); // ��ӷָ���
				ImGui::Spacing();
			}

			{
				ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);//��ʾ���λ��		
				ImGui::Text("Camera Chunk Position: (%.0f, %.0f)", floor(camera.Position.x / chunkSize), floor(camera.Position.z / chunkSize));//��ʾ�����������λ��
				ImGui::Text("Chunk Size: %d x 64 x %d", chunkSize, chunkSize);//��ʾ�����С
				ImGui::Spacing();
				ImGui::Separator(); // ��ӷָ���
				ImGui::Spacing();
			}

			{
				// �޸���Ұ�������
				ImGui::SliderInt("View Distance", &viewDistance, 1, 3);//������, �����޸���Ұ�������
				ImGui::Spacing();
				ImGui::Text("Chunk Count: ", chunkManager.getChunks().size());//��ʾ�ɼ���������
				ImGui::Spacing();
				if (ImGui::Button("Apply View Distance"))//��ť, ����Ӧ����Ұ�������
				{
					chunkManager.stopLoading();
					chunkManager.clearChunks();
					chunkManager.setViewDistance(viewDistance);
					chunkManager.startLoading();
				}
				ImGui::Spacing();
				ImGui::Separator(); // ��ӷָ���
				ImGui::Spacing();
			}

			{
				static float weight1 = 1.0f;
				float weight2 = 1.0f - weight1;

				// �޸�����Ȩ��
				ImGui::SliderFloat("##", &weight1, 0.0f, 1.0f); // ʹ��##ȥ����������ǩ
				ImGui::Spacing();
				ImGui::Text("OpenSimplex2 Weight: %.2f", weight1); // ��ʾ�ֶ����õ� weight1
				ImGui::Text("OpenSimplex2S Weight: %.2f", weight2); // ��ʾ�Զ������ weight2
				ImGui::Spacing();
				// Ӧ����������
				if (ImGui::Button("Apply Weight And Regenerate Chunks")) {
					chunkManager.stopLoading();
					chunkManager.clearChunks();
					chunkManager.setNoiseWeights(weight1, weight2);
					chunkManager.clearChunksFolder();//����chunks�ļ���
					chunkManager.startLoading();

				}
				ImGui::Spacing();
				ImGui::Separator(); // ��ӷָ���
				ImGui::Spacing();
			}

			{
				// �������λ�ð�ť
				if (ImGui::Button("Reset Camera")) {
					chunkManager.clearChunks();
					camera.Position = originLocation;
				}

				ImGui::Spacing();

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
				ImGui::Spacing();
				ImGui::Separator(); // ��ӷָ���
				ImGui::Spacing();
			}

			{
				ImGui::Text("Mouse Control: %s", cameraControlEnabled ? "Enabled" : "Disabled");//���״̬	
				ImGui::Text("Right Mouse Button:Toggle Camera Control");//ע���Ҽ��л��������
				ImGui::Spacing();
				ImGui::Separator(); // ��ӷָ���
				ImGui::Spacing();
			}

			{
				ImGui::Text("WASD: Move");//WASD�����ƶ�
				ImGui::Text("Q/E: Up/Down");//Q/E��������
				ImGui::Text("Space: Toggle Wireframe");//�ո���л��߿�ģʽ
				ImGui::Spacing();
				ImGui::Separator(); // ��ӷָ���
				ImGui::Spacing();
			}

			// ��ӡ��ǰimgui���ڵĴ�С
			//ImVec2 windowSize = ImGui::GetWindowSize();
			//ImGui::Text("Window Size: (%.0f, %.0f)", windowSize.x, windowSize.y);

			ImGui::End();
		}
		
		// �������λ��
		glm::vec3 cameraPosition = camera.Position;
		
		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //���������Ļ���õ���ɫΪ����ɫ
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//�����ɫ����

		ourShader.use();//ʹ����ɫ������

		ourShader.setVec3("viewPos", cameraPosition); // �����λ�ô��ݸ���ɫ��

		// ���/�۲����
		glm::mat4 view = camera.GetViewMatrix();//��ȡ�۲����
		ourShader.setMat4("view", view);//���ù۲����
		// ����ͶӰ�������ɫ��
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)(SCR_WIDTH - ImGui_Width)/ (float)SCR_HEIGHT, 0.1f, 100.0f);//͸��ͶӰ
		ourShader.setMat4("projection", projection);//����ͶӰ����

		glm::mat4 projectionViewMatrix = projection * view;//ͶӰ���� * �۲���� = ͶӰ�۲����
		Frustum frustum{};
		frustum.calculateFrustum(projectionViewMatrix);

		//glBindVertexArray(VAO);//��VAO����(ֻ��һ��VAO����ʱ���Ǳ����,�������ǻ��ǰ���,�����ɺ�ϰ��)

		// ��Ⱦ��ǰ���ص�����
		for (const auto& chunkPair : chunkManager.getChunks()) {
			const Chunk& chunk = chunkPair.second;// �����.second��ʾmap�е�ֵ, .first��ʾmap�еļ�
			
			// ͨ���ɼ�������Ⱦ����
			if (frustum.isAABBInFrustum(chunk.getMinBounds(), chunk.getMaxBounds()))
			//if (true)
			{
				glm::mat4 model = glm::mat4(1.0f); // ģ�;���
				model = glm::translate(model, chunk.getChunkPosition());
				ourShader.setMat4("model", model); // ����ģ�;���

				const auto& vertices = chunk.getChunkVisibleFacesVertices();

				unsigned int VAO, VBO;//VAO�Ƕ����������, VBO�Ƕ��㻺�����
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
				glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

				// ��� VAO �� VBO
				glBindVertexArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				// ɾ�� VAO �� VBO, �ͷ���Դ, �����ڴ�й©
				glDeleteVertexArrays(1, &VAO);
				glDeleteBuffers(1, &VBO);
			}
		}

		// ��ȾGUI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: ��������������ѯIO�¼�(��������, ����ƶ���)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ��ֹ�����߳�
	updateChunks = false;
	chunkUpdateThread.join();

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
		camera.Position.y -= camera.MovementSpeed * deltaTime * 2;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.Position.y += camera.MovementSpeed * deltaTime * 2;
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
	SCR_HEIGHT = height;
	SCR_WIDTH = width;
	glViewport(ImGui_Width , 0, SCR_WIDTH - ImGui_Width, SCR_HEIGHT);//�����ӿڴ�С
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