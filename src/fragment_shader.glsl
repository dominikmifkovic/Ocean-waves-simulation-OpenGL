#version 330 core
out vec4 FragColor;

in vec3 FragPos; // Fragment position (from vertex shader)
in vec3 Normal;  // Normal (from vertex shader)

uniform vec3 lightPos;      // Light position in world space
uniform vec3 viewPos;       // Camera position (for specular reflection)
uniform vec3 lightColor;    // Light color
uniform vec3 objectColor;   // Object color (e.g., grey)

void main() {
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);  // Shininess factor
    vec3 specular = specularStrength * spec * lightColor;

    // Combine results
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
