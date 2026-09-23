// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "LoadingProcessInterface.generated.h"

/**
 * ILoadingProcessInterface —— "我正在忙，别收加载界面"接口。
 *
 * 任何对象（组件、子系统、Actor）都可以实现它并注册到 ULoadingScreenManager。
 * 只要有一个实现者返回 true，加载界面就会一直挂着。
 * 这样各系统不需要互相知道对方，就能共同决定"什么时候才算加载完"。
 */
UINTERFACE(BlueprintType)
class COMMONLOADINGSCREEN_API ULoadingProcessInterface : public UInterface
{
	GENERATED_BODY()
};

class COMMONLOADINGSCREEN_API ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	/** 安全询问：先看对象是否实现了本接口，没实现就直接返回 false（不会崩）。
	 *  @param OutReason 当返回 true 时，写入"为什么还在加载"的说明文字（便于调试）。 */
	static bool ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason);

	/** 由实现者重写：返回 true 表示"我还在忙，别收加载界面"。 */
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const
	{
		return false;
	}
};
