#include "camera.hpp"
#include "block.hpp"

#include <vector>
#include "glm/glm.hpp"
#include <glm/gtx/compatibility.hpp>

#define DEBUG
class Person
{
  public:
    Camera camera;
    glm::vec3 position;
    Block standing_object;
    float eyeHeight;

    Person(Camera camera, glm::vec3 position) : camera(camera), eyeHeight(3.0)
    {
        position = {camera.Position.x, camera.Position.y, camera.Position.z};
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        camera.ProcessKeyboard(direction, deltaTime);
    }

    float heightCheckInterval = 0.1f; // check per 0.1s
    float lastHeightCheckTime = 0.0f;
    float smoothingFactor = 10.0f;

    void Update_yPos(float deltaTime,
                     const std::vector<std::vector<float>>& heightMap)
    {
#ifdef DEBUG
        std::cout << "camera pos: " << camera.Position.x << ','
                  << camera.Position.y << ',' << camera.Position.z << std::endl;

        std::cout << "terrian pos: " << camera.Position.x << ','
                  << heightMap[camera.Position.x][camera.Position.z] << ','
                  << camera.Position.z << std::endl;
#endif

        float terrainHeight = heightMap[camera.Position.x][camera.Position.z];

        float targetHeight = terrainHeight + eyeHeight;
        lastHeightCheckTime = 0.0f;
        camera.Position.y = glm::lerp(camera.Position.y, targetHeight,
                                      smoothingFactor * deltaTime);
        // camera.Position.y = targetHeight;
    }
};
