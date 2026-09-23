// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"

#include "CommonMessagingSubsystem.generated.h"

class FSubsystemCollectionBase;
class UCommonGameDialogDescriptor;
class UObject;

/** Possible results from a dialog */
/** 对话框的结果类型。 */
UENUM(BlueprintType)
enum class ECommonMessagingResult : uint8
{
	/** 玩家点了"是 / 确认"。 */
	Confirmed,
	/** 玩家点了"否"。 */
	Declined,
	/** 玩家点了"取消 / 忽略"。 */
	Cancelled,
	/** 对话框被代码强制关掉，玩家并未做出选择。 */
	Killed,
	Unknown UMETA(Hidden)
};

DECLARE_DELEGATE_OneParam(FCommonMessagingResultDelegate, ECommonMessagingResult /* Result */);

/**
 * UCommonMessagingSubsystem —— 对话框子系统（LocalPlayer 级）。
 *
 * 它是"弹确认框"这件事的统一入口：调用方只管构造一个描述对象丢进来，
 * 至于具体长什么样、推到哪个图层，由子系统内部（以及配置里的对话框类）决定。
 * 因为是 LocalPlayer 级子系统，分屏时每位玩家各自弹各自的框。
 */
UCLASS(config = Game)
class COMMONGAME_API UCommonMessagingSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UCommonMessagingSubsystem() { }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** 弹出确认对话框，玩家选择后通过回调返回结果。 */
	virtual void ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate());
	/** 弹出错误对话框（与 ShowConfirmation 走同一套机制，只是语义不同，便于 UI 区分样式）。 */
	virtual void ShowError(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate());

private:

};
