find_package(Vulkan COMPONENTS volk REQUIRED)
set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)

add_library(engine
  rhi/rhi.h
  rhi/rhi.cpp
  rhi/vkRHI.h
  rhi/vkRHI.cpp
  rhi/d3d12RHI.h
  rhi/d3d12RHI.cpp
)
target_link_libraries(engine PRIVATE d3d12 dxgi)
target_compile_features(engine PUBLIC cxx_std_20)
target_include_directories(engine PRIVATE
  ${Vulkan_INCLUDE_DIR}
  ../external/agility-sdk-${AGILITY_SDK_VERSION}/build/native/include
)
