#include "rhi.h"
#include "d3d12RHI.h"
#include "vkRHI.h"
#include <iostream>
#include <stdexcept>

#define JST_DECLARE_RHI_FUNCTION(f, ret, args) PFN_##jst##f jst##f
JST_FOREACH_RHI_FUNCTION(JST_DECLARE_RHI_FUNCTION);
#undef JST_DECLARE_RHI_FUNCTION

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

