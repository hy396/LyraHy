// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSettingRegistry.h"

#include "GameSettingCollection.h"
#include "GameSettingAction.h"
#include "UObject/WeakObjectPtr.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingRegistry)

#define LOCTEXT_NAMESPACE "GameSetting"

//--------------------------------------
// UGameSettingRegistry
//--------------------------------------

UGameSettingRegistry::UGameSettingRegistry()
{
}

// 初始化：记下宿主玩家，然后调用子类的 OnInitialize —— 设置树就是在这里构建起来的。
void UGameSettingRegistry::Initialize(ULocalPlayer* InLocalPlayer)
{
	OwningLocalPlayer = InLocalPlayer;
	OnInitialize(InLocalPlayer);

	//UGameFeaturesSubsystem
}

// 重新生成整棵设置树：先清空已注册的项，再走一次 OnInitialize。
// 常用于玩家身份/平台特性变化后，设置项集合需要整体刷新时。
void UGameSettingRegistry::Regenerate()
{
	for (UGameSetting* Setting : RegisteredSettings)
	{
		Setting->MarkAsGarbage();
	}
	RegisteredSettings.Reset();
	TopLevelSettings.Reset();

	OnInitialize(OwningLocalPlayer);
}

// 是否所有设置项都已完成异步初始化（全部 IsReady）。界面据此决定何时可以显示。
bool UGameSettingRegistry::IsFinishedInitializing() const
{
	bool bReady = true;
	for (UGameSetting* Setting : RegisteredSettings)
	{
		if (!Setting->IsReady())
		{
			bReady = false;
			break;
		}
	}

	return bReady;
}

// 保存：把所有设置项逐个 Apply，让它们把值真正写到游戏/平台配置里。
void UGameSettingRegistry::SaveChanges()
{

}

// 按过滤状态收集可见设置项：从顶层项开始递归。
void UGameSettingRegistry::GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings)
{
	TArray<UGameSetting*> RootSettings;
	if (FilterState.GetSettingRootList().Num() > 0)
	{
		RootSettings.Append(FilterState.GetSettingRootList());
	}
	else
	{
		RootSettings.Append(TopLevelSettings);
	}

	for (UGameSetting* TopLevelSetting : RootSettings)
	{
		if (const UGameSettingCollection* TopLevelCollection = Cast<UGameSettingCollection>(TopLevelSetting))
		{
			TopLevelCollection->GetSettingsForFilter(FilterState, InOutSettings);
		}
		else
		{
			if (FilterState.DoesSettingPassFilter(*TopLevelSetting))
			{
				InOutSettings.Add(TopLevelSetting);
			}
		}
	}
}

// 按 DevName 在"已注册全量表"里查找（含各级子项，不限于顶层）。
UGameSetting* UGameSettingRegistry::FindSettingByDevName(const FName& SettingDevName)
{
	for (UGameSetting* Setting : RegisteredSettings)
	{
		if (Setting->GetDevName() == SettingDevName)
		{
			return Setting;
		}
	}

	return nullptr;
}

// 注册一个顶层设置项：加入顶层列表，并递归注册它的所有内部子项。
void UGameSettingRegistry::RegisterSetting(UGameSetting* InSetting)
{
	if (InSetting)
	{
		TopLevelSettings.Add(InSetting);
		InSetting->SetRegistry(this);
		RegisterInnerSettings(InSetting);
	}
}

// 递归注册子项：把设置项及其后代全部加进 RegisteredSettings，并绑定变更回调。
// 注意 UGameSettingAction 内部也可能藏着不直接显示的设置项，所以要用 GetChildSettings 递归。
void UGameSettingRegistry::RegisterInnerSettings(UGameSetting* InSetting)
{
	InSetting->OnSettingChangedEvent.AddUObject(this, &ThisClass::HandleSettingChanged);
	InSetting->OnSettingAppliedEvent.AddUObject(this, &ThisClass::HandleSettingApplied);
	InSetting->OnSettingEditConditionChangedEvent.AddUObject(this, &ThisClass::HandleSettingEditConditionsChanged);

	// Not a fan of this, but it makes sense to aggregate action events for simplicity.
	if (UGameSettingAction* ActionSetting = Cast<UGameSettingAction>(InSetting))
	{
		ActionSetting->OnExecuteNamedActionEvent.AddUObject(this, &ThisClass::HandleSettingNamedAction);
	}
	// Not a fan of this, but it makes sense to aggregate navigation events for simplicity.
	else if (UGameSettingCollectionPage* NewPageCollection = Cast<UGameSettingCollectionPage>(InSetting))
	{
		NewPageCollection->OnExecuteNavigationEvent.AddUObject(this, &ThisClass::HandleSettingNavigation);
	}

#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!RegisteredSettings.Contains(InSetting), TEXT("This setting has already been registered!"));
	ensureAlwaysMsgf(nullptr == RegisteredSettings.FindByPredicate([&InSetting](UGameSetting* ExistingSetting) { return InSetting->GetDevName() == ExistingSetting->GetDevName(); }), TEXT("A setting with this DevName has already been registered!  DevNames must be unique within a registry."));
#endif

	RegisteredSettings.Add(InSetting);

	for (UGameSetting* ChildSetting : InSetting->GetChildSettings())
	{
		RegisterInnerSettings(ChildSetting);
	}
}

// 转发：某个设置项被应用了（子类可重写 OnSettingApplied 做额外处理，如写存档）。
void UGameSettingRegistry::HandleSettingApplied(UGameSetting* Setting)
{
	OnSettingApplied(Setting);
}

// 转发：某个设置项发生变化 —— 这是 UI 显示"有未保存改动"的信号来源。
void UGameSettingRegistry::HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason)
{
	OnSettingChangedEvent.Broadcast(Setting, Reason);
}

void UGameSettingRegistry::HandleSettingEditConditionsChanged(UGameSetting* Setting)
{
	OnSettingEditConditionChangedEvent.Broadcast(Setting);
}

void UGameSettingRegistry::HandleSettingNamedAction(UGameSetting* Setting, FGameplayTag GameSettings_Action_Tag)
{
	OnSettingNamedActionEvent.Broadcast(Setting, GameSettings_Action_Tag);
}

// 转发：请求导航进入某个设置项的子页面。
void UGameSettingRegistry::HandleSettingNavigation(UGameSetting* Setting)
{
	OnExecuteNavigationEvent.Broadcast(Setting);
}

#undef LOCTEXT_NAMESPACE

