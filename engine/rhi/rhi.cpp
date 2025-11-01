#include "rhi.h"
#include "vkRHI.h"
#include <iostream>
#include <stdexcept>

JstResult jstInitRHI(JstGraphicsBackend backend) {
  try {
    switch (backend) {
    case JstVulkan:
      std::cout << "Selected Vulkan backend" << std::endl;
      jst::InitVulkan();
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
