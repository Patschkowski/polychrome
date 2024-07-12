module;

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_win32.h>

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

  const vk::Win32 surface_create_info{};

  surface_ = vk::raii::SurfaceKHR(instance_, surface_create_info);

  // Find a physical device that matches our demands.
  vk::raii::PhysicalDevices physical_devices{instance_};
  const auto                suitable_physical_device_iter{
    std::find_if(physical_devices.begin(),
                 physical_devices.end(),
                 [](const auto& physical_device) {
                   // const auto properties{physical_device.getProperties()};
                   // return vk::PhysicalDeviceType::eDiscreteGpu ==
                   // properties.deviceType;
                   return true;
                 })};

  if (physical_devices.end() == suitable_physical_device_iter) {
    throw std::runtime_error{"no suitable physical device"};
  }

  const auto queue_family_properties{
    suitable_physical_device_iter->getQueueFamilyProperties()};

  auto queue_family_properties_iter{queue_family_properties.begin()};
  while (queue_family_properties.end() != queue_family_properties_iter) {
    const auto can_present{suitable_physical_device_iter->getSurfaceSupportKHR(
      std::distance(queue_family_properties.begin(),
                    queue_family_properties_iter),
      surface)};

    if (vk::QueueFlags{vk::QueueFlagBits::eGraphics}
        == (queue_family_properties_iter->queueFlags
            & vk::QueueFlagBits::eGraphics)) {
      break;
    }
  }

  const auto queue_family_properties_iter{std::find_if(
    queue_family_properties.begin(),
    queue_family_properties.end(),
    [](const auto& properties) {
      return vk::QueueFlags{vk::QueueFlagBits::eGraphics}
             == (properties.queueFlags & vk::QueueFlagBits::eGraphics);
    })};

  if (queue_family_properties.end() == queue_family_properties_iter) {
    throw std::runtime_error{"no matching queue found"};
  }

  const float                     queue_prios[]{1.0f};
  const vk::DeviceQueueCreateInfo dev_queue_create_info[]{
    vk::DeviceQueueCreateInfo{
      {},
      static_cast<std::uint32_t>(std::distance(queue_family_properties.begin(),
                                               queue_family_properties_iter)),
      1u,
      queue_prios}};
  const vk::DeviceCreateInfo dev_create_info{{}, 1, dev_queue_create_info};

  device_ = suitable_physical_device_iter->createDevice(dev_create_info);
}
