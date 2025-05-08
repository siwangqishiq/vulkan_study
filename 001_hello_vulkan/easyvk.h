#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <span>
#include <memory>
#include <functional>
#include <concepts>
#include <format>
#include <chrono>
#include <numeric>
#include <numbers>
#include <format>
#include <cstring>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "stb_image.h"

//Vulkan
#ifdef _WIN32                        //考虑平台是Windows的情况（请自行解决其他平台上的差异）
#define VK_USE_PLATFORM_WIN32_KHR    //在包含vulkan.h前定义该宏，会一并包含vulkan_win32.h和windows.h
// #define NOMINMAX                     //定义该宏可避免windows.h中的min和max两个宏与标准库中的函数名冲突
#endif
#include "vulkan/vulkan.h"

// #define NDEBUG

namespace vulkan{
    class GraphicsBase{
        static GraphicsBase singleton;
        GraphicsBase() = default;
        GraphicsBase(GraphicsBase&&) = delete;
        ~GraphicsBase();

        uint32_t apiVersion = VK_API_VERSION_1_0;
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceProperties physicalDeviceProperties;
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        std::vector<VkPhysicalDevice> availablePhysicalDevices;

        VkDevice device;

        uint32_t queueFamilyIndex_graphics = VK_QUEUE_FAMILY_IGNORED;
        uint32_t queueFamilyIndex_presentation = VK_QUEUE_FAMILY_IGNORED;
        uint32_t queueFamilyIndex_compute = VK_QUEUE_FAMILY_IGNORED;

        VkQueue queue_graphics;
        VkQueue queue_presentation;
        VkQueue queue_compute;

        VkSurfaceKHR surface;
        std::vector <VkSurfaceFormatKHR> availableSurfaceFormats;

        VkSwapchainKHR swapchain;
        std::vector <VkImage> swapchainImages;
        std::vector <VkImageView> swapchainImageViews;
        VkSwapchainCreateInfoKHR swapchainCreateInfo = {};

        std::vector<const char*> instanceLayers;
        std::vector<const char*> instanceExtensions;
        std::vector<const char*> deviceExtensions;

        VkDebugUtilsMessengerEXT debugMessenger;

        VkResult CreateSwapchain_Internal();

        VkResult GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, 
            bool enableGraphicsQueue, 
            bool enableComputeQueue, uint32_t (&queueFamilyIndices)[3]);
        
        VkResult CreateDebugMessenger();
    public:
        static GraphicsBase& Base();

        static uint32_t width;
        static uint32_t height;

        uint32_t ApiVersion() const {
            return apiVersion;
        }

        VkInstance Instance() const {
            return instance;
        }

        VkPhysicalDevice PhysicalDevice() const {
            return physicalDevice;
        }

        const VkPhysicalDeviceProperties& PhysicalDeviceProperties() const {
            return physicalDeviceProperties;
        }

        const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties() const {
            return physicalDeviceMemoryProperties;
        }

        VkPhysicalDevice AvailablePhysicalDevice(uint32_t index) const {
            return availablePhysicalDevices[index];
        }

        uint32_t AvailablePhysicalDeviceCount() const {
            return uint32_t(availablePhysicalDevices.size());
        }

        VkDevice Device() const {
            return device;
        }

        uint32_t QueueFamilyIndex_Graphics() const {
            return queueFamilyIndex_graphics;
        }

        uint32_t QueueFamilyIndex_Presentation() const {
            return queueFamilyIndex_presentation;
        }

        uint32_t QueueFamilyIndex_Compute() const {
            return queueFamilyIndex_compute;
        }

        VkQueue Queue_Graphics() const {
            return queue_graphics;
        }

        VkQueue Queue_Presentation() const {
            return queue_presentation;
        }

        VkQueue Queue_Compute() const {
            return queue_compute;
        }

        VkSurfaceKHR Surface() const {
            return surface;
        }
        
        const VkFormat& AvailableSurfaceFormat(uint32_t index) const {
            return availableSurfaceFormats[index].format;
        }
        const VkColorSpaceKHR& AvailableSurfaceColorSpace(uint32_t index) const {
            return availableSurfaceFormats[index].colorSpace;
        }
        uint32_t AvailableSurfaceFormatCount() const {
            return uint32_t(availableSurfaceFormats.size());
        }

        VkSwapchainKHR Swapchain() const {
            return swapchain;
        }
        VkImage SwapchainImage(uint32_t index) const {
            return swapchainImages[index];
        }
        VkImageView SwapchainImageView(uint32_t index) const {
            return swapchainImageViews[index];
        }
        uint32_t SwapchainImageCount() const {
            return uint32_t(swapchainImages.size());
        }
        const VkSwapchainCreateInfoKHR& SwapchainCreateInfo() const {
            return swapchainCreateInfo;
        }

        const std::vector<const char*>& InstanceLayers() const {
            return instanceLayers;
        }

        const std::vector<const char*>& InstanceExtensions() const {
            return instanceExtensions;
        }

        const std::vector<const char*>& DeviceExtensions() const {
            return deviceExtensions;
        }

        VkResult CheckInstanceLayers(std::span<const char*> layersToCheck) const;

        VkResult CheckInstanceExtensions(
            std::span<const char*> extensionsToCheck, 
            const char* layerName = nullptr) const;
        
        VkResult CheckDeviceExtensions(std::span<const char*> extensionsToCheck, const char* layerName = nullptr) const {
            return VK_SUCCESS;
        }

        //Non-const函数
        void AddInstanceLayer(const char* layerName) {
            instanceLayers.push_back(layerName);
        }
        void AddInstanceExtension(const char* extensionName) {
            instanceExtensions.push_back(extensionName);
        }
        void AddDeviceExtension(const char* extensionName) {
            deviceExtensions.push_back(extensionName);
        }

        VkResult UseLatestApiVersion();

        VkResult CreateInstance(VkInstanceCreateFlags flags = 0);

        void Surface(VkSurfaceKHR surface) {
            if (!this->surface){
                this->surface = surface;
            }
        }

        VkResult GetPhysicalDevices();

        VkResult DeterminePhysicalDevice(uint32_t deviceIndex = 0, 
            bool enableGraphicsQueue = true, 
            bool enableComputeQueue = true);

        VkResult CreateDevice(VkDeviceCreateFlags flags = 0);

        VkResult GetSurfaceFormats() {
            return VK_SUCCESS;
        }
        VkResult SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat) {
            return VK_SUCCESS;
        }

        VkResult CreateSwapchain(bool limitFrameRate = true, VkSwapchainCreateFlagsKHR flags = 0);

        void InstanceLayers(const std::vector<const char*>& layerNames) {
            instanceLayers = layerNames;
        }
        void InstanceExtensions(const std::vector<const char*>& extensionNames) {
            instanceExtensions = extensionNames;
        }
        void DeviceExtensions(const std::vector<const char*>& extensionNames) {
            deviceExtensions = extensionNames;
        }

        VkResult RecreateSwapchain() {
            return VK_SUCCESS;
        }

        void dispose();
    };

    inline GraphicsBase GraphicsBase::singleton;
}
