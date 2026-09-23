// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * AsyncAction_CommonUserInitialize.h
 *
 * 把“用户初始化/登录”包装成蓝图可直接 await 的异步节点，
 * 内部只是转发给 UCommonUserSubsystem::TryToInitializeUser。
 */
#include "CommonUserSubsystem.h"
#include "Engine/CancellableAsyncAction.h"

#include "AsyncAction_CommonUserInitialize.generated.h"

enum class ECommonUserOnlineContext : uint8;
enum class ECommonUserPrivilege : uint8;
struct FInputDeviceId;

class FText;
class UObject;
struct FFrame;

/**
 * Async action to handle different functions for initializing users
 */
/**
 * 用户初始化异步动作（UCancellableAsyncAction）。
 * 蓝图里调用 InitializeForLocalPlay / LoginForOnlinePlay 拿到节点，
 * 绑定 OnInitializationComplete 即可等待结果。
 */
UCLASS()
class COMMONUSER_API UAsyncAction_CommonUserInitialize : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	// 为本地单机游玩初始化：可新建 LocalPlayer，可使用访客登录
	/**
	 * Initializes a local player with the common user system, which includes doing platform-specific login and privilege checks.
	 * When the process has succeeded or failed, it will broadcast the OnInitializationComplete delegate.
	 *
	 * @param LocalPlayerIndex	Desired index of ULocalPlayer in Game Instance, 0 will be primary player and 1+ for local multiplayer
	 * @param PrimaryInputDevice Primary input device for the user, if invalid will use the system default
	 * @param bCanUseGuestLogin	If true, this player can be a guest without a real system net id
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncAction_CommonUserInitialize* InitializeForLocalPlay(UCommonUserSubsystem* Target, int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	// 为联机游玩登录：不会新建 LocalPlayer
	/**
	 * Attempts to log an existing user into the platform-specific online backend to enable full online play
	 * When the process has succeeded or failed, it will broadcast the OnInitializationComplete delegate.
	 *
	 * @param LocalPlayerIndex	Index of existing LocalPlayer in Game Instance
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncAction_CommonUserInitialize* LoginForOnlinePlay(UCommonUserSubsystem* Target, int32 LocalPlayerIndex);

	// 初始化成功或失败都会广播这个多播委托
	/** Call when initialization succeeds or fails */
	UPROPERTY(BlueprintAssignable)
	FCommonUserOnInitializeCompleteMulticast OnInitializationComplete;

	// 立刻以失败结束并按需触发回调
	/** Fail and send callbacks if needed */
	void HandleFailure();

	// 转发回调：只在 ShouldBroadcastDelegates 为真时才真正广播
	/** Wrapper delegate, will pass on to OnInitializationComplete if appropriate */
	UFUNCTION()
	virtual void HandleInitializationComplete(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);

protected:
	// 真正启动初始化（异步动作框架在节点激活时调用）
	/** Actually start the initialization */
	virtual void Activate() override;

	// 弱引用目标子系统，避免异步期间子系统被销毁后悬空
	TWeakObjectPtr<UCommonUserSubsystem> Subsystem;
	// 本次初始化使用的完整参数
	FCommonUserInitializeParams Params;
};
