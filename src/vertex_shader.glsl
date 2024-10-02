#version 330 core
layout (location = 0) in vec3 aPos;   // Position attribute
layout (location = 1) in vec3 aNormal; // Normal attribute

out vec3 FragPos; // To pass to the fragment shader
out vec3 Normal;  // To pass to the fragment shader

uniform mat4 mvp;
uniform mat4 model;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0)); // Calculate fragment position in world space
    Normal = mat3(transpose(inverse(model))) * aNormal; // Correct normal
    gl_Position = mvp * vec4(aPos, 1.0);
}
