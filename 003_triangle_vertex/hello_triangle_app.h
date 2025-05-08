#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "vulkan/vulkan.h"

#include <vector>
#include <array>
#include <string>
#include <set>
#include "glm/glm.hpp"

// #define NDEBUG 1

#if defined(__APPLE__) && (defined(__aarch64__) || defined(__arm64__))
    #define IS_MACOS_ARM
#endif

#ifdef IS_MACOS_ARM
#include "vulkan/vulkan_metal.h"
#endif

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

std::vector<char> ReadFile(const std::string &path);

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT messageType, 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger);

struct QueueFamilyIndices{
    int graphFamily = -1;
    int presentFamily = -1;

    bool isComplete(){
        return graphFamily >= 0 && presentFamily >= 0;
    }
};

struct SwapChainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes; 
};

static inline uint32_t Clamp(uint32_t value , uint32_t min , uint32_t max){
    if(value < min){
        return min;
    }else if(value > max){
        return max;
    }else{
        return value;
    }
}

struct Vertex{
    glm::vec2 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription bindingDesc();
    static std::array<VkVertexInputAttributeDescription , 2> attributeDesc();
};

class HelloTriangleApp{
public:
    const static uint64_t MAX_UINT_VALUE = 0xffffffffffffffff;

    const static int WIDTH = 800;
    const static int HEIGHT = 600;
    const static int MAX_FRAMES_IN_FLIGHT = 2; //可同时绘制的帧数量

    void run();

    void initWindow();
    void initVulkan();

    void mainloop();

    void drawFrame();
    void dispose();
private:
    std::vector<Vertex> vertexData = {
        {{0.0f, -0.5f},{1.0f, 1.0f , 1.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f , 0.0f}},
        {{-0.5f, 0.5f},{0.0f, 0.0f , 1.0f}},
    };

    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue graphQueue;
    VkQueue presentQueue;

    uint32_t graphQueueFamilyIndex;
    uint32_t presentQueueFamilyIndex;

    VkSwapchainKHR swapchain;
    VkExtent2D swapchainExtent;
    VkFormat swapchainFormat;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphPipeline;
    std::vector<VkFramebuffer> swapchainFrameBuffers;

    VkCommandPool cmdPool;
    std::vector<VkCommandBuffer> cmdBuffers;

    //同步对象
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishSemaphores;
    std::vector<VkFence> inFlightFences;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;

    std::vector<const char*> validationLayer = {
        "VK_LAYER_KHRONOS_validation"
    };

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    //当前操作的帧索引
    uint32_t currentFrameIndex = 0;

    void createInstance();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();

    void setupDebugMessenger();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    void createSurface();
    void pickPhysicalDevice();
    void createLogicDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createGraphPipline();
    void createFrameBuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createVertexBuffer();

    void cleanupSwapChain();

    bool isDeviceSuitable(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice phyDevice);
    bool checkDeviceExtensionSupport(VkPhysicalDevice phyDevice);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice phyDevice);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> &presentModes);
    VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR &surfaceCapabilities);

    VkShaderModule createShaderModule(std::vector<char> &code);

    void recreateSwapChain();

    void recordRenderCmds(uint32_t imageIndex, VkCommandBuffer cmdBuffer);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};





