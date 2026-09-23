// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonPreLoadScreen.h"

#include "Misc/App.h"
#include "SCommonPreLoadingScreenWidget.h"

#define LOCTEXT_NAMESPACE "CommonPreLoadingScreen"

// 只在【非编辑器】且【能渲染】时才创建控件——命令行工具、专用服务器不需要画面。
void FCommonPreLoadScreen::Init()
{
	if (!GIsEditor && FApp::CanEverRender())
	{
		EngineLoadingWidget = SNew(SCommonPreLoadingScreenWidget);
	}
}

#undef LOCTEXT_NAMESPACE
