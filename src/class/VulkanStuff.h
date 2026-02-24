#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

namespace vks {
  class VulkanApp {
    public:
      VulkanApp();
      ~VulkanApp();

      void Init(const char* appName);
      void RenderScene();
      Cleanup();
    private:
      void CreateInstance(const char* appName);
      VkInstance m_instance = NULL;
  };
}
