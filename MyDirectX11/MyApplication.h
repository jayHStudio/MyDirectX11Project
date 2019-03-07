// JayH
// 다이렉트X11을 공부하기 위한 연습장

#pragma once

#include "../Core/Application.h"


class MyApplication final : public Application
{

public:
	MyApplication(HINSTANCE hInstance);
	virtual ~MyApplication();

	bool Initialize() override;
	void UpdateScene(float deltaTime) override;
	void DrawScene() override;

private:

};