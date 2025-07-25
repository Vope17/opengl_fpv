#include <utility>
#include <vector>

struct Position
{
    float x, y, z;
    Position(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z)
    {
    }

    bool operator==(const Position& other) const
    {
        return x == other.x && y == other.y;
    }
};

struct SnakeAttri
{
    Position pos;
    // for OpenGL Render
    std::vector<Position> corner;
};

enum Direction
{
    P_UP,
    P_DOWN,
    P_LEFT,
    P_RIGHT,
};

class Snake
{
  public:
    std::vector<Position> body;
    Direction current_dir;
    int grid_width, grid_height;

    Snake(int grid_width, int grid_height)
        : grid_width(grid_width), grid_height(grid_height)
    {
        body.push_back({(float)grid_width / 2, (float)grid_height / 2});
        body.push_back({(float)grid_width / 2 - 1, (float)grid_height / 2});
        body.push_back({(float)grid_width / 2 - 2, (float)grid_height / 2});
        current_dir = P_RIGHT;
    }
    void setDirection(Direction dir)
    {
        // 防止蛇反向移動
        if ((current_dir == P_UP && dir != P_DOWN) ||
            (current_dir == P_DOWN && dir != P_UP) ||
            (current_dir == P_LEFT && dir != P_RIGHT) ||
            (current_dir == P_RIGHT && dir != P_LEFT))
        {
            current_dir = dir;
        }
    }
    bool update()
    {
        // current_dir = next_dir;

        // 計算新頭部位置
        Position new_head = body[0];
        switch (current_dir)
        {
        case P_UP:
            new_head.y++;
            break;
        case P_DOWN:
            new_head.y--;
            break;
        case P_LEFT:
            new_head.x--;
            break;
        case P_RIGHT:
            new_head.x++;
            break;
        }

        // 檢查邊界碰撞
        if (new_head.x < 0 || new_head.x >= grid_width || new_head.y < 0 ||
            new_head.y >= grid_height)
        {
            return false; // 遊戲結束
        }

        // 檢查自身碰撞
        for (const auto& segment : body)
        {
            if (new_head == segment)
            {
                return false; // 遊戲結束
            }
        }

        // 移動蛇
        body.insert(body.begin(), new_head);
        body.pop_back(); // 移除尾巴（除非吃到食物）

        return true; // 繼續遊戲
    }
};
