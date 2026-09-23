// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "CommonMessagingSubsystem.h"

#include "CommonGameDialog.generated.h"

/** 对话框上的一个按钮选项：结果值 + 可覆盖的显示文字。 */
USTRUCT(BlueprintType)
struct FConfirmationDialogAction
{
	GENERATED_BODY()

public:
	/** 【必填】该按钮对应的结果值。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECommonMessagingResult Result = ECommonMessagingResult::Unknown;

	/** 【可选】自定义按钮文字；不填则用结果值对应的默认文字。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText OptionalDisplayText;

	bool operator==(const FConfirmationDialogAction& Other) const
	{
		return Result == Other.Result &&
			OptionalDisplayText.EqualTo(Other.OptionalDisplayText);
	}
};

/**
 * UCommonGameDialogDescriptor —— 对话框的"数据描述"。
 *
 * 把"标题 + 正文 + 有哪些按钮"打包成一个对象，交给 UCommonMessagingSubsystem 去显示。
 * 好处是内容与表现分离：换一套对话框 UI 不需要改调用方代码。
 */
UCLASS()
class COMMONGAME_API UCommonGameDialogDescriptor : public UObject
{
	GENERATED_BODY()
	
public:
	/** 快捷创建：只有一个"确定"按钮。 */
	static UCommonGameDialogDescriptor* CreateConfirmationOk(const FText& Header, const FText& Body);
	/** 快捷创建："确定 / 取消"两个按钮。 */
	static UCommonGameDialogDescriptor* CreateConfirmationOkCancel(const FText& Header, const FText& Body);
	/** 快捷创建："是 / 否"两个按钮。 */
	static UCommonGameDialogDescriptor* CreateConfirmationYesNo(const FText& Header, const FText& Body);
	/** 快捷创建："是 / 否 / 取消"三个按钮。 */
	static UCommonGameDialogDescriptor* CreateConfirmationYesNoCancel(const FText& Header, const FText& Body);

public:
	/** The header of the message to display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Header;
	
	/** The body of the message to display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Body;

	/** The confirm button's input action to use. */
	UPROPERTY(BlueprintReadWrite)
	TArray<FConfirmationDialogAction> ButtonActions;
};


/**
 * UCommonGameDialog —— 对话框控件基类。
 *
 * 游戏应继承它做出自己的对话框外观，并在配置里指定；
 * 子系统弹出时只认这个基类接口（SetupDialog / KillDialog），不关心具体实现。
 */
UCLASS(Abstract)
class COMMONGAME_API UCommonGameDialog : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	UCommonGameDialog();
	
	/** 用描述对象填充对话框内容，并登记"玩家做出选择后要回调谁"。 */
	virtual void SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback);

	/** 强制关闭对话框（未产生玩家选择，结果记为 Killed）。 */
	virtual void KillDialog();
};
