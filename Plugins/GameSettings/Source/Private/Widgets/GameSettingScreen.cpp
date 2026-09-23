// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/GameSettingScreen.h"

#include "GameSettingCollection.h"
#include "Widgets/GameSettingPanel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingScreen)

class UWidget;

#define LOCTEXT_NAMESPACE "GameSetting"

void UGameSettingScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

// 界面激活：确保注册表已创建并开始跟踪改动。
void UGameSettingScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	ChangeTracker.WatchRegistry(Registry);

	OnSettingsDirtyStateChanged(HaveSettingsBeenChanged());
}

// 界面关闭：先尝试退出子页面；若已在顶层则取消所有未保存改动。
// 这样"直接关掉设置界面"就等于"放弃改动"，符合玩家预期。
void UGameSettingScreen::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
}

// 惰性创建注册表：第一次用到时才 CreateRegistry 并 Initialize。
UGameSettingRegistry* UGameSettingScreen::GetOrCreateRegistry()
{
	if (Registry == nullptr)
	{
		UGameSettingRegistry* NewRegistry = this->CreateRegistry();
		NewRegistry->OnSettingChangedEvent.AddUObject(this, &ThisClass::HandleSettingChanged);

		Settings_Panel->SetRegistry(NewRegistry);

		Registry = NewRegistry;
	}

	return Registry;
}

UWidget* UGameSettingScreen::NativeGetDesiredFocusTarget() const
{
	if (UWidget* Target = BP_GetDesiredFocusTarget())
	{
		return Target;
	}

	return Settings_Panel;
}

// 应用改动：交给 ChangeTracker 逐个落地，然后通知蓝图脏状态变为 false。
void UGameSettingScreen::ApplyChanges()
{
	if (ChangeTracker.HaveSettingsBeenChanged())
	{
		ChangeTracker.ApplyChanges();
		ClearDirtyState();
		Registry->SaveChanges();
	}
}

// 取消改动：交给 ChangeTracker 还原所有脏项到初始值。
void UGameSettingScreen::CancelChanges()
{
	ChangeTracker.RestoreToInitial();
	ClearDirtyState();
}

void UGameSettingScreen::ClearDirtyState()
{
	ChangeTracker.ClearDirtyState();

	OnSettingsDirtyStateChanged(false);
}

// 尝试退出一层子页面；已在最顶层时返回 false，交由调用方决定（例如关闭整个界面）。
bool UGameSettingScreen::AttemptToPopNavigation()
{
	if (Settings_Panel->CanPopNavigationStack())
	{
		Settings_Panel->PopNavigationStack();
		return true;
	}

	return false;
}

UGameSettingCollection* UGameSettingScreen::GetSettingCollection(FName SettingDevName, bool& HasAnySettings)
{
	HasAnySettings = false;
	
	if (UGameSettingCollection* Collection = GetRegistry()->FindSettingByDevNameChecked<UGameSettingCollection>(SettingDevName))
	{
		TArray<UGameSetting*> InOutSettings;
		
		FGameSettingFilterState FilterState;
		Collection->GetSettingsForFilter(FilterState, InOutSettings);

		HasAnySettings = InOutSettings.Num() > 0;
		
		return Collection;
	}

	return nullptr;
}

void UGameSettingScreen::NavigateToSetting(FName SettingDevName)
{
	NavigateToSettings({SettingDevName});
}

void UGameSettingScreen::NavigateToSettings(const TArray<FName>& SettingDevNames)
{
	FGameSettingFilterState FilterState;

	for (const FName SettingDevName : SettingDevNames)
	{
		if (UGameSetting* Setting = GetRegistry()->FindSettingByDevNameChecked<UGameSetting>(SettingDevName))
		{
			FilterState.AddSettingToRootList(Setting);
		}
	}
	
	Settings_Panel->SetFilterState(FilterState);
}

void UGameSettingScreen::HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason)
{
	OnSettingsDirtyStateChanged(true);
}

#undef LOCTEXT_NAMESPACE
