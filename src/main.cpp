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
                        //
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
    Shader ourShader("shaders/texture.vert", "shaders/texture.frag");
    Shader lightingShader("shaders/light.vert", "shaders/light.frag");
    Shader lightCubeShader("shaders/lightSource.vert",
                           "shaders/lightSource.frag");

    // set up texture
    // ------------------------------------------------------------------
    Texture texture1("media/container.jpg");
    Texture texture2("media/awesomeface.png");

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    // texture attributes
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightCube_vertices),
                 lightCube_vertices, GL_STATIC_DRAW);
    // set the vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    texture1.active2D();
    texture2.active2D();

    Map map(MAP_WIDTH, MAP_HEIGHT);

    // Learing Light OpenGL

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
        ourShader.use();
        ourShader.setInt("texture1", texture1.ID); // or with shader class
        ourShader.setInt("texture2", texture2.ID); // or with shader class

        // be sure to activate shader when setting uniforms/drawing objects

        glm::mat4 view;
        view = player.camera.GetViewMatrix();
        //// note that we're translating the scene in the reverse direction of
        //// where we want to move
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        ourShader.setMat4("view", view);

        glm::mat4 projection = glm::perspective(
            glm::radians(player.camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        for (unsigned int i = 0; i < map.map.size(); ++i)
        {
            // float angle = 20.0f * i;
            glm::mat4 model = glm::mat4(1.0f);

            ourShader.setMat4("model", model);
            // trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));

            glm::mat4 trans = glm::mat4(1.0f);
            trans = glm::translate(trans, map.map[i].pos);
            ourShader.setMat4("trans", trans);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

        // view/projection transformations
        projection = glm::perspective(glm::radians(player.camera.Zoom),
                                      (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                      0.1f, 100.0f);
        view = player.camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        glm::vec3 objectPos(15.0f, 10.0f, 22.0f);
        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, objectPos);
        model = glm::scale(model, glm::vec3(5.0f)); // a smaller cube
        lightingShader.setMat4("model", model);

        lightingShader.setVec3("viewPos", player.camera.Position);

        glm::vec3 lightPos(10.0f, 10.0f, 20.0f);
        lightingShader.setVec3("lightPos", lightPos);
        // also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        // model = glm::translate(model, lightPos);
        // model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        // model = glm::rotate(model, (float)glfwGetTime() *
        // glm::radians(80.0f),
        //                     glm::vec3(0.5f, 1.0f, 0.0f));
        //  步驟 1：將 A 物體平移到相對於 B 物體的位置（即將 B
        //  物體作為臨時原點）
        model = glm::translate(model, objectPos);

        //// 步驟 2：應用旋轉操作（例如圍繞 y 軸旋轉，角度隨時間變化）
        float angle = (float)glfwGetTime() * glm::radians(90.0f); //
        //     旋轉角度，隨時間變化
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

        // 步驟 3：將 A 物體平移回來，恢復到世界坐標系中的正確位置
        model = glm::translate(model, lightPos);

        // 步驟 4：如果 A 物體有自己的初始位置或偏移，應用它
        model = glm::translate(model,
                               lightPos - objectPos); // 初始位置相對於 B
        //                       的偏移

        lightCubeShader.setMat4("model", model);
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        lightingShader.use();
        glm::vec3 currentLightPos =
            glm::vec3(model[3][0], model[3][1], model[3][2]);
        lightingShader.setVec3("lightPos", currentLightPos);

        // render the cube
        glBindVertexArray(lightingVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
