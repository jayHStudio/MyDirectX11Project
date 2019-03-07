// JayH

#include "EngineMinimal.h"
#include "Helper.h"
#include "Application.h"

#include <sstream>
#include <windowsx.h>

namespace
{
	Application* gApp = nullptr;
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return gApp->MsgProc(hWnd, msg, wParam, lParam);
}


Application::Application(HINSTANCE hInstance)
	: mHandleAppInstance(hInstance)
{
	gApp = this;
}


Application::~Application()
{
	// COM �������̽��� ������ �ڿ����� �ݵ�� Release�Լ��� �ڿ��� �����ؾ��Ѵ�.
	ReleaseCOM(pRenderTargetView);
	ReleaseCOM(pDepthStencilView);
	ReleaseCOM(pSwapChain);
	ReleaseCOM(pDepthStencilBuffer);

	// ���� ������ ��� �ʱ�ȭ�Ѵ�.
	if ( pD3DImmediateContext )
	{
		pD3DImmediateContext->ClearState();
	}

	ReleaseCOM(pD3DImmediateContext);
	ReleaseCOM(pD3DDevice);
}


bool Application::Initialize()
{
	if ( InitializeMainWindow() == false )
	{
		return false;
	}

	if ( InitializeDirect3D() == false )
	{
		return false;
	}

	return true;
}


bool Application::InitializeMainWindow()
{
	// �����츦 �����ϱ� ���� WNDCLASS ����ü�� ä���.
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mHandleAppInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"BasicWndClass";

	// ��������, �� WNDCLASS �ν��Ͻ�('â Ŭ����')�� Windows�� ����Ѵ�.
	// �׷��� ���� �ܰ迡�� �� â Ŭ������ �����ؼ� â�� ������ �� �ִ�.
	if ( !RegisterClass(&wc) )
	{
		MessageBox(0, L"RegisterClass FAILED", 0, 0);

		return false;
	}

	// WNDCLASS �ν��Ͻ��� ���������� ��ϵǾ��ٸ� CreateWIndow �Լ��� â�� ������ �� �ִ�.
	// �� �Լ��� ���� �ÿ��� ������ â�� �ڵ�(HWND Ÿ���� ��)��, ���� �ÿ��� ���� 0�� �ڵ��� �����Ѵ�.
	// â �ڵ��� Ư�� â�� ��Ī�ϴ� �� ���̴� ������, Windows�� ���������� �����Ѵ�. â�� �ٷ�� Win32 API �Լ� �߿���
	// �ڽ��� �۾��� â�� �ĺ��ϱ� ���� �� HWND ���� �޴� �͵��� ����.

	RECT rect = { 0, 0, mScreenWidth, mScreenHeight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	mHandleMainWindow = CreateWindow(
		L"BasicWndClass",			// ����� â Ŭ������ �̸� ( �տ��� ����ߴ� �� )
		L"Win32Basic",				// â�� ����
		WS_OVERLAPPEDWINDOW,		// ��Ÿ�� �÷��׵�
		CW_USEDEFAULT,				// â ��ġ�� x ��ǥ����
		CW_USEDEFAULT,				// â ��ġ�� y ��ǥ����
		width,						// â�� �ʺ�
		height,						// â�� ����
		0,							// �θ� â �ڵ�
		0,							// �޴� �ڵ�
		mHandleAppInstance,			// ���� ���α׷� �ν��Ͻ� �ڵ�
		0);							// �߰� ���� �÷��׵�

	if ( mHandleMainWindow == 0 )
	{
		MessageBox(0, L"CreateWindow FAILED", 0, 0);

		return false;
	}

	// �����찡 �����Ǿ �ٷ� ȭ�鿡 ��Ÿ������ �ʴ´�. ������ �����츦 ������ ȭ�鿡 ǥ���ϰ� �����ϱ� ���ؼ���
	// ���� �� �Լ��� ȣ�����־�� �Ѵ�. �� �Լ� ���, ǥ�� �Ǵ� ������ â�� �ڵ��� �޴´�. 
	// �� �ڵ��� �Լ��� ǥ�� �Ǵ� ������ â�� �������� �˷��ִ� ������ �Ѵ�.
	ShowWindow(mHandleMainWindow, SW_SHOW);
	UpdateWindow(mHandleMainWindow);

	return true;
}


bool Application::InitializeDirect3D()
{
	/* --------------------------------------------------------------------------------------------------------------------- */
	/// ����̽� ��ġ�� ����̽� ������ġ�� �����Ѵ�.
	/* --------------------------------------------------------------------------------------------------------------------- */

	// ����̽��� ���鶧 �μ��� ���޵� �÷��� ���̴�.
	// �÷��׿� D3D11_CREATE_DEVICE_DEBUG�� �����ϸ� ���â�� ����� �޽����� ������ �ִ�.
	// ���� ����׸��� �����Ҷ����� �߰����� ������ ������� ��ũ�κб�� �÷��׸� �����Ѵ�.
	UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hResult = D3D11CreateDevice(
		/* pAdapter = */ nullptr,								// ���÷��� ����͸� �����Ѵ�. nullptr���̸� �⺻ ���÷��̾ ���ȴ�.
		/* DriverType = */ mD3DDriverType,					// ����̹�Ÿ���� �����Ѵ�. �ϵ���� �׷��� ������ ����Ѵ�.
		/* Software = */ 0,									// ����̹�Ÿ���� ����Ʈ����� �����Ҷ� ����Ʈ���� �����⸦ �����Ѵ�.
		/* Flags = */ createDeviceFlags,						// �߰����� ��ġ ���� �÷��׸� �����Ѵ�.
		/* pFeatureLevels = */ nullptr,						// ��ɷ����� �����Ѵ�. nullptr�� �ǳ׸� �ְ����� �����ȴ�.
		/* FeatureLevels = */ 0,								// ��ɷ����� �ε����� ���������Ѵ�. pFeatureLevels�� �迭�� ���޵ɶ��� ����Ѵ�.
		/* SDKVersion = */ D3D11_SDK_VERSION,					// �׻� D3D11_SDK_VERSION���� �����Ѵ�.
		/* **ppDevice = */ OUT &pD3DDevice,					// ������ ����̽��� �����ش�.
		/* *pFeatureLevel = */ OUT &featureLevel,				// pFeatureLevels �迭���� ó������ �����Ǵ� ����� �����ش�.
		/* **ppImmediateContext */ OUT &pD3DImmediateContext	// ������ ��ġ������ �����ش�.
	);

	if ( FAILED(hResult) )
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if ( featureLevel != D3D_FEATURE_LEVEL_11_0 )
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	// Direct3D 11�� �����ϴ� ��� �׷��� �ϵ����� MSAA(���߻��ø�AA)�� �����Ѵ�.
	// ���⼭�� 4X MSAA�� �����Ѵٰ� �����ϰ� ǰ�� ������ �����´�.
	hResult = pD3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);

	if ( FAILED(hResult) )
	{
		MessageBox(0, L"4X MSAA unsupported.", 0, 0);
	}

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// ����ü���� �����ϱ� ���� ����ü���� Ư���� �����ϴ� ����ü�� �����Ѵ�.
	/* --------------------------------------------------------------------------------------------------------------------- */

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = mScreenWidth;										// �ĸ� ������ �ʺ�
	swapChainDesc.BufferDesc.Height = mScreenHeight;									// �ĸ� ������ ����
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;								// ���÷��� ��� ������(����)
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;								// ���÷��� ��� ������(�и�)
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						// �ĸ� ���� �ȼ� ����
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// ���÷��� ��ĵ���� ���
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					// ���÷��� ��� ���

	// 4X MSAA�� ����ϱ⿡ ���� ������ �޸��Ѵ�.
	if ( bEnable4xMsaa == true )
	{
		swapChainDesc.SampleDesc.Count = 4;
		swapChainDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else
	{
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// ������ ���뵵�� �����Ѵ�.
	swapChainDesc.BufferCount = 1;									// �ĸ������ ������ �����Ѵ�.
	swapChainDesc.OutputWindow = mHandleMainWindow;					// �ĸ���۸� �׸� �������� �ڵ鰪�� �����Ѵ�.
	swapChainDesc.Windowed = true;									// �������带 �����Ѵ�.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			// ��ȯ ȿ���� �����Ѵ�.
	swapChainDesc.Flags = 0;										// �߰����� �÷��׸� �����Ѵ�.

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// ������ ����ü���� ������ ������ ����ü���� �����Ѵ�.
	/* --------------------------------------------------------------------------------------------------------------------- */

	IDXGIDevice* pDXGIDevice = nullptr;
	pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);

	IDXGIAdapter* pDXGIAdapter = nullptr;
	pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);

	IDXGIFactory* pDXGIFactory = nullptr;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDXGIFactory);

	pDXGIFactory->CreateSwapChain(pD3DDevice, &swapChainDesc, OUT &pSwapChain);

	// ���丮 �ν��Ͻ��� ����ü���� ���������� ���丮 �ν��Ͻ��� �����ϱ� ���� �ڿ����� ����������Ѵ�.
	ReleaseCOM(pDXGIDevice);
	ReleaseCOM(pDXGIAdapter);
	ReleaseCOM(pDXGIFactory);

	OnResize();

	return true;
}


void Application::OnResize()
{
	assert(pD3DDevice != nullptr);
	assert(pD3DImmediateContext != nullptr);
	assert(pSwapChain != nullptr);

	// ������ �ִ� �ڿ����� �����Ѵ�. ũ�⸦ �ٽ� �����ϰų� �������� �ٲٷ��� ���θ������� �ʿ䰡 �ֱ� �����̴�.
	ReleaseCOM(pRenderTargetView);
	ReleaseCOM(pDepthStencilView);
	ReleaseCOM(pDepthStencilBuffer);

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// ����ü���� ����� �������ϰ� ����Ÿ�ٺ並 ������Ѵ�.
	/* --------------------------------------------------------------------------------------------------------------------- */

	HRESULT hResult = pSwapChain->ResizeBuffers(1, mScreenWidth, mScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	if ( FAILED(hResult) )
	{
		MessageBox(0, L"ResizeBuffers Failed.", 0, 0);
	}

	ID3D11Texture2D* pBackBuffer;
	hResult = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), OUT reinterpret_cast<void**>(&pBackBuffer));

	if ( FAILED(hResult) )
	{
		MessageBox(0, L"GetBuffer Failed.", 0, 0);
	}

	hResult = pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, OUT &pRenderTargetView);

	if ( FAILED(hResult) )
	{
		MessageBox(0, L"CreateRenderTargetView Failed.", 0, 0);
	}

	// GetBuffer�� ȣ���ϸ� ����ü���� ����Ű�� �ĸ���ۿ� ���� ������ �����ϴµ�
	// ���⼭ �������Ű�� ������ ���� Ƚ���� -1 �����Ͽ� �޸𸮰� ����� �����Ǳ� �����̴�.
	ReleaseCOM(pBackBuffer);

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// ���� - ���ٽ� ���۸� ����� ���� ����ü ������ �����Ѵ�.
	/* --------------------------------------------------------------------------------------------------------------------- */

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = mScreenWidth;
	depthStencilDesc.Height = mScreenHeight;
	depthStencilDesc.MipLevels = 1;			// ����-���ٽ� ���۴� �Ӹ��� �ϳ��� �����Ѵ�.
	depthStencilDesc.ArraySize = 1;			// ����-���ٽ� ������ ��쿡�� �ؽ�ó�� ������ �ϳ��� �ʿ��ϴ�.
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 4X MSAA�� ����ϱ⿡ ���� ������ �޸��Ѵ�.
	if ( bEnable4xMsaa == true )
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;			// ����-���ٽ� ���۴� GPU�� �б� ���� ������ ������.
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// �ڿ��� ���������ο� � ������ ������ �÷��׸� �����Ѵ�.
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// ������ ������ ���� - ���ٽ� ���ۿ� �並 ����� ���������ο� ���ε��Ѵ�.
	/* --------------------------------------------------------------------------------------------------------------------- */

	hResult = pD3DDevice->CreateTexture2D(&depthStencilDesc, nullptr, OUT &pDepthStencilBuffer);

	if ( FAILED(hResult) )
	{
		MessageBox(0, L"Create DepthStencilBuffer Failed.", 0, 0);
	}

	hResult = pD3DDevice->CreateDepthStencilView(pDepthStencilBuffer, nullptr, OUT &pDepthStencilView);

	if ( FAILED(hResult) )
	{
		MessageBox(0, L"Create DepthStencilView Failed.", 0, 0);
	}

	// ���������ο� ����-���ٽ� �ڿ��並 OM(��º��ձ�)�� ���ε��Ѵ�.
	pD3DImmediateContext->OMSetRenderTargets(
		1,						// ������ �ϴ� ���� ����� �����̴�. ���� ����� ���� ������ �ִ�.
		&pRenderTargetView,		// ���������ο� ���� ���� ��� ����� ����Ű�� �����͵��� ���� �迭�� ù ���Ҹ� ����Ű�� �����ʹ�.
		pDepthStencilView		// ���������ο� ���� ����-���ٽ� �並 ����Ű�� �������̴�.
	);

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// ����Ʈ�� �����Ѵ�.
	/* --------------------------------------------------------------------------------------------------------------------- */

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mScreenWidth);
	mScreenViewport.Height = static_cast<float>(mScreenHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	pD3DImmediateContext->RSSetViewports(1, &mScreenViewport);
}


LRESULT Application::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{

		case WM_ACTIVATE:
		{
			if ( LOWORD(wParam) == WA_INACTIVE )
			{
				bAppPaused = true;
				mTimer.Stop();
			}
			else
			{
				bAppPaused = false;
				mTimer.Start();
			}

			return 0;
		}

		case WM_SIZE:
		{
			mScreenWidth = LOWORD(lParam);
			mScreenHeight = HIWORD(lParam);

			if ( pD3DDevice )
			{
				if ( wParam == SIZE_MINIMIZED )
				{
					bAppPaused = true;
					bMinimized = true;
					bMaximized = false;
				}
				else if ( wParam == SIZE_MAXIMIZED )
				{
					bAppPaused = false;
					bMinimized = false;
					bMaximized = true;
					OnResize();
				}
				else if ( wParam == SIZE_RESTORED )
				{
					if ( bMinimized )
					{
						bAppPaused = false;
						bMinimized = false;
						OnResize();
					}

					else if ( bMaximized )
					{
						bAppPaused = false;
						bMaximized = false;
						OnResize();
					}
					else if ( bResizing )
					{

					}
					else
					{
						OnResize();
					}
				}
			}
			return 0;
		}

		case WM_ENTERSIZEMOVE:
		{
			bAppPaused = true;
			bResizing = true;
			mTimer.Stop();

			return 0;
		}

		case WM_EXITSIZEMOVE:
		{
			bAppPaused = false;
			bResizing = false;
			mTimer.Start();
			OnResize();

			return 0;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);

			return 0;
		}

		case WM_MENUCHAR:
		{
			return MAKELRESULT(0, MNC_CLOSE);
		}

		case WM_GETMINMAXINFO:
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;

			return 0;
		}

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			return 0;
		}

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			return 0;
		}

		case WM_MOUSEMOVE:
		{
			OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


int Application::RunGameLoop()
{
	MSG msg = { 0 };

	mTimer.Reset();

	while ( msg.message != WM_QUIT )
	{
		if ( PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			mTimer.Tick();

			if ( bAppPaused == false )
			{
				CalculateFrameStats();
				UpdateScene(mTimer.GetDeltaTimeSeconds());
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}


void Application::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ( (mTimer.GetTotalTimeSeconds() - timeElapsed) >= 1.0f )
	{
		float fps = (float)frameCnt;
		float msPerFrame = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << mMainWindowCaption << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << msPerFrame << L" (ms)";
		SetWindowText(mHandleMainWindow, outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}