#include "d3d12RHI.h"
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <cassert>
#include <vector>
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

#define DX_CHECK(err) assert((err) >= 0); // TODO

static std::vector<JstPhysicalDevice> physicalDevices;

static int GetPhysicalDevices(const JstPhysicalDevice** physicalDevicesOut) {
  *physicalDevicesOut = physicalDevices.data();
  return (int)physicalDevices.size();
}

static void DestroyRHI2() {
	physicalDevices.clear();
}

void jst::InitD3D12(JstBool validationEnabled) {
	ComPtr<IDXGIFactory4> factory;
	DX_CHECK(CreateDXGIFactory2(validationEnabled ? DXGI_CREATE_FACTORY_DEBUG : 0u, IID_PPV_ARGS(&factory)));

	ComPtr<IDXGIAdapter1> pCurAdapter;
	for (UINT i = 0; factory->EnumAdapters1(i, &pCurAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		DXGI_ADAPTER_DESC1 curAdapterDesc;
		DX_CHECK(pCurAdapter->GetDesc1(&curAdapterDesc));
		physicalDevices.push_back({});
		wcstombs(physicalDevices.back().name, curAdapterDesc.Description, sizeof(physicalDevices.back().name));
	}

	jstGetPhysicalDevices = GetPhysicalDevices;
	jstDestroyRHI = DestroyRHI2;
}
