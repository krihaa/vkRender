#include <iostream>

#include "render/render.h"

auto main(int, char**) -> int {

    try {
        auto window = WindowBuilder().Build();
        auto event = window.HandleEvents();
        auto render = Render(window);

        auto stop = false;
        while(!stop) {
            auto event = window.HandleEvents();

            if(event.type == Event::Type::WindowQuit)
            {
                stop = true;
                break;
            }

            switch (event.type)
            {
                case Event::Type::WindowResize:
                    render.Resize(event.window.width, event.window.height);
                    break;
            }

            render.DrawFrame();
        }
    }
    catch(std::exception e)
    {
        warn(e.what());
        return 0;
    }

    return 0;
}