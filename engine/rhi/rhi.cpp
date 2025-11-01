#include "rhi.h"
#include "vkRHI.h"
#include <iostream>
#include <stdexcept>

PFN_jstGetPhysicalDevices jstGetPhysicalDevices;

JstResult jstInitRHI(JstGraphicsBackend backend, JstBool validationEnabled) {
  try {
    switch (backend) {
    case JstVulkan:
      std::cout << "Selected Vulkan backend" << std::endl;
      jst::InitVulkan(validationEnabled);
      break;
    case JstD3D12:
      std::cout << "Selected D3D12 backend" << std::endl;
      return JstFailed; // not supported yet
    default:
      return JstFailed;
    }
  } catch (std::exception& e) {
    return JstFailed;
  }
  return JstSuccess;
}
