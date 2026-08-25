#include "vulkan/vulkan.hpp"
#include <cassert>
#include <cstddef>
#include <vulkan/vulkan_core.h>
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <glm/glm.hpp>

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  // this is needed to tell Vulkan how to pass this data format to the shader
  static vk::VertexInputBindingDescription getBindingDescription() {
    auto bindDesc = vk::VertexInputBindingDescription(
        0, sizeof(Vertex), vk::VertexInputRate::eVertex);
    return bindDesc;
  }

  // second structure to describe vertex input handling
  static std::array<vk::VertexInputAttributeDescription, 2>
  getAttributeDescriptions() {
    // binding: tells vulkan which binding the vertex data comes from
    // format: type (and size) of data of the attribute - S:signed U:unsigned
    auto posAttributes = vk::VertexInputAttributeDescription(
        0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos));
    auto colorAttributes = vk::VertexInputAttributeDescription(
        1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color));
    return {posAttributes, colorAttributes};
  }
};
