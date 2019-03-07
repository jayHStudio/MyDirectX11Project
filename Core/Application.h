// JayH

#pragma once

#include <string>
#include "GameTimer.h"


class Application
{

	/* -------------------------------------------------------------------------------- */

public:
	Application(HINSTANCE hInstance);
	virtual ~Application();

	virtual bool Initialize();
	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene() = 0;

	inline HINSTANCE GetAppInstanceHandle() const { return mHandleAppInstance; }
	inline HWND GetMainWindowHandle() const { return mHandleMainWindow; }
	inline float GetAspectRatio() const { return static_cast<float>(mScreenWidth) / mScreenHeight; }
	
	virtual void OnResize();

	virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	int RunGameLoop();

protected:
	void CalculateFrameStats();

private:
	bool InitializeMainWindow();
	bool InitializeDirect3D();

	/* -------------------------------------------------------------------------------- */

protected:
	HINSTANCE mHandleAppInstance = 0;
	HWND mHandleMainWindow = 0;
	bool bAppPaused = false;
	bool bMinimized = false;
	bool bMaximized = false;
	bool bResizing = false;
	UINT m4xMsaaQuality = 0;
	GameTimer mTimer;

	ID3D11Device* pD3DDevice = nullptr;
	ID3D11DeviceContext* pD3DImmediateContext = nullptr;
	IDXGISwapChain* pSwapChain = nullptr;
	ID3D11Texture2D* pDepthStencilBuffer = nullptr;
	ID3D11RenderTargetView* pRenderTargetView = nullptr;
	ID3D11DepthStencilView* pDepthStencilView = nullptr;
	D3D11_VIEWPORT mScreenViewport;

	std::wstring mMainWindowCaption = L"D3D11 Application";
	D3D_DRIVER_TYPE mD3DDriverType = D3D_DRIVER_TYPE_HARDWARE;
	int mScreenWidth = 800;
	int mScreenHeight = 600;
	bool bEnable4xMsaa = false;

	/* -------------------------------------------------------------------------------- */

};