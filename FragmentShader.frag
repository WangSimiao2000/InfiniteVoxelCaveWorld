#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D ourTexture;

void main()
{
	FragColor = texture(ourTexture, TexCoord);//这里是直接将纹理赋值给颜色
	//FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);//这里是将纹理和颜色混合
}