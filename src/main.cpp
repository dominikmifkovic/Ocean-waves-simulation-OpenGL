#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    uniform float uTime;
    uniform vec2 waveDirections[4];
    uniform float waveAmplitudes[4];
    uniform float waveFrequencies[4];
    uniform float wavePhases[4];

    out vec3 FragPos;
    out vec3 Normal;

    vec3 gerstnerWave(vec3 pos, vec2 dir, float amplitude, float frequency, float phase) {
        float dotDir = dot(dir, pos.xz);
        float theta = dotDir * frequency + uTime + phase;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        vec3 displacement;
        displacement.x = dir.x * amplitude * cosTheta;
        displacement.y = amplitude * sinTheta;
        displacement.z = dir.y * amplitude * cosTheta;
        return pos + displacement;
    }

    void main() {
        vec3 pos = aPos;
        for(int i = 0; i < 4; i++) {
            pos = gerstnerWave(pos, waveDirections[i], waveAmplitudes[i], waveFrequencies[i], wavePhases[i]);
        }
        FragPos = vec3(model * vec4(pos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    struct Light {
        vec3 direction;
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
    };
    
    uniform vec3 objectColor;
    uniform vec3 viewPos;
    uniform Light light;

    in vec3 FragPos;
    in vec3 Normal;

    void main() {
        vec3 ambient = light.ambient * objectColor;
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, -light.direction), 0.0);
        vec3 diffuse = light.diffuse * diff * objectColor;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(light.direction, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = light.specular * spec;
        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result, 1.0);
        FragColor.a = 0.5;
    }
)";

void processInput(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1200,900,"OpenGL Ocean",NULL,NULL);
    if(window == NULL){
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,&vertexShaderSource,NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(vertexShader,512,NULL,infoLog);
        std::cout << "VERTEX SHADER COMPILATION FAILED\n" << infoLog << std::endl;
    }
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&fragmentShaderSource,NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(fragmentShader,512,NULL,infoLog);
        std::cout << "FRAGMENT SHADER COMPILATION FAILED\n" << infoLog << std::endl;
    }
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram,GL_LINK_STATUS,&success);
    if(!success){
        glGetProgramInfoLog(shaderProgram,512,NULL,infoLog);
        std::cout << "SHADER PROGRAM LINKING FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    int gridSize = 100;
    float gridExtent = 10.0f;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    for(int z = 0; z <= gridSize; z++){
        for(int x = 0; x <= gridSize; x++){
            float xpos = ((float)x / gridSize - 0.5f) * gridExtent * 2;
            float zpos = ((float)z / gridSize - 0.5f) * gridExtent * 2;
            vertices.push_back(xpos);
            vertices.push_back(0.0f);
            vertices.push_back(zpos);
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }
    }
    for(int z = 0; z < gridSize; z++){
        for(int x = 0; x < gridSize; x++){
            int start = z * (gridSize + 1) + x;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + gridSize + 1);
            indices.push_back(start + 1);
            indices.push_back(start + gridSize + 2);
            indices.push_back(start + gridSize + 1);
        }
    }
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),&vertices[0],GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned int),&indices[0],GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glm::vec3 lightDir(-0.2f,-1.0f,-0.3f);
    glm::vec3 lightAmbient(0.2f,0.2f,0.2f);
    glm::vec3 lightDiffuse(0.5f,0.5f,0.5f);
    glm::vec3 lightSpecular(1.0f,1.0f,1.0f);
    float timeValue = 0.0f;
    glm::vec2 waveDirections[4] = {
        glm::vec2(1.0f,0.0f),
        glm::vec2(0.0f,1.0f),
        glm::vec2(0.7f,0.7f),
        glm::vec2(-0.7f,0.7f)
    };
    float waveAmplitudes[4] = {0.3f,0.2f,0.1f,0.05f};
    float waveFrequencies[4] = {1.0f,1.5f,0.8f,1.2f};
    float wavePhases[4] = {0.0f,1.0f,2.0f,3.0f};
    while(!glfwWindowShouldClose(window)){
        processInput(window);
        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform3fv(glGetUniformLocation(shaderProgram,"light.direction"),1,glm::value_ptr(lightDir));
        glUniform3fv(glGetUniformLocation(shaderProgram,"light.ambient"),1,glm::value_ptr(lightAmbient));
        glUniform3fv(glGetUniformLocation(shaderProgram,"light.diffuse"),1,glm::value_ptr(lightDiffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram,"light.specular"),1,glm::value_ptr(lightSpecular));
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f,20.0f,20.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"view"),1,GL_FALSE,glm::value_ptr(view));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),1200.0f/900.0f,0.1f,100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"projection"),1,GL_FALSE,glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(shaderProgram,"objectColor"),0.0f,0.5f,1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram,"viewPos"),0.0f,10.0f,10.0f);
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_FALSE,glm::value_ptr(model));
        timeValue += 0.01f;
        glUniform1f(glGetUniformLocation(shaderProgram,"uTime"),timeValue);
        glUniform2fv(glGetUniformLocation(shaderProgram,"waveDirections"),4,glm::value_ptr(waveDirections[0]));
        glUniform1fv(glGetUniformLocation(shaderProgram,"waveAmplitudes"),4,waveAmplitudes);
        glUniform1fv(glGetUniformLocation(shaderProgram,"waveFrequencies"),4,waveFrequencies);
        glUniform1fv(glGetUniformLocation(shaderProgram,"wavePhases"),4,wavePhases);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1,&VAO);
    glDeleteBuffers(1,&VBO);
    glDeleteBuffers(1,&EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
