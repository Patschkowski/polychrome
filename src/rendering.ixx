module;

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

export module rendering;

export class renderer {
public:
  renderer();

  void draw(){};

private:
  vk::raii::Context  context_;
  vk::raii::Instance instance_{nullptr};
  vk::raii::Device   device_{nullptr};
};
