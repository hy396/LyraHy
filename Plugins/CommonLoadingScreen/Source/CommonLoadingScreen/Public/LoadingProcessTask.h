// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LoadingProcessInterface.h"
#include "UObject/Object.h"

#include "LoadingProcessTask.generated.h"

struct FFrame;

/**
 * ULoadingProcessTask —— 蓝图可用的"加载占位任务"。
 *
 * 想在蓝图里说"这段时间别收加载界面"，就创建一个它；事情做完后调 Unregister()。
 * 它会自动注册到 ULoadingScreenManager，省掉 C++ 里实现接口的麻烦。
 */
UCLASS(BlueprintType)
class COMMONLOADINGSCREEN_API ULoadingProcessTask : public UObject, public ILoadingProcessInterface
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject"))
	/** 创建并注册一个加载占位任务。参数是"为什么要占着加载界面"的说明文字。 */
	static ULoadingProcessTask* CreateLoadingScreenProcessTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason);

public:
	ULoadingProcessTask() { }

	/** 注销自己。事情做完后【必须】调用，否则加载界面永远不会消失。 */
	UFUNCTION(BlueprintCallable)
	void Unregister();

	/** 修改"为什么占着加载界面"的说明文字（调试日志里会看到）。 */
	UFUNCTION(BlueprintCallable)
	void SetShowLoadingScreenReason(const FString& InReason);

	/** 只要本任务还没被 Unregister，就一直返回 true。 */
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	
	/** 占着加载界面的原因文字。 */
	FString Reason;
};
