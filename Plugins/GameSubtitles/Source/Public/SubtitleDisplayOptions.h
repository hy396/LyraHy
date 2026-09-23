// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/DataAsset.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"

#include "SubtitleDisplayOptions.generated.h"

/** 字幕文字尺寸档位。 */
UENUM()
enum class ESubtitleDisplayTextSize : uint8
{
	ExtraSmall,
	Small,
	Medium,
	Large,
	ExtraLarge,
	ESubtitleDisplayTextSize_MAX
};

/** 字幕文字颜色档位。 */
UENUM()
enum class ESubtitleDisplayTextColor : uint8
{
	White,
	Yellow,
	ESubtitleDisplayTextColor_MAX
};

/** 字幕文字描边/阴影样式。 */
UENUM()
enum class ESubtitleDisplayTextBorder : uint8
{
	None,
	Outline,
	DropShadow,
	ESubtitleDisplayTextBorder_MAX
};

/** 字幕背景不透明度档位。 */
UENUM()
enum class ESubtitleDisplayBackgroundOpacity : uint8
{
	Clear,
	Low,
	Medium,
	High,
	Solid,
	ESubtitleDisplayBackgroundOpacity_MAX
};

/**
 * USubtitleDisplayOptions —— 字幕显示样式配置（数据资产）。
 *
 * 把"档位 → 具体样式值"做成数组，下标就是上面各枚举的值。
 * 这样玩家在设置里选一个档位（如"大号字"、"黄色"、"描边"），
 * 界面就能直接按下标取到对应的字号/颜色/边框尺寸，无需写一堆 switch。
 */
UCLASS(BlueprintType)
class GAMESUBTITLES_API USubtitleDisplayOptions : public UDataAsset
{
	GENERATED_BODY()

public:
	USubtitleDisplayOptions() { }

public:
	/** 字幕使用的字体。 */
	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	FSlateFontInfo Font;

	/** 各尺寸档位对应的字号，下标为 ESubtitleDisplayTextSize。 */
	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	int32 DisplayTextSizes[(int32)ESubtitleDisplayTextSize::ESubtitleDisplayTextSize_MAX];

	/** 各颜色档位对应的颜色，下标为 ESubtitleDisplayTextColor。 */
	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	FLinearColor DisplayTextColors[(int32)ESubtitleDisplayTextColor::ESubtitleDisplayTextColor_MAX];

	/** 各描边档位对应的边框尺寸，下标为 ESubtitleDisplayTextBorder。 */
	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	float DisplayBorderSize[(int32)ESubtitleDisplayTextBorder::ESubtitleDisplayTextBorder_MAX];

	/** 各背景档位对应的不透明度，下标为 ESubtitleDisplayBackgroundOpacity。 */
	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	float DisplayBackgroundOpacity[(int32)ESubtitleDisplayBackgroundOpacity::ESubtitleDisplayBackgroundOpacity_MAX];

	/** 字幕背景的画刷（可以是纯色、九宫格图片等）。 */
	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	FSlateBrush BackgroundBrush;
};
