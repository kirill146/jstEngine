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

const uint32_t JST_N_QUEUES_UNKNOWN = 0xffffffff;

struct JstPhysicalDevice {
  char name[256];
  JstPhysDevFlagsMask flags;
  uint32_t nGraphicsQueues;
  uint32_t nComputeQueues;
  uint32_t nTransferQueues;
};

#define JST_DEFINE_HANDLE(obj) typedef void* Jst##obj
JST_DEFINE_HANDLE(Queue);
JST_DEFINE_HANDLE(Device);
JST_DEFINE_HANDLE(Swapchain);

typedef int JstBool;

// MACRO's arguments should be (f, ret, args)
#define JST_FOREACH_RHI_FUNCTION(MACRO)                                                                                \
  MACRO(GetPhysicalDevices, int, (const JstPhysicalDevice** physicalDevices));                                         \
  MACRO(DestroyRHI, void, ());                                                                                         \
  MACRO(CreateDevice, JstResult,                                                                                       \
        (int physicalDeviceId, JstDevice* device, JstQueue* graphicsQueue, JstQueue* computeQueue,                     \
         JstQueue* transferQueue));                                                                                    \
  MACRO(DestroyDevice, void, (JstDevice device));                                                                      \
  /* width and height don't have effect on Win32, swapchain image size is determined by HWND's client area */          \
  MACRO(CreateSwapchain, JstResult,                                                                                    \
        (JstDevice device, JstQueue graphicsQueue, void* windowHandle, uint32_t minImageCount, uint32_t width,         \
         uint32_t height, JstSwapchain* swapchain));                                                                   \
  MACRO(DestroySwapchain, void, (JstSwapchain swapchain));

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
