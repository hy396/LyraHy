--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--
-- GameplayEvent.Window
---@type ANS_ComboWindow_C
local M = UnLua.Class()

-- 发送窗口事件。Magnitude：开启时 = 窗口时长（秒），关闭时 = 0
local function SendWindowEvent(MeshComp, Magnitude, Tag)
    local Owner = MeshComp and MeshComp:GetOwner()

    if not Owner then
        return
    end

    if not Tag or not Tag:IsValid() then
        print("[连招窗口] WindowTag 未配置或无效")
        return
    end

    local Payload = UE.FGameplayEventData()
    Payload.EventMagnitude = Magnitude
    UE.UAbilitySystemBlueprintLibrary.SendGameplayEventToActor(Owner, Tag, Payload)
end

-- 编辑器里显示的名字。直接显示 tag，一眼看出这个窗口发的是什么事件
-- 注意：函数名是 GetNotifyName，不要写成 GetNotifyName_Implementation
function M:GetNotifyName()
    local Tag = self.WindowTag
    if Tag and Tag:IsValid() then
        return tostring(Tag.TagName)
    end
    return "连招窗口(未配置)"
end

-- 窗口开启：TotalDuration 就是你在蒙太奇里拖出来的区间长度，
-- 直接当窗口时长传出去，改动画长度窗口自动跟随
function M:Received_NotifyBegin(MeshComp, Animation, TotalDuration, EventReference)
    SendWindowEvent(MeshComp, TotalDuration, self.WindowTag)
    return true
end

-- 窗口关闭：正常播完 / 被切段 / 被受击打断，引擎都会走到这里
function M:Received_NotifyEnd(MeshComp, Animation, EventReference)
    SendWindowEvent(MeshComp, 0, self.WindowTag)
    return true
end

return M