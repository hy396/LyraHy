// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtrTemplates.h"

enum class EGameSettingChangeReason : uint8;

class UGameSetting;
class UGameSettingRegistry;
struct FObjectKey;

/**
 * FGameSettingRegistryChangeTracker —— 设置改动跟踪器。
 *
 * 它监听整个注册表，把所有被改过的项记进"脏列表"，从而支撑三个功能：
 *   - HaveSettingsBeenChanged()  → 界面上显示"有未保存的改动"
 *   - ApplyChanges()             → 保存（逐个 Apply，并把初始值刷新为新值）
 *   - RestoreToInitial()         → 取消（逐个还原到打开界面时的初始值）
 */
class GAMESETTINGS_API FGameSettingRegistryChangeTracker : public FNoncopyable
{
public:
	FGameSettingRegistryChangeTracker();
	~FGameSettingRegistryChangeTracker();

	/** 开始监听某个注册表（先停掉上一个）。 */
	void WatchRegistry(UGameSettingRegistry* InRegistry);

	/** 停止监听并解绑回调。 */
	void StopWatchingRegistry();

	/** 应用（保存）所有被改动的项，并把它们的初始值刷新为当前值。 */
	void ApplyChanges();

	/** 还原所有被改动的项到打开界面时的初始值（相当于"取消"）。 */
	void RestoreToInitial();

	/** 清空脏标记，但不还原任何值（用于保存成功后复位状态）。 */
	void ClearDirtyState();

	/** 是否正在批量还原中（用于避免还原过程中把改动又记成新的脏项）。 */
	bool IsRestoringSettings() const { return bRestoringSettings; }

	/** 自上次保存/清空以来，是否有设置项被改过。 */
	bool HaveSettingsBeenChanged() const { return bSettingsChanged; }

private:
	void HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason);

	bool bSettingsChanged = false;
	bool bRestoringSettings = false;

	/** 正在监听的注册表（弱引用，不阻止它被销毁）。 */
	TWeakObjectPtr<UGameSettingRegistry> Registry;

	/** 被改过、尚未处理的设置项（脏列表）。 */
	TMap<FObjectKey, TWeakObjectPtr<UGameSetting>> DirtySettings;
};
