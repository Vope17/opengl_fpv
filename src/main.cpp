#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <math.h>
#include <vector>

#include "glm/detail/type_vec.hpp"
#include "FastNoiseLite.h"

#include "person.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "map.hpp"
#include "vertice.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// settings
#define MAP_WIDTH 100
#define MAP_HEIGHT 100
#define SCR_WIDTH 800
#define SCR_HEIGHT 600

Person player(Camera(glm::vec3(0.0f, 10.0f, 15.0f)),
              glm::vec3(0.0f, 7.0f, 15.0f));

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;
float fov = 45.0f;

// timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

static GLFWwindow* windowInit()
{

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }
    return window;
}

static void clearScreen()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

int main()
{
    GLFWwindow* window = windowInit();
    if (window == NULL)
        return -1;
    // ------------------------------------
    // load shader program
    Shader floorShader("shaders/texture.vert", "shaders/texture.frag");
    Shader lightingShader("shaders/light.vert", "shaders/light.frag");
    Shader lightSourceShader("shaders/lightSource.vert",
                             "shaders/lightSource.frag");

    // set up texture
    // ------------------------------------------------------------------
    Texture texture1("media/container.jpg");
    Texture texture2("media/awesomeface.png");
    Texture texture3("media/container2.png");
    Texture texture3_specular("media/container2_specular.png");
    Texture texture3_specular_color("media/container2_specular_color.png");
    Texture texture4_emission("media/matrix.jpg");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    unsigned int VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices,
                 GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
    //              GL_STATIC_DRAW);

    // position attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    // texture attributes
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered
    // VBO as the vertex attribute's bound vertex buffer object so afterwards we
    // can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound
    // element buffer object IS stored in the VAO; keep the EBO bound.
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally
    // modify this VAO, but this rarely happens. Modifying other VAOs requires a
    // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
    // VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // lighting object
    unsigned int lightingVAO, lightingVBO;
    glGenVertexArrays(1, &lightingVAO);
    glBindVertexArray(lightingVAO);
    // we only need to bind to the VBO, the container's VBO's data already
    // contains the data.
    glGenBuffers(1, &lightingVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightingVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices,
                 GL_STATIC_DRAW);
    // set the vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    // normal vector attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // second, configure the light's VAO (VBO stays the same; the vertices are
    // the same for the light object which is also a 3D cube)

    // the source of light
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer),
    // no need to fill it; the VBO's data already contains all we need (it's
    // already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, lightingVBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    texture1.active2D();
    texture2.active2D();
    texture3.active2D();
    texture3_specular.active2D();
    // texture3_specular_color.active2D();
    texture4_emission.active2D();
    lightingShader.use();
    lightingShader.setInt("material.diffuse", texture3.ID);
    lightingShader.setInt("material.specular", texture3_specular.ID);
    lightingShader.setInt("material.emission", texture4_emission.ID);

    Map map(MAP_WIDTH, MAP_HEIGHT);

    // Setup view and projection space
    glm::mat4 view;
    glm::mat4 projection =
        glm::perspective(glm::radians(player.camera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    // Setup object and light source position
    glm::vec3 objectPos(15.0f, 10.0f, 22.0f);
    glm::vec3 lightPos(10.0f, 10.0f, 20.0f);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // std::cout << player.camera.Position.x << "," <<
        // player.camera.Position.y
        //           << "," << player.camera.Position.z << std::endl;

        //  input
        //  -----
        processInput(window);

        // render
        // ------
        clearScreen();

        // seeing as we only have a single VAO
        // there's no need to bind it every time, but we'll do
        // so to keep things a bit more organized
        // glDrawArrays(GL_TRIANGLES, 0, 15);

        player.Update_yPos(deltaTime, map.heightMap);
        glBindVertexArray(VAO);

        // view/projection transformations
        view = glm::translate(player.camera.GetViewMatrix(),
                              glm::vec3(0.0f, 0.0f, -3.0f));
        projection = glm::perspective(glm::radians(player.camera.Zoom),
                                      (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                      0.1f, 100.0f);

        // floor shader
        floorShader.use();
        floorShader.setInt("texture1", texture1.ID); // or with shader class
        floorShader.setInt("texture2", texture2.ID); // or with shader class
        floorShader.setMat4("view", view);
        floorShader.setMat4("projection", projection);
        map.createFloor(floorShader);

        // lightSource shader
        glm::mat4 model = glm::mat4(1.0f);
        lightSourceShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);

        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setMat4("view", view);
        lightSourceShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // lighting shader(the object which is spoted by light source)
        lightingShader.use();
        glm::vec3 lightColor(1.0f);
        // lightColor.x = sin(glfwGetTime() * 2.0f);
        // lightColor.y = sin(glfwGetTime() * 0.7f);
        // lightColor.z = sin(glfwGetTime() * 1.3f);

        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);

        lightingShader.setVec3("light.ambient", ambientColor);
        lightingShader.setVec3("light.diffuse", diffuseColor);
        lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // lightingShader.setVec3("material.ambient", ambientColor);
        lightingShader.setFloat("material.shininess", 32.0f);

        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, objectPos);
        // model = glm::scale(model, glm::vec3(2.0f)); // a smaller cube
        lightingShader.setMat4("model", model);
        lightingShader.setVec3("viewPos", player.camera.Position);
        lightingShader.setVec3("lightPos", lightPos);

        glBindVertexArray(lightingVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // glfw: swap buffers and poll IO events (keys
        // pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 步驟 1：將 A 物體平移到相對於 B 物體的位置（即將 B
    // 物體作為臨時原點）
    // model = glm::translate(model, objectPos);
    // // 步驟 2：應用旋轉操作（例如圍繞 y
    // // 軸旋轉，角度隨時間變化）旋轉角度，隨時間變化
    // float angle = (float)glfwGetTime() * glm::radians(90.0f);
    // model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    // // 步驟 3：將 A 物體平移回來，恢復到世界坐標系中的正確位置
    // model = glm::translate(model, lightPos);
    // // 步驟 4：如果 A 物體有自己的初始位置或偏移，應用它
    // model = glm::translate(model, lightPos - objectPos); //
    // 初始位置相對於 B 的偏移

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightingVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    // glDeleteProgram(shaderProgram_orange);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        player.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        player.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        player.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        player.ProcessKeyboard(RIGHT, deltaTime);
}
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset =
        lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    player.camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    player.camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}
