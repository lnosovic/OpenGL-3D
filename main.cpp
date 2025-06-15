// Autor: Lazar Nosovic RA21/2021

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

//GLM biblioteke
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
using namespace glm;


unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

//Kamera
vec3 cameraPos = vec3(0.0f, 2.0f, 10.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
float cameraYaw = -90.0f;
float cameraPitch = 0.0f;
float cameraRadius = 10.0f;
float cameraSpeed = 50.0f;
float fov = 45.0f;

//Vreme
float deltaTime = 0.0f;
float lastFrame = 0.0f;


const double FRAME_TIME = 1.0 / 60.0;

struct Particle {
    vec3 position;
    vec3 velocity;
    vec3 rotationAxis;
    float rotationAngle;
    float life;
};

vector<Particle> particles;
const int MAX_PARTICLES = 100;
float particleSpawnTimer = 0.0f;
float particleSpawnRate = 0.05f;
const float WATER_LEVEL = -0.4f;
const vec3 FOUNTAIN_TOP = vec3(0.0f, 2.85f, 0.0f);


int main(void)
{


    if (!glfwInit())
    {
        cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth = 1280;
    unsigned int wHeight = 720;
    const char wTitle[] = "Fontana";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);

    if (window == NULL)
    {
        cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glViewport(0, 0, wWidth, wHeight);

    if (glewInit() != GLEW_OK)
    {
        cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");

    unsigned int VAO[4];
    glGenVertexArrays(4, VAO);
    unsigned int VBO[4];
    glGenBuffers(4, VBO);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            VODA           +++++++++++++++++++++++++++++++++++++++++++++++++

    float water[] =
    {
        //X    Y    Z       R    G    B    A            S   T
        -0.5f, 0.0f,  0.5f,    0.6f, 0.6f, 0.6f, 1.0f,  0.0f,1.0f,
         0.5f, 0.0f,  0.5f,    0.6f, 0.6f, 0.6f, 1.0f,  1.0f,1.0f,
        -0.5f, 0.0f, -0.5f,    0.6f, 0.6f, 0.6f, 1.0f,  0.0f,0.0f,
         0.5f, 0.0f, -0.5f,    0.6f, 0.6f, 0.6f, 1.0f,  1.0f,0.0f
    };
    unsigned int stride = (3 + 4 + 2) * sizeof(float);

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(water), water, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    //++++++++++++++++++++++++++++++++++++++++++++++++++++++            KOCKA            +++++++++++++++++++++++++++++++++++++++++++++++++
    float cube[] = {
        //X    Y    Z       R    G    B    A            S   T
         0.5f,  0.5f, -0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   1.0f, 1.0f,

        -0.5f, -0.5f,  0.5f,  0.8f, 0.8f, 0.8f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.8f, 0.8f, 0.8f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.8f, 0.8f, 0.8f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.8f, 0.8f, 0.8f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.8f, 0.8f, 0.8f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.8f, 0.8f, 0.8f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  0.75f, 0.75f, 0.75f, 1.0f,    1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.75f, 0.75f, 0.75f, 1.0f,    1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.75f, 0.75f, 0.75f, 1.0f,    0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.75f, 0.75f, 0.75f, 1.0f,    0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.75f, 0.75f, 0.75f, 1.0f,    0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.75f, 0.75f, 0.75f, 1.0f,    1.0f, 1.0f,

         0.5f, -0.5f, -0.5f,  0.85f, 0.85f, 0.85f, 1.0f,    0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.85f, 0.85f, 0.85f, 1.0f,    1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.85f, 0.85f, 0.85f, 1.0f,    1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.85f, 0.85f, 0.85f, 1.0f,    1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.85f, 0.85f, 0.85f, 1.0f,    0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.85f, 0.85f, 0.85f, 1.0f,    0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  0.9f, 0.9f, 0.9f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.9f, 0.9f, 0.9f, 1.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.9f, 0.9f, 0.9f, 1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.9f, 0.9f, 0.9f, 1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.9f, 0.9f, 0.9f, 1.0f,   0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.9f, 0.9f, 0.9f, 1.0f,   1.0f, 1.0f,
    };

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++            PIRAMIDA            +++++++++++++++++++++++++++++++++++++++++++++++++

    float pyramid[] = {
        //X    Y    Z       R    G    B    A            S   T
        //BAZA
        -0.5f, -0.5f, -0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.6f, 0.6f, 0.6f, 1.0f,   0.0f, 0.0f,

        // STRANICE
         0.0f,  0.5f,  0.0f,  0.9f, 0.9f, 0.9f, 1.0f,   0.5f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.8f, 1.0f,   1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.8f, 1.0f,   0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   0.0f, 0.0f,
         0.0f,  0.5f,  0.0f,  0.9f, 0.9f, 0.9f, 1.0f,   0.5f, 1.0f,

        -0.5f, -0.5f, -0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   0.0f, 0.0f,
         0.0f,  0.5f,  0.0f,  0.9f, 0.9f, 0.9f, 1.0f,   0.5f, 01.0f,

         0.0f,  0.5f,  0.0f,  0.9f, 0.9f, 0.9f, 1.0f,   0.5f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.7f, 0.7f, 0.7f, 1.0f,   0.0f, 0.0f,
    };

    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid), pyramid, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++            TEKSTURA            +++++++++++++++++++++++++++++++++++++++++++++++++


    unsigned int textureShader = createShader("texture.vert", "texture.frag");
    unsigned texture = loadImageToTexture("texture2.png");
    unsigned waterTexture = loadImageToTexture("water.jpg");
    unsigned  marbleTexture = loadImageToTexture("marble.jpg");
    unsigned  smallCubeTexture = loadImageToTexture("Sky_Blue.png");

    float textureVertices[] =
    {
        1.0f,1.0f,    1.0f,1.0f,
        -1.0f,1.0f,   0.0f,1.0f,
        1.0f,-1.0f,   1.0f,0.0f,
        -1.0f,-1.0f,  0.0f,0.0f,

    };
    unsigned int textureStride = (2 + 2) * sizeof(float);
    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureVertices), textureVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, textureStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, textureStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Tekstura pozadine
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned int uTexLoc = glGetUniformLocation(textureShader, "uTex");
    glUseProgram(textureShader);
    glUniform1i(uTexLoc, 0);
    glUseProgram(0);

    //Tekstura vode
    glBindTexture(GL_TEXTURE_2D, waterTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    //Tekstura fontane
    glBindTexture(GL_TEXTURE_2D, marbleTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);


    //Tekstura male kocke
    glBindTexture(GL_TEXTURE_2D, smallCubeTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int uUniTexLoc = glGetUniformLocation(unifiedShader, "uTex");
    glUseProgram(unifiedShader);
    glUniform1i(uUniTexLoc, 0);
    glUseProgram(0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++

    mat4 model = mat4(1.0f); 
    unsigned int modelLoc = glGetUniformLocation(unifiedShader, "uM");

    mat4 view; 
    view = lookAt(cameraPos, vec3(0.0f, 1.0f, 0.0f), cameraUp); 
    unsigned int viewLoc = glGetUniformLocation(unifiedShader, "uV");


    mat4 projection = perspective(radians(fov), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    unsigned int projectionLoc = glGetUniformLocation(unifiedShader, "uP");

    //// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++

    glUseProgram(unifiedShader);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        auto fpsStartTime = chrono::high_resolution_clock::now();

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        particleSpawnTimer += deltaTime;
        if (particleSpawnTimer > particleSpawnRate) {
            particleSpawnTimer = 0.0f;
            int newParticles = 1;
            for (int i = 0; i < newParticles; ++i) {
                if (particles.size() < MAX_PARTICLES) {
                    Particle p;
                    p.position = FOUNTAIN_TOP;
                    p.velocity = vec3((rand() % 100 - 50) / 25.0f, 3.0f, (rand() % 100 - 50) / 25.0f);
                    p.rotationAxis = normalize(vec3(rand() % 100, rand() % 100, rand() % 100));
                    p.rotationAngle = 0.0f;
                    p.life = 5.0f;
                    particles.push_back(p);
                }
            }
        }

        for (int i = 0; i < particles.size(); ++i) {
            particles[i].life -= deltaTime;
            if (particles[i].life > 0.0f && particles[i].position.y > WATER_LEVEL) {
                particles[i].velocity.y -= 9.81f * deltaTime;
                particles[i].position += particles[i].velocity * deltaTime;
                particles[i].rotationAngle += deltaTime * 5.0f;
            }
            else {
                particles.erase(particles.begin() + i);
                i--;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(unifiedShader);

        int screenWidth, screenHeight;
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

        projection = perspective(radians(fov), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
        cameraPos.x = sin(radians(cameraYaw)) * cos(radians(cameraPitch)) * cameraRadius;
        cameraPos.y = sin(radians(cameraPitch)) * cameraRadius;
        cameraPos.z = cos(radians(cameraYaw)) * cos(radians(cameraPitch)) * cameraRadius;
        view = lookAt(cameraPos, vec3(0.0f, 1.0f, 0.0f), cameraUp);

        glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uP"), 1, GL_FALSE, value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uV"), 1, GL_FALSE, value_ptr(view));


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++ CRTANJE VODE +++++++++++++++++++++++++++++++++++++++++++++++++
        //CRTANJE STRUKTURE FONTANE
        //TEMELJ
        glBindTexture(GL_TEXTURE_2D, marbleTexture);
        model = mat4(1.0f);
        model = translate(model, vec3(0.0f, -0.75f, 0.0f));
        model = scale(model, vec3(5.0f, 0.5f, 5.0f));
        glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uM"), 1, GL_FALSE, value_ptr(model));
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //STUB
        model = mat4(1.0f);
        model = translate(model, vec3(0.0f, 0.5f, 0.0f));
        model = scale(model, vec3(0.5f, 2.0f, 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uM"), 1, GL_FALSE, value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //GORNJI DEO
        model = mat4(1.0f);
        model = translate(model, vec3(0.0f, 1.75f, 0.0f));
        model = scale(model, vec3(1.5f, 0.5f, 1.5f));
        glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uM"), 1, GL_FALSE, value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        //VRH FONTANE
        model = mat4(1.0f);
        model = translate(model, vec3(0.0f, 2.45f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uM"), 1, GL_FALSE, value_ptr(model));
        glBindVertexArray(VAO[2]);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        //CRTANJE VODE
        glBindTexture(GL_TEXTURE_2D, waterTexture);
        model = mat4(1.0f);
        model = translate(model, vec3(0.0f, -0.5f + 0.01, 0.0f));
        model = scale(model, vec3(4.8f, 1.0f, 4.8f));
        glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uM"), 1, GL_FALSE, value_ptr(model));
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //CRTANJE CESTICA
        glBindVertexArray(VAO[1]);
        glBindTexture(GL_TEXTURE_2D, smallCubeTexture);
        for (const auto& p : particles) {
            model = mat4(1.0f);
            model = translate(model, p.position);
            model = rotate(model, p.rotationAngle, p.rotationAxis);
            model = scale(model, vec3(0.1f));
            glUniformMatrix4fv(glGetUniformLocation(unifiedShader, "uM"), 1, GL_FALSE, value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //Pozadina
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(textureShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(VAO[3]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisable(GL_BLEND);
        glBindVertexArray(0);

        //Ogranicenje na 60 fps
        auto fpsEndTime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = fpsEndTime - fpsStartTime;
        double sleepTime = FRAME_TIME - elapsed.count();
        if (sleepTime > 0) {
            this_thread::sleep_for(chrono::duration<double>(sleepTime));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++


    glDeleteBuffers(4, VBO);
    glDeleteVertexArrays(4, VAO);
    glDeleteProgram(unifiedShader);
    glDeleteProgram(textureShader);

    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Kretanje kamere
    float speed = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPitch += speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPitch -= speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraYaw -= speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraYaw += speed;

    // Ogranicavanje pitcha
    if (cameraPitch > 89.0f) cameraPitch = 89.0f;
    if (cameraPitch < -89.0f) cameraPitch = -89.0f;

    // Zumiranje 
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        fov -= 15.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        fov += 15.0f * deltaTime;

    // Ogranicavanje fova
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 90.0f) fov = 90.0f;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}