// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingValue.h"
#include "Math/Range.h"

#include "GameSettingValueScalar.generated.h"

class UObject;

/**
 * UGameSettingValueScalar —— 连续数值设置项（滑块型）。
 *
 * 例如"音量 0~100""鼠标灵敏度"。核心概念是"真实值"与"归一化值"两套坐标：
 *   - 真实值       ：GetValue / SetValue，业务意义上的值（如 0~100）
 *   - 归一化值     ：GetValueNormalized / SetValueNormalized，统一映射到 0~1 供滑块使用
 * 两者通过 GetSourceRange()（真实值区间）做换算，这样 UI 滑块永远只处理 0~1。
 */
UCLASS(abstract)
class GAMESETTINGS_API UGameSettingValueScalar : public UGameSettingValue
{
	GENERATED_BODY()

public:
	UGameSettingValueScalar();

	void SetValueNormalized(double NormalizedValue);
	double GetValueNormalized() const;

	TOptional<double> GetDefaultValueNormalized() const
	{
		TOptional<double> DefaultValue = GetDefaultValue();
		if (DefaultValue.IsSet())
		{
			return FMath::GetMappedRangeValueClamped(GetSourceRange(), TRange<double>(0, 1), DefaultValue.GetValue());
		}
		return TOptional<double>();
	}

	/** 默认值（可选）。未设置表示该项不可重置。 */
	virtual TOptional<double> GetDefaultValue() const						PURE_VIRTUAL(, return TOptional<double>(););

	/** 设置真实值。 */
	virtual void SetValue(double Value, EGameSettingChangeReason Reason = EGameSettingChangeReason::Change)	PURE_VIRTUAL(, );

	/** 取当前真实值。 */
	virtual double GetValue() const									PURE_VIRTUAL(, return 0;);

	/** 真实值的取值范围（用于归一化换算）。 */
	virtual TRange<double> GetSourceRange() const						PURE_VIRTUAL(, return TRange<double>(););

	/** 真实值的最小步进（滑块拖一格变多少）。 */
	virtual double GetSourceStep() const								PURE_VIRTUAL(, return 0.01;);
	double GetNormalizedStepSize() const
	{
		TRange<double> SourceRange = GetSourceRange();
		return GetSourceStep() / FMath::Abs(SourceRange.GetUpperBoundValue() - SourceRange.GetLowerBoundValue());
	}
	/** 显示给玩家的格式化文本（如 "75%"、"1920x1080"）。 */
	virtual FText GetFormattedText() const							PURE_VIRTUAL(, return FText::GetEmpty(););
	
	virtual FString GetAnalyticsValue() const override
	{
		return LexToString(GetValue());
	}

protected:
};
