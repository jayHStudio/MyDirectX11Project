// JayH
// ���̷�ƮX11�� �����ϱ� ���� ������

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