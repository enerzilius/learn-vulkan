#include "VulkanStuff.h"
#include <iostream>
#include <vector>
#include <assert.h>
#include <vulkan/vulkan.h>

vks::VulkanApp::VulkanApp() {
  // Initialize member variables if needed
  m_instance = VK_NULL_HANDLE;
}

vks::VulkanApp::~VulkanApp() {
  // Clean up Vulkan resources
  if (m_instance != VK_NULL_HANDLE) {
    vkDestroyInstance(m_instance, NULL);
  }
}

void vks::VulkanApp::Init(const char* appName) {
  CreateInstance(appName);
} 

void vks::VulkanApp::CreateInstance(const char* appName) {
  std::vector<const char*> layers = {
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_KHRONOS_profiles"
  };
  
  std::vector<const char*> extensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32 
    "VK_KHR_win32_surface",
#endif
#ifdef __linux__
    "VK_KHR_xcb_surface",
#endif
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
  };

	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = appName,
		.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		.pEngineName = "Enengine",
		.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};

	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = (uint32_t)(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount = (uint32_t)(extensions.size()),
		.ppEnabledExtensionNames = extensions.data()
	};

  VkResult res = vkCreateInstance(&createInfo, NULL, &m_instance);
  if(res != VK_SUCCESS) {
    std::cout<<"ERROR: Failed to create instance - "<<res<<"\n";
    exit(1);
  }
  std::cout<<"Vulkan instace created!!\n\n";
}

void vks::VulkanApp::RenderScene() {
  //std::cout<<"!";
}
