// JayH

#include "../Core/EngineMinimal.h"
#include "MyApplication.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	MyApplication theApp(hInstance);

	if ( theApp.Initialize() == false )
	{
		return 0;
	}

	return theApp.RunGameLoop();
}