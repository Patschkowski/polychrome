module;

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

module rendering;

renderer::renderer()
{
  {
    const vk::ApplicationInfo    app_info{"Polychrome",
                                       VK_MAKE_VERSION(0, 1, 0),
                                       "PolyEngine",
                                       VK_MAKE_VERSION(0, 1, 0),
                                       VK_API_VERSION_1_0};
    const vk::InstanceCreateInfo inst_create_info{{}, &app_info};

    instance_ = context_.createInstance(inst_create_info);
  }

  // Find a physical device that matches our demands.
  vk::raii::PhysicalDevices physical_devices{instance_};
  const vk::DeviceCreateInfo dev_create_info{};

  device_ = physical_devices[0].createDevice(dev_create_info);
}
