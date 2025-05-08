#include <iostream>
#include "hello_triangle_app.h"
#include <fstream>

std::vector<char> ReadFile(const std::string &path){
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger){
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger){
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, messenger, nullptr);
    }
}


void HelloTriangleApp::run(){
    initWindow();
    initVulkan();
    mainloop();
    dispose();
}


void HelloTriangleApp::initWindow(){
    std::cout << "initWindow" << std::endl;
    
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API , GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Hello Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *win,int w,int h){
    });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
        if(key == GLFW_KEY_ESCAPE){
            glfwSetWindowShouldClose(window, true);
        }   
    });
}

void HelloTriangleApp::initVulkan(){
#ifdef _WIN32
    _putenv("VK_LOADER_DEBUG=none");
#endif

    //
    std::cout << "initVulkan" << std::endl;
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphPipline();
    createFrameBuffers();
    createCommandPool();

    createVertexBuffer();
    createCommandBuffers();
    createSyncObjects();
}

void HelloTriangleApp::mainloop(){
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        drawFrame();
    }//end while
    vkDeviceWaitIdle(device);
}

void HelloTriangleApp::createInstance(){
    std::cout << "createInstance" << std::endl;
    if(enableValidationLayers && !checkValidationLayerSupport()){
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_4;
    appInfo.pApplicationName = "Hello Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "Purple";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = 0;
#ifdef IS_MACOS_ARM
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();

#ifdef IS_MACOS_ARM
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    for(auto &e : extensions){
        std::cout << "extension: " << e << std::endl;
    }
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if(enableValidationLayers){
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.enabledLayerCount = validationLayer.size();
        createInfo.ppEnabledLayerNames = validationLayer.data();

        createInfo.pNext = &debugCreateInfo;
    }else{
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
        throw std::runtime_error("failed to create instance!");
    }
}

bool HelloTriangleApp::checkValidationLayerSupport(){
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if(layerCount == 0){
        return false;
    }
    
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    for(auto &requestLayerName : validationLayer){
        for(VkLayerProperties &prop : layers){
            if(std::string(prop.layerName) == std::string(requestLayerName)){
                return true;
            }
        }//end for each
    }//end for each
    std::cerr << "Not found available layers!" << std::endl;
    return false;
}

std::vector<const char*> HelloTriangleApp::getRequiredExtensions(){
    uint32_t glfwExtensionsCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
    
    std::vector<const char*> extensions(glfwExtensions , glfwExtensions + glfwExtensionsCount);
    if(enableValidationLayers){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

void HelloTriangleApp::setupDebugMessenger(){
    if(!enableValidationLayers){
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) 
        != VK_SUCCESS){
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void HelloTriangleApp::
    populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo){
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    
    createInfo.pfnUserCallback = debugCallback;
}

void HelloTriangleApp::createSurface(){
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS){
        throw std::runtime_error("failed to create surface");
    }
    std::cout << "Create surface success" << std::endl;
}

void HelloTriangleApp::pickPhysicalDevice(){
    std::cout << "pick PhysicalDevice" << std::endl;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if(deviceCount == 0){
        throw std::runtime_error("not found physical device");
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    for(const auto &device : physicalDevices){
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }//end for each

    if(physicalDevice == VK_NULL_HANDLE){
        physicalDevice = physicalDevices[0];
    }
    std::cout << "pick suitable physical device" << std::endl;
    VkPhysicalDeviceProperties prop;
    vkGetPhysicalDeviceProperties(physicalDevice, &prop);
    std::cout << "physical device name:" << prop.deviceName 
        << "\tDriverVersion :" << prop.driverVersion << std::endl;
}

bool HelloTriangleApp::isDeviceSuitable(VkPhysicalDevice phyDevice){
    QueueFamilyIndices indices = findQueueFamilies(phyDevice);
    // std::cout << "indices graph index = " << indices.graphFamily << std::endl;
    // std::cout << "indices present index = " << indices.presentFamily << std::endl;
    bool isExtensionSupported = checkDeviceExtensionSupport(phyDevice);
    bool swapChainSupport = false;

    if(isExtensionSupported){
        auto details = querySwapChainSupport(phyDevice);
        swapChainSupport = !details.formats.empty() && !details.presentModes.empty();
    }
    return indices.isComplete() && isExtensionSupported && swapChainSupport;
}

QueueFamilyIndices HelloTriangleApp::findQueueFamilies(VkPhysicalDevice phyDevice){
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &queueFamilyCount , nullptr);
    if(queueFamilyCount == 0){
        return indices;
    }

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &queueFamilyCount , queueFamilies.data());

    for(int i = 0 ; i < queueFamilies.size() ;i++){
        VkQueueFamilyProperties &prop = queueFamilies[i];
        if(prop.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphFamily = i;
        }

        VkBool32 isSupportPresent = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(phyDevice,
            i, surface, &isSupportPresent);
        if(isSupportPresent == VK_TRUE){
            indices.presentFamily = i;
        }

        if(indices.isComplete()){
            break;
        }
    }//end for i
    return indices;
}

bool HelloTriangleApp::checkDeviceExtensionSupport(VkPhysicalDevice phyDevice){
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &extensionCount, 
        availableExtensions.data());
    
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }//end for each
    return requiredExtensions.empty();
}

SwapChainSupportDetails HelloTriangleApp::querySwapChainSupport(VkPhysicalDevice phyDevice){
    SwapChainSupportDetails detail;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDevice,surface, &detail.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, surface , &formatCount, nullptr);
    if(formatCount > 0){
        detail.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, surface , &formatCount, detail.formats.data());
    }

    uint32_t presentCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, surface, &presentCount, nullptr);
    if(presentCount > 0){
        detail.presentModes.resize(presentCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, surface, &presentCount, detail.presentModes.data());
    }
    return detail;
}

void HelloTriangleApp::createLogicDevice(){
    std::cout << "create LogicDevice" << std::endl;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        static_cast<uint32_t>(indices.graphFamily),
        static_cast<uint32_t>(indices.presentFamily)
    };
    float queuePriority = 1.0f;
    for(uint32_t queueFamily : uniqueQueueFamilies){
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.queueFamilyIndex = queueFamily;

        queueCreateInfos.push_back(queueCreateInfo);
    }//end for each

    VkPhysicalDeviceFeatures phyFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pEnabledFeatures = &phyFeatures;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS){
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, static_cast<uint32_t>(indices.graphFamily), 
        0 , &graphQueue);
    vkGetDeviceQueue(device, static_cast<uint32_t>(indices.presentFamily), 
        0 , &presentQueue);

    graphQueueFamilyIndex = indices.graphFamily;
    presentQueueFamilyIndex = indices.presentFamily;
    
    std::cout << "create logic device success create graph queue and present queue!" << std::endl;
}

void HelloTriangleApp::createSwapChain(){
    std::cout << "createSwapChain" << std::endl;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    std::cout << "surfaceFormat : " 
        << surfaceFormat.colorSpace << ", " 
        << surfaceFormat.format
        << std::endl;
    std::cout << "presentMode:" << presentMode << std::endl;
    std::cout  << "extent: " << extent.width << "," << extent.height << std::endl;

    std::cout  << "minImageCount : " << 
        swapChainSupport.capabilities.minImageCount << 
        " maxImageCount : " << swapChainSupport.capabilities.maxImageCount << std::endl;

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if(swapChainSupport.capabilities.maxImageCount > 0 
        && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    std::cout  << "image count: " << imageCount << std::endl;

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.flags = 0;
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = extent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = VK_TRUE;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamiliesIndex[] = {
        static_cast<uint32_t>(indices.graphFamily),
        static_cast<uint32_t>(indices.presentFamily)
    };

    if(queueFamiliesIndex[0] != queueFamiliesIndex[1]){
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamiliesIndex;
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    }else{
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if(vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapchain) != VK_SUCCESS){
        throw std::runtime_error("create swap chain failed!");
    }

    swapchainFormat = surfaceFormat.format;
    swapchainExtent = extent;
    
    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
    std::cout << "swapchainImageCount = " << swapchainImageCount << std::endl;
    swapchainImages.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());
}

void HelloTriangleApp::createImageViews(){
    std::cout << "create image views swapchainImagessize = " << swapchainImages.size() << std::endl;
    swapchainImageViews.resize(swapchainImages.size());
    for(int i = 0; i < swapchainImages.size() ;i++){
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapchainImages[i];
        imageViewCreateInfo.format = swapchainFormat;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;

        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.levelCount = 1;

        if(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i])!= VK_SUCCESS){
            throw std::runtime_error("create image view failed");
        }
    }//end for i
}

void HelloTriangleApp::createRenderPass(){
    std::cout << "create render pass" << std::endl;
    VkAttachmentDescription colorAttach{};
    colorAttach.format = swapchainFormat;
    colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachRef{};
    colorAttachRef.attachment = 0;
    colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachRef;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkAttachmentDescription attachs[] = {colorAttach};
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = attachs;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;

    if(vkCreateRenderPass(device, &createInfo, nullptr, &renderPass) != VK_SUCCESS){
        throw std::runtime_error("create render pass failed!");
    }
}

void HelloTriangleApp::createGraphPipline(){
    std::cout << "Create Graph Pipeline" << std::endl;

    auto vertShaderSpvCode = ReadFile("build/vert.spv");
    auto fragShaderSpvCode = ReadFile("build/frag.spv");

    std::cout << "vertShaderSpvCode size :" << vertShaderSpvCode.size() << std::endl;
    std::cout << "fragShaderSpvCode size :" << fragShaderSpvCode.size() << std::endl;

    VkShaderModule vertexShaderModule = createShaderModule(vertShaderSpvCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderSpvCode);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertexShaderModule;
    vertShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo , fragShaderStageCreateInfo};

    //顶点输入描述符
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    auto vertexInputBind = Vertex::bindingDesc();
    auto vertexInputDescs = Vertex::attributeDesc();
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBind;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputDescs.size();
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputDescs.data();

    //图元装配形式设定
    VkPipelineInputAssemblyStateCreateInfo inputAssembleCreateInfo{};
    inputAssembleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembleCreateInfo.primitiveRestartEnable = VK_FALSE;
    inputAssembleCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    //视口设置
    VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
    viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    VkViewport viewport{};
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = swapchainExtent.width;
    viewport.height = swapchainExtent.height;
    viewportCreateInfo.viewportCount = 1;
    viewportCreateInfo.pViewports = &viewport;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;
    viewportCreateInfo.scissorCount = 1;
    viewportCreateInfo.pScissors = &scissor;

    //光栅化设置
    VkPipelineRasterizationStateCreateInfo rasterCreateInfo{};
    rasterCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterCreateInfo.depthClampEnable = VK_FALSE;
    rasterCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterCreateInfo.lineWidth = 1.0f;
    rasterCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterCreateInfo.depthBiasEnable = VK_FALSE;

    //多重采样设置
    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    //混合模式设置blend
    VkPipelineColorBlendAttachmentState colorBlendAttach{};
    colorBlendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT 
        | VK_COLOR_COMPONENT_G_BIT 
        | VK_COLOR_COMPONENT_B_BIT 
        | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttach.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{};
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttach;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendCreateInfo.blendConstants[0] = 0.0f;
    colorBlendCreateInfo.blendConstants[1] = 0.0f;
    colorBlendCreateInfo.blendConstants[2] = 0.0f;
    colorBlendCreateInfo.blendConstants[3] = 0.0f;

    //动态状态 不在管线创建时指定 而是在CommandBuf中动态指定
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    // dynamicStateCreateInfo.dynamicStateCount = 0;

    //创建layout
    VkPipelineLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = 0;
    layoutCreateInfo.pushConstantRangeCount = 0;
    if(vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("create pipeline layout failed!");
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo; 
    pipelineCreateInfo.pInputAssemblyState = &inputAssembleCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, 
        &pipelineCreateInfo, nullptr, &graphPipeline) != VK_SUCCESS){
        throw std::runtime_error("create graph pipeline failed!");
    }

    vkDestroyShaderModule(device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);

    std::cout << "Create pipeline success." << std::endl;
}

void HelloTriangleApp::createFrameBuffers(){
    swapchainFrameBuffers.resize(swapchainImageViews.size());
    for(int i = 0 ; i < swapchainImageViews.size() ;i++){
        VkFramebufferCreateInfo createInfo{};

        VkImageView attachments[] = {
            swapchainImageViews[i]
        };

        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = renderPass;
        createInfo.layers = 1;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = attachments;
        createInfo.width = swapchainExtent.width;
        createInfo.height = swapchainExtent.height;

        if(vkCreateFramebuffer(device, &createInfo, nullptr, &swapchainFrameBuffers[i])
            != VK_SUCCESS){
            throw std::runtime_error("create framebuffer failed!");
        }
    }//end for i
    std::cout << "Create framebuffers success." << std::endl;
}

void HelloTriangleApp::createCommandPool(){
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    VkCommandPoolCreateInfo cmdPoolCreateInfo{};
    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolCreateInfo.queueFamilyIndex = static_cast<uint32_t>(indices.graphFamily);

    if(vkCreateCommandPool(device, &cmdPoolCreateInfo , nullptr, &cmdPool) != VK_SUCCESS){
        throw std::runtime_error("create command pool failed!");
    }

    std::cout << "Create Command Pool success." << std::endl;
}

void HelloTriangleApp::createCommandBuffers(){
    cmdBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
    allocateInfo.commandPool = cmdPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if(vkAllocateCommandBuffers(device, &allocateInfo, cmdBuffers.data()) != VK_SUCCESS){
        throw std::runtime_error("allocate command buffer failed!");
    }

    std::cout << "Create Command buffers success."<< std::endl;
}

void HelloTriangleApp::createVertexBuffer(){
    std::cout << "Create VertexBuffer" << std::endl;
    VkBufferCreateInfo bufCreateInfo{};
    bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufCreateInfo.size = sizeof(Vertex) * vertexData.size();
    bufCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device, &bufCreateInfo, nullptr , &vertexBuffer) != VK_SUCCESS){
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

    std::cout << "MemoryRequirements size: " 
        << memRequirements.size 
        << " realSize:" << bufCreateInfo.size << std::endl;
    std::cout << "MemoryRequirements alignment: " << memRequirements.alignment << std::endl;
    std::cout << "MemoryRequirements memoryTypeBits: " << memRequirements.memoryTypeBits << std::endl;

    VkMemoryAllocateInfo memAllocateInfo{};
    memAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocateInfo.allocationSize = memRequirements.size;
    memAllocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    
    if(vkAllocateMemory(device, &memAllocateInfo , nullptr, &vertexMemory) != VK_SUCCESS){
        throw std::runtime_error("Failed allocate vertex memory!");
    }


    if(vkBindBufferMemory(device, vertexBuffer, vertexMemory, 0) != VK_SUCCESS){
        throw std::runtime_error("Bind vertex buffer memory failed!");
    }

    //fill vertex data
    void *data;
    vkMapMemory(device, vertexMemory, 0, memRequirements.size, 0, &data);
        memcpy(data, vertexData.data(), (size_t)memRequirements.size);
    vkUnmapMemory(device, vertexMemory);
}

//找到合适的内存类型
uint32_t HelloTriangleApp::findMemoryType(uint32_t typeFilter, 
    VkMemoryPropertyFlags properties){
    
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    for(uint32_t i = 0; i < memProps.memoryTypeCount; i++){
        if((typeFilter & (1 << i)) 
            && ((memProps.memoryTypes[i].propertyFlags & properties) == properties)){
            return i;
        }
    }//end for i

    throw std::runtime_error("not found suitable memory type!");
}

void HelloTriangleApp::createSyncObjects(){
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        if(vkCreateSemaphore(device, &semaphoreCreateInfo , nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS){
            throw std::runtime_error("create semaphore failed!");
        }

        if(vkCreateSemaphore(device, &semaphoreCreateInfo , nullptr, &renderFinishSemaphores[i]) != VK_SUCCESS){
            throw std::runtime_error("create semaphore failed!");
        }

        if(vkCreateFence(device, &fenceCreateInfo , nullptr, &inFlightFences[i]) != VK_SUCCESS){
            throw std::runtime_error("create fence failed!");
        }
    }//end for i
    std::cout << "create sync objects success." << std::endl;
}

void HelloTriangleApp::cleanupSwapChain(){
    for(auto &frameBuf : swapchainFrameBuffers){
        vkDestroyFramebuffer(device, frameBuf, nullptr);
    }//end for each

    for(auto &imageView : swapchainImageViews){
        vkDestroyImageView(device, imageView, nullptr);
    }//end for each

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

VkShaderModule HelloTriangleApp::createShaderModule(std::vector<char> &code){
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<uint32_t*>(code.data());

    VkShaderModule module;
    if(vkCreateShaderModule(device, &createInfo , nullptr, &module) != VK_SUCCESS){
        throw std::runtime_error("create shader module failed!");
    }
    return module;
}

void HelloTriangleApp::recreateSwapChain(){
    std::cout << "recreate swap chain." << std::endl;
    int width = 0;
    int height = 0;

    do{
        glfwGetFramebufferSize(window, &width, &height);
        std::cout << "recreate framebuffsize " << width << " , " << height << std::endl;
        glfwWaitEvents();
    }while(width ==0 || height == 0);

    vkDeviceWaitIdle(device);
    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFrameBuffers();
}

VkSurfaceFormatKHR HelloTriangleApp::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> &availableFormats){
    for (VkSurfaceFormatKHR& availableFormat : availableFormats){
        if(availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return availableFormat;
        }
    }//end for each
    return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApp::chooseSwapPresentMode(std::vector<VkPresentModeKHR> &presentModes){
    for(VkPresentModeKHR &presentMode : presentModes){
        if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR){
            return presentMode;
        }
    }//end for each
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApp::chooseSwapExtent(VkSurfaceCapabilitiesKHR &surfaceCapabilities){
    if(surfaceCapabilities.currentExtent.width != 0xffffffff){
        return surfaceCapabilities.currentExtent;
    }else{
        int width,height;
        glfwGetFramebufferSize(window, &width, &height);
        std::cout << "get frame buffer size : " << width << "," << height << std::endl;
        VkExtent2D ret = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        ret.width = Clamp(ret.width , surfaceCapabilities.minImageExtent.width, 
            surfaceCapabilities.maxImageExtent.width);       
        ret.height = Clamp(ret.height , surfaceCapabilities.minImageExtent.height, 
            surfaceCapabilities.maxImageExtent.height);
        return ret;
    }
}

void HelloTriangleApp::drawFrame(){
    // std::cout << "draw frame" << std::endl;
    auto pCurrentFence = &inFlightFences[currentFrameIndex];
    vkWaitForFences(device, 1, pCurrentFence , VK_TRUE, MAX_UINT_VALUE);

    uint32_t imageIndex = 0;
    VkResult requestImageResult = vkAcquireNextImageKHR(device, swapchain, MAX_UINT_VALUE, 
        imageAvailableSemaphores[currentFrameIndex], VK_NULL_HANDLE, &imageIndex);

    if(requestImageResult == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
        return;
    }else if( requestImageResult != VK_SUCCESS && requestImageResult != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error("failed to acquire image!");
    }
    vkResetFences(device, 1, pCurrentFence);

    // std::cout << "imageIndex = " << imageIndex << std::endl;
    vkResetCommandBuffer(cmdBuffers[currentFrameIndex], 0);
    recordRenderCmds(imageIndex , cmdBuffers[currentFrameIndex]);
    //submit render commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemas[] = {imageAvailableSemaphores[currentFrameIndex]};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemas;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffers[currentFrameIndex];
    submitInfo.signalSemaphoreCount = 1;
    VkSemaphore finishSema[] = {renderFinishSemaphores[currentFrameIndex]};
    submitInfo.pSignalSemaphores = finishSema;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = waitStages;

    if(vkQueueSubmit(graphQueue, 1, &submitInfo, inFlightFences[currentFrameIndex]) != VK_SUCCESS){
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    //present
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    uint32_t imgIndices[] = {imageIndex};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = finishSema;
    VkSwapchainKHR swapChains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    auto presentResult = vkQueuePresentKHR(presentQueue , &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
    } else if(presentResult != VK_SUCCESS){
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangleApp::recordRenderCmds(uint32_t imageIndex , VkCommandBuffer cmdBuffer){
    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) != VK_SUCCESS){
        throw std::runtime_error("command buffer begin failed!");
    }

    VkRenderPassBeginInfo passBeginInfo{};
    passBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    passBeginInfo.renderPass = renderPass;
    // std::cout << "imageIndex = " << imageIndex << "  swapchainFrameBuffers size " << swapchainFrameBuffers.size() << std::endl;
    passBeginInfo.framebuffer = swapchainFrameBuffers[imageIndex];
    passBeginInfo.renderArea.offset = {0 ,0};
    passBeginInfo.renderArea.extent = swapchainExtent;
    VkClearValue clearValues;
    clearValues.color = {0.0f, 0.0f, 0.0f, 1.0f};
    passBeginInfo.pClearValues = &clearValues;
    passBeginInfo.clearValueCount = 1;
    vkCmdBeginRenderPass(cmdBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        //begin render
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphPipeline);

        //set viewport
        VkViewport viewport{};
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchainExtent.width);
        viewport.height = static_cast<float>(swapchainExtent.height);
        vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

        //set scissor
        VkRect2D scissorRect{};
        scissorRect.offset = {0 , 0};
        scissorRect.extent = swapchainExtent;
        vkCmdSetScissor(cmdBuffer , 0, 1, &scissorRect);

        VkBuffer buffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);

        vkCmdDraw(cmdBuffer, vertexData.size(), 1, 0, 0);
    vkCmdEndRenderPass(cmdBuffer);

    if(vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS){
        throw std::runtime_error("command buffer end failed!");
    }
}

void HelloTriangleApp::dispose(){
    std::cout << "~dispose" << std::endl;

    for(auto &f : inFlightFences){
        vkDestroyFence(device, f, nullptr);
    }//end for each

    for(auto &sema : imageAvailableSemaphores){
        vkDestroySemaphore(device, sema, nullptr);
    }//end for each

    for(auto &sema : renderFinishSemaphores){
        vkDestroySemaphore(device, sema, nullptr);
    }//end for each

    vkFreeMemory(device, vertexMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer , nullptr);

    vkDestroyCommandPool(device, cmdPool, nullptr);

    cleanupSwapChain();

    vkDestroyPipeline(device, graphPipeline , nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout , nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

VkVertexInputBindingDescription Vertex::bindingDesc(){
    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindingDesc.stride = sizeof(Vertex);
    return bindingDesc;
}

std::array<VkVertexInputAttributeDescription , 2> Vertex::attributeDesc(){
    std::array<VkVertexInputAttributeDescription , 2> descs;

    //position
    descs[0].binding = 0;
    descs[0].offset = 0;
    descs[0].format = VK_FORMAT_R32G32_SFLOAT;
    descs[0].location = 0;

    //color
    descs[1].binding = 0;
    descs[1].offset = offsetof(Vertex, color);
    descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descs[1].location = 1;
    
    return descs;
}
