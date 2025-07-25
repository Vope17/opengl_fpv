#pragma once

#include "block.hpp"
#include <vector>
#include "FastNoiseLite.h"
class Map
{
  public:
    std::vector<Block> map;
    std::vector<std::vector<float>> heightMap;
    int width, depth;

    Map(int width, int depth) : width(width), depth(depth)
    {
        heightMap = std::vector<std::vector<float>>(
            width + 1, std::vector<float>(depth + 1));
        map = generateTerrain(width, depth);
    }
    std::vector<Block> generateTerrain(int width, int depth, float scale = 0.5f,
                                       float heightScale = 10.0f)
    {
        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetFrequency(0.1f);
        noise.SetSeed(1337);

        std::vector<Block> map;
        for (int x = 0; x < width; ++x)
        {
            for (int z = 0; z < depth; ++z)
            {
                // Use Perlin Noise to get the height
                float noiseValue =
                    noise.GetNoise(static_cast<float>(x) * scale,
                                   static_cast<float>(z) * scale);

                float height = (noiseValue + 1.0f) * 0.5f * heightScale;
                map.emplace_back(static_cast<float>(x), height,
                                 static_cast<float>(z));

                float topHeight = height + 1.0f;
                heightMap[x][z] = topHeight;
            }
        }
        return map;
    }
};
