--[[
GA_SimpMeleeAttack_C.lua —— 连招攻击能力

资产现状：
  GA 蓝图里有 AttackMontages 数组（AM_Combo_Attack_05_01 ~ 04 共 4 个独立蒙太奇），
  段号 = 数组索引（UnLua 的 TArray 是 1-based，所以第一段是 :Get(1)）。
  每个蒙太奇上都挂了 ANS_ComboPreInput，其 PreInputTag 默认是 GameplayEvent.PreInput。
  三个状态标志 IsCanInput / IsCanNext / IsBuffer 是 GA 蓝图的 bool 变量。

必须补的蓝图配置：
  GA 蓝图上加一个 GameplayTag 类型变量 **PreInputTag**，默认值填 GameplayEvent.PreInput。
  为什么不能在 Lua 里用字符串造 tag：UE 5.8 里 FGameplayTag::RequestGameplayTag 和
  UGameplayTagsManager::RequestGameplayTag 都**不是 UFUNCTION**
  （GameplayTagContainer.h:57 / GameplayTagsManager.h:375，纯 C++ 声明），Lua 调不到；
  UBlueprintGameplayTagLibrary 里只有 MatchesTag / MakeLiteralGameplayTag 这类，
  没有任何"字符串 → FGameplayTag"的入口。所以 tag 只能由蓝图变量给进来。

职责划分：
  ANS：只负责窗口时机，Begin 发 Magnitude=1、End 发 Magnitude=0（纯计时器，不存状态）
  GA ：输入捕获（WaitInputPress）+ 缓冲标志 + 切段（播下一个蒙太奇）+ 生命周期
  （缓冲状态放 GA 而不是 ANS：ANS 是动画资产上的对象，服务器也会触发它，
    往它身上挂状态容易两端对不上；放在 GA 里和旧 C++ 的写法也一脉相承）

状态流转（每一段）：
  段开始 → ANS Begin(Mag=1) → IsCanInput=true【缓冲中】按键 → IsBuffer=true
         → ANS End(Mag=0)  → IsCanNext=true
              ├─ IsBuffer=true  → 立即切下一段
              └─ IsBuffer=false →【后续监听】按键即切下一段
  本段蒙太奇播完 → 结束能力，后续监听截止
  被打断       → 只清标志，不结束（能力已经在结束流程里了）

踩过的坑（别再踩）：
  1. 不要定义与 UFunction 同名的 Lua 函数（例如 M:K2_EndAbility）。
     UnLua 的 Index（UnLuaLib.cpp:163）先查 Lua 表再查 UFunction，同名会直接把引擎
     实现顶掉，结果能力永远结束不了。要挂清理钩子请用 K2_OnEndAbility。
  2. Advance() 必须在挂好 WaitGameplayEvent / WaitInputPress 之后再调，
     否则第一段的窗口事件和开局按键都会丢。
  3. 最后一段不要主动 EndAbility，否则最后一段动画会被连根掐掉。
]]
---@type GA_SimpMeleeAttack_C
local M = UnLua.Class()

-- 打完最后一段之后：true = 回到第一段继续循环；false = 收招（当前选择）
local bLoopCombo = true

-- 安全结束 task：task 可能已完成或被销毁，直接调会踩到已释放对象
local function SafeEndTask(Task)
    if not Task then return end
    pcall(function() Task:EndTask() end)
end

--------------------------------------------------------------------------------
-- 生命周期
--------------------------------------------------------------------------------

function M:K2_ActivateAbility()
    if not self:K2_CommitAbility() then
        self:K2_EndAbility()
        return
    end

    self.StepIndex      = 0
    self.CurrentMontage = nil
    self.MontageTask    = nil
    self.IsCanInput     = false
    self.IsCanNext      = false
    self.IsBuffer       = false

    if self:GetStepCount() < 1 then
        self:K2_EndAbility()
        return
    end

    local Tag = self.PreInputTag
    if not Tag or not UE.UBlueprintGameplayTagLibrary.IsGameplayTagValid(Tag) then
        -- 拿不到 tag 就别硬撑：挂一个无效 tag 的监听不会报错，但连招窗口会静默失效
        print("[连招] GA 蓝图的 PreInputTag 未配置或无效，连招窗口失效（只能出第一段）")
    else
        -- 顺序关键：先挂事件监听，再播蒙太奇。
        -- 蒙太奇开头的通知在播放当帧就会发出，监听挂晚了那次事件会静默丢失，
        -- 表现为"第一段不响应、第二段起才正常"。
        local WaitEventTask = UE.UAbilityTask_WaitGameplayEvent.WaitGameplayEvent(
            self, Tag, nil, false, true)
        WaitEventTask.EventReceived:Add(self, self.OnPreInputEvent)
        WaitEventTask:ReadyForActivation()
    end

    self:SetupWaitInputPress()

    -- 用 Advance() 启动第一段。别再单独调一次 PlayStep，否则第一段会被播两遍
    self:Advance()
end

-- 能力结束的兜底：清标志 + 收掉 task，免得影响下一次连招
function M:K2_OnEndAbility(bWasCancelled)
    self:ResetComboFlags()
    SafeEndTask(self.MontageTask)
    self.MontageTask = nil
end

--------------------------------------------------------------------------------
-- 输入
--------------------------------------------------------------------------------

function M:SetupWaitInputPress()
    -- UnLua 不会替你填 UFUNCTION 的默认参数，第二个 bTestAlreadyPressed 必须显式传，
    -- 否则报 "parameter 1: bool needed but got no value"
    local Task = UE.UAbilityTask_WaitInputPress.WaitInputPress(self, false)
    Task.OnPress:Add(self, self.OnAttackPressed)
    Task:ReadyForActivation()
end

function M:OnAttackPressed(TimeWaited)
    -- WaitInputPress 是一次性任务，触发后必须重建，否则第二次按键收不到
    self:SetupWaitInputPress()

    if self.IsCanInput then
        self.IsBuffer = true
    end

    -- 窗口已关且没缓存过 → 后续监听阶段，按下去就切
    if self.IsCanNext then
        self:Advance()
    end
end

--------------------------------------------------------------------------------
-- 预输入事件（ANS 发过来的）
--------------------------------------------------------------------------------

function M:OnPreInputEvent(Payload)
    local Magnitude = Payload and Payload.EventMagnitude or 0

    if Magnitude == 1 then
        -- 窗口开启，开始缓存。顺手清掉上一段可能残留的缓存标记
        self.IsCanInput = true
        self.IsBuffer   = false
        return
    end

    -- Mag=2 是预留的"切段请求"，当前 ANS 不会发。
    -- 留着是将来若把结算搬回 ANS 时不用改 GA
    if Magnitude == 2 then
        self:Advance()
        return
    end

    if Magnitude == 0 then
        -- 窗口关闭，转入后续监听
        self.IsCanNext = true
        if self.IsBuffer then
            self:Advance()
        end
    end
end

--------------------------------------------------------------------------------
-- 段推进
--------------------------------------------------------------------------------

function M:GetStepCount()
    local Montages = self.AttackMontages
    if not Montages then return 0 end
    return Montages:Length()
end

function M:Advance()
    self:ResetComboFlags()

    if self.StepIndex >= self:GetStepCount() then
        if not bLoopCombo then
            -- 打完最后一段就收招。这里**不能**调 K2_EndAbility：
            -- 此刻最后一段才刚要播，结束能力会把它连动画一起掐掉，
            -- 表现为"最后一段永远打不出来"。让它自然播完，由 OnCompleted 收尾。
            return
        end
        self.StepIndex = 1
    else
        self.StepIndex = self.StepIndex + 1
    end

    self:PlayStep(self.StepIndex)
end

function M:PlayStep(Index)
    local Montages = self.AttackMontages
    if not Montages or Index < 1 or Index > Montages:Length() then return end
    local Montage = Montages:Get(Index)
    if not Montage then return end

    -- 上一段的 task 先结束掉。不结束会堆积：切段时旧蒙太奇被停，
    -- 旧 task 的中断回调照样广播，里面的清理逻辑会误伤刚开的新段。
    -- EndTask 之后 task 不再广播委托，所以 OnMontageInterrupted 只会被真正受击打断触发。
    SafeEndTask(self.MontageTask)
    self.MontageTask = nil

    self.CurrentMontage = Montage

    -- 补齐 UE5 的完整签名（UnLua 不填默认参数，9 个都得给全）。
    -- FName 类型的参数（TaskInstanceName / StartSection）要传 **Lua 字符串**，
    -- 传 UE.FName(...) 反而会报 "string needed but got userdata"。
    -- StartSection 传空串（= NAME_None）从头播，别写死 "Default"，蒙太奇里未必有这个 section
    local Task = UE.UAbilityTask_PlayMontageAndWait.CreatePlayMontageAndWaitProxy(
        self,
        "ComboStep",
        Montage,
        1.0,
        "",
        true,
        1.0,
        0.0,
        false
    )
    Task.OnCompleted:Add(self, self.OnMontageCompleted)
    Task.OnInterrupted:Add(self, self.OnMontageCompleted)
    Task.OnBlendOut:Add(self, self.OnMontageCompleted)
    Task:ReadyForActivation()
    self.MontageTask = Task
end

--------------------------------------------------------------------------------
-- 蒙太奇回调
--------------------------------------------------------------------------------

function M:OnMontageCompleted()
    -- 本段播完：能力结束，后续监听到此为止
    self.MontageTask = nil
    self:ResetComboFlags()
    self:K2_EndAbility()
end

function M:OnMontageInterrupted()
    -- 受击 / 其他技能打断：只清标志。
    -- 不调 K2_EndAbility —— 能力此刻已在结束流程中，再调会撞 NotActive 校验刷 warning。
    -- OnBlendOut / OnCancelled 故意不接：
    --   BlendOut 正常播完也会先触发，会把收招提前掐掉；
    --   Cancelled 时能力已经在结束流程里。
    self:ResetComboFlags()
end

function M:ResetComboFlags()
    self.IsBuffer   = false
    self.IsCanInput = false
    self.IsCanNext  = false
end

return M
