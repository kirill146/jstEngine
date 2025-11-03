#include "d3d12RHI.h"
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#define DX_CHECK(err) assert((err) >= 0); // TODO

struct JstD3D12Device {
  ID3D12Device* device;
  ID3D12CommandQueue* directQueue;
  ID3D12CommandQueue* computeQueue;
  ID3D12CommandQueue* copyQueue;
};

static std::vector<JstPhysicalDevice> physicalDevices;
static std::vector<ComPtr<IDXGIAdapter1>> dxgiAdapters;

static int GetPhysicalDevices(const JstPhysicalDevice** physicalDevicesOut) {
  *physicalDevicesOut = physicalDevices.data();
  return (int)physicalDevices.size();
}

static void DestroyRHI() {
  dxgiAdapters.clear();
  physicalDevices.clear();
}

static JstResult CreateSwapchain(int width, int height) {
  return JstSuccess;
}

static JstResult CreateDevice(int physicalDeviceId, JstDevice* device, JstQueue* graphicsQueue, JstQueue* computeQueue,
                              JstQueue* transferQueue) {
  ComPtr<ID3D12Device> d3d12Device;
  if (FAILED(
          D3D12CreateDevice(dxgiAdapters[physicalDeviceId].Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device))))
  {
    return JstFailed;
  }

  D3D12_COMMAND_QUEUE_DESC queueDesc;
  queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.NodeMask = 0u;
  ComPtr<ID3D12CommandQueue> d3d12DirectQueue;
  if (graphicsQueue != nullptr) {
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (FAILED(d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&d3d12DirectQueue)))) {
      return JstFailed;
    }
  }
  ComPtr<ID3D12CommandQueue> d3d12ComputeQueue;
  if (computeQueue != nullptr) {
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
    if (FAILED(d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&d3d12ComputeQueue)))) {
      return JstFailed;
    }
  }
  ComPtr<ID3D12CommandQueue> d3d12CopyQueue;
  if (transferQueue != nullptr) {
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
    if (FAILED(d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&d3d12CopyQueue)))) {
      return JstFailed;
    }
  }

  JstD3D12Device* dev = new JstD3D12Device{
    .device = d3d12Device.Detach(),
    .directQueue = d3d12DirectQueue.Detach(),
    .computeQueue = d3d12ComputeQueue.Detach(),
    .copyQueue = d3d12CopyQueue.Detach(),
  };
  *device = (JstDevice)dev;
  if (graphicsQueue) {
    *graphicsQueue = (JstQueue)dev->directQueue;
  }
  if (computeQueue) {
    *computeQueue = (JstQueue)dev->computeQueue;
  }
  if (transferQueue) {
    *transferQueue = (JstQueue)dev->copyQueue;
  }

  return JstSuccess;
}

static void DestroyDevice(JstDevice device) {
  JstD3D12Device* dev = (JstD3D12Device*)device;
  dev->device->Release();
  if (dev->directQueue) {
    dev->directQueue->Release();
  }
  if (dev->computeQueue) {
    dev->computeQueue->Release();
  }
  if (dev->copyQueue) {
    dev->copyQueue->Release();
  }
  delete dev;
}

void jst::InitD3D12(JstBool validationEnabled) {
  // enable the debug layer
  {
    ComPtr<ID3D12Debug> debugController;
    if (validationEnabled && SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
      debugController->EnableDebugLayer();
    }
  }

  ComPtr<IDXGIFactory4> factory;
  DX_CHECK(CreateDXGIFactory2(validationEnabled ? DXGI_CREATE_FACTORY_DEBUG : 0u, IID_PPV_ARGS(&factory)));

  ComPtr<IDXGIAdapter1> adapter;
  for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
    DXGI_ADAPTER_DESC1 adapterInfo;
    DX_CHECK(adapter->GetDesc1(&adapterInfo));
    physicalDevices.push_back({});
    wcstombs(physicalDevices.back().name, adapterInfo.Description, sizeof(physicalDevices.back().name));
    dxgiAdapters.push_back(adapter);
  }

#define JST_SET_RHI_FUNCTION(f, ret, args) jst##f = f
  JST_FOREACH_RHI_FUNCTION(JST_SET_RHI_FUNCTION);
#undef JST_SET_RHI_FUNCTION
}
