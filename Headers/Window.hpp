#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct Input
{
    struct MouseInput
    {
        glm::vec2 position = glm::vec2(0);      
        glm::vec2 offset = glm::vec2(0);      
        bool leftPress = false;
        bool rightPress = false;
        bool middlePress = false;
    };

    struct KeyboardInput
    {
		bool keySpace = false;
		bool keyApostrophe = false;
		bool keyComma = false;
		bool keyMinus = false;
		bool keyPeriod = false;
		bool keySlash = false;
		bool key0 = false;
		bool key1 = false;
		bool key2 = false;
		bool key3 = false;
		bool key4 = false;
		bool key5 = false;
		bool key6 = false;
		bool key7 = false;
		bool key8 = false;
		bool key9 = false;
		bool keySemicolon = false;
		bool keyEqual = false;
		bool keyA = false;
		bool keyB = false;
		bool keyC = false;
		bool keyD = false;
		bool keyE = false;
		bool keyF = false;
		bool keyG = false;
		bool keyH = false;
		bool keyI = false;
		bool keyJ = false;
		bool keyK = false;
		bool keyL = false;
		bool keyM = false;
		bool keyN = false;
		bool keyO = false;
		bool keyP = false;
		bool keyQ = false;
		bool keyR = false;
		bool keyS = false;
		bool keyT = false;
		bool keyU = false;
		bool keyV = false;
		bool keyW = false;
		bool keyX = false;
		bool keyY = false;
		bool keyZ = false;
		bool keyLeftBracket = false;
		bool keyBackslash = false;
		bool keyRightBracket = false;
		bool keyGraveAccent = false;
		bool keyEscape = false;
		bool keyEnter = false;
		bool keyTab = false;
		bool keyBackspace = false;
		bool keyInsert = false;
		bool keyDelete = false;
		bool keyRight = false;
		bool keyLeft = false;
		bool keyDown = false;
		bool keyUp = false;
		bool keyPageUp = false;
		bool keyPageDown = false;
		bool keyHome = false;
		bool keyEnd = false;
		bool keyCapsLock = false;
		bool keyScrollLock = false;
		bool keyNumLock = false;
		bool keyPrintScreen = false;
		bool keyPause = false;
		bool keyF1 = false;
		bool keyF2 = false;
		bool keyF3 = false;
		bool keyF4 = false;
		bool keyF5 = false;
		bool keyF6 = false;
		bool keyF7 = false;
		bool keyF8 = false;
		bool keyF9 = false;
		bool keyF10 = false;
		bool keyF11 = false;
		bool keyF12 = false;
		bool keyF13 = false;
		bool keyF14 = false;
		bool keyF15 = false;
		bool keyF16 = false;
		bool keyF17 = false;
		bool keyF18 = false;
		bool keyF19 = false;
		bool keyF20 = false;
		bool keyF21 = false;
		bool keyF22 = false;
		bool keyF23 = false;
		bool keyF24 = false;
		bool keyF25 = false;
		bool keyPad0 = false;
		bool keyPad1 = false;
		bool keyPad2 = false;
		bool keyPad3 = false;
		bool keyPad4 = false;
		bool keyPad5 = false;
		bool keyPad6 = false;
		bool keyPad7 = false;
		bool keyPad8 = false;
		bool keyPad9 = false;
		bool keyPadDecimal = false;
		bool keyPadDivide = false;
		bool keyPadMultiply = false;
		bool keyPadSubtract = false;
		bool keyPadAdd = false;
		bool keyPadEnter = false;
		bool keyLeftShift = false;
		bool keyLeftControl = false;
		bool keyLeftAlt = false;
		bool keyLeftSuper = false;
		bool keyRightShift = false;
		bool keyRightControl = false;
		bool keyRightAlt = false;
		bool keyRightSuper = false;
		bool keyMenu = false;
		bool keyLast = false;
    };

    struct WindowInput
    {
        bool close = false;
        bool resize = false;
        bool minimized = false;
        bool maximized = false;
        glm::uvec2 size = glm::uvec2(0);
    };
    
    MouseInput mouse;
    KeyboardInput keyboard;
    WindowInput window;
};

struct WindowUserPointer
{
    Input input;
};

struct WindowCreateInfo
{
    int width = 800, height = 600;
    const char* title;
};

class Window
{
    public:
        Window() {}
        Window(const WindowCreateInfo& createInfo);


        Input GetInput();
        glm::uvec2 GetFramebufferSize();

		GLFWwindow* GetNativeWindow() { return mWindow; }

        ~Window();

    private:
        static uint32_t sWindowCount;
        static bool sGlfwInitialized;
        GLFWwindow* mWindow;
        WindowUserPointer mUserPointer;
};

void PollEvent();