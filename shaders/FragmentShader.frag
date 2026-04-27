#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos; // 从顶点着色器传入的顶点位置

// texture sampler
uniform sampler2D ourTexture;
uniform vec3 lightDir; // 平行光方向
uniform vec3 viewPos; // 视点位置

// 雾效参数
uniform vec3 fogColor;    // 雾的颜色（与背景色一致）
uniform float fogNear;    // 雾开始距离
uniform float fogFar;     // 雾完全覆盖距离

void main()
{
    // 光照参数
    float ambientStrength = 0.5;
    float specularStrength = 0.1;
    float shininess = 32.0;
    vec3 lightColor = vec3(1.0, 0.95, 0.9);

    // 计算基本变量
    vec3 normal = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
    vec3 lightDirNormalized = normalize(lightDir);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // 从纹理中获取材质颜色
    vec3 materialColor = texture(ourTexture, TexCoord).rgb;

    // 环境光（基于面朝向的 AO：顶面最亮，侧面次之，底面最暗）
    float aoFactor = 0.5 + 0.5 * normal.y; // 顶面=1.0, 侧面=0.5, 底面=0.0
    float ao = mix(0.4, 1.0, aoFactor);     // 底面衰减到 0.4，顶面保持 1.0
    vec3 ambient = ambientStrength * ao * lightColor * materialColor;

    // 漫反射
    float diff = max(dot(normal, lightDirNormalized), 0.0);
    vec3 diffuse = diff * lightColor * materialColor;

    // 镜面反射
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // 合成光照颜色
    vec3 litColor = ambient + diffuse + specular;

    // 边缘暗化：利用 UV 坐标在每个体素面边缘加深，形成网格轮廓
    vec2 edgeDist = min(fract(TexCoord), 1.0 - fract(TexCoord)); // 到最近边缘的距离 [0, 0.5]
    float edge = smoothstep(0.0, 0.05, min(edgeDist.x, edgeDist.y)); // 边缘 5% 范围内暗化
    litColor *= mix(0.7, 1.0, edge); // 边缘处衰减到 70%

    // 距离雾：线性插值
    float dist = length(viewPos - FragPos);
    float fogFactor = clamp((fogFar - dist) / (fogFar - fogNear), 0.0, 1.0);
    vec3 finalColor = mix(fogColor, litColor, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}
