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
	// COM 인터페이스로 생성된 자원들은 반드시 Release함수로 자원을 해제해야한다.
	ReleaseCOM(pRenderTargetView);
	ReleaseCOM(pDepthStencilView);
	ReleaseCOM(pSwapChain);
	ReleaseCOM(pDepthStencilBuffer);

	// 문맥 정보를 모두 초기화한다.
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
	// 윈도우를 생성하기 위해 WNDCLASS 구조체를 채운다.
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

	// 다음으로, 이 WNDCLASS 인스턴스('창 클래스')를 Windows에 등록한다.
	// 그래야 다음 단계에서 이 창 클래스에 기초해서 창을 생성할 수 있다.
	if ( !RegisterClass(&wc) )
	{
		MessageBox(0, L"RegisterClass FAILED", 0, 0);

		return false;
	}

	// WNDCLASS 인스턴스가 성공적으로 등록되었다면 CreateWIndow 함수로 창을 생성할 수 있다.
	// 이 함수는 성공 시에는 생성된 창의 핸들(HWND 타입의 값)을, 실패 시에는 값이 0인 핸들을 리턴한다.
	// 창 핸들은 특정 창을 지칭하는 데 쓰이는 값으로, Windows가 내부적으로 관리한다. 창을 다루는 Win32 API 함수 중에는
	// 자신이 작업할 창을 식별하기 위해 이 HWND 값을 받는 것들이 많다.

	RECT rect = { 0, 0, mScreenWidth, mScreenHeight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	mHandleMainWindow = CreateWindow(
		L"BasicWndClass",			// 사용할 창 클래스의 이름 ( 앞에서 등록했던 것 )
		L"Win32Basic",				// 창의 제목
		WS_OVERLAPPEDWINDOW,		// 스타일 플래그들
		CW_USEDEFAULT,				// 창 위치의 x 좌표성분
		CW_USEDEFAULT,				// 창 위치의 y 좌표성분
		width,						// 창의 너비
		height,						// 창의 높이
		0,							// 부모 창 핸들
		0,							// 메뉴 핸들
		mHandleAppInstance,			// 응용 프로그램 인스턴스 핸들
		0);							// 추가 생성 플래그들

	if ( mHandleMainWindow == 0 )
	{
		MessageBox(0, L"CreateWindow FAILED", 0, 0);

		return false;
	}

	// 윈도우가 생성되어도 바로 화면에 나타나지는 않는다. 생성한 윈도우를 실제로 화면에 표시하고 갱신하기 위해서는
	// 다음 두 함수를 호출해주어야 한다. 두 함수 모두, 표시 또는 갱신할 창의 핸들을 받는다. 
	// 그 핸들은 함수가 표시 또는 갱신할 창이 무엇인지 알려주는 역할을 한다.
	ShowWindow(mHandleMainWindow, SW_SHOW);
	UpdateWindow(mHandleMainWindow);

	return true;
}


bool Application::InitializeDirect3D()
{
	/* --------------------------------------------------------------------------------------------------------------------- */
	/// 디바이스 장치와 디바이스 문맥장치를 생성한다.
	/* --------------------------------------------------------------------------------------------------------------------- */

	// 디바이스를 만들때 인수로 전달될 플래그 값이다.
	// 플래그에 D3D11_CREATE_DEVICE_DEBUG를 지정하면 출력창에 디버그 메시지를 받을수 있다.
	// 따라서 디버그모드로 빌드할때에는 추가적인 정보를 얻기위해 매크로분기로 플래그를 전달한다.
	UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hResult = D3D11CreateDevice(
		/* pAdapter = */ nullptr,								// 디스플레이 어댑터를 지정한다. nullptr값이면 기본 디스플레이어가 사용된다.
		/* DriverType = */ mD3DDriverType,					// 드라이버타입을 지정한다. 하드웨어 그래픽 가속을 사용한다.
		/* Software = */ 0,									// 드라이버타입을 소프트웨어로 지정할때 소프트웨어 구동기를 지정한다.
		/* Flags = */ createDeviceFlags,						// 추가적인 장치 생성 플래그를 지정한다.
		/* pFeatureLevels = */ nullptr,						// 기능레벨을 지정한다. nullptr를 건네면 최고레벨이 지정된다.
		/* FeatureLevels = */ 0,								// 기능레벨을 인덱스로 직접지정한다. pFeatureLevels에 배열을 전달될때만 사용한다.
		/* SDKVersion = */ D3D11_SDK_VERSION,					// 항상 D3D11_SDK_VERSION으로 지정한다.
		/* **ppDevice = */ OUT &pD3DDevice,					// 생성한 디바이스를 돌려준다.
		/* *pFeatureLevel = */ OUT &featureLevel,				// pFeatureLevels 배열에서 처음으로 지원되는 기능을 돌려준다.
		/* **ppImmediateContext */ OUT &pD3DImmediateContext	// 생성된 장치문맥을 돌려준다.
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

	// Direct3D 11에 대응하는 모든 그래픽 하드웨어는 MSAA(다중샘플링AA)를 지원한다.
	// 여기서는 4X MSAA가 지원한다고 가정하고 품질 수준을 가져온다.
	hResult = pD3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);

	if ( FAILED(hResult) )
	{
		MessageBox(0, L"4X MSAA unsupported.", 0, 0);
	}

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// 스왑체인을 생성하기 위해 스왑체인의 특성을 설정하는 구조체를 설정한다.
	/* --------------------------------------------------------------------------------------------------------------------- */

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = mScreenWidth;										// 후면 버퍼의 너비
	swapChainDesc.BufferDesc.Height = mScreenHeight;									// 후면 버퍼의 높이
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;								// 디스플레이 모드 갱신율(분자)
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;								// 디스플레이 모드 갱신율(분모)
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						// 후면 버퍼 픽셀 형식
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// 디스플레이 스캔라인 모드
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					// 디스플레이 비례 모드

	// 4X MSAA를 사용하기에 따라서 설정을 달리한다.
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

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 버퍼의 사용용도를 지정한다.
	swapChainDesc.BufferCount = 1;									// 후면버퍼의 개수를 지정한다.
	swapChainDesc.OutputWindow = mHandleMainWindow;					// 후면버퍼를 그릴 윈도우의 핸들값을 지정한다.
	swapChainDesc.Windowed = true;									// 윈도우모드를 설정한다.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			// 교환 효과를 서술한다.
	swapChainDesc.Flags = 0;										// 추가적인 플래그를 지정한다.

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// 설정된 스왑체인의 설정을 가지고 스왑체인을 생성한다.
	/* --------------------------------------------------------------------------------------------------------------------- */

	IDXGIDevice* pDXGIDevice = nullptr;
	pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);

	IDXGIAdapter* pDXGIAdapter = nullptr;
	pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);

	IDXGIFactory* pDXGIFactory = nullptr;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDXGIFactory);

	pDXGIFactory->CreateSwapChain(pD3DDevice, &swapChainDesc, OUT &pSwapChain);

	// 팩토리 인스턴스로 스왑체인을 생성했으면 팩토리 인스턴스를 생성하기 위한 자원들을 해제해줘야한다.
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

	// 기존에 있던 자원들을 해제한다. 크기를 다시 조정하거나 설정값을 바꾸려면 새로만들어야할 필요가 있기 때문이다.
	ReleaseCOM(pRenderTargetView);
	ReleaseCOM(pDepthStencilView);
	ReleaseCOM(pDepthStencilBuffer);

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// 스왑체인의 사이즈를 재조정하고 랜더타겟뷰를 재생성한다.
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

	// GetBuffer를 호출하면 스왑체인이 가르키는 후면버퍼에 대한 참조가 증가하는데
	// 여기서 릴리즈시키는 이유는 참조 횟수를 -1 감소하여 메모리가 제대로 해졔되기 위함이다.
	ReleaseCOM(pBackBuffer);

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// 깊이 - 스텐실 버퍼를 만들기 위한 구조체 정보를 설정한다.
	/* --------------------------------------------------------------------------------------------------------------------- */

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = mScreenWidth;
	depthStencilDesc.Height = mScreenHeight;
	depthStencilDesc.MipLevels = 1;			// 깊이-스텐실 버퍼는 밉맵이 하나만 존재한다.
	depthStencilDesc.ArraySize = 1;			// 깊이-스텐실 버퍼의 경우에는 텍스처의 개수가 하나만 필요하다.
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 4X MSAA를 사용하기에 따라서 설정을 달리한다.
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

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;			// 깊이-스텐실 버퍼는 GPU만 읽기 쓰기 권한을 가진다.
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// 자원을 파이프라인에 어떤 식으로 묶을지 플래그를 지정한다.
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// 설정된 정보로 깊이 - 스텐실 버퍼와 뷰를 만들고 파이프라인에 바인드한다.
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

	// 파이프라인에 깊이-스텐실 자원뷰를 OM(출력병합기)에 바인드한다.
	pD3DImmediateContext->OMSetRenderTargets(
		1,						// 묶고자 하는 랜더 대상의 개수이다. 랜더 대상을 여럿 묶을수 있다.
		&pRenderTargetView,		// 파이프라인에 묶을 랜더 대상 뷰들을 가리키는 포인터들을 담은 배열의 첫 원소를 가리키는 포인터다.
		pDepthStencilView		// 파이프라인에 묶을 깊이-스텐실 뷰를 가리키는 포인터이다.
	);

	/* --------------------------------------------------------------------------------------------------------------------- */
	/// 뷰포트를 설정한다.
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