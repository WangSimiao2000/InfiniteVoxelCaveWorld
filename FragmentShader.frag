#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D ourTexture;

void main()
{
	FragColor = texture(ourTexture, TexCoord);//������ֱ�ӽ�����ֵ����ɫ
	//FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);//�����ǽ��������ɫ���
}