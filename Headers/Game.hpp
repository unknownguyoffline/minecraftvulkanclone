#pragma once
#include "CommonIncludes.hpp"
#include "Macros.hpp"

class Game
{
public:
    void Initialize();
    void Terminate();
    void Run();

    Game();
    ~Game();
private:
    Window mWindow;
    vkn::VulkanContext mVulkanContext;
};