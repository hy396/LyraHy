// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "SubtitleDisplayOptions.h"

#include "SubtitleDisplaySubsystem.generated.h"

class FSubsystemCollectionBase;
class ULocalPlayer;
class UObject;

/**
 * FSubtitleFormat —— 玩家当前选择的字幕显示格式（哪几档）。
 * 它只记录"选了哪个档位"，具体数值由 USubtitleDisplayOptions 资产提供。
 */
USTRUCT(BlueprintType)
struct GAMESUBTITLES_API FSubtitleFormat
{
	GENERATED_BODY()

public:
	FSubtitleFormat()
		: SubtitleTextSize(ESubtitleDisplayTextSize::Medium)
		, SubtitleTextColor(ESubtitleDisplayTextColor::White)
		, SubtitleTextBorder(ESubtitleDisplayTextBorder::None)
		, SubtitleBackgroundOpacity(ESubtitleDisplayBackgroundOpacity::Medium)
	{
	}

public:
	UPROPERTY(EditAnywhere, Category = "Display Info")
	ESubtitleDisplayTextSize SubtitleTextSize;

	UPROPERTY(EditAnywhere, Category = "Display Info")
	ESubtitleDisplayTextColor SubtitleTextColor;

	UPROPERTY(EditAnywhere, Category = "Display Info")
	ESubtitleDisplayTextBorder SubtitleTextBorder;

	UPROPERTY(EditAnywhere, Category = "Display Info")
	ESubtitleDisplayBackgroundOpacity SubtitleBackgroundOpacity;
};

/**
 * USubtitleDisplaySubsystem —— 字幕显示设置子系统（GameInstance 级）。
 *
 * 职责很单一：保存当前玩家选择的字幕格式，并在其变化时广播 DisplayFormatChangedEvent，
 * 让所有字幕控件刷新样式。这样"设置里改字号 → 所有字幕立刻变大"就能自然实现。
 */
UCLASS(DisplayName = "Subtitle Display")
class GAMESUBTITLES_API USubtitleDisplaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 字幕显示格式发生变化时广播（参数是新的格式）。字幕控件订阅它来刷新样式。 */
	DECLARE_EVENT_OneParam(USubtitleDisplaySubsystem, FDisplayFormatChangedEvent, const FSubtitleFormat& /*DisplayFormat*/);
	FDisplayFormatChangedEvent DisplayFormatChangedEvent;

public:
	/** 通过 LocalPlayer 取到所属 GameInstance 上的本子系统（LocalPlayer 为空则返回 nullptr）。 */
	static USubtitleDisplaySubsystem* Get(const ULocalPlayer* LocalPlayer);

public:
	USubtitleDisplaySubsystem();

	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	/** 设置当前字幕格式，并广播 DisplayFormatChangedEvent 通知所有字幕控件刷新。 */
	void SetSubtitleDisplayOptions(const FSubtitleFormat& InOptions);

	/** 取当前字幕格式。 */
	const FSubtitleFormat& GetSubtitleDisplayOptions() const;

private:
	/** 当前生效的字幕格式。 */
	UPROPERTY()
	FSubtitleFormat SubtitleFormat;
};
