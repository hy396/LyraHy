// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/Widget.h"
#include "Styling/SlateTypes.h"
#include "SubtitleDisplaySubsystem.h"

#include "SubtitleDisplay.generated.h"

class USubtitleDisplayOptions;

struct FSubtitleFormat;

/**
 * USubtitleDisplay —— 字幕显示控件（UMG 层，供蓝图使用）。
 *
 * 它是对底层 Slate 控件 SSubtitleDisplay 的 UMG 包装，
 * 负责把"选项资产里的样式值 + 玩家选的档位"换算成具体的 Slate 样式并应用上去。
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisableNativeTick))
class GAMESUBTITLES_API USubtitleDisplay : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	/** 本控件使用的显示格式档位（会被子系统广播的全局设置覆盖）。 */
	UPROPERTY(EditAnywhere, Category = "Display Info")
	FSubtitleFormat Format;

	/** 样式选项资产：提供档位 → 具体字号/颜色/边框的映射。 */
	UPROPERTY(EditAnywhere, Category = "Display Info")
	TObjectPtr<USubtitleDisplayOptions> Options;

	/** 文字超过该宽度时自动换行；为 0 或负数表示不换行。 */
	UPROPERTY(EditAnywhere, Category="Display Info")
	float WrapTextAt;
	
	UFUNCTION(BlueprintCallable, Category = Subtitles, Meta = (Tooltip = "True if there are subtitles currently.  False if the subtitle text is empty."))
	bool HasSubtitles() const;

	/** Preview text to be displayed when designing the widget */
	UPROPERTY(EditAnywhere, Category="Preview")
	bool bPreviewMode;

	/** Preview text to be displayed when designing the widget */
	UPROPERTY(EditAnywhere, Category="Preview")
	FText PreviewText;

public:

	// UWidget Public Interface
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const;
#endif
	// End UWidget Public Interface

protected:

	// UWidget Protected Interface
	virtual TSharedRef<class SWidget> RebuildWidget() override;
	// End UWidget Protected Interface

	/** 子系统通知"字幕格式变了"时的回调：更新 Format 并重建样式。 */
	void HandleSubtitleDisplayOptionsChanged(const FSubtitleFormat& InDisplayFormat);
	
private:

	/** 根据当前 Format 档位 + Options 资产，重新生成文字样式和背景画刷。 */
	void RebuildStyle();

private:

	/** 由 RebuildStyle 生成的文字样式（运行时产物，不序列化）。 */
	UPROPERTY(Transient)
	FTextBlockStyle GeneratedStyle;

	/** 由 RebuildStyle 生成的背景画刷（运行时产物，不序列化）。 */
	UPROPERTY(Transient)
	FSlateBrush GeneratedBackgroundBorder;

	/** 真正负责显示字幕的底层 Slate 控件。 */
	TSharedPtr<class SSubtitleDisplay> SubtitleWidget;
};
