// Copyright Epic Games, Inc. All Rights Reserved.

#include "AsyncAction_CommonUserInitialize.h"

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_CommonUserInitialize)

// 工厂函数：为“本地单机游玩”创建并配置一个初始化异步节点（未传输入设备时用默认设备）
UAsyncAction_CommonUserInitialize* UAsyncAction_CommonUserInitialize::InitializeForLocalPlay(UCommonUserSubsystem* Target, int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin)
{
	if (!PrimaryInputDevice.IsValid())
	{
		// Set to default device
		PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
	}

	UAsyncAction_CommonUserInitialize* Action = NewObject<UAsyncAction_CommonUserInitialize>();

	Action->RegisterWithGameInstance(Target);

	if (Target && Action->IsRegistered())
	{
		Action->Subsystem = Target;
		
		Action->Params.RequestedPrivilege = ECommonUserPrivilege::CanPlay;
		Action->Params.LocalPlayerIndex = LocalPlayerIndex;
		Action->Params.PrimaryInputDevice = PrimaryInputDevice;
		Action->Params.bCanUseGuestLogin = bCanUseGuestLogin;
		Action->Params.bCanCreateNewLocalPlayer = true;
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

// 工厂函数：为“联机游玩”创建并配置一个登录异步节点
UAsyncAction_CommonUserInitialize* UAsyncAction_CommonUserInitialize::LoginForOnlinePlay(UCommonUserSubsystem* Target, int32 LocalPlayerIndex)
{
	UAsyncAction_CommonUserInitialize* Action = NewObject<UAsyncAction_CommonUserInitialize>();

	Action->RegisterWithGameInstance(Target);

	if (Target && Action->IsRegistered())
	{
		Action->Subsystem = Target;
		
		Action->Params.RequestedPrivilege = ECommonUserPrivilege::CanPlayOnline;
		Action->Params.LocalPlayerIndex = LocalPlayerIndex;
		Action->Params.bCanCreateNewLocalPlayer = false;
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

// 启动阶段就失败：下一帧以失败回调结束，保证蓝图节点不会永远挂起
void UAsyncAction_CommonUserInitialize::HandleFailure()
{
	const UCommonUserInfo* UserInfo = nullptr;
	if (Subsystem.IsValid())
	{
		UserInfo = Subsystem->GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex);
	}
	HandleInitializationComplete(UserInfo, false, NSLOCTEXT("CommonUser", "LoginFailedEarly", "Unable to start login process"), Params.RequestedPrivilege, Params.OnlineContext);
}

// 初始化结束（成功或失败）：广播一次结果后立刻标记节点可回收
void UAsyncAction_CommonUserInitialize::HandleInitializationComplete(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	if (ShouldBroadcastDelegates())
	{
		OnInitializationComplete.Broadcast(UserInfo, bSuccess, Error, RequestedPrivilege, OnlineContext);
	}

	SetReadyToDestroy();
}

// 节点激活入口：把完成回调绑到子系统上并真正发起初始化请求
void UAsyncAction_CommonUserInitialize::Activate()
{
	if (Subsystem.IsValid())
	{
		Params.OnUserInitializeComplete.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UAsyncAction_CommonUserInitialize, HandleInitializationComplete));
		bool bSuccess = Subsystem->TryToInitializeUser(Params);

		if (!bSuccess)
		{
			// Call failure next frame
			FTimerManager* TimerManager = GetTimerManager();
			
			if (TimerManager)
			{
				TimerManager->SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UAsyncAction_CommonUserInitialize::HandleFailure));
			}
		}
	}
	else
	{
		SetReadyToDestroy();
	}	
}

