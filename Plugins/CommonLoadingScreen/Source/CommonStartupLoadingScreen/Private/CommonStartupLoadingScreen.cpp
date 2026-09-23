// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonPreLoadScreen.h"
#include "Misc/App.h"
#include "Modules/ModuleManager.h"
#include "PreLoadScreenManager.h"

#define LOCTEXT_NAMESPACE "FCommonLoadingScreenModule"

/**
 * FCommonStartupLoadingScreenModule —— 启动画面模块的入口。
 *
 * 本模块的加载阶段是 PreLoadingScreen（极早期），因此它能赶在引擎初始化完之前
 * 就把启动画面注册上去。
 */
class FCommonStartupLoadingScreenModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	bool IsGameModule() const override;

private:
	/** PreLoadScreenManager 要清理时的回调：我们也跟着释放自己的资源。 */
	void OnPreLoadScreenManagerCleanUp();

	/** 持有的启动画面对象。 */
	TSharedPtr<FCommonPreLoadScreen> PreLoadingScreen;
};


// 注册启动画面。注意两处跳过条件：
//   1) 专用服务器不需要加载任何画面资源（但命令行工具不跳过，好让 Cook 能把资源收集进去）；
//   2) 编辑器里、或不能渲染的环境下也不注册。
void FCommonStartupLoadingScreenModule::StartupModule()
{
	// No need to load these assets on dedicated servers.
	// Still want to load them in commandlets so cook catches them
	if (!IsRunningDedicatedServer())
	{
		PreLoadingScreen = MakeShared<FCommonPreLoadScreen>();
		PreLoadingScreen->Init();

		if (!GIsEditor && FApp::CanEverRender() && FPreLoadScreenManager::Get())
		{
			FPreLoadScreenManager::Get()->RegisterPreLoadScreen(PreLoadingScreen);
			FPreLoadScreenManager::Get()->OnPreLoadScreenManagerCleanUp.AddRaw(this, &FCommonStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp);
		}
	}
}

// PreLoadScreenManager 开始清理，说明引擎已经起来了，我们这份早期画面可以功成身退。
void FCommonStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp()
{
	//Once the PreLoadScreenManager is cleaning up, we can get rid of all our resources too
	PreLoadingScreen.Reset();
	ShutdownModule();
}

void FCommonStartupLoadingScreenModule::ShutdownModule()
{

}

// 声明这是游戏模块（而非引擎模块），影响 UObject 反射的注册方式。
bool FCommonStartupLoadingScreenModule::IsGameModule() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCommonStartupLoadingScreenModule, CommonStartupLoadingScreen)
