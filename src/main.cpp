#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <ctime>
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubeMap(vector<std::string> faces);

int randRange(int low,int high);
void renderQuad();
void renderCube();

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    bool spotlight = true;
    PointLight pointLight;
    float ambientLight = 0.0f;

    ProgramState()
            : camera(glm::vec3(-0.5f, 5.0f, 100.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
    glm::vec3 tempPosition=glm::vec3(0.0f, 2.0f, 0.0f);
    float tempRotation=0.0f;
};
void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n'
        << spotlight << '\n'
        << ambientLight << '\n';
}
void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z
           >> spotlight
           >> ambientLight;
    }
}
ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Projekat", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    //Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // tell stb_image.h to flip loaded textures on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // build and compile shaders
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader gBufferShader("resources/shaders/gBuffer.vs", "resources/shaders/gBuffer.fs");
    Shader lightingPassShader("resources/shaders/lightingPass.vs","resources/shaders/lightingPass.fs");
    Shader lightShowShader("resources/shaders/lightShow.vs","resources/shaders/lightShow.fs");
    // load models
    Model ourModel("resources/objects/backpack/backpack.obj");
    ourModel.SetShaderTextureNamePrefix("material.");

    // model stene
    Model rock("resources/objects/BigStone/Low_uw.obj");
    rock.SetShaderTextureNamePrefix("material.");

    //model vecih stena
    Model bigRock("resources/objects/stone/SmallArch_Obj.obj");
    bigRock.SetShaderTextureNamePrefix("material.");
    //stene
    Model Rock6("resources/objects/Rock_5/Rock_5/Rock_5.obj");
    Rock6.SetShaderTextureNamePrefix("material.");

    //model vanzemaljca
    Model alien("resources/objects/GrayAlien/10469_GrayAlien_v01.obj");
    alien.SetShaderTextureNamePrefix("material.");

    //model UFO
    Model ufo("resources/objects/ufo/Low_poly_UFO.obj");
    ufo.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    //postavljamo vertexe

    //kamenje
    float transparentVertices2[] = {

            0.0f, -0.5f,  0.0f, 0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
            0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  0.0f,  1.0f,
            1.0f,  0.5f,0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  1.0f,

            0.0f, -0.5f,  0.0f, 0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
            1.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f,  1.0f,  1.0f,
            1.0f, -0.5f,  0.0f, 0.0f, 0.0f, 1.0f,  1.0f,  0.0f
    };


    //ravan
    float ravanVertices[] = {
            100.0f,  0.0f, -100.0f, 0.0f, 1.0f, 0.0f,   100.0f, 100.0f, // top right
            100.0f, 0.0f, 100.0f, 0.0f, 1.0f, 0.0f,  100.0f, 0.0f, // bottom right
            -100.0f, 0.0f, 100.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // bottom left
            -100.0f,  0.0f, -100.0f, 0.0f, 1.0f, 0.0f,  0.0f, 100.0f  // top left
    };
    unsigned int ravanIndices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };

    //skybox
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };
    //kamenje VAO
    unsigned int transparentVAO2, transparentVBO2;
    glGenVertexArrays(1, &transparentVAO2);
    glGenBuffers(1, &transparentVBO2);
    glBindVertexArray(transparentVAO2);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices2), transparentVertices2, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    //ravan VAO
    unsigned int ravanVBO, ravanVAO, ravanEBO;
    glGenVertexArrays(1, &ravanVAO);
    glGenBuffers(1, &ravanVBO);
    glGenBuffers(1, &ravanEBO);
    glBindVertexArray(ravanVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ravanVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ravanVertices), ravanVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ravanEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ravanIndices), ravanIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    vector<glm::vec3> stone
            {
                    glm::vec3(-1.5f, 0.5f, -0.48f),
                    glm::vec3( 1.5f, 0.5f, 0.51f),
                    glm::vec3( 0.0f, 0.5f, 0.7f),
                    glm::vec3(-0.7f, 0.5f, -2.3f),
                    glm::vec3 (1.0f, 0.5f, -1.2f),
                    glm::vec3 (-0.1f, 0.5f, -0.63f),
                    glm::vec3 (-1.75f, 0.5f, 1.0f),
                    glm::vec3 (-0.6f, 0.5f, -2.0f)
            };

    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/sand.jpg").c_str());
    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/Black.jpg").c_str());
    unsigned int stoneTexture = loadTexture(FileSystem::getPath("resources/textures/ss.png").c_str());
    //load textures
    vector<std::string> faces{
            FileSystem::getPath("resources/textures/skybox/badomen_rt.png"),
            FileSystem::getPath("resources/textures/skybox/badomen_lf.png"),
            FileSystem::getPath("resources/textures/skybox/badomen_up.png"),
            FileSystem::getPath("resources/textures/skybox/badomen_dn.png"),
            FileSystem::getPath("resources/textures/skybox/badomen_ft.png"),
            FileSystem::getPath("resources/textures/skybox/badomen_bk.png")

    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);

    unsigned int cubeMapTexture = loadCubeMap(faces);

    //g buffer
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    unsigned int gPosition, gNormal, gAlbedoSpec;
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    lightShowShader.use();
    const unsigned int br_vanzemaljaca = 10;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    srand(NULL);
    for (unsigned int i = 0; i < br_vanzemaljaca; i++)
    {
        lightPositions.push_back(glm::vec3(-0.35f, 6.55f, i * 12.0f));
        float rColor = ((rand() % 100) / 200.0f) + 0.1;
        float gColor = ((rand() % 100) / 200.0f) + 0.1;
        float bColor = ((rand() % 100) / 200.0f) + 0.1;
        lightColors.push_back(glm::vec3(rColor, gColor, bColor));
    }

    lightingPassShader.use();
    lightingPassShader.setInt("gPosition", 0);
    lightingPassShader.setInt("gNormal", 1);
    lightingPassShader.setInt("gAlbedoSpec", 2);

    ourShader.use();
    ourShader.setInt("material.texture_diffuse1", 0);
    ourShader.setInt("material.texture_specular1", 1);

    glm::vec3 lightPos(-5.0f, 4.0f, -5.0f);
    glm::mat4 model = glm::mat4(1.0f);

    int randArrayX[50];
    for(int i=0;i<50;i++)
        randArrayX[i]= randRange(-50,50);
    int randArrayY[50];
    for(int i=0;i<50;i++)
        randArrayY[i]= randRange(-50,50);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        gBufferShader.use();
        gBufferShader.setMat4("projection", projection);
        gBufferShader.setMat4("view", view);
/*
        glDisable(GL_CULL_FACE);

        model = glm::mat4(1.0f);
        gBufferShader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        glBindVertexArray(ravanVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glEnable(GL_CULL_FACE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



     /*   //
        lightingPassShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        // send light relevant uniforms
        for (unsigned int i = 0; i < lightPositions.size(); i++) {
            lightingPassShader.setVec3("lights[" + std::to_string(i) + "].position", lightPositions[i]);
            lightingPassShader.setVec3("lights[" + std::to_string(i) + "].direction", glm::vec3(0.5f, 0.5f, 0.5f));

            lightingPassShader.setVec3("lights[" + std::to_string(i) + "].color",
                                       sin((float) glfwGetTime() * lightColors[i]) / 2.0f + 0.5f);
            lightingPassShader.setVec3("lights[" + std::to_string(i) + "].ambient", glm::vec3(0.01f));
            lightingPassShader.setVec3("lights[" + std::to_string(i) + "].diffuse", 1.0f, 1.0f, 1.0f);
            lightingPassShader.setVec3("lights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);

            lightingPassShader.setFloat("lights[" + std::to_string(i) + "].constant", 1.0f);
            lightingPassShader.setFloat("lights[" + std::to_string(i) + "].linear", 0.06f);
            lightingPassShader.setFloat("lights[" + std::to_string(i) + "].quadratic", 0.032f);

        }
        lightingPassShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        if(programState->spotlight)
            lightingPassShader.setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
        else
            lightingPassShader.setVec3("dirLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingPassShader.setVec3("dirLight.diffuse", 0.05f, 0.05f, 0.05);
        lightingPassShader.setVec3("dirLight.specular", 0.2f, 0.2f, 0.2f);
        lightingPassShader.setVec3("viewPos", programState->camera.Position);
        // finally render quad
        renderQuad();

        // copy content of geometry's depth buffer to default framebuffer's depth buffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

*/

       // don't forget to enable shader before setting uniforms
        ourShader.use();
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        // view/projection transformations
        //glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
          //                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


        // directional light
        ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        if(programState->ambientLight)
            ourShader.setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
        else
            ourShader.setVec3("dirLight.ambient", 0.0f, 0.0f, 0.0f);
        ourShader.setVec3("dirLight.diffuse", 0.05f, 0.05f, 0.05);
        ourShader.setVec3("dirLight.specular", 0.2f, 0.2f, 0.2f);


        ourShader.setVec3("svetlo.position", programState->camera.Position);
        ourShader.setVec3("svetlo.direction", programState->camera.Front);
        ourShader.setVec3("svetlo.ambient", 0.0f, 0.0f, 0.0f);
        if (programState->spotlight) {
            ourShader.setVec3("svetlo.diffuse", 3.0f, 3.0f, 3.0f);
            ourShader.setVec3("svetlo.specular", glm::vec3(0.2f));
        } else {
            ourShader.setVec3("svetlo.diffuse", 0.0f, 0.0f, 0.0f);
            ourShader.setVec3("svetlo.specular", 0.0f, 0.0f, 0.0f);
        }
        ourShader.setVec3("svetlo.diffuse", 0.0f, 0.0f, 0.0f);
        ourShader.setVec3("svetlo.specular", 0.0f, 0.0f, 0.0f);

        ourShader.setFloat("svetlo.constant", 1.0f);
        ourShader.setFloat("svetlo.linear", 0.09f);
        ourShader.setFloat("svetlo.quadratic", 0.032f);
        ourShader.setFloat("svetlo.cutOff", glm::cos(glm::radians(10.0f)));
        ourShader.setFloat("svetlo.outerCutOff", glm::cos(glm::radians(15.0f)));

        //Renderovanje stena:
        for(int i=0;i<50;i++){
            model = glm::mat4(1.0f);
            int x,z;
            srand(time(NULL));
            model = glm::translate(model, glm::vec3(randArrayX[i], 0.0f,randArrayY[i]));
            model = glm::scale(model, glm::vec3(1.0f));
            model = glm::rotate(model, glm::radians(programState->tempRotation), glm::vec3(0, 1, 0));
            ourShader.setMat4("model", model);
            rock.Draw(ourShader);
        }

        //Renderovanje vecih stena:
        for(int i=0;i<40;i+=7){
            model = glm::mat4(1.0f);
            int x,z;
            srand(time(NULL));
            model = glm::translate(model, glm::vec3(randArrayY[i], 0.6f,randArrayX[i]));
            model = glm::scale(model, glm::vec3(0.15f));
            model = glm::rotate(model, glm::radians(programState->tempRotation), glm::vec3(0, 1, 0));
            ourShader.setMat4("model", model);
            bigRock.Draw(ourShader);
        }

        //UFO

        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(0.0f,5.0f,0.0f));
        model = glm::scale(model, glm::vec3(0.7f));
        ourShader.setMat4("model", model);
        ufo.Draw(ourShader);

        //kamenje
        glDisable(GL_CULL_FACE);
        glBindVertexArray(transparentVAO2);
        glBindTexture(GL_TEXTURE_2D, stoneTexture);
        model = glm::mat4(1.0f);
        for (unsigned int i = 0; i < stone.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, stone[i]);
            //model = glm::rotate(model, (float)i*60.0f, glm::vec3(0.0, 0.1, 0.0));
            ourShader.setMat4("model", model);
            ourShader.setMat4("projection", projection);
            ourShader.setMat4("view", view);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }


        //ravan
        glDisable(GL_CULL_FACE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);
        glBindVertexArray(ravanVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glEnable(GL_CULL_FACE);



        //Vanzemaljci
        lightShowShader.use();
        for(int i=0;i<10;i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(randArrayX[i+4], 0.0f, randArrayY[i+2]));
            model = glm::scale(model, glm::vec3(0.05f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
            lightShowShader.setMat4("model", model);
            lightShowShader.setMat4("projection",projection);
            lightShowShader.setMat4("view",view);
            lightShowShader.setVec3("lightColor",glm::vec3(0.2f,5.0f,0.2f));
            renderCube();
            alien.Draw(lightShowShader);
        }

        //skybox
        skyboxShader.use();
        skyboxShader.setInt("skybox", 0);

        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);

        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);


        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            programState->CameraMouseMovementUpdateEnabled = false;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            programState->CameraMouseMovementUpdateEnabled = true;
        }

    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS && programState->ImGuiEnabled) {
        programState->CameraMouseMovementUpdateEnabled = !programState->CameraMouseMovementUpdateEnabled;
        if (programState->CameraMouseMovementUpdateEnabled == true)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        programState->camera.Position = glm::vec3(0.0f, 0.0f, 3.0f);
        programState->camera.Yaw = 0.0f;
        programState->camera.Pitch = 0.0f;
        programState->camera.Front = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        programState->ambientLight = !programState->ambientLight;
    }
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if(programState->ImGuiEnabled) {
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(600, 130), ImGuiCond_Once);
            ImGui::Begin("Camera settings:", NULL, ImGuiWindowFlags_NoCollapse);
            const Camera &c = programState->camera;
            ImGui::Text("Camera Info:");
            ImGui::Indent();
            ImGui::Bullet();
            ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
            ImGui::Bullet();
            ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
            ImGui::Bullet();
            ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
            ImGui::End();
        }

        {
            static float f = 0.0f;
            ImGui::Begin("Hello window");
            ImGui::DragFloat("pointLight.constant", &programState->ambientLight, 0.05, 0.0, 1.0);
            ImGui::End();
        }

    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

unsigned int loadCubeMap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "CubeMap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
unsigned int loadTexture(char const * path)
{

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

int randRange(int low,int high){
    return rand()%(high-low) + low;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}