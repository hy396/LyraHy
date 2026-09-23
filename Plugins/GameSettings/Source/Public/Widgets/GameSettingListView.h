// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ListView.h"

#include "GameSettingListView.generated.h"

class STableViewBase;

class UGameSettingCollection;
class ULocalPlayer;
class UGameSettingVisualData;

/**
 * UGameSettingListView —— 设置项列表视图。
 *
 * 它是 UListView 的子类，用来展示一批 UGameSetting。
 * 注意：每个条目控件都必须派生自 UGameSettingListEntryBase，否则无法正确绑定数据。
 */
UCLASS(meta = (EntryClass = GameSettingListEntryBase))
class GAMESETTINGS_API UGameSettingListView : public UListView
{
	GENERATED_BODY()

public:
	UGameSettingListView(const FObjectInitializer& ObjectInitializer);

	/** 给某个设置项临时改显示名（按 DevName 覆盖），不改设置项本身。 */
	void AddNameOverride(const FName& DevName, const FText& OverrideName);

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& InCompileLog) const override;
#endif

protected:
	virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;
	virtual bool OnIsSelectableOrNavigableInternal(UObject* SelectedItem) override;

protected:
	/** 视觉数据资产：决定每种设置项用哪个条目控件类来渲染。 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UGameSettingVisualData> VisualData;

private:
	/** DevName → 覆盖显示名 的映射表。 */
	TMap<FName, FText> NameOverrides;
};
