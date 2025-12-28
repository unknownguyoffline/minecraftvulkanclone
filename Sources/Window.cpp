#include "Window.hpp"
#include <print>

#define GET_USERPOINTER(window) (WindowUserPointer*)glfwGetWindowUserPointer(window)

bool Window::sGlfwInitialized = false;
uint32_t Window::sWindowCount = 0;



void windowCloseCallback(GLFWwindow* window)
{
    WindowUserPointer* userPointer = GET_USERPOINTER(window);
    userPointer->input.window.close = true;
}

void windowSizeCallback(GLFWwindow*  window, int width, int height)
{
    WindowUserPointer* userPointer = GET_USERPOINTER(window);
    userPointer->input.window.size = glm::uvec2(width, height);
}

Window::Window(const WindowCreateInfo& createInfo)
{
    if(!sGlfwInitialized)
        glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, nullptr, nullptr);

    mUserPointer.input.window.size = {createInfo.width, createInfo.height};
    assert(mWindow != nullptr);

    sWindowCount++;

    glfwSetWindowUserPointer(mWindow, &mUserPointer);
    glfwSetWindowCloseCallback(mWindow, windowCloseCallback);
    glfwSetWindowSizeCallback(mWindow, windowSizeCallback);
}

Window::~Window()
{
    glfwDestroyWindow(mWindow);
    sWindowCount--;
    if(sWindowCount == 0)
    glfwTerminate();
}

Input Window::GetInput() { return mUserPointer.input; }

void PollEvent() 
{
    glfwPollEvents();
}
