#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite")

#include "pch.h"
#include "DxfDrawer.h"
#include <iostream>
#include <windowsx.h>
#include <cmath>
#include <d2d1_1.h>
#include <d2d1.h>
#include <dwrite.h>

POINT DxfDrawer::drag_start;
POINT DxfDrawer::pan_offset = {};
D2D1_MATRIX_3X2_F DxfDrawer::zoom_matrix = D2D1::Matrix3x2F::Identity();

int WINAPI WinMain(
	HINSTANCE /* hInstance */,
	HINSTANCE /* hPrevInstance */,
	LPSTR /* lpCmdLine */,
	int /* nCmdShow */
)
{
	// Use HeapSetInformation to specify that the process should
	// terminate if the heap manager detects an error in any heap used
	// by the process.
	// The return value is ignored, because we want to continue running in the
	// unlikely event that HeapSetInformation fails.
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			DxfDrawer app;

			if (SUCCEEDED(app.Initialize()))
			{
				app.RunMessageLoop();
			}
		}
		CoUninitialize();
	}

	return 0;
}

DxfDrawer::DxfDrawer() :
	m_hwnd(NULL),
	m_pDirect2dFactory(NULL),
	m_pRenderTarget(NULL),
	m_pLightSlateGrayBrush(NULL),
	m_pCornflowerBlueBrush(NULL),
	strokeStyleFixedThickness(NULL)
{}

DxfDrawer::~DxfDrawer()
{
	SafeRelease(&m_pDirect2dFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
}

void DxfDrawer::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

HRESULT DxfDrawer::Initialize()
{
	HRESULT hr;

	// Initialize device-independent resources, such
	// as the Direct2D factory.
	hr = CreateDeviceIndependentResources();

	if (SUCCEEDED(hr))
	{
		// Register the window class.
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = DxfDrawer::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"directX_2d_DxfViewer";

		RegisterClassEx(&wcex);

		m_hwnd = CreateWindowExA(
			WS_EX_APPWINDOW,
			"directX_2d_DxfViewer",
			"DXF Viewer",
			(WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION),
			0,
			0,
			SW_MAXIMIZE,
			SW_MAXIMIZE,
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this);

		if (m_hwnd) {
			ShowWindow(m_hwnd, 3);
		}
	}

	return hr;
}

HRESULT DxfDrawer::CreateDeviceIndependentResources()
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

HRESULT DxfDrawer::CreateDeviceResources()
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

void DxfDrawer::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
}

HRESULT DxfDrawer::Render()
{
	HRESULT hr = S_OK;

	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

		// Draw a grid background.
		int width = static_cast<int>(rtSize.width);
		int height = static_cast<int>(rtSize.height);

		ID2D1RectangleGeometry* m_pRectangle1;
		ID2D1RectangleGeometry* m_pRectangle2;

		m_pDirect2dFactory->CreateRectangleGeometry(
			D2D1::RectF(
				rtSize.width / 2 - 50.0f,
				rtSize.height / 2 - 50.0f,
				rtSize.width / 2 + 50.0f,
				rtSize.height / 2 + 50.0f
			),
			&m_pRectangle1);
		m_pDirect2dFactory->CreateRectangleGeometry(
			D2D1::RectF(
				rtSize.width / 2 - 100.0f,
				rtSize.height / 2 - 100.0f,
				rtSize.width / 2 + 100.0f,
				rtSize.height / 2 + 100.0f
			),
			&m_pRectangle2);

		// Draw a filled rectangle.
		m_pRenderTarget->FillGeometry(m_pRectangle1, m_pLightSlateGrayBrush);

		// Draw the outline of a rectangle.
		m_pRenderTarget->DrawGeometry(m_pRectangle2, m_pCornflowerBlueBrush, 2, strokeStyleFixedThickness);

		hr = m_pRenderTarget->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}

		return hr;
	}

}

void DxfDrawer::UpdateTransform()
{
	D2D1_MATRIX_3X2_F transform =
		zoom_matrix *
		D2D1::Matrix3x2F::Translation(pan_offset.x, pan_offset.y);

	m_pRenderTarget->SetTransform(transform);
}

void DxfDrawer::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here, because the error will be returned again
		// the next time EndDraw is called.
		m_pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}

LRESULT CALLBACK DxfDrawer::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		DxfDrawer* dxfDrawer = (DxfDrawer*)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(dxfDrawer)
		);

		result = 1;
	}
	else
	{
		DxfDrawer* dxfDrawer = reinterpret_cast<DxfDrawer*>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA
			)));

		bool wasHandled = false;

		if (dxfDrawer)
		{
			switch (message)
			{
			case WM_MOUSEMOVE:
			{
				static POINT lastMousePos;

				if (wParam & MK_MBUTTON)
				{
					POINT currentMousePos{};
					currentMousePos.x = GET_X_LPARAM(lParam);
					currentMousePos.y = GET_Y_LPARAM(lParam);

					pan_offset.x += static_cast<float>(currentMousePos.x - lastMousePos.x);
					pan_offset.y += static_cast<float>(currentMousePos.y - lastMousePos.y);
					dxfDrawer->UpdateTransform();

					lastMousePos = currentMousePos;
					InvalidateRect(hwnd, NULL, FALSE);  // Trigger redraw
				}
				else
				{
					lastMousePos.x = GET_X_LPARAM(lParam);
					lastMousePos.y = GET_Y_LPARAM(lParam);
				}

				result = 0;
				wasHandled = true;
				break;
			}

			case WM_MOUSEWHEEL:
			{
				int delta = GET_WHEEL_DELTA_WPARAM(wParam);
				POINT pos = { GET_X_LPARAM(lParam),  GET_Y_LPARAM(lParam) };

				POINT pos2;
				GetCursorPos(&pos2);
				ScreenToClient(hwnd, &pos2);

				float zoom = {};

				if (delta > 0) { zoom = 1.3; }
				else { zoom = (1 / 1.3); };

				D2D1::Matrix3x2F new_matrix = D2D1::Matrix3x2F::Scale(D2D1::Size(zoom, zoom),
					D2D1::Point2F((pos2.x - pan_offset.x), (pos2.y - pan_offset.y)));
				zoom_matrix = zoom_matrix * new_matrix;

				dxfDrawer->UpdateTransform();

				InvalidateRect(hwnd, NULL, FALSE);  // Trigger redraw

				result = 0;
				wasHandled = true;
				break;
			}
			case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				dxfDrawer->OnResize(width, height);
				result = 0;
				wasHandled = true;
				break;
			}

			case WM_DISPLAYCHANGE:
			{
				InvalidateRect(hwnd, NULL, FALSE);
				result = 0;
				wasHandled = true;
				break;
			}

			case WM_PAINT:
			{
				dxfDrawer->Render();
				ValidateRect(hwnd, NULL);
				result = 0;
				wasHandled = true;
				break;
			}

			case WM_DESTROY:
			{
				PostQuitMessage(0);
				result = 1;
				wasHandled = true;
				break;
			}
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}
	return result;
}