--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--
-- ANS_ComboPreInput.lua
-- 连招预输入期：这段区间内按下的攻击键会被缓冲，
-- 区间结束时若缓冲里有输入，自动进入下一段
-- 蓝图变量：PreInputTag（Gameplay Tag，如 GameplayEvent.PreInput）—— 注意是大写 I，
-- 旧项目里写成 Prelnput（小写 L）是笔误，全项目只有那个注释里出现过
---@type ANS_ComboPreInput_C
local M = UnLua.Class()
local function Send(MeshComp, Magnitude, Tag)
    local Owner = MeshComp and MeshComp:GetOwner()
    if not Owner or not Tag or not UE.UBlueprintGameplayTagLibrary.IsGameplayTagValid(Tag) then return end
    local Payload = UE.FGameplayEventData()
    Payload.EventMagnitude = Magnitude
    UE.UAbilitySystemBlueprintLibrary.SendGameplayEventToActor(Owner, Tag, Payload)
end

function M:GetNotifyName()
    local Tag = self.PreInputTag
    if Tag and UE.UBlueprintGameplayTagLibrary.IsGameplayTagValid(Tag) then return tostring(Tag.TagName) end
    return "预输入期(未配置)"
end

function M:Received_NotifyBegin(MeshComp, Animation, TotalDuration, EventReference)
    Send(MeshComp, 1, self.PreInputTag)   -- 1 = 开始接受预输入
    return true
end

function M:Received_NotifyEnd(MeshComp, Animation, EventReference)
    Send(MeshComp, 0, self.PreInputTag)   -- 0 = 预输入期结束，触发结算
    return true
end

-- function M:Received_NotifyTick(MeshComp, Animation, FrameDeltaTime)
-- end

return M