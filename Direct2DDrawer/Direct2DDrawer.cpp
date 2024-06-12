//-----------------------------------------------------------------------------
// Modified version of the "Meshes" sample from the DirectX SDK.
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <d2d1_1.h>
#include <d2d1.h>
#include <dwrite.h>
#include <d3d11.h>

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
ID3D11Device*           dev;
ID3D11DeviceContext*    devcon;
IDXGISwapChain*         m_pSwapChain;
HWND                    m_hwnd;
IDXGISurface* pBackBuffer;
ID2D1RenderTarget*&     m_pBackBufferRT;
ID2D1Factory1*          m_pDirect2dFactory;
ID2D1HwndRenderTarget*  m_pRenderTarget;
ID2D1SolidColorBrush*   m_pLightSlateGrayBrush;
ID2D1SolidColorBrush*   m_pCornflowerBlueBrush;
ID2D1StrokeStyle1*      strokeStyleFixedThickness;

//-----------------------------------------------------------------------------
// Name: CreateDeviceIndependentResources()
// Desc: Creates the independent resources needed to draw Direct 2D 
//-----------------------------------------------------------------------------
HRESULT CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    if (SUCCEEDED(hr))
    {
        // Create stroke style
        m_pDirect2dFactory->CreateStrokeStyle(
            D2D1::StrokeStyleProperties1(
                D2D1_CAP_STYLE_FLAT,
                D2D1_CAP_STYLE_FLAT,
                D2D1_CAP_STYLE_FLAT,
                D2D1_LINE_JOIN_MITER,
                10.0f,
                D2D1_DASH_STYLE_SOLID,
                0.0f, D2D1_STROKE_TRANSFORM_TYPE_FIXED),
            nullptr, 0, &strokeStyleFixedThickness);
    }

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
// Name: InitGeometry()
// Desc: Load dxf geometries
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
    HRESULT hr;

    if (m_pBackBufferRT)
    {
        m_pBackBufferRT->BeginDraw();

        D2D1_SIZE_F targetSize = m_pBackBufferRT->GetSize();

        for (int i = 0; i <= targetSize.width; i += (targetSize.width / 50))
        {
            ID2D1PathGeometry* path_geometry;
            m_pDirect2dFactory->CreatePathGeometry(&path_geometry);

            ID2D1GeometrySink* pSink = NULL;
            hr = path_geometry->Open(&pSink);
            pSink->BeginFigure(D2D1::Point2F(i, 0), D2D1_FIGURE_BEGIN_HOLLOW);
            pSink->AddLine(D2D1::Point2F(i, targetSize.height));
        }

        m_pBackBufferRT->EndDraw();
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initializes DirectX, the scene, geometry, and more.
//-----------------------------------------------------------------------------
extern "C" __declspec(dllexport) IDirect3DSurface9* WINAPI Initialize(HWND hwnd, int width, int height)
{
    // Initialize Direct3D
    if (SUCCEEDED(InitD3D(hwnd)))
    {
        // Create the scene geometry
        if (SUCCEEDED(InitGeometry()))
        {
            if (FAILED(g_pd3dDevice->CreateRenderTarget(width, height,
                D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0,
                true, // lockable (true for compatibility with Windows XP.  False is preferred for Windows Vista or later)
                &g_pd3dSurface, NULL)))
            {
                MessageBox(NULL, L"NULL!", L"Missing File", 0);
                return NULL;
            }
            g_pd3dDevice->SetRenderTarget(0, g_pd3dSurface);
        }
    }
    return g_pd3dSurface;
}

//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
extern "C" __declspec(dllexport) VOID WINAPI Cleanup()
{
    if (m_pSwapChain != NULL)
        m_pSwapChain->Release();

    if (dev != NULL)
        dev->Release();

    if (devcon != NULL)
        devcon->Release();

    if (m_pRenderTarget != NULL)
        m_pRenderTarget->Release();

    if (m_pLightSlateGrayBrush != NULL)
        m_pLightSlateGrayBrush->Release();

    if (m_pCornflowerBlueBrush != NULL)
        m_pCornflowerBlueBrush->Release();
}

//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform matrices.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
    // Set up world matrix
    D3DXMATRIXA16 matWorld;
    D3DXMatrixRotationY(&matWorld, timeGetTime() / 1000.0f);
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the 
    // origin, and define "up" to be in the y-direction.
    D3DXVECTOR3 vEyePt(0.0f, 3.0f, -5.0f);
    D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
extern "C" __declspec(dllexport) VOID WINAPI Render()
{
    // Clear the backbuffer and the zbuffer
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    // Begin the scene
    if (SUCCEEDED(g_pd3dDevice->BeginScene()))
    {
        // Setup the world, view, and projection matrices
        SetupMatrices();

        // Meshes are divided into subsets, one for each material. Render them in
        // a loop
        for (DWORD i = 0; i < g_dwNumMaterials; i++)
        {
            // Set the material and texture for this subset
            g_pd3dDevice->SetMaterial(&g_pMeshMaterials[i]);
            g_pd3dDevice->SetTexture(0, g_pMeshTextures[i]);

            // Draw the mesh subset
            g_pMesh->DrawSubset(i);
        }

        // End the scene
        g_pd3dDevice->EndScene();
    }
}