// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateWidgetStyleAsset.h"
#include "Styling/SlateWidgetStyleContainerBase.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Accessibility/SlateWidgetAccessibleTypes.h"

class FText;
struct FSlateBrush;

/**
 * SSubtitleDisplay —— 真正把字幕画出来的 Slate 控件。
 *
 * 它不自己去查字幕内容，而是订阅引擎全局 FSubtitleManager 的 OnSetSubtitleText 委托，
 * 由 UMediaSubtitlesPlayer 那边推过来的文本驱动显示。
 */
class GAMESUBTITLES_API SSubtitleDisplay : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SSubtitleDisplay)
		: _TextStyle( &FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText") )
		, _WrapTextAt(0.f)
		, _ManualSubtitles(false)
		{}

	SLATE_STYLE_ARGUMENT( FTextBlockStyle, TextStyle )

	/** Whether text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs. */
	SLATE_ATTRIBUTE( float, WrapTextAt )

	/** 为 true 时只显示手动设置的字幕文本，忽略来自字幕管理器的自动字幕。 */
	SLATE_ATTRIBUTE(bool, ManualSubtitles )

	SLATE_END_ARGS()

	~SSubtitleDisplay();

	void Construct( const FArguments& InArgs );

	void SetTextStyle(const FTextBlockStyle& InTextStyle);

	void SetBackgroundBrush(const FSlateBrush* InSlateBrush);

	/** 手动设置当前要显示的字幕文本（用于 ManualSubtitles 模式或预览）。 */
	void SetCurrentSubtitleText(const FText& SubtitleText);

	/** 当前是否有字幕在显示（文本非空即为 true）。 */
	bool HasSubtitles() const;

	/** See WrapTextAt attribute */
	void SetWrapTextAt(const TAttribute<float>& InWrapTextAt);

private:
	/** 来自 FSubtitleManager 的回调：字幕文本更新了，刷新显示。 */
	void HandleSubtitleChanged(const FText& SubtitleText);

private:

	/** 字幕的背景框。 */
	TSharedPtr<class SBorder> Background;

	/** 真正承载字幕文字的富文本控件（用 SRichTextBlock 是为了支持富文本标签）。 */
	TSharedPtr<class SRichTextBlock> TextDisplay;
};
