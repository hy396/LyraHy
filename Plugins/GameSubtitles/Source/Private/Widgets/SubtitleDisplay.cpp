// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SubtitleDisplay.h"
#include "Widgets/SSubtitleDisplay.h"

#include "Engine/GameInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SubtitleDisplay)

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif	// WITH_EDITOR

#define LOCTEXT_NAMESPACE "SubtitleDisplay"

USubtitleDisplay::USubtitleDisplay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, WrapTextAt(0) // No wrapping by default
{
}

bool USubtitleDisplay::HasSubtitles() const
{
	return (SubtitleWidget.IsValid() && SubtitleWidget->HasSubtitles());
}

void USubtitleDisplay::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	
	RebuildStyle();
	
	if (IsDesignTime() || bPreviewMode)
	{
		SubtitleWidget->SetCurrentSubtitleText(PreviewText);
	}
}

void USubtitleDisplay::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	SubtitleWidget.Reset();

	if (USubtitleDisplaySubsystem* SubtitleDisplay = UGameInstance::GetSubsystem<USubtitleDisplaySubsystem>(GetGameInstance()))
	{
		SubtitleDisplay->DisplayFormatChangedEvent.RemoveAll(this);
	}
}

TSharedRef<SWidget> USubtitleDisplay::RebuildWidget()
{
	if (USubtitleDisplaySubsystem* SubtitleDisplay = UGameInstance::GetSubsystem<USubtitleDisplaySubsystem>(GetGameInstance()))
	{
		SubtitleDisplay->DisplayFormatChangedEvent.AddUObject(this, &ThisClass::HandleSubtitleDisplayOptionsChanged);
		Format = SubtitleDisplay->GetSubtitleDisplayOptions();
	}

	SubtitleWidget = SNew(SSubtitleDisplay)
		.TextStyle(&GeneratedStyle)
		.WrapTextAt(WrapTextAt)
		.ManualSubtitles(IsDesignTime() || bPreviewMode);

	RebuildStyle();
	
	return SubtitleWidget.ToSharedRef();
}

// 子系统广播"字幕设置变了"：更新本控件的格式并重建样式。
void USubtitleDisplay::HandleSubtitleDisplayOptionsChanged(const FSubtitleFormat& InDisplayFormat)
{
	if (SubtitleWidget.IsValid())
	{
		Format = InDisplayFormat;
		RebuildStyle();
	}
}

// 把"档位 → 具体样式值"翻译一遍：按 Format 里的各档位下标，
// 到 Options 资产对应的数组里取出字号、颜色、边框尺寸、背景不透明度，
// 生成 Slate 用的 FTextBlockStyle 和背景画刷，再推给底层 SSubtitleDisplay。
void USubtitleDisplay::RebuildStyle()
{
	GeneratedStyle = FTextBlockStyle();

	if (Options)
	{
		GeneratedStyle.Font = Options->Font;
		GeneratedStyle.Font.Size = Options->DisplayTextSizes[(int32)Format.SubtitleTextSize];
		GeneratedStyle.ColorAndOpacity = Options->DisplayTextColors[(int32)Format.SubtitleTextColor];

		switch (Format.SubtitleTextBorder)
		{
		case ESubtitleDisplayTextBorder::DropShadow:
		{
			const float ShadowSize = FMath::Max(1.0f, Options->DisplayBorderSize[(int32)ESubtitleDisplayTextBorder::DropShadow] * (float)Format.SubtitleTextSize / 2.0f);
			GeneratedStyle.SetShadowOffset(FVector2D(ShadowSize, ShadowSize));
			break;
		}
		case ESubtitleDisplayTextBorder::Outline:
		{
			const float OutlineSize = FMath::Max(1.0f, Options->DisplayBorderSize[(int32)ESubtitleDisplayTextBorder::Outline] * (float)Format.SubtitleTextSize);
			GeneratedStyle.Font.OutlineSettings.OutlineSize = OutlineSize;
			break;
		}
		case ESubtitleDisplayTextBorder::None:
		default:
			break;
		}

		FLinearColor CurrentBackgroundColor = Options->BackgroundBrush.TintColor.GetSpecifiedColor();
		CurrentBackgroundColor.A = Options->DisplayBackgroundOpacity[(int32)Format.SubtitleBackgroundOpacity];
		GeneratedBackgroundBorder = Options->BackgroundBrush;
		GeneratedBackgroundBorder.TintColor = CurrentBackgroundColor;

		if (SubtitleWidget.IsValid())
		{
			SubtitleWidget->SetTextStyle(GeneratedStyle);
			SubtitleWidget->SetBackgroundBrush(&GeneratedBackgroundBorder);
		}
	}
}

#if WITH_EDITOR

void USubtitleDisplay::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	if (!Options)
	{
		CompileLog.Error(FText::Format(LOCTEXT("Error_USubtitleDisplay_MissingOptions", "{0} has no subtitle Options asset specified."), FText::FromString(GetName())));
	}
}

#endif

#undef LOCTEXT_NAMESPACE

