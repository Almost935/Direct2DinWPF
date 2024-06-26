//-----------------------------------------------------------------------------
// Modified version of the "Meshes" sample from the DirectX SDK.
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <d2d1_1.h>
#include <d2d1.h>
#include <dwrite.h>
#include <d3d11.h>
#include <vector>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

template<class Interface>
inline void
SafeRelease(
	Interface** ppInterfaceToRelease
)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
ID3D11Device*						dev;
ID3D11DeviceContext*				devcon;
IDXGISwapChain*						m_pSwapChain;
HWND                                m_hwnd;
IDXGISurface*						pBackBuffer;
ID2D1RenderTarget*					m_pBackBufferRT;
ID2D1Factory1*						m_pDirect2dFactory;
ID2D1HwndRenderTarget*				m_pRenderTarget;
ID2D1SolidColorBrush*				m_pLightSlateGrayBrush;
ID2D1SolidColorBrush*				m_pCornflowerBlueBrush;
ID2D1StrokeStyle1*					strokeStyleFixedThickness;

//------------------------------------------------------------------------;or-----
// Name: CreateDeviceIndependentResources()
// Desc: Creates the independent resources needed to draw Direct 2D 
//-----------------------------------------------------------------------------
HRESULT CreateDeviceIndependentResources()
{
	HRESULT hr = S_OK;

	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

	// Create stroke style
	hr = m_pDirect2dFactory->CreateStrokeStyle(
		D2D1::StrokeStyleProperties1(
			D2D1_CAP_STYLE_FLAT,
			D2D1_CAP_STYLE_FLAT,
			D2D1_CAP_STYLE_FLAT,
			D2D1_LINE_JOIN_MITER,
			10.0f,
			D2D1_DASH_STYLE_SOLID,
			0.0f, D2D1_STROKE_TRANSFORM_TYPE_FIXED),
		nullptr, 0, &strokeStyleFixedThickness);

	return hr;
}

//-----------------------------------------------------------------------------
// Name: CreateDeviceIndependentResources()
// Desc: Creates the independent resources needed to draw Direct 2D 
//-----------------------------------------------------------------------------
HRESULT CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right,
			rc.bottom
		);

		// Create a Direct2D render target.
		hr = m_pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget
		);

		auto renderTargetSize = m_pRenderTarget->GetSize();

		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSlateGray),
				&m_pLightSlateGrayBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			// Create a blue brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
				&m_pCornflowerBlueBrush
			);
		}
	}

	return hr;
}

//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct2D
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{

	HRESULT hr;
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;     // supports Direct 2D
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hWnd;                                // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	// create a device, device context and swap chain using the information in the scd struct
	hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&m_pSwapChain,
		&dev,
		NULL,
		&devcon);
	if (FAILED(hr)) { return E_FAIL; }


	CreateDeviceResources();

	// Create the DXGI Surface Render Target.
	float dpi = GetDpiForWindow(hWnd);

	D2D1_RENDER_TARGET_PROPERTIES props =
		D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
			dpi,
			dpi);

	// Get a surface in the swap chain
	hr = m_pSwapChain->GetBuffer(
		0,
		IID_PPV_ARGS(&pBackBuffer)
	);
	// Get a surface in the swap chain
	hr = m_pSwapChain->GetBuffer(
		0,
		IID_PPV_ARGS(&pBackBuffer)
	);

	// Create a Direct2D render target that can draw into the surface in the swap chain
	hr = m_pDirect2dFactory->CreateDxgiSurfaceRenderTarget(pBackBuffer, &props, &m_pBackBufferRT);



	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DrawGeometry()
// Desc: Draws dxf geometries
//-----------------------------------------------------------------------------
//HRESULT DrawGeometry()
//{
//	
//}

//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initializes DirectX, the scene, geometry, and more.
//-----------------------------------------------------------------------------
extern "C" __declspec(dllexport) IDXGISurface* WINAPI Initialize(HWND hwnd, int width, int height)
{
	// Initialize Direct3D
	if (SUCCEEDED(InitD3D(hwnd)))
	{
		if (SUCCEEDED(CreateDeviceIndependentResources()))
		{
			return pBackBuffer;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
extern "C" __declspec(dllexport) VOID WINAPI Cleanup()
{
	SafeRelease(&dev);
	SafeRelease(&devcon);
	SafeRelease(&m_pSwapChain);
	SafeRelease(&pBackBuffer);
	SafeRelease(&m_pBackBufferRT);
	SafeRelease(&m_pDirect2dFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
	SafeRelease(&strokeStyleFixedThickness);
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
extern "C" __declspec(dllexport) VOID WINAPI Render()
{
	HRESULT hr = S_OK;

	if (m_pBackBufferRT &&
		SUCCEEDED(hr))
	{
		m_pBackBufferRT->BeginDraw();
		D2D1_SIZE_F targetSize = m_pBackBufferRT->GetSize();

		for (int i = 0; i <= targetSize.width; i += (targetSize.width / 50))
		{
			m_pBackBufferRT->DrawLine(D2D1::Point2F(i, 0), D2D1::Point2F(i, i), m_pCornflowerBlueBrush, 1, strokeStyleFixedThickness);
		}
		m_pBackBufferRT->EndDraw();
	}
}