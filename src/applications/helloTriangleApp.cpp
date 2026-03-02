#if defined (__INTELLISENSE__ ) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include "../class/VulkanStuff.cpp"
#include <vector>
#include <map>

const std::vector<char const*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = false;
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
  vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::Device device = nullptr;
  GLFWwindow* window;
  vk::raii::Queue graphicsQueue = nullptr;
	std::vector<const char *> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};


  void initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, APP_NAME, nullptr, nullptr);
  }

  void initVulkan() {
    createInstance();
    //setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
  }

  void cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  void createInstance() {
    constexpr VkApplicationInfo appInfo = {
      .pApplicationName   = APP_NAME,
      .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
      .pEngineName        = "Enengine", 
      .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
      .apiVersion         = vk::ApiVersion14
    };
    
    auto requiredExtensions = getRequiredInstanceExtensions();

    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt = std::ranges::find_if(requiredExtensions,
		                         [&extensionProperties](auto const &requiredExtension) {
			                         return std::ranges::none_of(extensionProperties,
			                                                     [requiredExtension](auto const &extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
		                         });
		if (unsupportedPropertyIt != requiredExtensions.end())
		{
			throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
		} 

    std::vector<char const*> requiredLayers;
    if(enableValidationLayers) requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    
    auto layerProperties = context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(
      requiredLayers,
		  [&layerProperties](auto const &requiredLayer) {
			  return std::ranges::none_of(layerProperties, 
          [requiredLayer](auto const &layerProperty) { 
            return strcmp(layerProperty.layerName, requiredLayer) == 0; 
          }
        );
      }
    );

    if(unsupportedLayerIt != requiredLayers.end()) {
      throw std::runtime_error("Required layer not supported: "+std::string(*unsupportedLayerIt));
    }
    
    VkInstanceCreateInfo createInfo {
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
      .ppEnabledLayerNames = requiredLayers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data()
    };
    
    instance = vk::raii::Instance(context, createInfo);
    std::cout<<"-- INSTANCIA VULKAN CRIADA\n";
  }

  std::vector<const char*> getRequiredInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); 
    
    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
  }

  void pickPhysicalDevice() {
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    auto devices = instance.enumeratePhysicalDevices();

    if(devices.empty()) throw std::runtime_error("Failed to find GPUs with Vulkan support");
    std::multimap<int, vk::raii::PhysicalDevice> candidates;

    for(const auto& device : devices) {
      auto deviceProperties = device.getProperties();
      auto deviceFeatures = device.getFeatures();
      uint32_t score = 0;

      if(!deviceFeatures.geometryShader || deviceProperties.apiVersion >= VK_API_VERSION_1_3) continue;

      if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;
      }

      score += deviceProperties.limits.maxImageDimension2D;
      candidates.insert(std::make_pair(score, device));
    }

    if(candidates.rbegin()->first > 0) physicalDevice = candidates.rbegin()->second;
    else std::runtime_error("Failed to find a suitable GPU :(");
  }

  uint32_t findQueueFamilies(vk::raii::PhysicalDevice& physicalDevice) {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    
    auto graphicsQueueFamilyProperty =
      std::find_if(queueFamilyProperties.begin(),
                   queueFamilyProperties.end(),
                   []( vk::QueueFamilyProperties const & qfp ) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; } );

    return static_cast<uint32_t>( std::distance( queueFamilyProperties.begin(), graphicsQueueFamilyProperty ) );
  }

  void createLogicalDevice() {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    uint32_t graphicsIndex = findQueueFamilies(physicalDevice);
    float queuePriority = 0.5f;
    VkDeviceQueueCreateInfo deviceQueueCreateInfo {
      .queueFamilyIndex = graphicsIndex,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority
    };

    vk::PhysicalDeviceFeatures deviceFeatures;

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain(
      vk::PhysicalDeviceFeatures2{},
      vk::PhysicalDeviceVulkan13Features{}.setDynamicRendering(true),
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{}.setExtendedDynamicState(true)
    );

    VkDeviceCreateInfo deviceCreateInfo{
      .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
      .ppEnabledExtensionNames = requiredDeviceExtension.data()
    };

    device = vk::raii::Device(physicalDevice, deviceCreateInfo);
    graphicsQueue = vk::raii::Queue(device, graphicsIndex, 0);
  }
};

int main() {
  try {
    HelloTriangleApp app;
    app.run();
  } catch (const std::exception& e){
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
