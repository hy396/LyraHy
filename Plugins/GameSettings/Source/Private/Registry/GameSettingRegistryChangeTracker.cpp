// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSettingRegistryChangeTracker.h"

#include "GameSettingRegistry.h"
#include "GameSettingValue.h"

#define LOCTEXT_NAMESPACE "GameSetting"

FGameSettingRegistryChangeTracker::FGameSettingRegistryChangeTracker()
{
}

FGameSettingRegistryChangeTracker::~FGameSettingRegistryChangeTracker()
{
	if (UGameSettingRegistry* StrongRegistry = Registry.Get())
	{
		StrongRegistry->OnSettingChangedEvent.RemoveAll(this);
	}
}

// 开始监听：先解绑旧的，再绑到新注册表的"设置项变更"事件上。
void FGameSettingRegistryChangeTracker::WatchRegistry(UGameSettingRegistry* InRegistry)
{
	ClearDirtyState();
	StopWatchingRegistry();

	if (Registry.Get() != InRegistry)
	{
		Registry = InRegistry;
		InRegistry->OnSettingChangedEvent.AddRaw(this, &FGameSettingRegistryChangeTracker::HandleSettingChanged);
	}
}

// 停止监听并解绑回调（析构时也会调用，防止回调到已销毁对象）。
void FGameSettingRegistryChangeTracker::StopWatchingRegistry()
{
	if (UGameSettingRegistry* StrongRegistry = Registry.Get())
	{
		StrongRegistry->OnSettingChangedEvent.RemoveAll(this);
		Registry.Reset();
	}
}

void FGameSettingRegistryChangeTracker::ClearDirtyState()
{
	ensure(!bRestoringSettings);
	if (bRestoringSettings)
	{
		return;
	}

	bSettingsChanged = false;
	DirtySettings.Reset();
}

// 应用所有脏项：逐个 Apply，然后刷新它们的初始值并清空脏列表。
// 刷新初始值这一步很关键——否则用户保存后再改再取消，会撤销到保存前的值。
void FGameSettingRegistryChangeTracker::ApplyChanges()
{
	for (auto Entry : DirtySettings)
	{
		if (UGameSettingValue* SettingValue = Cast<UGameSettingValue>(Entry.Value))
		{
			SettingValue->Apply();
			SettingValue->StoreInitial();
		}
	}

	ClearDirtyState();
}

// 还原所有脏项到打开界面时的初始值（相当于"取消"）。
// bRestoringSettings 标志用来避免还原动作本身又被记成新的脏项。
void FGameSettingRegistryChangeTracker::RestoreToInitial()
{
	ensure(!bRestoringSettings);
	if (bRestoringSettings)
	{
		return;
	}

	{
		TGuardValue<bool> LocalGuard(bRestoringSettings, true);
		for (auto Entry : DirtySettings)
		{
			if (UGameSettingValue* SettingValue = Cast<UGameSettingValue>(Entry.Value))
			{
				SettingValue->RestoreToInitial();
			}
		}
	}

	ClearDirtyState();
}

// 记录脏项。注意：处于还原过程中（bRestoringSettings）时不再记账，否则会死循环。
void FGameSettingRegistryChangeTracker::HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason)
{
	if (bRestoringSettings)
	{
		return;
	}

	bSettingsChanged = true;
	DirtySettings.Add(FObjectKey(Setting), Setting);
}

#undef LOCTEXT_NAMESPACE
