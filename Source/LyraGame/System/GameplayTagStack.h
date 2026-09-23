// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 标签栈：用一个 GameplayTag 加一个整数计数表示「某样东西有几个」。
 *
 * Lyra 用它表示弹药、道具数量等（例如 Tag=Item.Weapon.Ammo, Count=30）。
 * 底层用 FFastArraySerializer 做增量复制，只传变化的那一格，比较省带宽。
 */
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "GameplayTagStack.generated.h"

struct FGameplayTagStackContainer;
struct FNetDeltaSerializeInfo;

// 一个标签栈：标签 + 数量
/**
 * Represents one stack of a gameplay tag (tag + count)
 */
USTRUCT(BlueprintType)
struct FGameplayTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGameplayTagStack()
	{}

	FGameplayTagStack(FGameplayTag InTag, int32 InStackCount)
		: Tag(InTag)
		, StackCount(InStackCount)
	{
	}

	// 调试用字符串，形如 Tag x Count
	FString GetDebugString() const;

// 只允许容器修改标签与数量
private:
	friend FGameplayTagStackContainer;

	// 标签
	UPROPERTY()
	FGameplayTag Tag;

	// 数量
	UPROPERTY()
	int32 StackCount = 0;
};

// 标签栈容器：对外提供增删查接口，对内维护数组 + 加速用的 Map
/** Container of gameplay tag stacks */
USTRUCT(BlueprintType)
struct FGameplayTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FGameplayTagStackContainer()
	//	: Owner(nullptr)
	{
	}

public:
	// 增加指定数量的栈（数量小于 1 时什么也不做）
	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	void AddStack(FGameplayTag Tag, int32 StackCount);

	// 减少指定数量的栈；减到 0 会把该标签整个移除（数量小于 1 时什么也不做）
	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	void RemoveStack(FGameplayTag Tag, int32 StackCount);

	// 取该标签的数量，不存在返回 0
	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	int32 GetStackCount(FGameplayTag Tag) const
	{
		return TagToCountMap.FindRef(Tag);
	}

	// 该标签是否至少有一个栈
	// Returns true if there is at least one stack of the specified tag
	bool ContainsTag(FGameplayTag Tag) const
	{
		return TagToCountMap.Contains(Tag);
	}

	// FFastArraySerializer 契约：见下面三个回调
	//~FFastArraySerializer contract
	// 数组项被复制删除前调用
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	// 数组项被复制新增后调用
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	// 数组项被复制修改后调用
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	// 增量序列化：只传变化的部分
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameplayTagStack, FGameplayTagStackContainer>(Stacks, DeltaParms, *this);
	}

private:
	// 真正参与复制的数组
	// Replicated list of gameplay tag stacks
	UPROPERTY()
	TArray<FGameplayTagStack> Stacks;
	
	// 加速查询用的 Map：标签 -> 数量
	// Accelerated list of tag stacks for queries
	TMap<FGameplayTag, int32> TagToCountMap;
};

// 告诉引擎这个结构带增量序列化器
template<>
struct TStructOpsTypeTraits<FGameplayTagStackContainer> : public TStructOpsTypeTraitsBase2<FGameplayTagStackContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
