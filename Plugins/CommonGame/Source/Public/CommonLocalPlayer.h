// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/LocalPlayer.h"

#include "CommonLocalPlayer.generated.h"

class APawn;
class APlayerController;
class APlayerState;
class FViewport;
class UObject;
class UPrimaryGameLayout;
struct FSceneViewProjectionData;

/**
 * UCommonLocalPlayer —— 增强版 LocalPlayer。
 *
 * 它解决一个很实际的问题：PlayerController / PlayerState / Pawn 并不是在
 * LocalPlayer 创建时就有的，而是随后陆续才分配上来。本类把它们各做一个委托，
 * 并提供 CallAndRegister_ 系列函数——如果已经有了就立刻回调一次，还没有就注册等着。
 * 这样调用方不用自己判断"现在到底有没有"。
 */
UCLASS(config=Engine, transient)
class COMMONGAME_API UCommonLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	UCommonLocalPlayer();

	/** 本玩家分配到 PlayerController 时广播。 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerControllerSetDelegate, UCommonLocalPlayer* LocalPlayer, APlayerController* PlayerController);
	FPlayerControllerSetDelegate OnPlayerControllerSet;

	/** 本玩家分配到 PlayerState 时广播。 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerStateSetDelegate, UCommonLocalPlayer* LocalPlayer, APlayerState* PlayerState);
	FPlayerStateSetDelegate OnPlayerStateSet;

	/** 本玩家分配到 Pawn 时广播。 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerPawnSetDelegate, UCommonLocalPlayer* LocalPlayer, APawn* Pawn);
	FPlayerPawnSetDelegate OnPlayerPawnSet;

	/** 若 PlayerController 已存在则【立刻】调用一次回调；否则注册，等分配到了再调用。
	 *  返回句柄用于之后取消注册。 */
	FDelegateHandle CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate);
	FDelegateHandle CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate);
	FDelegateHandle CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate);

public:
	virtual bool GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const override;

	/** 本玩家的视图当前是否参与渲染（分屏切换时会临时关掉某个玩家的视图）。 */
	bool IsPlayerViewEnabled() const { return bIsPlayerViewEnabled; }
	void SetIsPlayerViewEnabled(bool bInIsPlayerViewEnabled) { bIsPlayerViewEnabled = bInIsPlayerViewEnabled; }

	/** 取本玩家对应的根 UI 布局（由 UGameUIPolicy 创建并管理）。 */
	UPrimaryGameLayout* GetRootUILayout() const;

private:
	bool bIsPlayerViewEnabled = true;
};
