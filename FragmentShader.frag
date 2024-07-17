#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos; // �Ӷ�����ɫ������Ķ���λ��

// texture sampler
uniform sampler2D ourTexture;
uniform vec3 lightDir; // ƽ�йⷽ��
uniform vec3 viewPos; // �ӵ�λ��

void main()
{
	// ���������ɫ
    float ambientStrength = 0.6; // ������ǿ��
    float specularStrength = 0.1; // ���淴��ǿ��
    float shininess = 32.0; // �߹�ϵ��
    vec3 lightColor = vec3(1.0, 0.95, 0.9);

	// �����������
    vec3 normal = normalize(cross(dFdx(FragPos), dFdy(FragPos))); 
    vec3 lightDirNormalized = normalize(lightDir); // ƽ�йⷽ���Ѿ��Ǳ�׼����
    vec3 viewDir = normalize(viewPos - FragPos); // ���߷���
    vec3 halfwayDir = normalize(lightDir + viewDir); // �������

     // �������л�ȡ������ɫ
    vec3 materialColor = texture(ourTexture, TexCoord).rgb;

    // ������ǿ��
    vec3 ambient = ambientStrength * lightColor * materialColor;

    // ������ǿ��
    float diff = max(dot(normal, lightDirNormalized), 0.0);
    vec3 diffuse = diff * lightColor * materialColor;

    // ���淴��ǿ��
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // �ϳ�������ɫ
    vec3 finalColor = ambient + diffuse + specular;

    // ��������ɫ��ֵ��Ƭ����ɫ���
    FragColor = vec4(finalColor, 1.0);

	//FragColor = texture(ourTexture, TexCoord);//������ֱ�ӽ�����ֵ����ɫ
}