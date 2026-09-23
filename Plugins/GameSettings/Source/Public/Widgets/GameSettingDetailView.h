// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidgetPool.h"

#include "GameSettingDetailView.generated.h"

class UCommonRichTextBlock;
class UCommonTextBlock;
class UGameSetting;
class UGameSettingDetailExtension;
class UGameSettingVisualData;
class UObject;
class UVerticalBox;
struct FStreamableHandle;

/**
 * UGameSettingDetailView —— 右侧"详情面板"。
 *
 * 显示当前选中/悬停设置项的名称、描述、动态详情、警告文本、被禁用原因，
 * 并按 UGameSettingVisualData 的配置动态挂上若干"详情扩展控件"。
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingDetailView : public UUserWidget
{
	GENERATED_BODY()
public:
	UGameSettingDetailView(const FObjectInitializer& ObjectInitializer);

	void FillSettingDetails(UGameSetting* InSetting);

	//UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	void CreateDetailsExtension(UGameSetting* InSetting, TSubclassOf<UGameSettingDetailExtension> ExtensionClass);

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UGameSettingVisualData> VisualData;

	/** 详情扩展控件的对象池（避免反复创建销毁）。 */
	UPROPERTY(Transient)
	FUserWidgetPool ExtensionWidgetPool;

	UPROPERTY(Transient)
	TObjectPtr<UGameSetting> CurrentSetting;

	/** 异步加载扩展控件类时的句柄，防止加载过程中控件被销毁导致悬空。 */
	TSharedPtr<FStreamableHandle> StreamingHandle;

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonTextBlock> Text_SettingName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> RichText_Description;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> RichText_DynamicDetails;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> RichText_WarningDetails;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> RichText_DisabledDetails;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UVerticalBox> Box_DetailsExtension;
};
