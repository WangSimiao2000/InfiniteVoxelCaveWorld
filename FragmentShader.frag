#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos; // 从顶点着色器传入的顶点位置

// texture sampler
uniform sampler2D ourTexture;
uniform vec3 lightDir; // 平行光方向
uniform vec3 viewPos; // 视点位置

void main()
{
	// 定义光照颜色
    float ambientStrength = 0.6; // 环境光强度
    float specularStrength = 0.1; // 镜面反射强度
    float shininess = 32.0; // 高光系数
    vec3 lightColor = vec3(1.0, 0.95, 0.9);

	// 计算基本变量
    vec3 normal = normalize(cross(dFdx(FragPos), dFdy(FragPos))); 
    vec3 lightDirNormalized = normalize(lightDir); // 平行光方向已经是标准化的
    vec3 viewDir = normalize(viewPos - FragPos); // 视线方向
    vec3 halfwayDir = normalize(lightDir + viewDir); // 半程向量

     // 从纹理中获取材质颜色
    vec3 materialColor = texture(ourTexture, TexCoord).rgb;

    // 环境光强度
    vec3 ambient = ambientStrength * lightColor * materialColor;

    // 漫反射强度
    float diff = max(dot(normal, lightDirNormalized), 0.0);
    vec3 diffuse = diff * lightColor * materialColor;

    // 镜面反射强度
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // 合成最终颜色
    vec3 finalColor = ambient + diffuse + specular;

    // 将最终颜色赋值给片段颜色输出
    FragColor = vec4(finalColor, 1.0);

	//FragColor = texture(ourTexture, TexCoord);//这里是直接将纹理赋值给颜色
}