// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUserTypes.h"
#include "OnlineError.h"

// 统一错误转换：OSSv1 取错误码与错误信息，OSSv2 取 ErrorId 与文本；两边语义不同，注意别混用
void FOnlineResultInformation::FromOnlineError(const FOnlineErrorType& InOnlineError)
{
#if COMMONUSER_OSSV1
	bWasSuccessful = InOnlineError.WasSuccessful();
	ErrorId = InOnlineError.GetErrorCode();
	ErrorText = InOnlineError.GetErrorMessage();
#else
	bWasSuccessful = InOnlineError != UE::Online::Errors::Success();
	ErrorId = InOnlineError.GetErrorId();
	ErrorText = InOnlineError.GetText();
#endif
}
