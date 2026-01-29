#include <autophage/ecs/component_storage.hpp>

#include <cassert>
#include <iostream>


using namespace autophage;
using namespace autophage::ecs;

struct Position
{
    float x, y;
};

int main()
{
    std::cout << "Starting ECS Debug\n";

    ComponentArray<Position> positions;
    std::cout << "Created ComponentArray\n";

    Entity e1{1, 1};

    std::cout << "Setting e1\n";
    positions.set(e1, {10.0f, 20.0f});

    std::cout << "Checking has(e1)\n";
    bool has = positions.has(e1);
    std::cout << "Has e1: " << has << "\n";

    if (has) {
        auto* pos = positions.get(e1);
        std::cout << "Pos: " << pos->x << ", " << pos->y << "\n";
    }

    std::cout << "Done\n";
    return 0;
}
