#include "vulkan/vulkan.hpp"
#include <cassert>
#include <vulkan/vulkan_core.h>
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../utils/utils.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

#define APP_NAME "Hello Triangle"
constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

class HelloTriangleApp {
public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device device = nullptr;
  uint32_t queueIndex = ~0;
  GLFWwindow *window;
  vk::raii::Queue graphicsQueue = nullptr;
  std::vector<const char *> requiredDeviceExtension = {
      vk::KHRSwapchainExtensionName};
  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> swapChainImages;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  vk::Extent2D swapChainExtent;
  std::vector<vk::raii::ImageView> swapChainImageViews;

  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline graphicsPipeline = nullptr;
  vk::raii::CommandPool commandPool = nullptr;
  vk::raii::CommandBuffer commandBuffer = nullptr;

  // semaphores and fences used for synchronization
  vk::raii::Semaphore presentCompleteSemaphore = nullptr;
  vk::raii::Semaphore renderFinishedSemaphore = nullptr;
  vk::raii::Fence drawFence = nullptr;

  void initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, APP_NAME, nullptr, nullptr);
  }

  void initVulkan() {
    createInstance();
    // setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createGraphicsPipeline();
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      drawFrame();
    }
  }

  void cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  void createInstance() {
    vk::ApplicationInfo appInfo = {};
    appInfo.pApplicationName = APP_NAME,
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Enengine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = vk::ApiVersion14;

    auto requiredExtensions = getRequiredInstanceExtensions();

    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt = std::ranges::find_if(
        requiredExtensions,
        [&extensionProperties](auto const &requiredExtension) {
          return std::ranges::none_of(
              extensionProperties,
              [requiredExtension](auto const &extensionProperty) {
                return strcmp(extensionProperty.extensionName,
                              requiredExtension) == 0;
              });
        });
    if (unsupportedPropertyIt != requiredExtensions.end()) {
      throw std::runtime_error("Required extension not supported: " +
                               std::string(*unsupportedPropertyIt));
    }

    std::vector<char const *> requiredLayers;
    if (enableValidationLayers)
      requiredLayers.assign(validationLayers.begin(), validationLayers.end());

    auto layerProperties = context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(
        requiredLayers, [&layerProperties](auto const &requiredLayer) {
          return std::ranges::none_of(
              layerProperties, [requiredLayer](auto const &layerProperty) {
                return strcmp(layerProperty.layerName, requiredLayer) == 0;
              });
        });

    if (unsupportedLayerIt != requiredLayers.end()) {
      throw std::runtime_error("Required layer not supported: " +
                               std::string(*unsupportedLayerIt));
    }

    vk::InstanceCreateInfo createInfo{};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
    createInfo.ppEnabledLayerNames = requiredLayers.data();
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    instance = vk::raii::Instance(context, createInfo);
    std::cout << "-- INSTANCIA VULKAN CRIADA\n";
  }

  std::vector<const char *> getRequiredInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
  }

  bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice) {
    // Check if the physicalDevice supports the Vulkan 1.3 API version
    bool supportsVulkan1_3 =
        physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

    // Check if any of the queue families support graphics operations
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    bool supportsGraphics =
        std::ranges::any_of(queueFamilies, [](auto const &qfp) {
          return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
        });

    // Check if all required physicalDevice extensions are available
    auto availableDeviceExtensions =
        physicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions = std::ranges::all_of(
        requiredDeviceExtension,
        [&availableDeviceExtensions](auto const &requiredDeviceExtension) {
          return std::ranges::any_of(
              availableDeviceExtensions,
              [requiredDeviceExtension](auto const &availableDeviceExtension) {
                return strcmp(availableDeviceExtension.extensionName,
                              requiredDeviceExtension) == 0;
              });
        });

    // Check if the physicalDevice supports the required features (dynamic
    // rendering and extended dynamic state)
    auto features = physicalDevice.template getFeatures2<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    bool supportsRequiredFeatures =
        features.template get<vk::PhysicalDeviceVulkan13Features>()
            .dynamicRendering &&
        features
            .template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
            .extendedDynamicState;

    // Return true if the physicalDevice meets all the criteria
    return supportsVulkan1_3 && supportsGraphics &&
           supportsAllRequiredExtensions && supportsRequiredFeatures;
  }

  void pickPhysicalDevice() {
    std::vector<vk::raii::PhysicalDevice> physicalDevices =
        instance.enumeratePhysicalDevices();
    auto const devIter =
        std::ranges::find_if(physicalDevices, [&](auto const &physicalDevice) {
          return isDeviceSuitable(physicalDevice);
        });
    if (devIter == physicalDevices.end()) {
      throw std::runtime_error("failed to find a suitable GPU!");
    }
    physicalDevice = *devIter;
  }

  uint32_t findQueueFamilies(vk::raii::PhysicalDevice &physicalDevice) {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
        physicalDevice.getQueueFamilyProperties();
    uint32_t queueIndex = ~0;
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size();
         qfpIndex++) {
      if ((queueFamilyProperties[qfpIndex].queueFlags &
           vk::QueueFlagBits::eGraphics) &&
          physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface)) {
        // found a queue family that supports both graphics and present
        queueIndex = qfpIndex;
        break;
      }
    }

    return queueIndex;
  }

  void createLogicalDevice() {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
        physicalDevice.getQueueFamilyProperties();

    queueIndex = findQueueFamilies(physicalDevice);

    if (queueIndex == ~0) {
      throw std::runtime_error(
          "Could not find a queue for graphics and present -> terminating");
    }

    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.queueFamilyIndex = queueIndex;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

    vk::PhysicalDeviceFeatures deviceFeatures;

    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain(
            vk::PhysicalDeviceFeatures2{},
            vk::PhysicalDeviceVulkan13Features{}.setDynamicRendering(true),
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{}
                .setExtendedDynamicState(true));

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>();
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledExtensionCount =
        static_cast<uint32_t>(requiredDeviceExtension.size());
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtension.data();

    device = vk::raii::Device(physicalDevice, deviceCreateInfo);
    graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
  }

  void createSurface() {
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0)
      throw std::runtime_error("failed to create window surface!");
    surface = vk::raii::SurfaceKHR(instance, _surface);
  }

  void createSwapChain() {
    auto surfaceCapabilities =
        physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    swapChainExtent = chooseSwapExtent(surfaceCapabilities);
    uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);
    std::vector<vk::SurfaceFormatKHR> availableFormats =
        physicalDevice.getSurfaceFormatsKHR(*surface);
    swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

    std::vector<vk::PresentModeKHR> availablePresentModes =
        physicalDevice.getSurfacePresentModesKHR(*surface);
    vk::PresentModeKHR presentMode =
        chooseSwapPresentMode(availablePresentModes);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.surface = *surface;
    swapChainCreateInfo.minImageCount = minImageCount;
    swapChainCreateInfo.imageFormat = swapChainSurfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = swapChainExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = true;

    swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    swapChainImages = swapChain.getImages();
  }

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      std::vector<vk::SurfaceFormatKHR> const &availableFormats) {
    const auto formatIt =
        std::ranges::find_if(availableFormats, [](const auto &format) {
          return format.format == vk::Format::eB8G8R8A8Srgb &&
                 format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });
    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
  }

  vk::PresentModeKHR chooseSwapPresentMode(
      std::vector<vk::PresentModeKHR> const &availablePresentModes) {
    assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) {
      return presentMode == vk::PresentModeKHR::eFifo;
    }));
    return std::ranges::any_of(availablePresentModes,
                               [](const vk::PresentModeKHR value) {
                                 return vk::PresentModeKHR::eMailbox == value;
                               })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
  }

  vk::Extent2D
  chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    return {std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                                 capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                                 capabilities.maxImageExtent.height)};
  }

  uint32_t chooseSwapMinImageCount(
      vk::SurfaceCapabilitiesKHR const &surfaceCapabilities) {
    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    if ((0 < surfaceCapabilities.maxImageCount) &&
        (surfaceCapabilities.maxImageCount < minImageCount)) {
      minImageCount = surfaceCapabilities.maxImageCount;
    }
    return minImageCount;
  }

  void createImageViews() {
    assert(swapChainImageViews.empty());

    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = swapChainSurfaceFormat.format;
    imageViewCreateInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0,
                                            1, 0, 1};
    imageViewCreateInfo.components = {
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};

    for (auto &image : swapChainImages) {
      imageViewCreateInfo.image = image;
      swapChainImageViews.emplace_back(device, imageViewCreateInfo);
    }
  }

  void createGraphicsPipeline() {
    auto shaderCode = readFile("build/shaders.spv");
    auto shaderModule = createShaderModule(shaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = shaderModule;
    vertShaderStageInfo.pName = "vertMain";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = shaderModule;
    fragShaderStageInfo.pName = "fragMain";

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                        fragShaderStageInfo};

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                   vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

    vk::Viewport viewport{0.0f,
                          0.0f,
                          static_cast<float>(swapChainExtent.width),
                          static_cast<float>(swapChainExtent.height),
                          0.0f,
                          1.0f};

    vk::Rect2D scissor{vk::Offset2D{0, 0}, swapChainExtent};

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport, viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = vk::False;
    rasterizer.rasterizerDiscardEnable = vk::False;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = vk::False;
    rasterizer.lineWidth = 1.0f;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.sampleShadingEnable = vk::False;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = vk::False;
    colorBlendAttachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    // vk::PipelineColorBlendAttachmentState colorBlendAttachment{
    //.blendEnable         = vk::True,
    //.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
    //.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
    //.colorBlendOp        = vk::BlendOp::eAdd,
    //.srcAlphaBlendFactor = vk::BlendFactor::eOne,
    //.dstAlphaBlendFactor = vk::BlendFactor::eZero,
    //.alphaBlendOp        = vk::BlendOp::eAdd,
    //.colorWriteMask      = vk::ColorComponentFlagBits::eR |
    // vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
    // vk::ColorComponentFlagBits::eA};

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable = vk::False;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = shaderStages;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    graphicsPipelineCreateInfo.pViewportState = &viewportState;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizer;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampling;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlending;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicState;
    graphicsPipelineCreateInfo.layout = pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = nullptr;

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats =
        &swapChainSurfaceFormat.format;

    vk::StructureChain<vk::GraphicsPipelineCreateInfo,
                       vk::PipelineRenderingCreateInfo>
        pipelineCreateInfoChain{graphicsPipelineCreateInfo,
                                pipelineRenderingCreateInfo};

    graphicsPipeline = vk::raii::Pipeline(
        device, nullptr,
        pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
  }

  [[nodiscard]] vk::raii::ShaderModule
  createShaderModule(const std::vector<char> &code) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size() * sizeof(char),
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    vk::raii::ShaderModule shaderModule{device, createInfo};
    return shaderModule;
  }

  // command pools manage the memory used for the buffers and command buffers
  void createCommandPool() {
    vk::CommandPoolCreateInfo poolInfo{};
    // eTransient would hint that commandBuffers are changed constantly, which
    // may change memory allocation behaviour permite que os commandBuffers
    // eResetCommandBuffer allows commandBuffers to be rerecorded individually
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = queueIndex;
    commandPool = vk::raii::CommandPool(device, poolInfo);
  }

  void createCommandBuffer() {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    commandBuffer =
        std::move(vk::raii::CommandBuffers(device, allocInfo).front());
  }

  void recordCommandBuffer(uint32_t imageIndex) {
    // starting dynamic rendering
    commandBuffer.begin({});

    // specifying attachments for rendering
    transitionImageLayout(
        imageIndex, vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {}, // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite,         // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput  // dstStage
    );

    // setting up color attachments
    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo{};
    attachmentInfo.imageView = swapChainImageViews[imageIndex];
    attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
    // what happens to the rendered image after being cleaed
    // in this case, stored for later use
    attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
    attachmentInfo.clearValue = clearColor; // sets black as the clear color

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea = {.offset = {0, 0}, .extent = swapChainExtent};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &attachmentInfo;

    // basic drawing commands
    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               *graphicsPipeline);
    commandBuffer.setViewport(
        0,
        vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width),
                     static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0,
                             vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

    int vertexCount = 3;
    int instancedRendering = 1; // not using instanced rendering
    // offset of vertexBuffer
    // defines the lowest value of SV_vertexID
    int firstVertex = 0;
    // offset for instanced rendering, defines lowest value of SV_InstanceID
    int firstInstance = 0;
    commandBuffer.draw(vertexCount, instancedRendering, firstVertex,
                       firstInstance);

    commandBuffer.endRendering();

    // after rendering, transition the swapchain image to
    // vk::ImageLayout::ePresentSrcKHR so it can eb rendered to the screen
    transitionImageLayout(
        imageIndex, vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
        {},                                                 // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe           // dstStage
    );

    commandBuffer.end();
  }

  // transitions the image layout to one that can be rendered on
  void transitionImageLayout(uint32_t imageIndex, vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout,
                             vk::AccessFlags2 srcAccessMask,
                             vk::AccessFlags2 dstAccessMask,
                             vk::PipelineStageFlags2 srcStageMask,
                             vk::PipelineStageFlags2 dstStageMask) {
    vk::ImageMemoryBarrier2 barrier{};
    barrier.srcStageMask = srcStageMask;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstStageMask = dstStageMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = swapChainImages[imageIndex];
    barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

    vk::DependencyInfo dependencyInfo{};
    dependencyInfo.dependencyFlags = {};
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &barrier;
    commandBuffer.pipelineBarrier2(dependencyInfo);
  }

  // drawing a frame consists on:
  // Wait for previous frame to finish => acquire image rom swap
  // swapChainImages
  // => record commandBuffer that draws the scene on the image => submit
  // recorded buffer => present the swapchain image
  // Sync methods:
  // semaphore -> used to order the operations
  // fence -> lets CPU know when GPU is done rendering
  void drawFrame() {
    // waiting for the previous frame
    auto fenceResult = device.waitForFences(*drawFence, vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
      throw std::runtime_error("failed to wait for fence!\n");
    }
    device.resetFences(*drawFence);

    // getting image from framebuffer and recording the command
    auto [result, imageIndex] = swapChain.acquireNextImage(
        UINT64_MAX, *presentCompleteSemaphore, nullptr);
    recordCommandBuffer(imageIndex);

    const vk::PresentInfoKHR presentInfoKHR(1, &*renderFinishedSemaphore, 1,
                                            &*swapChain, &imageIndex);

    result = graphicsQueue.presentKHR(presentInfoKHR);
  }

  void createSyncObjects() {
    // initializing sync objects
    presentCompleteSemaphore =
        vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    renderFinishedSemaphore =
        vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    drawFence = vk::raii::Fence(device, fenceCreateInfo);

    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo(
        1, &*presentCompleteSemaphore, &waitDestinationStageMask, 1,
        &*commandBuffer, 1, &*renderFinishedSemaphore);
    graphicsQueue.submit(submitInfo, *drawFence);
  }
};

int main() {
  try {
    HelloTriangleApp app;
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
