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

JstResult jstInitRHI(JstGraphicsBackend backend);

#ifdef __cplusplus
}
#endif
