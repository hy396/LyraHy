// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PreLoadScreenBase.h"

class SWidget;

/**
 * FCommonPreLoadScreen —— 引擎【启动阶段】的加载画面（PreLoadScreen）。
 *
 * 它运行在引擎极早期——那时 UObject / UMG 都还没就绪，只能用纯 Slate 画。
 * 作用是让游戏一启动就立刻有画面，而不是先黑屏一会儿。
 */
class FCommonPreLoadScreen : public FPreLoadScreenBase
{
public:
	
    /*** IPreLoadScreen Implementation ***/
	/** 创建要显示的 Slate 控件。 */
	virtual void Init() override;
	/** 声明自己是"引擎加载画面"类型（区别于"早期加载画面"）。 */
	virtual EPreLoadScreenTypes GetPreLoadScreenType() const override { return EPreLoadScreenTypes::EngineLoadingScreen; }
	/** 把要显示的控件交给 PreLoadScreenManager。 */
	virtual TSharedPtr<SWidget> GetWidget() override { return EngineLoadingWidget; }
private:

	/** 启动画面控件（纯 Slate，因为此时 UMG 还不可用）。 */
	TSharedPtr<SWidget> EngineLoadingWidget;
};
