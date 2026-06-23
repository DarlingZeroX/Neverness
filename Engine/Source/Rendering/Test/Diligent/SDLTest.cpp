// SDLTest.cpp - 简单测试 SDL
#include <iostream>
#include <SDL3/SDL.h>

int main()
{
    std::cout << "SDL version: " << SDL_VERSION << std::endl;
    std::cout << "SDL_Init starting..." << std::endl;
    
    int result = SDL_Init(SDL_INIT_VIDEO);
    std::cout << "SDL_Init returned: " << result << std::endl;
    
    if (result != 0)
    {
        const char* err = SDL_GetError();
        std::cout << "SDL_GetError: [" << (err ? err : "null") << "]" << std::endl;
        
        // Try with just events
        std::cout << "Trying SDL_INIT_EVENTS..." << std::endl;
        result = SDL_Init(SDL_INIT_EVENTS);
        std::cout << "SDL_INIT_EVENTS returned: " << result << std::endl;
    }
    else
    {
        SDL_Window* wnd = SDL_CreateWindow("Test", 640, 480, 0);
        if (wnd)
        {
            std::cout << "Window created!" << std::endl;
            SDL_Delay(1000);
            SDL_DestroyWindow(wnd);
        }
        else
        {
            std::cout << "Window failed: " << SDL_GetError() << std::endl;
        }
    }
    
    SDL_Quit();
    std::cout << "Done" << std::endl;
    return 0;
}
'@

[System.IO.File]::WriteAllText("E:\Neverness\Engine\Source\Experiments\NNRuntimeDiligent\Tests\SDLTest.cpp", $content, [System.Text.UTF8Encoding]::new($false))
Write-Output "Written"
