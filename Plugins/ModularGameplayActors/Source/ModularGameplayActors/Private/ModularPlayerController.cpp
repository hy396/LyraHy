// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModularPlayerController.h"

#include "Components/ControllerComponent.h"
#include "Components/GameFrameworkComponentManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularPlayerController)

// 把自己登记为“组件接收者”。
// 只有登记过的 Actor，GameFeature 插件才能通过 UGameFrameworkComponentManager 往它身上动态挂组件。
// 必须早于“Actor 就绪”事件派发，否则组件会挂不上去。
void AModularPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// 注销“组件接收者”身份，清掉 GameFrameworkComponentManager 中对本 Actor 的引用，避免悬空。
void AModularPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

// PlayerController 必须等到真正分配了 Player 之后才可用，所以就绪事件放在这一步派发，
// 而不是像其他 Modular 类那样放在 BeginPlay。
void AModularPlayerController::ReceivedPlayer()
{
	// Player controllers always get assigned a player and can't do much until then
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::ReceivedPlayer();

	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->ReceivedPlayer();
	}
}

// 每帧把 Tick 转发给身上所有 UControllerComponent（这些组件通常由 GameFeature 动态挂上来）。
void AModularPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->PlayerTick(DeltaTime);
	}
}
