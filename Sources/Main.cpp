#include "Game.hpp"
#include "Macros.hpp"
#include <glm/gtc/matrix_transform.hpp>

int main()
{
    Game* game = new Game();
    game->Run();
    delete game;
}
