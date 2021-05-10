#include <Windows.h>
#include <iostream>
#include <atlstr.h>
#include <mutex>
#include <unordered_map>
#include "window.h"


#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_win32.h"
#include "../log/log.h"

std::vector<const char*> Window::GetInstanceExtensions()
{
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    auto i = vk::enumerateInstanceExtensionProperties();
    std::vector<const char*> extensions;
    for(auto p : i)
    {
        if(strcmp(p.extensionName,VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        }
        if(strcmp(p.extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
        {
            extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        }
    }

    if(extensions.size() != 2)
    {
        throw(std::exception("Unable to get InstanceExtensions"));
    }

    return extensions;
}

auto get_error_message()
{
    auto error = GetLastError();

    LPVOID message = nullptr;

    auto size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
        , NULL
        , error
        , 0
        , (LPWSTR)&message
        , 0
        , NULL
    );

    std::string error_message = ((LPSTR)message);
    LocalFree(message);

    return error_message;
}


const LPCWSTR WINDOW_CLASS_NAME = L"WINDOWCLASS4389329";

class PlatformWindow
{
private:
    std::unordered_map<Event::Type, Event> event_queue;
public:
    HWND hwnd;
    HINSTANCE hinstance;

    vk::SurfaceKHR CreateWindowSurface(vk::Instance instance)
    {
        VkWin32SurfaceCreateInfoKHR info;
        info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        info.pNext = NULL;
        info.flags = 0;
        info.hwnd = hwnd;
        info.hinstance = GetModuleHandleW(NULL);


        VkSurfaceKHR tmp;
        auto result = vkCreateWin32SurfaceKHR(static_cast<VkInstance>(instance), &info, nullptr, &tmp);
        if(result != VK_SUCCESS)
        {
            throw(std::exception("Unable to create Win32Surface"));
        }
        return static_cast<vk::SurfaceKHR>(tmp);
    }

    LRESULT HandleMessage(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
            case WM_SIZE:
            {
                Event event;
                event.type = Event::Type::WindowResize;
                event.window.width = LOWORD(lparam);
                event.window.height = HIWORD(lparam);

                event_queue[event.type] = event;
                break;
            }

            case WM_DESTROY:
            {
                Event event;
                event.type = Event::Type::WindowQuit;
                event_queue[event.type] = event;
                break;
            }

        }
        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    Event HandleEvents()
    {
        MSG msg = {};
        Event event;
        if (PeekMessageW(&msg, hwnd, 0, 0, PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if(!event_queue.empty())
        {
            event = event_queue.begin()->second;
            event_queue.erase(event_queue.begin());
        }
        return event;
    }

    ~PlatformWindow() = default;
};

vk::SurfaceKHR Window::CreateWindowSurface(vk::Instance instance)
{
    return window->CreateWindowSurface(instance);
}

Event Window::HandleEvents() 
{
    return window->HandleEvents();
}
Window::Window(PlatformWindow* window): window(window)
{}
Window::~Window() = default;

static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    PlatformWindow* instance = nullptr;
    if(msg == WM_NCCREATE) {
        CREATESTRUCT* create = (CREATESTRUCT*)lparam;
        instance = (PlatformWindow*)create->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)instance);

        instance->hwnd = hwnd;
        instance->hinstance = create->hInstance;
        
    } else {
        instance = (PlatformWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    if (instance)
    {
        return instance->HandleMessage(msg, wparam, lparam);
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}


std::once_flag register_class;

Window WindowBuilder::Build()
{
    std::call_once( register_class, [] {
        WNDCLASSEXW window_class;
        window_class.cbSize = sizeof(window_class);
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.cbClsExtra = 0;
        window_class.cbWndExtra = 0;
        window_class.lpszClassName = WINDOW_CLASS_NAME;
        window_class.hbrBackground = NULL;
        window_class.lpfnWndProc  = window_proc;
        window_class.hInstance = NULL;
        window_class.hIcon = NULL;
        window_class.hIconSm = NULL;
        window_class.hCursor = NULL;
        window_class.lpszMenuName = NULL;

        if (RegisterClassExW(&window_class) == 0)
        {
            throw(std::exception(get_error_message().c_str()));
        }
    });


    PlatformWindow* window = new PlatformWindow();
    
    int width = this->width.value_or(CW_USEDEFAULT);
    int height = this->height.value_or(CW_USEDEFAULT);
    int x = this->x.value_or(CW_USEDEFAULT);
    int y = this->y.value_or(CW_USEDEFAULT);

    auto hwnd = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        L"WINDOW",
        WS_OVERLAPPEDWINDOW,
        x,
        y,
        width,
        height,
        NULL,
        NULL,
        GetModuleHandleW(NULL),
        window
    );
    if(!hwnd)
    {
        delete(window);
        throw(std::exception(get_error_message().c_str()));
    }

    window->hwnd = hwnd;

    ShowWindow(hwnd, SW_SHOWNORMAL);
    return Window(window);
}


