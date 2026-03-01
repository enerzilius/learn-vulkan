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
GLFWwindow* window;


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

    if(candidates.rbegin()->first > 0) physicalDevice = candidates.rbegin()->second();
    else std::runtime_error("Failed to find a suitable GPU :(");
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
