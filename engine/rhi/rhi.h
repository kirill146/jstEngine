#pragma once
#include <stdint.h>

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

enum JstPhysDevFlags {
  JstPhysDevHasGraphicsQueue = 0x00000001,
  JstPhysDevHasComputeQueue = 0x00000002,
  JstPhysDevHasTransferQueue = 0x00000002,
  JstPhysDevFlagsForceUint32 = 0x7fffffff,
};
typedef uint32_t JstPhysDevFlagsMask;

struct JstPhysicalDevice {
  char name[256];
  JstPhysDevFlagsMask flags;
};

#define JST_DEFINE_HANDLE(obj) typedef void* Jst##obj
JST_DEFINE_HANDLE(Queue);
JST_DEFINE_HANDLE(Device);

typedef int JstBool;

// MACRO's arguments should be (f, ret, args)
#define JST_FOREACH_RHI_FUNCTION(MACRO)                                                            \
  MACRO(GetPhysicalDevices, int, (const JstPhysicalDevice** physicalDevices));                     \
  MACRO(DestroyRHI, void, ());                                                                     \
  MACRO(CreateDevice, JstResult,                                                                   \
        (int physicalDeviceId, JstDevice* device, JstQueue* graphicsQueue, JstQueue* computeQueue, \
         JstQueue* transferQueue));                                                                \
  MACRO(DestroyDevice, void, (JstDevice device));                                                  \
  MACRO(CreateSwapchain, JstResult, (int width, int height));

#define JST_TYPEDEF_RHI_FUNCTION(f, ret, args) typedef ret(*PFN_##jst##f) args
#define JST_DECLARE_RHI_FUNCTION_EXTERN(f, ret, args) extern PFN_##jst##f jst##f

JST_FOREACH_RHI_FUNCTION(JST_TYPEDEF_RHI_FUNCTION);
JST_FOREACH_RHI_FUNCTION(JST_DECLARE_RHI_FUNCTION_EXTERN);

#undef JST_TYPEDEF_RHI_FUNCTION
#undef JST_DECLARE_RHI_FUNCTION_EXTERN

JstResult jstInitRHI(JstGraphicsBackend backend, JstBool validationEnabled);

#ifdef __cplusplus
}
#endif
