### Vulkan

下载 Vulkan SDK: https://vulkan.lunarg.com/sdk/home

Layer Vs Extension
层和扩展在Vulkan的核心功能之外，提供了对特定需求、特定平台的支持，还包括特定厂商提供的功能
层有显著的作用范围 
层只有实例级别的
而扩展则分实例级别和设备级别 前者与特定设备无关 可能作用于所有Vulkan相关的内容 而后者则作用于与特定设备相关的内容


## 创建步骤
### 实例

### 物理设备

### 逻辑设备

### 创建交换链
VkImage 引用一片物理设备内存 将该片内存上的数据用作图像 
VkImageView 指定图像的使用方式

```
vkCreateSwapChainKHR( 
    VkDevice device, 
    const VkSwapChainCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkSwapChainKHR *pSwapchain
);

struct VkSwapChainCreateInfoKHR{
    VkStructureType sType,
    const void* pNext,
    VkSwapchainCreateFlagsKHR flags,

    VkSurfaceKHR surface, //Window surface的handle

    uint32_t minImageCount , // 交换链中图像的最少数量

    VkFormat imageFormat, //交换链中图像的格式

    VkColorSpaceKHR imageColorSpace, //交换链中图像的色彩空间

    VkExtent2D imageExtent, //交换链图像的尺寸
    
    uint32_t imageArrayLayers, //对于多视点（multiview）或立体显示设备，需要提供一个视点数，对于普通的2D显示设备，该值为1
    
    VkSharingMode imageSharingMode, //交换链中图像的分享模式，应优先使用单个队列族独占访问VK_SHARING_MODE_EXCLUSIVE以达到最佳性能

    uint32_t queueFamilyIndexCount,//若imageSharingMode为VK_SHARING_MODE_CONCURRENT 这里需指定将会访问交换链图像的队列族总数

    const uint32_t* pQueueFamilyIndices,//续上一条，以及具体的队列族索引

    VkSurfaceTransformFlagBitsKHR preTransform ,// 对交换链图像的变换，比如旋转90°、镜像等

    VkCompositeAlphaFlagBitsKHR compositeAlpha, //指定如何处理交换链图像的透明度

    VkPresentModeKHR presentMode, // 呈现方式
    VkBool32 clipped,//是否允许舍弃掉交换链图像应有 但窗口中不会显示的像素

    VkSwapchainKHR oldSwapchain //旧的交换链，在重建交换链时填入

};
```

#### PresentMode
    VK_PRESENT_MODE_IMMEDIATE_KHR 
    立即模式 会产生画面撕裂
    
    VK_PRESENT_MODE_FIFO_KHR 
    交换链是一个队列，当显示器刷新时，显示器会从队列前端获取图像，而程序则会将渲染后的图像插入到队列后端 如果队列已满 程序就必须等待

    VK_PRESENT_MODE_FIFO_RELAXED_KHR 
    此模式与上一个模式的区别仅在于应用程序延迟且队列在上一个垂直空白时为空。图像不会等待下一个垂直空白，而是在到达时立即传输。这可能会导致明显的画面撕裂

    VK_PRESENT_MODE_MAILBOX_KHR 
    这是第二种模式的另一种变体。队列已满时，应用程序不会阻塞，而是直接用新图像替换已排队的图像。此模式可以尽可能快地渲染帧，同时避免画面撕裂，从而比标准垂直同步减少延迟问题。这通常被称为“三重缓冲”，尽管仅仅存在三个缓冲区并不一定意味着帧率已解锁。


#### SwapChainExtent 

### GraphicsPipleLine 图形渲染管线









