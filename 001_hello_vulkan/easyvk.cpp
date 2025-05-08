
#include "easyvk.h"

namespace vulkan{
    uint32_t GraphicsBase::width = 0;
    uint32_t GraphicsBase::height = 0;

    GraphicsBase& GraphicsBase::Base(){
        return singleton;
    }

    GraphicsBase::~GraphicsBase(){
        std::cout << "GraphicsBase::~GraphicsBase\n";
    }

    VkResult GraphicsBase::CreateSwapchain_Internal() {
        return VK_SUCCESS;
    }

    VkResult GraphicsBase::CreateInstance(VkInstanceCreateFlags flags) {
        #ifndef NDEBUG
        AddInstanceLayer("VK_LAYER_KHRONOS_validation");
        AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        #endif

        VkApplicationInfo applicationInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .apiVersion = apiVersion
        };

        std::cout << "instanceLayers count = " << instanceLayers.size() << std::endl;
        if(instanceLayers.size() > 0){
            std::cout << "instanceLayers " << instanceLayers[0] << std::endl;
        }

        std::cout << "instanceExtensions count = " << instanceExtensions.size() << std::endl;
        if(instanceExtensions.size() > 0){
            std::cout << "instanceExtensions : " << instanceExtensions[0] << std::endl;
        }
        
        VkInstanceCreateInfo instanceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .flags = flags,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = uint32_t(instanceLayers.size()),
            .ppEnabledLayerNames = instanceLayers.data(),
            .enabledExtensionCount = uint32_t(instanceExtensions.size()),
            .ppEnabledExtensionNames = instanceExtensions.data()
        };

        if(VkResult result = vkCreateInstance(&instanceCreateInfo , nullptr, &instance)){
            std::cout << "[ graphicsBase ] ERROR\nFailed to create a vulkan instance\n";
            return result;
        }

        std::cout << std::format(
            "Vulkan API Version: {}.{}.{}\n",
            VK_VERSION_MAJOR(apiVersion),
            VK_VERSION_MINOR(apiVersion),
            VK_VERSION_PATCH(apiVersion));

        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> layers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
        // std::cout << "instance layers :\n";
        for(auto &layer : layers){
            // std::cout << layer.layerName << "\t" << layer.description << "\n";
        }//end for each
        
        #ifndef NDEBUG
        CreateDebugMessenger();
        #endif
        
        return VK_SUCCESS;
    }

    VkResult GraphicsBase::UseLatestApiVersion()
    {
        if(vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion")){
            return vkEnumerateInstanceVersion(&apiVersion);
        }
        return VK_SUCCESS;
    }

    VkResult GraphicsBase::CreateDebugMessenger(){
        std::cout << "create CreateDebugMessenger.\n";
        static PFN_vkDebugUtilsMessengerCallbackEXT DebugUtilsMessengerCallback = [](
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        )->VkBool32 {
            std::cout << std::format("{}\n\n", pCallbackData->pMessage);
            return VK_FALSE;
        };

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = DebugUtilsMessengerCallback
        };
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (vkCreateDebugUtilsMessenger) {
            VkResult result = vkCreateDebugUtilsMessenger(instance, &debugUtilsMessengerCreateInfo, nullptr, &debugMessenger);
            if (result)
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to create a debug messenger!\nError code: {}\n", int32_t(result));
            return result;
        }
        std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the function pointer of vkCreateDebugUtilsMessengerEXT!\n");
        return VK_RESULT_MAX_ENUM;
    }

    VkResult GraphicsBase::CheckInstanceExtensions(
        std::span<const char*> extensionsToCheck, 
        const char* layerName) const {
           
        uint32_t extensionCount;
        std::vector<VkExtensionProperties> availableExtensions;
        if (VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr)){
            if(layerName == nullptr){
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of instance extensions!\n");
            }else{
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of instance extensions!\nLayer name:{}\n", layerName);
            }
            return result;
        }

        if (extensionCount > 0){
            availableExtensions.resize(extensionCount);
            VkResult result = vkEnumerateInstanceExtensionProperties(layerName, 
                &extensionCount, availableExtensions.data());
            if(result != VK_SUCCESS){
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate instance extension properties!\nError code: {}\n", int32_t(result));
                return result;
            }

            for (auto& i : extensionsToCheck) {
                bool found = false;
                for (auto& j : availableExtensions){
                    if (!strcmp(i, j.extensionName)) {
                        found = true;
                        break;
                    }
                }//end for each
                if (!found){
                    i = nullptr;
                }
            }
        }else{
            for (auto& i : extensionsToCheck){
                i = nullptr;
            }
        }//end if

        return VK_SUCCESS;
    }

    VkResult GraphicsBase::CheckInstanceLayers(std::span<const char*> layersToCheck) const{
        uint32_t layerCount;
        std::vector<VkLayerProperties> availableLayers;
        if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr)) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of instance layers!\n");
            return result;
        }

        if (layerCount > 0){
            availableLayers.resize(layerCount);
            if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data())) {
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate instance layer properties!\nError code: {}\n", int32_t(result));
                return result;
            }
            for (auto& i : layersToCheck) {
                bool found = false;
                for (auto& j : availableLayers){
                    if (!strcmp(i, j.layerName)) {
                        found = true;
                        break;
                    }
                }
                if (!found){
                    i = nullptr;
                }
            }
        }else{
            for(auto& i : layersToCheck){
                i = nullptr;
            }//end for each
        }
        return VK_SUCCESS;
    }

    VkResult GraphicsBase::GetPhysicalDevices() {
        uint32_t deviceCount;
        if (VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of physical devices!\nError code: {}\n", int32_t(result));
            return result;
        }

        if (deviceCount == 0){
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to find any physical device supports vulkan!\n");
            return VK_ERROR_DEVICE_LOST;
        }

        availablePhysicalDevices.resize(deviceCount);
        VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, availablePhysicalDevices.data());
        if(result){
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate physical devices!\nError code: {}\n", int32_t(result));
        }
        
        std::cout << std::format("Physical devices list: {}\n", deviceCount);
        for(VkPhysicalDevice &phyDevice : availablePhysicalDevices){
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(phyDevice, &deviceProperties);
            std::cout << deviceProperties.deviceName << std::endl;
        }//end for each

        // physicalDevice = availablePhysicalDevices[0];//选择第一个
        return VK_SUCCESS;
    }

    VkResult GraphicsBase::GetQueueFamilyIndices(
            VkPhysicalDevice physicalDevice, 
            bool enableGraphicsQueue, 
            bool enableComputeQueue, 
            uint32_t (&queueFamilyIndices)[3]){
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        if (queueFamilyCount == 0){
            return VK_RESULT_MAX_ENUM;
        }

        std::vector<VkQueueFamilyProperties> queueFamilyPropertieses(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyPropertieses.data());

        auto& [ig, ip, ic] = queueFamilyIndices;
        ig = VK_QUEUE_FAMILY_IGNORED;
        ip = VK_QUEUE_FAMILY_IGNORED;
        ic = VK_QUEUE_FAMILY_IGNORED;
        

        // VK_NULL_HANDLE;
        // vkCreateComputePipelines()
        for (uint32_t i = 0; i < queueFamilyCount; i++){
            VkBool32 supportGraphics = enableGraphicsQueue && queueFamilyPropertieses[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
            VkBool32 supportPresentation = false;
            VkBool32 supportCompute = enableComputeQueue && queueFamilyPropertieses[i].queueFlags & VK_QUEUE_COMPUTE_BIT;

            if (surface){
                if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportPresentation)) {
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to determine if the queue family supports presentation!\nError code: {}\n", int32_t(result));
                    return result;
                }
            }

            if (supportGraphics && supportCompute) {
                //若需要呈现，最好是三个队列族索引全部相同
                if (supportPresentation) {
                    ig = ip = ic = i;
                    break;
                }
                //除非ig和ic都已取得且相同，否则将它们的值覆写为i，以确保两个队列族索引相同
                if (ig != ic ||ig == VK_QUEUE_FAMILY_IGNORED){
                    ig = ic = i;
                }
                //如果不需要呈现，那么已经可以break了
                if (!surface)
                    break;
            }

            //若任何一个队列族索引可以被取得但尚未被取得，将其值覆写为i
            if (supportGraphics && ig == VK_QUEUE_FAMILY_IGNORED){
                ig = i;
            }
            
            if (supportPresentation && ip == VK_QUEUE_FAMILY_IGNORED){
                ip = i;
            }
                
            if (supportCompute && ic == VK_QUEUE_FAMILY_IGNORED){
                ic = i;
            }
        }//end for i

        if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue 
            || ip == VK_QUEUE_FAMILY_IGNORED && surface ||ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue){
            return VK_RESULT_MAX_ENUM;
        }

        queueFamilyIndex_graphics = ig;
        queueFamilyIndex_presentation = ip;
        queueFamilyIndex_compute = ic;

        return VK_SUCCESS;
    }

    VkResult GraphicsBase::DeterminePhysicalDevice(uint32_t deviceIndex, 
            bool enableGraphicsQueue, 
            bool enableComputeQueue) {
        static constexpr uint32_t notFound = INT32_MAX;
        struct queueFamilyIndexCombination {
            uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
            uint32_t presentation = VK_QUEUE_FAMILY_IGNORED;
            uint32_t compute = VK_QUEUE_FAMILY_IGNORED;
        };

        static std::vector<queueFamilyIndexCombination> queueFamilyIndexCombinations(availablePhysicalDevices.size());
        auto& [ig, ip, ic] = queueFamilyIndexCombinations[deviceIndex];
        if (ig == notFound && enableGraphicsQueue ||
            ip == notFound && surface ||
            ic == notFound && enableComputeQueue){
            return VK_RESULT_MAX_ENUM;
        }
        

        if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue ||
                ip == VK_QUEUE_FAMILY_IGNORED && surface ||
                ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue){
            uint32_t indices[3];
            VkResult result = GetQueueFamilyIndices(availablePhysicalDevices[deviceIndex], enableGraphicsQueue, enableComputeQueue, indices);
            if (result == VK_SUCCESS ||result == VK_RESULT_MAX_ENUM){
                if (enableGraphicsQueue){
                    ig = indices[0] & INT32_MAX;
                }
                    
                if (surface){
                    ip = indices[1] & INT32_MAX;
                }
                    
                if (enableComputeQueue){
                    ic = indices[2] & INT32_MAX;
                }
            } 

            if (result){
                return result;
            }
        }else{
            queueFamilyIndex_graphics = enableGraphicsQueue ? ig : VK_QUEUE_FAMILY_IGNORED;
            queueFamilyIndex_presentation = surface ? ip : VK_QUEUE_FAMILY_IGNORED;
            queueFamilyIndex_compute = enableComputeQueue ? ic : VK_QUEUE_FAMILY_IGNORED;
        }

        physicalDevice = availablePhysicalDevices[deviceIndex];
        return VK_SUCCESS;
    }

    VkResult GraphicsBase::CreateDevice(VkDeviceCreateFlags flags) {
        const float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfos[3] = {
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority 
            },

            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority 
            },

            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority 
            } 
        };

        uint32_t queueCreateInfoCount = 0;
        if(queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED){
            queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_graphics;
        }

        if(queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED 
            &&queueFamilyIndex_presentation != queueFamilyIndex_graphics){
            queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_presentation;
        }

        if(queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED &&
            queueFamilyIndex_compute != queueFamilyIndex_graphics &&
            queueFamilyIndex_compute != queueFamilyIndex_presentation){
            queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_compute;
        }

        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
        VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .flags = flags,
            .queueCreateInfoCount = queueCreateInfoCount,
            .pQueueCreateInfos = queueCreateInfos,
            .enabledExtensionCount = uint32_t(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = &physicalDeviceFeatures
        };

        if (VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device)) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to create a vulkan logical device!\nError code: {}\n", int32_t(result));
            return result;
        }

        if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED){
            vkGetDeviceQueue(device, queueFamilyIndex_graphics, 0, &queue_graphics);
        }
            
        if (queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED){
            vkGetDeviceQueue(device, queueFamilyIndex_presentation, 0, &queue_presentation);
        }
            
        if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED){
            vkGetDeviceQueue(device, queueFamilyIndex_compute, 0, &queue_compute);
        }

        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

        //输出所用的物理设备名称
        std::cout << std::format("Renderer: {}\n", physicalDeviceProperties.deviceName);
        std::cout << std::format("queue family index : {},{},{} \n", queueFamilyIndex_graphics,
            queueFamilyIndex_presentation, queueFamilyIndex_compute);
        return VK_SUCCESS;
    }

    VkResult GraphicsBase::CreateSwapchain(
        bool limitFrameRate, VkSwapchainCreateFlagsKHR flags){
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities)) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get physical device surface capabilities!\nError code: {}\n", int32_t(result));
            return result;
        }

        std::cout << "minImageCount : " << surfaceCapabilities.minImageCount << std::endl;
        std::cout << "maxImageCount : " << surfaceCapabilities.maxImageCount << std::endl;
        std::cout << std::format("imagesize :  {},{}\n", 
            surfaceCapabilities.currentExtent.width , surfaceCapabilities.currentExtent.height);

        
        return VK_SUCCESS;
    }


    void GraphicsBase::dispose(){
        vkDestroySurfaceKHR(instance, surface , nullptr);

        // vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (vkDestroyDebugUtilsMessengerEXT) {
            vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroyDevice(device , nullptr);
        vkDestroyInstance(instance , nullptr);
    }
}

