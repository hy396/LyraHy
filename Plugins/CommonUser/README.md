# CommonUser 插件说明

> 目录：`Plugins/CommonUser`
> 模块：`CommonUser`（Runtime / LoadingPhase: Default）
> 来源：Epic Games（Lyra 示例工程通用插件）

---

## 一、插件用途与适用场景

CommonUser 把「本地玩家 / 平台账号 / 在线服务账号」这套最容易写乱的东西统一封装成一套
与平台无关的接口，屏蔽 **OSSv1（OnlineSubsystem）** 与 **OSSv2（OnlineServices）** 两套在线层的差异。

它解决三类问题：

| 问题 | CommonUser 的做法 |
| --- | --- |
| 玩家是谁？ | `UCommonUserInfo` 统一描述：平台用户、输入设备、LocalPlayer、NetId、权限缓存 |
| 能不能玩 / 能不能联机？ | `ECommonUserPrivilege` + `ECommonUserAvailability` + `ECommonUserPrivilegeResult` 三层判定 |
| 怎么建房 / 找房 / 进房？ | `UCommonSessionSubsystem` 提供 Host / Find / QuickPlay / Join 四个入口 |

适用场景：

- 需要支持**多本地玩家**（分屏）或**运行时加入/退出**的游戏；
- 需要支持**主机平台**（平台层账号与服务层账号分离，需要处理「按 Start 键」、账号选择界面）；
- 需要给玩家显示**明确的联机失败原因**（没订阅、家长控制、需要更新、被封禁……）；
- 想让上层 UI 只写一套代码，不关心底层是 Steam / EOS / PSN / Xbox Live。

---

## 二、目录结构

```
Plugins/CommonUser/
├── CommonUser.uplugin
├── Resources/                     图标等资源
└── Source/CommonUser/
    ├── CommonUser.Build.cs        构建脚本（内含 OSS 版本开关）
    ├── Public/
    │   ├── CommonUserTypes.h               通用枚举与结果结构
    │   ├── CommonUserSubsystem.h           核心：用户管理子系统
    │   ├── CommonSessionSubsystem.h        联机会话/匹配子系统
    │   ├── CommonUserBasicPresence.h       富状态（Rich Presence）推送
    │   ├── AsyncAction_CommonUserInitialize.h  蓝图异步初始化节点
    │   └── CommonUserModule.h              模块入口
    └── Private/
        ├── CommonUserSubsystem.cpp         （最大，2600+ 行）
        ├── CommonSessionSubsystem.cpp       （1460 行）
        ├── CommonUserBasicPresence.cpp
        ├── AsyncAction_CommonUserInitialize.cpp
        ├── CommonUserTypes.cpp
        └── CommonUserModule.cpp
```

---

## 三、核心类 / 模块及其职责

### 1. `UCommonUserSubsystem`（GameInstance 子系统）—— 核心

本地用户登录的总管。对外主要接口：

| 函数 | 作用 |
| --- | --- |
| `TryToInitializeForLocalPlay` | 单机游玩初始化。可新建 LocalPlayer、可使用访客登录，申请 `CanPlay` |
| `TryToLoginForOnlinePlay` | 联机游玩登录。不新建 LocalPlayer，申请 `CanPlayOnline` |
| `TryToInitializeUser` | 通用入口，用 `FCommonUserInitializeParams` 完整描述一次请求 |
| `ListenForLoginKeyInput` | 注册「按任意键 / 按 Start 键」触发登录 |
| `CancelUserInitialization` | 取消进行中的初始化 |
| `TryToLogOutUser` | 登出（可选连 LocalPlayer 一起销毁） |
| `ResetUserState` | 出错返回主菜单时重置状态 |
| `SetTraitTags` / `HasTraitTag` | 由游戏告知平台特性（是否主机、是否单一在线用户） |
| `GetUserInfoForXxx` | 六种查询方式：LocalPlayer 索引 / 平台用户 / NetId / 输入设备…… |

**登录流水线**（`ProcessLoginRequest` 驱动，`FUserLoginRequest` 里每个阶段各有独立状态）：

```
TransferPlatformAuth   用平台令牌登录服务层（OSSv1 不支持，直接失败）
        │ 失败
        ▼
    AutoLogin          静默自动登录
        │ 失败
        ▼
   ShowLoginUI         弹出平台账号选择/登录界面（需要玩家交互）
        │ 失败
        ▼
 QueryUserPrivilege    查询所需权限（能否联机等）
        │
        ▼
   完成/失败回调       OnUserInitializeComplete / OnHandleSystemMessage
```

### 2. `UCommonUserInfo`

一个本地用户的运行时快照：绑定了哪个平台用户（`FPlatformUserId`）、哪个主输入设备、
LocalPlayer 索引、是否访客，以及按在线上下文缓存的 NetId 与权限结果。
**只读**；真正的登录动作都在子系统里。

### 3. `UCommonSessionSubsystem`（GameInstance 子系统）

联机会话管理。四个请求/结果对象：

| 类 | 作用 |
| --- | --- |
| `UCommonSession_HostSessionRequest` | 建房参数（联机模式 / 是否用大厅 / 地图 / 最大人数 / 额外 URL 参数） |
| `UCommonSession_SearchSessionRequest` | 搜索参数，搜索完成后 `Results` 被填充并触发完成委托 |
| `UCommonSession_SearchResult` | 单个搜索结果，包装底层 `FOnlineSessionSearchResult` 或 Lobby |
| `ECommonSessionInformationState` | 会话可展示状态：OutOfGame / Matchmaking / InGame |

主要流程函数：`HostSession`、`FindSessions`、`QuickPlaySession`、`JoinSession`、`CleanUpSessions`。
成功后统一走 `InternalTravelToSession`（主机 ServerTravel，客户端 ClientTravel）。

### 4. `UCommonUserBasicPresence`

极简富状态推送。订阅 `OnSessionInformationChangedEvent`，把「主菜单 / 匹配中 / 游戏中」
加上模式名、地图名推给平台好友系统。默认关闭（`bEnableSessionsBasedPresence = false`）。

### 5. `UAsyncAction_CommonUserInitialize`

把初始化包装成蓝图可 `await` 的异步节点：
`InitializeForLocalPlay` / `LoginForOnlinePlay` 两个工厂函数，绑定 `OnInitializationComplete` 即可。

### 6. `FCommonUserTags`

GameplayTag 集合：`SystemMessage_Error_*`（系统消息）、`Platform_Trait_*`（平台特性）。

---

## 四、依赖的其他插件或模块

**uplugin 层**：`OnlineSubsystem`、`OnlineSubsystemUtils`、`OnlineServices`

**Build.cs 层**：

- 公共依赖：`Core`、`CoreOnline`、`GameplayTags`
- 按开关二选一：`OnlineSubsystem`（v1） 或 `OnlineServicesInterface`（v2）
- 私有依赖：`OnlineSubsystemUtils`、`CoreOnline`、`CoreUObject`、`Engine`、`Slate`、`SlateCore`、`ApplicationCore`、`InputCore`

**OSS 版本开关**（`CommonUser.Build.cs` 第 11 行）：

```csharp
bool bUseOnlineSubsystemV1 = true;               // true = OSSv1，false = OSSv2
PublicDefinitions.Add("COMMONUSER_OSSV1=" + (bUseOnlineSubsystemV1 ? "1" : "0"));
```

源码里大量 `#if COMMONUSER_OSSV1 ... #else ... #endif` 依赖这个宏。
**切换这个开关等于换一整套实现，务必两套都编译验证过。**

---

## 五、接入与使用要点

### 1. 最小接入流程（蓝图）

```
InitializeForLocalPlay(Target=CommonUserSubsystem, LocalPlayerIndex=0, PrimaryInputDevice=默认, bCanUseGuestLogin=true)
   └─ OnInitializationComplete → bSuccess ? 进游戏 : 弹 Error
```

联机时改用 `LoginForOnlinePlay`。

### 2. C++ 侧典型写法

```cpp
auto* Users = GetGameInstance()->GetSubsystem<UCommonUserSubsystem>();
auto* Sessions = GetGameInstance()->GetSubsystem<UCommonSessionSubsystem>();

// 建房
auto* Req = Sessions->CreateOnlineHostSessionRequest();
Req->MapID = FPrimaryAssetId(...) ;   // 必须是合法的 World 主资源
Req->MaxPlayerCount = 8;
Sessions->HostSession(PC, Req);
```

### 3. 必须注意的坑

1. **MapID 必须是 PrimaryAssetId**：`UCommonSession_HostSessionRequest::MapID` 若不是合法的
   World 主资源，`GetMapName()` 会拿不到地图名，`ValidateAndLogErrors` 直接失败。
2. **`TryToInitializeUser` 返回 false 时不会有回调**：说明流程根本没启动
   （参数非法、目标玩家不存在等），调用方必须自己处理这个分支。
3. **权限 ≠ 可用性**：`GetCachedPrivilegeResult` 是平台返回的原始结果，
   `GetPrivilegeAvailability` 才综合了当前登录状态，UI 一般用后者。
4. **访客用户**：`bIsGuest = true` 的用户 `GetPlatformUserId()` 指向宿主主用户，
   不要把它当成独立平台用户去登录。
5. **平台特性标签要自己设**：`SetTraitTags` 不会自动调用，
   不设 `Platform_Trait_*` 的话 `ShouldWaitForStartInput()` 的结果不符合主机平台预期。
6. **OSSv1 不支持平台授权**：`TransferPlatformAuth` 在 OSSv1 下一开始就置为 Failed，
   会直接跳到 AutoLogin，这是预期行为不是 bug。
7. **PreClientTravel 是改 URL 的唯一时机**：客户端连接串解析完之后、Travel 之前触发，
   要加自定义参数只能在这里改。
8. **Rich Presence 默认关**：需要的话在 `DefaultEngine.ini` 里把
   `bEnableSessionsBasedPresence` 设成 true，并填好 `PresenceStatus*` / `PresenceKey*` 这几个键名
   （键名取决于后端平台，Steam / EOS / PSN 各不相同）。
9. **专用服务器**：`UCommonSessionSubsystem` 内部有 `bIsDedicatedServer` 分支，
   专用服务器没有 LocalPlayer 也能建房；但 `UCommonUserSubsystem` 的用户流程不适用。
10. **切换 OSS 版本要全量重编**：`COMMONUSER_OSSV1` 影响头文件里的类型定义
    （例如 `FOnlineErrorType` 到底是 `FOnlineError` 还是 `UE::Online::EOnlineServices` 的错误类型），
    只重编一部分模块会出现奇怪的链接错误。
