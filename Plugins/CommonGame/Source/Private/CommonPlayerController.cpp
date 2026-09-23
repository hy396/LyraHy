// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonPlayerController.h"

#include "CommonLocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonPlayerController)

class APawn;

ACommonPlayerController::ACommonPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 分配到 Player 了：通知所属 LocalPlayer（触发 CallAndRegister_OnPlayerControllerSet 的回调）。
void ACommonPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	
	if (UCommonLocalPlayer* LocalPlayer = Cast<UCommonLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerControllerSet.Broadcast(LocalPlayer, this);

		if (PlayerState)
		{
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
		}
	}
}

// Pawn 变化：通知所属 LocalPlayer。
void ACommonPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (UCommonLocalPlayer* LocalPlayer = Cast<UCommonLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, InPawn);
	}
}

// 真正接管 Pawn：先走基类，再通知所属 LocalPlayer。
void ACommonPlayerController::OnPossess(APawn* APawn)
{
	Super::OnPossess(APawn);
	
	if (UCommonLocalPlayer* LocalPlayer = Cast<UCommonLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, APawn);
	}
}

// 不再接管 Pawn：清理并通知所属 LocalPlayer。
void ACommonPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	if (UCommonLocalPlayer* LocalPlayer = Cast<UCommonLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, nullptr);
	}
}

// PlayerState 在客户端上复制到位：通知所属 LocalPlayer。
void ACommonPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (PlayerState)
	{
		if (UCommonLocalPlayer* LocalPlayer = Cast<UCommonLocalPlayer>(Player))
		{
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
		}
	}
}
