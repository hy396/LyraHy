// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSettingCollection.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingCollection)

#define LOCTEXT_NAMESPACE "GameSetting"

//--------------------------------------
// UGameSettingCollection
//--------------------------------------

UGameSettingCollection::UGameSettingCollection()
{

}

void UGameSettingCollection::AddSetting(UGameSetting* Setting)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(Setting->GetSettingParent() == nullptr, TEXT("This setting already has a parent!"));
	ensureAlwaysMsgf(!Settings.Contains(Setting), TEXT("This collection already includes this setting!"));
#endif

	Settings.Add(Setting);
	Setting->SetSettingParent(this);

	if (LocalPlayer)
	{
		Setting->Initialize(LocalPlayer);
	}
}

TArray<UGameSettingCollection*> UGameSettingCollection::GetChildCollections() const
{
	TArray<UGameSettingCollection*> CollectionSettings;

	for (UGameSetting* ChildSetting : Settings)
	{
		if (UGameSettingCollection* ChildCollection = Cast<UGameSettingCollection>(ChildSetting))
		{
			CollectionSettings.Add(ChildCollection);
		}
	}

	return CollectionSettings;
}

// 递归收集：把自己孩子里通过过滤的项加进结果；空集合则整个不显示（避免出现空的分组标题）。
void UGameSettingCollection::GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings) const
{
	for (UGameSetting* ChildSetting : Settings)
	{
		// If the child setting is a collection, only add it to the set if it has any visible children.
		if (Cast<UGameSettingCollectionPage>(ChildSetting))
		{
			if (FilterState.DoesSettingPassFilter(*ChildSetting))
			{
				InOutSettings.Add(ChildSetting);
			}
		}
		else if (UGameSettingCollection* ChildCollection = Cast<UGameSettingCollection>(ChildSetting))
		{
			TArray<UGameSetting*> ChildSettings;
			ChildCollection->GetSettingsForFilter(FilterState, ChildSettings);

			if (ChildSettings.Num() > 0)
			{
				// Don't add the root setting to the returned items, it's the container of N-possible 
				// other settings and containers we're actually displaying right now.
				if (!FilterState.IsSettingInRootList(ChildSetting))
				{
					InOutSettings.Add(ChildSetting);
				}

				InOutSettings.Append(ChildSettings);
			}
		}
		else
		{
			if (FilterState.DoesSettingPassFilter(*ChildSetting))
			{
				InOutSettings.Add(ChildSetting);
			}
		}
	}
}

//--------------------------------------
// UGameSettingCollectionPage
//--------------------------------------

UGameSettingCollectionPage::UGameSettingCollectionPage()
{
}

void UGameSettingCollectionPage::OnInitialized()
{
	Super::OnInitialized();

#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!NavigationText.IsEmpty(), TEXT("You must provide a NavigationText for this setting."));
	ensureAlwaysMsgf(!DescriptionRichText.IsEmpty(), TEXT("You must provide a description for new page collections."));
#endif
}

// 子页面的过滤：只有 FilterState 允许包含嵌套页面（bIncludeNestedPages）时，
// 才会把它内部的第一层设置项也摊平进结果里。
void UGameSettingCollectionPage::GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings) const
{
	// If we're including nested pages, call the super and dump them all, otherwise, we pretend we have none for the filtering.
	// because our settings are displayed on another page.
	if (FilterState.bIncludeNestedPages || FilterState.IsSettingInRootList(this))
	{
		Super::GetSettingsForFilter(FilterState, InOutSettings);
	}
}

// 广播"导航进入本子页面"事件，由上层 Panel/Screen 接住并切换列表。
void UGameSettingCollectionPage::ExecuteNavigation()
{
	OnExecuteNavigationEvent.Broadcast(this);
}

#undef LOCTEXT_NAMESPACE

