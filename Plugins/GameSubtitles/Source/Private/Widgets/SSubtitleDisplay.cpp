// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SSubtitleDisplay.h"

#include "Kismet/GameplayStatics.h"
#include "SubtitleManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/SRichTextBlock.h"

struct FSlateBrush;

// 构建控件：订阅引擎全局字幕管理器 FSubtitleManager 的 OnSetSubtitleText 委托。
// 字幕内容不是本控件主动去取的，而是由 UMediaSubtitlesPlayer 调用 SetMovieSubtitle 推过来。
// （析构函数里必须 RemoveAll 取消订阅，否则会回调到已销毁的对象。）
void SSubtitleDisplay::Construct(const FArguments& InArgs)
{
	if (!InArgs._ManualSubtitles.Get())
	{
		FSubtitleManagerSetSubtitleText& OnSetSubtitleText = FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText();
		OnSetSubtitleText.AddSP(this, &SSubtitleDisplay::HandleSubtitleChanged);
	}

	ChildSlot
	[
		SAssignNew(Background, SBorder)
		.Visibility(EVisibility::Collapsed)
		.Padding(FMargin(7.0, 5.0))
		[
			SAssignNew(TextDisplay, SRichTextBlock)
			.TextStyle(InArgs._TextStyle)
			.Justification(ETextJustify::Center)
			.WrapTextAt(InArgs._WrapTextAt)
		]
	];
}

SSubtitleDisplay::~SSubtitleDisplay()
{
	FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().RemoveAll(this);
}

void SSubtitleDisplay::SetTextStyle(const FTextBlockStyle& InTextStyle)
{
	TextDisplay->SetTextStyle(InTextStyle);
}

void SSubtitleDisplay::SetBackgroundBrush(const FSlateBrush* InSlateBrush)
{
	Background->SetBorderImage(InSlateBrush);
}

void SSubtitleDisplay::SetCurrentSubtitleText(const FText& InSubtitleText)
{
	Background->SetVisibility(InSubtitleText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
	TextDisplay->SetText(InSubtitleText);
}

bool SSubtitleDisplay::HasSubtitles() const
{
	return !TextDisplay->GetText().IsEmpty();
}

void SSubtitleDisplay::SetWrapTextAt(const TAttribute<float>& InWrapTextAt)
{
	TextDisplay->SetWrapTextAt(InWrapTextAt);
}

// 字幕文本更新回调：把新文本写进富文本控件。
void SSubtitleDisplay::HandleSubtitleChanged(const FText& InSubtitleText)
{
	if (UGameplayStatics::AreSubtitlesEnabled())
	{
		Background->SetVisibility(InSubtitleText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
		TextDisplay->SetText(InSubtitleText);
	}
	else
	{
		Background->SetVisibility(EVisibility::Collapsed);
	}
}
