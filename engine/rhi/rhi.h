#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum JstResult {
  JstSuccess,
  JstFailed,
};

enum JstGraphicsBackend {
  JstVulkan,
  JstD3D12,
};

struct JstPhysicalDevice {
  char name[256];
};

typedef int JstBool;

typedef int(*PFN_jstGetPhysicalDevices)(JstPhysicalDevice const** physicalDevices);
extern PFN_jstGetPhysicalDevices jstGetPhysicalDevices;

JstResult jstInitRHI(JstGraphicsBackend backend, JstBool validationEnabled);

#ifdef __cplusplus
}
#endif
