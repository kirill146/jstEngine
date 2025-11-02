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

// MACRO's arguments should be (f, ret, args)
#define JST_FOREACH_RHI_FUNCTION(MACRO)                                        \
  MACRO(GetPhysicalDevices, int, (const JstPhysicalDevice** physicalDevices)); \
  MACRO(DestroyRHI, void, ())

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
