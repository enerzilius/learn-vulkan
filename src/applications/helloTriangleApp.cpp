#if defined (__INTELLISENSE__ ) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include "src/class/VulkanStuff.h"

class HelloTriangleApp {
public:
  void run() {
    initVulkan();
    mainLoop();  
    cleanup();   
  }
private:
  void initVulkan() {
    vks::VulkanApp app;
    app.Init("TESTE");
  }

  void mainLoop() {

  }

  void cleanup() {

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
