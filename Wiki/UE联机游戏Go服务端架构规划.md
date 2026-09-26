# UE 联机游戏 · Go 服务端架构规划

> 适用：UE 客户端 + Go 服务端 + UE Dedicated Server(DS) 的联机游戏
> 覆盖：网关/逻辑服/房间分层、UE 联调、运营后台、邮件与公告收发

---

## 0. 总览

### 0.1 分层架构

```
                          ┌─────────────────┐
                          │    UE Client    │
                          └────────┬────────┘
                     HTTP(登录/公告) │ TCP长连接+PB(实时业务)
                          ▼         ▼
┌──────────────────────────────────────────────────────────┐
│  接入层  Gateway（无状态，可水平扩展，LB）                  │
│  连接管理 / PB编解码 / 鉴权 / 心跳 / 限流 / 路由 / 广播      │
└───────────────────────────┬──────────────────────────────┘
                            │  内部 gRPC
┌───────────────────────────▼──────────────────────────────┐
│  逻辑层（模块边界清晰，初期可同进程）                       │
│  ┌────────┬────────┬────────┬────────┬────────┐          │
│  │Player  │ Room   │ Social │  Mail  │ Notice │  ...     │
│  │角色属性│ 房间   │ 社交   │  邮件   │  公告  │          │
│  └────────┴────────┴────────┴────────┴────────┘          │
└───────────────────────────┬──────────────────────────────┘
                            │
┌───────────────────────────▼──────────────────────────────┐
│  公共服务层                                                │
│  Auth  DSManager  Storage  Pay  Activity  Match  Log      │
└───────────┬──────────────────────────┬───────────────────┘
            │                          │
      ┌─────▼─────┐            ┌───────▼────────┐
      │ MySQL/Redis│            │ UE DS 进程池   │
      └───────────┘            │ (战斗同步 UDP) │
                               └────────────────┘

  ┌──────────────┐   HTTP+JWT    ┌─────────┐  gRPC  ┌────────┐
  │ 运营后台 Web │ ────────────► │  GM 服  │ ─────► │ 逻辑服 │
  └──────────────┘               └─────────┘        └────────┘
```

### 0.2 技术选型总表

| 层 / 链路 | 选型 | 理由 |
|---|---|---|
| 开发语言 | Go 1.22+ | 高并发、部署简单、生态够用 |
| UE ↔ Gateway | **TCP + Protobuf** 长连接 | 可靠有序、体积小、跨语言 |
| UE ↔ UE DS | **UDP**（UE NetDriver 自带） | 战斗同步，引擎原生支持 |
| Gateway ↔ 逻辑服 | **gRPC** | 强类型、流控、内网高效 |
| 逻辑服 ↔ 逻辑服 | **gRPC**（同步）+ **NATS/Kafka**（异步事件） | 按场景选 |
| 后台 ↔ GM 服 | **HTTP + JSON**（Gin） | 简单、易调试、易对接前端 |
| UE DS ↔ Go | **gRPC** | DS 回传战斗结果/状态 |
| 服务发现 | etcd / Consul | 节点注册与健康检查 |
| 持久化 | **MySQL 8**（主）+ **Redis 7**（缓存） | 事务 + 高速 |
| 长连接库 | 标准 `net` 或 `gnet` | 量小用 net，量大用 gnet |
| 日志/监控 | zap + Prometheus + Grafana | 可观测性 |

---

## 1. Go 服务端架构

### 1.1 进程/模块划分

**接入层**
- **Gateway**：**无状态**，水平扩展，前面挂 LB。
  - 职责：TCP 长连接管理、PB 编解码、token 鉴权、心跳保活、限流、路由转发、广播。
  - **不做业务逻辑**，保证轻量可扩。

**逻辑层**（按业务边界划分，注意：模块 ≠ 进程）
- `Player`：角色数据、属性成长、背包、货币
- `Room/Scene`：房间与副本管理、**DS 申请与回收**
- `Social`：好友、组队、聊天
- `Mail`：邮件
- `Notice`：公告
- `Task / Achievement`：任务、成就
- `Shop / Trade`：商城、交易

**公共服务层**
- `Auth`：账号、Token 签发与校验、区服列表
- `DSManager`：**UE DS 进程池调度**（申请/回收/健康检查）
- `Storage`：DB 访问封装（也可各服直连）
- `Pay`：充值回调与发货
- `Activity`：活动配置与判定
- `Match`：跨服匹配
- `GM`：后台接口入口，**隔离后台与内部服务**

**基础设施**：MySQL、Redis、etcd、NATS、Prometheus。

### 1.2 进程划分建议（按阶段演进）

| 阶段 | 部署形态 | 说明 |
|---|---|---|
| 验证期 | `gate` × 1 + `logic` × 1（所有业务合并） | **模块化 monolith**，进程内用 package 划清边界，迭代最快 |
| 上线期 | `gate` × N + `player` / `room` / `social` / `mail` 独立进程 | 按压力与发版频率拆分 |
| 规模化 | 微服务 + 服务发现 + MQ + 分库分表 | 跨服、弹性伸缩 |

> **建议**：不要一开始就拆十几个微服务。先用 monolith 但**模块边界写清楚**，等真的需要"独立扩缩容"或"独立发版"时再拆进程——拆早了只会增加调试和部署成本。

### 1.3 通信协议详细选型

| 链路 | 协议 | 关键理由 |
|---|---|---|
| 登录、拉公告、商城列表 | **HTTP** | 无状态、可缓存、调试方便 |
| 游戏内实时交互 | **TCP 长连接 + PB** | 需要服务端主动推送（邮件、聊天） |
| 战斗同步 | **UDP**（UE DS） | 高频、容忍少量丢包，引擎原生 |
| 后台管理 | **HTTP + JSON** | 运营系统，实时性要求低 |

**为什么业务主链路用 TCP 而不是 UDP？**
- 对 MMO / ARPG / 卡牌类，TCP 完全够用，且**省掉自己做可靠传输、重排、拥塞控制**。
- 若是 FPS/竞技类：移动同步可走 UDP/KCP，但**业务指令（领邮件、开背包）仍走 TCP**。
- 推荐形态：**TCP（业务） + UDP（UE DS 战斗）双通道**，各司其职。

**自定义包格式**（TCP 之上）：
```
Header（固定 16 字节，网络字节序）
  magic    uint16  // 0xABCD，非法包快速丢弃
  version  uint8   // 协议版本
  cmd      uint32  // 消息号
  seq      uint32  // 请求-响应配对 / 去重
  bodyLen  uint32  // body 长度
  reserve  uint16
Body
  protobuf 序列化字节（长度 = bodyLen）
```

**cmd 号分段规划**（避免冲突）：
```
1000-1999  登录/鉴权      2000-2999  玩家/背包
3000-3999  房间/战斗      4000-4999  邮件
5000-5999  公告           6000-6999  社交/好友/组队
7000-7999  商城/交易      9000-9999  系统/心跳/踢线
```

### 1.4 数据持久化选型

| 数据 | 存储 | 说明 |
|---|---|---|
| 玩家存档（角色/属性/背包/货币） | **MySQL** 权威 + **Redis** 缓存 | 在线时缓存，变更落库 |
| **邮件** | **MySQL** | 必须持久化，靠事务保证"只领一次" |
| **公告** | **MySQL** + Redis 缓存 | 读多写少，全服公告进缓存 |
| 在线状态 / Session / 所在 Gate | **Redis** | 临时，带 TTL，可丢 |
| 排行榜 | **Redis ZSet** | 天然有序 |
| 日志 | 文件 → ES / ClickHouse | 量大，异步写 |
| 充值订单 | **MySQL** | 强一致，需对账 |

**玩家存档读写策略**
- **在线时**：数据加载进 Redis（或 Player 服进程内存），读写走缓存。
- **落库时机**：定时（如 5 分钟） + 关键事件（下线、充值、领取邮件、交易）+ 服务优雅退出。
- **资产类操作**（货币、道具变更）：**立即落库 + DB 事务**，绝不能只信内存。

### 1.5 服务发现与 DS 管理

- **服务发现**：Gate / 逻辑服启动时向 etcd 注册地址与健康状态；Gate 通过服务发现拿到逻辑服地址，避免硬编码。
- **DS 调度流程**：
  1. 客户端请求进入房间 → `Room` 服处理
  2. `Room` 服向 `DSManager` 申请一个 DS 实例
  3. `DSManager` 从进程池取空闲 DS（不足则拉起新进程），返回 `IP:Port`
  4. `Room` 服把 DS 地址下发给客户端
  5. 客户端用 UDP 直连 DS 进入战斗
  6. 战斗结束 → `DSManager` 回收 DS（重置后复用或销毁）
- DS 与 Go 之间：DS 通过 gRPC 上报战斗开始/结束/结果，Go 侧据此结算奖励（**结算走逻辑服，DS 不直接发奖**）。

---

## 2. UE 联调方式

### 2.1 接入方式

UE 侧用 **C++ `FSocket` 建 TCP 连接**，集成 Protobuf。三种可选路径：

| 方案 | 做法 | 适用 |
|---|---|---|
| **A. 自研 TCP + PB**（推荐） | `FSocket`/`FTcpSocketBuilder` + 编译 libprotobuf 进 UE（或用 `ProtobufForUE` 类插件），自己封包解包 | 客户端 ↔ 业务服，最常用 |
| B. gRPC | 用 UE 的 gRPC 插件 | 更适合 **UE DS ↔ Go**，客户端直连偏重 |
| C. HTTP | `FHttpModule` | 登录、拉公告等非实时接口 |

**推荐组合**
- 登录 / 公告 / 商城列表 → **HTTP**（简单可缓存）
- 实时业务（移动指令、聊天、邮件推送） → **TCP 长连接 + PB**
- 战斗同步 → **UDP 连 UE DS**（引擎原生）

### 2.2 协议定义（Protobuf）

**一份 `.proto`，Go 与 C++ 各自生成代码**，避免手写协议不一致。

```protobuf
syntax = "proto3";
package game;

// ---- 登录 ----
message C2S_Login {
  string token     = 1;
  uint64 player_id = 2;
  string version   = 3;   // 客户端版本号，用于强更判定
}
message S2C_Login {
  int32  code   = 1;
  string msg    = 2;
  PlayerInfo player = 3;
}

// ---- 心跳 ----
message C2S_Heartbeat { int64 client_time = 1; }
message S2C_Heartbeat { int64 server_time = 1; }

// ---- 邮件 ----
message ItemStack { uint32 item_id = 1; uint32 count = 2; }

message MailInfo {
  uint64 mail_id    = 1;
  string title      = 2;
  string content    = 3;
  repeated ItemStack attachments = 4;
  int32  status     = 5;   // 0未读 1已读 2已领取 3已删除
  int64  create_time = 6;
  int64  expire_time = 7;
}
message C2S_GetMailList {}
message S2C_MailList { repeated MailInfo mails = 1; int32 unread = 2; }
message C2S_ClaimMail { uint64 mail_id = 1; string request_id = 2; }
message S2C_ClaimMailResult {
  int32 code = 1;                       // 0成功 / 其他失败
  repeated ItemStack got = 2;
}
message S2C_NewMail   { MailInfo mail = 1; }   // 服务端主动推送

// ---- 公告 ----
message NoticeInfo {
  uint64 notice_id = 1;
  string title     = 2;
  string content   = 3;
  int32  type      = 4;
  int64  version   = 5;
  string update_desc = 6;
  int64  start_time = 7;
  int64  end_time   = 8;
}
message S2C_NoticeList { repeated NoticeInfo notices = 1; }
message S2C_NewNotice  { NoticeInfo notice = 1; }
```

### 2.3 连接建立 / 心跳 / 重连

**建连流程**
1. UE 启动 → HTTP 调 `Auth`：`POST /login`（账号密码或第三方 token）
2. Auth 返回 `access_token` + Gate 地址列表
3. UE 建 TCP 连 Gate
4. UE 发 `C2S_Login{token, player_id, version}`
5. Gate 校验 token（本地验 JWT 或问 Auth）→ 绑定 `uid ↔ conn`，写 Redis 在线状态
6. 返回 `S2C_Login`（玩家数据 + 未读邮件数 + 有效公告）
7. 进入游戏

**心跳**
- UE 每 **5s** 发 `C2S_Heartbeat`；Gate 回 `S2C_Heartbeat`（带服务器时间，顺便对时）
- Gate：**15s** 未收到心跳 → 断开
- UE：**15s** 未收到服务端任何包 → 判定断线

**重连（指数退避）**
- 断线后：1s → 2s → 4s → 8s → …上限 30s 重试
- 重连带 `session_key + player_id`，Gate 查 Redis：session 仍有效 → **恢复会话**，不走完整登录
- session 已过期 → 重新走 HTTP 登录拿 token
- UI 全程提示"连接中…"

**消息可靠性**
- 请求带 `seq`，响应回带同一 `seq`；客户端超时重发
- **去重靠服务端幂等**（不是靠客户端只发一次）
- 推送类（新邮件）客户端回 ACK（可选，用于补发保障）

### 2.4 本地联调与测试流程

**本地环境**
```bash
docker-compose up -d    # MySQL + Redis + etcd 一键起
air                     # Go 热重载
```
UE 编辑器连 `127.0.01:<gate_port>`。

**联调与测试手段**
- **协议文档**：用脚本从 `.proto` 生成 cmd ↔ message 对照表，避免口头对齐
- **Bot 模拟器**：写一个 Go 小客户端，能连 Gate、登录、发心跳、领邮件——**不依赖 UE 就能自测服务端**
- **抓包**：开发模式打印收发包日志；或用 Wireshark 解自定义协议
- **单测**：重点覆盖 PB 编解码、邮件领取幂等、公告版本过滤
- **压测**：Bot 起 N 个连接测并发、心跳、重连风暴
- **UE 多开**：编辑器 PIE 多开 2–3 个客户端，测广播、聊天、房间同步

**推荐测试顺序**
1. Bot ↔ Go 单协议自测
2. UE 单客户端跑通：登录 → 主城 → 收邮件
3. PIE 多开：广播、聊天、组队
4. 断网 / 杀进程：验证重连
5. Bot 压测

---

## 3. 后台管理系统

### 3.1 是否需要？—— 需要，且是刚需

没有后台意味着每次发补偿、发公告都要人工改库或找程序介入，效率低且高危。必备能力：

- 发补偿邮件（版本更新、BUG 补偿）
- 发布全服公告（含更新时间说明、维护公告）
- 客服查玩家、封禁
- 操作审计（谁改了什么）

### 3.2 功能模块

| 模块 | 能力 |
|---|---|
| 邮件管理 | 个人 / 批量 / 全服邮件，带附件，支持定时发送、撤回 |
| 公告管理 | 发布 / 编辑 / 下线，设生效时间段、优先级、类型（维护/更新/活动） |
| 玩家管理 | 查询、详情、封禁/解封、数据修正（高危，需审批） |
| 活动管理 | 活动配置 |
| 数据看板 | 在线数、DAU、充值 |
| 权限管理 | 用户 / 角色 / 权限 |
| 操作审计 | 全量记录 |

### 3.3 权限设计（RBAC）

表：`admin_user` / `role` / `permission` / `user_role` / `role_permission`

| 角色 | 权限 |
|---|---|
| 只读（客服） | `player:view`、`mail:view` |
| 运营 | + `mail:send`、`notice:publish` |
| 管理员 | + `mail:send_global`、`player:ban` |
| 超管 | + 权限管理、数据修正 |

**安全要求**
- 后台部署内网 / 仅 VPN 可达 + IP 白名单
- JWT 鉴权，带过期
- **敏感操作二次确认 + 双人审批**（全服邮件、封号、改数据）
- 所有写操作落 `admin_audit_log`

### 3.4 接口设计

```
POST  /api/v1/auth/login              登录

POST  /api/v1/mail/send               发邮件
GET   /api/v1/mail/list               邮件列表
POST  /api/v1/mail/revoke             撤回（仅未领取）

POST  /api/v1/notice/publish          发布公告
PUT   /api/v1/notice/{id}             编辑
POST  /api/v1/notice/{id}/offline     下线
GET   /api/v1/notice/list             列表

GET   /api/v1/players/{id}            查玩家
POST  /api/v1/players/{id}/ban        封禁
```

**发邮件请求**
```json
{
  "target_type": "global",
  "player_ids": [],
  "title": "版本更新补偿",
  "content": "感谢支持，本次更新...",
  "attachments": [
    {"item_id": 1001, "count": 100}
  ],
  "expire_days": 30,
  "send_time": "2026-09-25T18:00:00Z"
}
```
**响应**
```json
{"code": 0, "msg": "ok", "data": {"global_mail_id": 10086}}
```

**调用链（后台与内部服务隔离）**
```
Web后台 ──HTTP──► GM服 ──gRPC──► Mail服 ──► DB
                    │                  └──► Gate ──► 在线玩家
                    └──► admin_audit_log
```
GM 服是**隔离层**：后台不直接碰逻辑服与 DB，便于审计和权限收敛。

---

## 4. 邮件与公告收发机制

### 4.1 邮件收发

**在线即时推送（push）**
1. Mail 服写 DB 成功
2. 查 Redis：玩家是否在线、在哪个 Gate 节点
3. 在线 → 经 Gate 下发 `S2C_NewMail`（可推全文或只推摘要由客户端再拉详情）
4. UE 收包 → 红点、未读数 +1、可弹窗

**离线补发（pull）**
1. 玩家登录 → Player 服加载数据
2. 调 Mail 服拉"未读/未领取且未过期"的邮件
3. 返回列表 → UE 展示红点

> **push 保证实时，pull 保证不丢**。两者操作同一份 DB 数据，靠 `status` 保持一致。

### 4.2 公告收发

**在线推送**
- 发布公告 → Notice 服写 DB → 经 Gate **全服广播** `S2C_NewNotice`
- （备选：客户端定时轮询，实现简单但不够实时，可作兜底）

**离线补发**
- 登录时拉 `version > player.last_notice_ver` 且处于生效期内的公告
- 或按 `start_time <= now <= end_time` 过滤未读公告

**更新时间说明**
- 公告表设 `update_desc` 字段，明确写维护时间段与更新内容
- 用 `version` 做判重：客户端记 `last_notice_ver`，简单可靠

### 4.3 状态由内存还是 DB 管理？

**结论：DB 是权威，Redis 是缓存，进程内存只做临时。**

| 状态 | 存储 | 说明 |
|---|---|---|
| 邮件记录（内容/状态/附件） | **MySQL（权威）** | 不能丢 |
| 邮件未读数 | Redis 缓存 | 丢了可从 DB 重算 |
| 在线状态 / 所在 Gate | Redis（带 TTL） | 临时 |
| 公告内容 | MySQL + Redis 缓存 | 读多写少 |
| 已读公告版本 | MySQL（玩家表字段） | 需持久化 |

> **铁律：凡是涉及资产（货币、道具）的变更，必须 DB 事务保证。** Redis 和内存只能加速，不能作为最终依据。

### 4.4 判重与"只领一次"（核心正确性）

三层防护：

**① DB 层 CAS（核心）**
```sql
UPDATE player_mail
SET status = 2, claim_time = NOW()
WHERE mail_id = ? AND player_id = ? AND status IN (0, 1);
```
- `RowsAffected == 1` → 允许发放
- `RowsAffected == 0` → 已被领取/已删除 → 拒绝
- 这是**原子操作**，并发下天然安全

**② 幂等键（防重复提交）**
- 客户端领邮件带 `request_id`（UUID）
- `mail_claim_log` 建 `UNIQUE(mail_id)`：一条邮件只能有一条领取流水
- 重复请求直接返回上次结果

**③ Redis 分布式锁（可选优化）**
- 领取前 `SETNX mail:claim:{mail_id}`，抢到再走 DB，减轻 DB 压力
- **锁只是优化，绝不能替代 DB 的 CAS**

**全服邮件：避免百万级 INSERT**
- 人数可控 → 直接批量 INSERT 每人一条
- 人数多 → **模板 + 惰性生成**（推荐）：
  1. 存一条 `global_mail` 模板
  2. 玩家表记 `last_global_mail_id` 水位
  3. 玩家登录时：取 `id > last_global_mail_id` 的模板 → 为其生成 `player_mail` 个人记录 → 更新水位
  4. 避免全服一次性插入几百万行

**过期清理**：定时任务把 `expire_time` 过期的邮件标记删除；已领取且超保留期的归档。

---

## 5. 关键数据表设计

### 5.1 玩家 `player`
```sql
CREATE TABLE player (
  id                  BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  account_id          BIGINT UNSIGNED NOT NULL COMMENT '账号ID',
  nickname            VARCHAR(64) NOT NULL,
  level               INT NOT NULL DEFAULT 1,
  exp                 BIGINT NOT NULL DEFAULT 0,
  vip_level           INT NOT NULL DEFAULT 0,
  gold                BIGINT NOT NULL DEFAULT 0,
  diamond             BIGINT NOT NULL DEFAULT 0,
  last_login_time     DATETIME,
  last_logout_time    DATETIME,
  last_notice_ver     BIGINT NOT NULL DEFAULT 0 COMMENT '已读公告版本(判重)',
  last_global_mail_id BIGINT NOT NULL DEFAULT 0 COMMENT '全服邮件水位(惰性生成)',
  status              TINYINT NOT NULL DEFAULT 0 COMMENT '0正常 1封禁',
  create_time         DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  update_time         DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  UNIQUE KEY uk_account (account_id),
  KEY idx_nickname (nickname)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

### 5.2 邮件 `player_mail`
```sql
CREATE TABLE player_mail (
  id           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  mail_id      BIGINT UNSIGNED NOT NULL COMMENT '全局唯一(雪花ID)',
  player_id    BIGINT UNSIGNED NOT NULL,
  mail_type    TINYINT NOT NULL DEFAULT 0 COMMENT '0系统 1个人 2全服 3活动',
  title        VARCHAR(128) NOT NULL,
  content      TEXT,
  attachments  JSON COMMENT '[{item_id,count}]',
  status       TINYINT NOT NULL DEFAULT 0 COMMENT '0未读 1已读 2已领取 3已删除',
  sender       VARCHAR(64) DEFAULT '系统',
  create_time  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  expire_time  DATETIME,
  read_time    DATETIME,
  claim_time   DATETIME,
  PRIMARY KEY (id),
  UNIQUE KEY uk_mail_id (mail_id),
  KEY idx_player_status (player_id, status),
  KEY idx_expire (expire_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

### 5.3 全服邮件模板 `global_mail`
```sql
CREATE TABLE global_mail (
  id           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  title        VARCHAR(128) NOT NULL,
  content      TEXT,
  attachments  JSON,
  mail_type    TINYINT NOT NULL DEFAULT 2,
  expire_days  INT NOT NULL DEFAULT 30,
  send_time    DATETIME COMMENT '定时发送时间,NULL=立即',
  operator_id  BIGINT COMMENT '后台操作人',
  create_time  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  KEY idx_send_time (send_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

### 5.4 公告 `notice`
```sql
CREATE TABLE notice (
  id           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  title        VARCHAR(128) NOT NULL,
  content      TEXT NOT NULL,
  notice_type  TINYINT NOT NULL DEFAULT 0 COMMENT '0普通 1维护 2更新 3活动',
  priority     INT NOT NULL DEFAULT 0 COMMENT '越大越靠前',
  version      BIGINT NOT NULL COMMENT '公告版本号(判重用)',
  start_time   DATETIME NOT NULL,
  end_time     DATETIME NOT NULL,
  update_desc  VARCHAR(255) COMMENT '更新时间说明',
  status       TINYINT NOT NULL DEFAULT 1 COMMENT '0下线 1上线',
  operator_id  BIGINT,
  create_time  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  KEY idx_version (version),
  KEY idx_time (start_time, end_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

### 5.5 公告已读 `notice_read`
```sql
CREATE TABLE notice_read (
  id         BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  player_id  BIGINT UNSIGNED NOT NULL,
  notice_id  BIGINT UNSIGNED NOT NULL,
  read_time  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  UNIQUE KEY uk_player_notice (player_id, notice_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```
> 简化方案：只维护 `player.last_notice_ver`，不记每篇已读，够用且省空间。

### 5.6 领取流水（幂等 + 审计）`mail_claim_log`
```sql
CREATE TABLE mail_claim_log (
  id          BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  mail_id     BIGINT UNSIGNED NOT NULL,
  player_id   BIGINT UNSIGNED NOT NULL,
  request_id  VARCHAR(64) COMMENT '客户端幂等键',
  attachments JSON,
  claim_time  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  UNIQUE KEY uk_mail (mail_id),
  UNIQUE KEY uk_request (request_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

### 5.7 后台账号与审计
```sql
CREATE TABLE admin_user (
  id            BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  username      VARCHAR(64) NOT NULL,
  password_hash VARCHAR(255) NOT NULL,
  status        TINYINT NOT NULL DEFAULT 1,
  create_time   DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  UNIQUE KEY uk_username (username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE admin_audit_log (
  id          BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  operator_id BIGINT UNSIGNED NOT NULL,
  action      VARCHAR(64) NOT NULL COMMENT 'mail:send / notice:publish / player:ban',
  target      VARCHAR(255),
  params      JSON,
  ip          VARCHAR(64),
  result      TINYINT,
  create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  KEY idx_operator (operator_id),
  KEY idx_time (create_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

---

## 6. 完整邮件发放时序（全服补偿 100 钻石）

### A. 运营后台操作
```
1  运营登录后台（JWT 鉴权）
2  填写：标题"版本更新补偿" / 内容 / 附件[{钻石,100}] / 过期30天 / 立即发送
3  后台校验权限（需 mail:send_global）
4  后台 ──HTTP──► GM服: POST /api/v1/mail/send {target_type:"global", ...}
5  GM服 写 admin_audit_log（操作人 / 参数 / IP / 时间）
6  GM服 ──gRPC──► Mail服: SendGlobalMail(req)
```

### B. Mail 服处理
```
7  Mail服 校验参数（附件合法性、过期时间、权限）
8  ── 事务开始 ──
   INSERT INTO global_mail(...) → 得到 global_mail_id = 10086
   ── 事务提交 ──
   同时写入 Redis 缓存该模板
9  立即发送：
   - 在线玩家 → 走 C 阶段推送
   - 离线玩家 → 不逐条插入，等登录惰性生成
10 Mail服 ──► GM服 成功（带 global_mail_id）
11 GM服 ──► 后台 {"code":0,"data":{"global_mail_id":10086}}
12 后台提示"发放成功"
```

### C. 在线玩家即时收到
```
13 Mail服 取在线玩家列表（Redis）
14 对在线玩家生成 player_mail 记录（或直接推模板引用）
   INSERT INTO player_mail(mail_id, player_id, ..., status=0)
15 查 Redis 得到玩家所在 Gate 节点
16 Mail服 ──gRPC──► Gate: PushToPlayer(player_id, cmd=S2C_NewMail, payload)
17 Gate ──TCP──► UE: S2C_NewMail{mail_id, title, ...}
18 UE 收到 → 红点 + 未读数 +1（可弹窗）
19 UE 回 ACK（可选）
```

### D. 离线玩家登录后补发
```
20 玩家启动 → HTTP 登录 → 拿 token + Gate 地址
21 UE 建 TCP 连 Gate → 发 C2S_Login
22 Gate ──► Player服: 加载玩家
23 Player服 ──gRPC──► Mail服: GetMailList(player_id, last_global_mail_id)
24 Mail服 检查 global_mail 中 id > last_global_mail_id 的模板
   → 为该玩家惰性生成 player_mail 记录
   → 更新 player.last_global_mail_id = 当前最大模板 id
25 Mail服 返回未读邮件列表 + 未读数
26 Gate ──► UE: S2C_Login{player, mails[], unread}
   （或 UE 单独调 C2S_GetMailList → S2C_MailList）
27 UE 展示红点 + 邮件列表
```

### E. 玩家领取附件（关键：只领一次）
```
28 玩家点"领取" → UE 发 C2S_ClaimMail{mail_id, request_id}
29 Gate ──► Mail服: ClaimMail(player_id, mail_id, request_id)
30 Mail服：
   a. （可选）Redis 分布式锁 SETNX mail:claim:{mail_id}
   b. ── 事务开始 ──
   c. CAS 更新：
      UPDATE player_mail SET status=2, claim_time=NOW()
      WHERE mail_id=? AND player_id=? AND status IN (0,1)
   d. 检查 RowsAffected：
      == 1 → 继续；== 0 → 回滚，返回"已领取/不存在"，结束
   e. INSERT INTO mail_claim_log(mail_id, player_id, request_id, attachments)
      （UNIQUE(mail_id) 冲突 → 回滚，判为重复）
   f. 发放附件：更新 player.diamond += 100（同一事务）
   g. ── 事务提交 ──
   h. 释放 Redis 锁
31 Mail服 ──► Gate ──► UE: S2C_ClaimMailResult{code:0, got:[{钻石,100}]}
32 UE 播放领取动画、刷新货币、红点消失
```

### F. 并发验证
```
玩家快速点两次（或网络重发）：
  请求1 → CAS 成功（RowsAffected=1）→ 发放 100 钻石 ✓
  请求2 → status 已变 2，CAS 失败（RowsAffected=0）→ 拒绝 ✗
  即使绕过 CAS，mail_claim_log 的 UNIQUE(mail_id) 也会拦截
  结果：钻石只加一次 ✓
```

---

## 7. 落地路线图

| 里程碑 | 周期 | 内容 |
|---|---|---|
| M1 基建 | 1–2 周 | docker-compose(MySQL/Redis/etcd)；Go 项目骨架；.proto 定义与编解码；Gate 跑通（UE 能连、登录、心跳） |
| M2 核心业务 | 2–3 周 | Player 服（角色/背包）；**邮件系统（含领取幂等）**；公告系统；后台 MVP（能发邮件/公告） |
| M3 联机 | 3–4 周 | Room 服 + DSManager；UE DS 房间与战斗同步；聊天/好友 |
| M4 打磨 | 持续 | 压测、监控告警、日志；权限细化与审批流；跨服（匹配/排行榜） |

---

## 附：几个容易踩的坑

1. **Gateway 里写业务逻辑** → 网关变重、难扩容。网关只做转发与连接管理。
2. **只信 Redis 判重** → Redis 宕机/过期就重复发奖。**DB CAS 才是最终保障**。
3. **全服邮件一次性 INSERT 百万行** → 拖垮 DB。用模板 + 惰性生成。
4. **协议不版本化** → 客户端升级后旧包解析出错。Header 带 `version`，强制更新要有开关。
5. **DS 直接发奖励** → DS 不可信且会重启，**战斗结果必须回传逻辑服结算**。
6. **后台不做审计** → 出事无法追溯。所有写操作必须落 `admin_audit_log`。
7. **过早微服务化** → 调试成本爆炸。先用 monolith + 清晰模块边界。
