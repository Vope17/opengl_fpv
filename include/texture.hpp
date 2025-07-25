#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Texture
{
  public:
    // the texture ID
    unsigned int ID;

    // constructor reads and builds the texture
    Texture(const char* imgPath)
    {
        stbi_set_flip_vertically_on_load(true);
        int width, height, nrChannels;
        unsigned char* data =
            stbi_load(imgPath, &width, &height, &nrChannels, 0);

        std::cout << imgPath << ": " << width << "x" << height << " "
                  << std::endl;

        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        // set the texture wrapping/filtering options (on the currently bound
        // texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        unsigned int format;
        if (nrChannels == 4)
        {
            format = GL_RGBA;
        }
        else if (nrChannels == 3)
        {
            format = GL_RGB;
        }

        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format,
                         GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
    }
    void active2D()
    {
        GLenum textureUnit = GL_TEXTURE0 + ID;
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, ID);
    }
    void active3D()
    {
        GLenum textureUnit = GL_TEXTURE0 + ID;
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_3D, ID);
    }
};

#endif
