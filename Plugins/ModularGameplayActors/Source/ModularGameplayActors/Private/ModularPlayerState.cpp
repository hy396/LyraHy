// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModularPlayerState.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Components/PlayerStateComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularPlayerState)

// 把自己登记为“组件接收者”。
// 只有登记过的 Actor，GameFeature 插件才能通过 UGameFrameworkComponentManager 往它身上动态挂组件。
// 必须早于“Actor 就绪”事件派发，否则组件会挂不上去。
void AModularPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// 派发 NAME_GameActorReady 事件，通知所有监听者“本 Actor 已就绪，可以挂组件了”。
// 注意特意放在 Super::BeginPlay() 之前：确保组件在基类 BeginPlay 逻辑执行前就已就位。
void AModularPlayerState::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

// 注销“组件接收者”身份，清掉 GameFrameworkComponentManager 中对本 Actor 的引用，避免悬空。
void AModularPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

// 重置时把 Reset 转发给身上所有 UPlayerStateComponent，让它们各自清掉自己的状态。
void AModularPlayerState::Reset()
{
	Super::Reset();

	TArray<UPlayerStateComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UPlayerStateComponent* Component : ModularComponents)
	{
		Component->Reset();
	}
}

// 跨 PlayerState 拷贝属性：玩家重连 / 换座位时，把旧 PlayerState 的数据搬到新的上面。
void AModularPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	TInlineComponentArray<UPlayerStateComponent*> PlayerStateComponents;
	GetComponents(PlayerStateComponents);
	for (UPlayerStateComponent* SourcePSComp : PlayerStateComponents)
	{
		// 按“同类型 + 同名”在目标 PlayerState 上找到与源组件配对的那个组件，再逐个拷贝。
		if (UPlayerStateComponent* TargetComp = Cast<UPlayerStateComponent>(static_cast<UObject*>(FindObjectWithOuter(PlayerState, SourcePSComp->GetClass(), SourcePSComp->GetFName()))))
		{
			SourcePSComp->CopyProperties(TargetComp);
		}
	}
}
