#version 330 core
out vec4 FragColor;

struct Light {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform vec3 viewPos;
uniform Light light;
uniform float shininess;
uniform sampler2D texture_diffuse1;
uniform vec3 fogColor;
uniform float renderDistance;

void main()
{
    // ambient
    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, TexCoords).rgb;  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * texture(texture_diffuse1, TexCoords).rgb;  
    
    vec3 result = ambient + diffuse + specular;
    if(fogColor != vec3(0.0f))
    {
        float distFromCam = distance(viewPos, FragPos);
        float fog = smoothstep(renderDistance - 40, renderDistance - 5, length(distFromCam));
        FragColor = vec4(mix(result, fogColor, fog), 1.0);
    }
    else
    {
        FragColor = vec4(result, 1.0);
    }
} 