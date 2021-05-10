#pragma once

//#ifndef WINDOW_H
//#define WINDOW_H

#include <optional>
#include <vector>
#include <memory>



enum WindowStyle{
    NORMAL,
    FRAMELESS,
};



struct Event
{
    public:

    enum Type
    {
        MouseMove,
        MousePress,
        KeyboardPress,
        WindowResize,
        WindowQuit,
        None,
    } type = Event::Type::None;

    struct Window {
        uint32_t width;
        uint32_t height;
    } window;
};


class PlatformWindow;
namespace vk
{
    class Instance;
    class SurfaceKHR;
};


class Window
{
private:
    std::unique_ptr<PlatformWindow> window;
public:

    Window(PlatformWindow* window);

    std::vector<const char*> static GetInstanceExtensions();

    vk::SurfaceKHR CreateWindowSurface(vk::Instance);

    Event HandleEvents();

    ~Window();

};

class WindowBuilder
{
    private:
    std::optional<uint32_t> width;
    std::optional<uint32_t> height;
    std::optional<int> x;
    std::optional<int> y;
    public:
    void SetSize(uint32_t width, uint32_t height)
    {
        this->width = width;
        this->height = height;
    }

    void SetPosition(int x, int y)
    {
        this->x = x;
        this->y = y;
    }

    Window Build();

};

static std::vector<const char*> GetInstanceExtensions();
    

//#endif



