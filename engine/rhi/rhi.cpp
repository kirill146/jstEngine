#include "rhi.h"
#include "vkRHI.h"
#include "d3d12RHI.h"
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
      jst::InitD3D12(validationEnabled);
      std::cout << "Selected D3D12 backend" << std::endl;
      break;
    default:
      return JstFailed;
    }
  } catch (std::exception& e) {
    return JstFailed;
  }
  return JstSuccess;
}
