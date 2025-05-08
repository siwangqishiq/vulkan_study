#include "easyvk.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


GLFWwindow *pWindow = nullptr;
GLFWmonitor *pMonitor = nullptr;
const char* windowTitle = "EasyVk";

bool InitWindow(VkExtent2D size, bool fullScreen = false,bool isResizable = true, bool limitFrameRate = true){
    using namespace vulkan;

    GraphicsBase::width = size.width;
    GraphicsBase::height = size.height;

    if(!glfwInit()){
        std::cerr << "[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n";
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, isResizable);


    pWindow = glfwCreateWindow(size.width , size.height, windowTitle, pMonitor, nullptr);
    if(!pWindow){
        std::cerr << "[ InitializeWindow ] ERROR\nFailed to create a glfw window!\n";
        glfwTerminate();
        return false;
    }

    #ifdef _WIN32
    GraphicsBase::Base().AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    GraphicsBase::Base().AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    #else
    uint32_t extensionCount = 0;
    const char** extensionNames;
    extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
    if (!extensionNames) {
        std::cout << std::format("[ InitializeWindow ]\nVulkan is not available on this machine!\n");
        glfwTerminate();
        return false;
    }
    for (size_t i = 0; i < extensionCount; i++){
        GraphicsBase::Base().AddInstanceExtension(extensionNames[i]);
    }
    #endif
    GraphicsBase::Base().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    GraphicsBase::Base().UseLatestApiVersion();
    GraphicsBase::Base().CreateInstance();

    //set surface
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (VkResult result = glfwCreateWindowSurface(GraphicsBase::Base().Instance(), pWindow, nullptr, &surface)){
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create a window surface!\nError code: {}\n", int32_t(result));
        glfwTerminate();
        return false;
    }
    GraphicsBase::Base().Surface(surface);
    
    if (GraphicsBase::Base().GetPhysicalDevices() ||
        GraphicsBase::Base().DeterminePhysicalDevice(0, true, false) ||
        GraphicsBase::Base().CreateDevice() ||
        GraphicsBase::Base().CreateSwapchain()){
        return false;
    }

    return true;
}

void TitleFps(){
    static double t0 = glfwGetTime();
    static double t1;
    static double dt;
    static int dframe = -1;
    static std::stringstream info;

    t1 = glfwGetTime();
    dframe++;
    if ((dt = t1 - t0) >= 1) {
        info.precision(1);
        info << windowTitle << "    " << std::fixed << dframe / dt << " FPS";
        glfwSetWindowTitle(pWindow, info.str().c_str());
        info.str("");//
        t0 = t1;
        dframe = 0;
    }
}

void TerminateWindow(){
    using namespace vulkan;
    
    GraphicsBase::Base().dispose();
    glfwTerminate();
}


