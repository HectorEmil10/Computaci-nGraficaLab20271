#version 330 core
struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform Material material;
uniform Light light1; // Cambiado a light1
uniform Light light2; // Agregada la luz 2

uniform sampler2D texture_diffuse;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 ambient1 = light1.ambient * material.diffuse;
    vec3 lightDir1 = normalize(light1.position - FragPos);
    float diff1 = max(dot(norm, lightDir1), 0.0);
    vec3 diffuse1 = light1.diffuse * diff1 * material.diffuse;
    vec3 reflectDir1 = reflect(-lightDir1, norm);
    float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), material.shininess);
    vec3 specular1 = light1.specular * (spec1 * material.specular);

    vec3 ambient2 = light2.ambient * material.diffuse;
    vec3 lightDir2 = normalize(light2.position - FragPos);
    float diff2 = max(dot(norm, lightDir2), 0.0);
    vec3 diffuse2 = light2.diffuse * diff2 * material.diffuse;
    vec3 reflectDir2 = reflect(-lightDir2, norm);
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), material.shininess);
    vec3 specular2 = light2.specular * (spec2 * material.specular);

    vec3 totalAmbient = ambient1 + ambient2;
    vec3 totalDiffuse = diffuse1 + diffuse2;
    vec3 totalSpecular = specular1 + specular2;

    vec3 result = totalAmbient + totalDiffuse + totalSpecular;
    
    color = vec4(result, 1.0f) * texture(texture_diffuse, TexCoords);
}