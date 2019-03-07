// JayH

#include "../Core/EngineMinimal.h"
#include "../Core/Helper.h"
#include "MyApplication.h"


MyApplication::MyApplication(HINSTANCE hInstance)
	: Application(hInstance)
{

}


MyApplication::~MyApplication()
{

}


bool MyApplication::Initialize()
{
	if ( Application::Initialize() == false )
	{
		return false;
	}

	return true;
}


void MyApplication::UpdateScene(float deltaTime)
{

}


void MyApplication::DrawScene()
{
	assert(pD3DImmediateContext);
	assert(pSwapChain);

	pD3DImmediateContext->ClearRenderTargetView(pRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	pD3DImmediateContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	pSwapChain->Present(0, 0);
}
