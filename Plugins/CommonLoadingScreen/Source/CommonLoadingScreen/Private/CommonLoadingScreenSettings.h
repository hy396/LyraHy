// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "UObject/SoftObjectPath.h"

#include "CommonLoadingScreenSettings.generated.h"

class UObject;

/**
 * UCommonLoadingScreenSettings —— 加载界面的配置项。
 *
 * 它派生自 UDeveloperSettingsBackedByCVars，所以这些设置【同时也是控制台变量】，
 * 可以在运行时用控制台命令临时改，方便调试加载界面。
 * 在项目设置里的显示名是 "Common Loading Screen"。
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Common Loading Screen"))
class UCommonLoadingScreenSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UCommonLoadingScreenSettings();

public:
	
	/** 加载界面用哪个 UMG 控件类（软引用，按需加载）。 */
	UPROPERTY(config, EditAnywhere, Category=Display, meta=(MetaClass="/Script/UMG.UserWidget"))
	FSoftClassPath LoadingScreenWidget;

	/** 加载界面在视口中的层级（Z 序）。默认 10000，保证盖在所有游戏 UI 之上。 */
	UPROPERTY(config, EditAnywhere, Category=Display)
	int32 LoadingScreenZOrder = 10000;

	/** 加载完成后【额外多挂几秒】再收界面。
	 *  目的：给纹理流送一点时间，避免刚进关卡时满屏模糊贴图。
	 *  注意：编辑器里默认不生效（为了加快迭代），可用 HoldLoadingScreenAdditionalSecsEvenInEditor 打开。 */
	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s, ConsoleVariable="CommonLoadingScreen.HoldLoadingScreenAdditionalSecs"))
	float HoldLoadingScreenAdditionalSecs = 2.0f;

	/** 超过多少秒还没收掉，就认为加载界面"卡死"了并报警（0 表示不检测）。 */
	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
	float LoadingScreenHeartbeatHangDuration = 0.0f;

	/** 每隔多少秒打一次日志，说明"现在是谁在拖着加载界面不放"（0 表示不打）。
	 *  排查"加载界面卡住不退"时非常有用。 */
	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
	float LogLoadingScreenHeartbeatInterval = 5.0f;

	/** 每帧都打印"为什么显示/隐藏加载界面"。调试用，日志会非常吵。 */
	UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="CommonLoadingScreen.LogLoadingScreenReasonEveryFrame"))
	bool LogLoadingScreenReasonEveryFrame = 0;

	/** 强制一直显示加载界面（调试用）。 */
	UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="CommonLoadingScreen.AlwaysShow"))
	bool ForceLoadingScreenVisible = false;

	/** 编辑器里是否也应用"额外多挂几秒"（调试加载界面外观时打开）。 */
	UPROPERTY(Transient, EditAnywhere, Category=Debugging)
	bool HoldLoadingScreenAdditionalSecsEvenInEditor = false;

	/** 编辑器里是否也让加载管理器每帧 Tick（默认开）。
	 *  【注意】原版此处上方的注释是复制粘贴遗留，写的仍是"额外多挂几秒"，
	 *  与本项实际含义不符——本项管的是 Tick，不是延迟。 */
	UPROPERTY(config, EditAnywhere, Category=Configuration)
	bool ForceTickLoadingScreenEvenInEditor = true;
};

