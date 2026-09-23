// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSetting.h"

#include "GameSettingCollection.generated.h"

struct FGameSettingFilterState;

//--------------------------------------
// UGameSettingCollection
//--------------------------------------

/**
 * UGameSettingCollection —— 设置项分组（集合）。
 *
 * 它本身也是个 UGameSetting，因此可以嵌套。用来把一批设置项归类，
 * 并统一做过滤（哪些项当前该显示）。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingCollection : public UGameSetting
{
	GENERATED_BODY()

public:
	UGameSettingCollection();

	/** 本集合直接拥有的所有设置项。 */
	virtual TArray<UGameSetting*> GetChildSettings() override { return Settings; }

	/** 只取子集合（不含普通设置项）。 */
	TArray<UGameSettingCollection*> GetChildCollections() const;

	/** 添加一个设置项：会自动设置它的父级，并在已初始化时立刻初始化它。 */
	void AddSetting(UGameSetting* Setting);
	virtual void GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings) const;

	/** 集合本身不可被选中（它是一个分组，不是一个可操作的项）。 */
	virtual bool IsSelectable() const { return false; }

protected:
	/** The settings owned by this collection. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> Settings;
};

//--------------------------------------
// UGameSettingCollectionPage
//--------------------------------------

/**
 * UGameSettingCollectionPage —— 可导航进入的"子页面"集合。
 *
 * 与普通集合的区别：它可被选中，选中后会触发导航事件，
 * 把界面切到它内部的那一批设置项上（即设置界面的二级页面）。
 */
UCLASS()
class GAMESETTINGS_API UGameSettingCollectionPage : public UGameSettingCollection
{
	GENERATED_BODY()

public:

	/** 用户选中本页、要求"导航进去"时广播（参数是被选中的设置项）。 */
	DECLARE_EVENT_OneParam(UGameSettingCollectionPage, FOnExecuteNavigation, UGameSetting* /*Setting*/);
	FOnExecuteNavigation OnExecuteNavigationEvent;

public:
	UGameSettingCollectionPage();

	FText GetNavigationText() const { return NavigationText; }
	void SetNavigationText(FText Value) { NavigationText = Value; }
#if !UE_BUILD_SHIPPING
	void SetNavigationText(const FString& Value) { SetNavigationText(FText::FromString(Value)); }
#endif
	
	virtual void OnInitialized() override;
	virtual void GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings) const override;
	virtual bool IsSelectable() const override { return true; }

	/**  */
	void ExecuteNavigation();

private:
	FText NavigationText;
};
