# GameHall · 轻量级联网游戏大厅

基于 **C++ + Select IO 多路复用** 实现的轻量级联网游戏大厅，包含用户系统、房间管理与实时对战。
服务端为单线程事件驱动模型，自研会话管理、消息路由与房间状态同步机制；客户端使用 **ImGui + DirectX 11** 构建界面，内置五子棋、贪吃蛇、推箱子等小游戏，其中五子棋支持完整的多人在线对战与服务端权威判定。

> 早期项目演示：https://www.bilibili.com/video/BV1L3epeeEP8

---

## 功能特性

**用户系统**
- 注册 / 登录 / 登出，账号持久化到 SQLite
- **抢占登录**：同一账号在别处登录时，服务端向旧连接下发顶号事件，由客户端二次确认后才完成踢线，避免误踢

**房间管理**
- 创建房间 / 刷新房间列表 / 加入 / 退出 / 返回 / 解散
- 房间内座位分配与切换、准备状态同步、再来一局投票
- 房主断线或房间空置时自动回收房间

**实时对战（五子棋）**
- 服务端持有 15×15 权威棋盘，落子由服务端校验并广播
- 四方向（水平 / 垂直 / 两条对角线）双向连子计数判定胜负，维护黑白双方比分

---

## 技术架构

### 服务端 `GameHallServerPro/`

| 模块 | 说明 |
| --- | --- |
| `EventLoopMgr` | 维护两条事件循环：**主循环**（网络 + 游戏逻辑）与 **DB 循环**（数据库 IO） |
| `TcpAccept` / `Session` | Select 非阻塞 IO，一个 `Session` 对应一条客户端连接，持有唯一 `netId` |
| `User` | 已登录用户的业务载体，接管登录后的消息处理 |
| `Room` / `RoomMgr` | 房间对象与房间表，管理成员、座位与游戏实例 |
| `GameBase` / `GameFactory` | 玩法抽象基类 + 工厂，按 `roomGameType` 多态创建具体游戏 |
| `DBMgr` | SQLite 封装，`dbver` 表做 schema 版本管理，首次启动自动建表 |

**异步数据库访问**：DB 操作被封装为 `IDBCell` 投递到 DB 事件循环执行，完成后再把结果封装成 `IGameCell` 回投主循环处理，避免数据库 IO 阻塞网络线程。主循环侧同理提供 `GameServer::Post()` 作为逻辑投递入口。

### 客户端 `GameHallPro/`

- `TcpClient`：Select 检测可读 + 循环拆包，与服务端对称
- UI 由 ImGui（含 `imgui_toggle` 扩展）绘制，DX11 渲染，图片资源见 `src/Image/`
- 玩法模块：`GoBangGame` / `SnakeGame` / `PushBoxGame`

### 通信协议

```
+---------------+---------------------------+
|  length (2B)  |   JSON body (length - 2)  |
+---------------+---------------------------+
```

- **2 字节长度前缀 + JSON 报文**，长度字段含自身，便于处理 TCP 粘包 / 拆包
- 双端均维护累积缓冲区：`recv` 追加到缓冲尾部，按长度字段循环取包，`OnNetMsg` 返回已消费字节数，剩余数据整体前移；返回 0 表示报文不完整，等待下一批数据
- 报文内 `cmd` 字段驱动消息路由，采用 **映射表（cmd → std::function）** 分发，替代 switch 分支

**三级路由**：未登录态由 `Session` 处理（注册 / 登录 / 登出 / 抢占确认）；登录后转交 `User`（房间 CRUD、刷新列表）；进入房间后转交 `Room`（座位、准备、落子、重开）。

主要指令：

| 方向 | cmd 示例 |
| --- | --- |
| C→S | `CS_Login` `CS_Register` `CS_Logout` `CS_SurePreemptLogin` |
| C→S | `CS_CreateRoom` `CS_RefreshRoomList` `CS_JoinRoom` `CS_ExitRoom` `CS_ReturnRoom` `CS_DeleteRoom` |
| C→S | `CS_MovePos` `CS_ReadyPos` `CS_GamePlay` `CS_GoBangDown` `CS_AgainGame` |
| S→C | `SC_*` 对应响应，含 `SC_PreemptLogin` 顶号事件 |

**连接生命周期**：`OnDisconnect` 中依次清理房间、用户与会话对象，保证异常断开不残留脏状态。

---

## 目录结构

```
GameHall/
├─ GameHallServerPro/          # 服务端（VS 工程）
│  ├─ src/                     # 业务源码
│  │  ├─ Session/              # 按指令拆分的消息处理单元
│  │  ├─ GameServer.{h,cpp}    # 全局会话/用户/房间管理 + 逻辑投递
│  │  ├─ EventLoopMgr.{h,cpp}  # 主循环 + DB 循环
│  │  ├─ DBMgr.{h,cpp}         # SQLite 与建表
│  │  └─ main.cpp              # 监听 9999 端口
│  └─ 3rd/XNet/                # 自研网络库（EventLoop / TcpSocket / TcpAccept / DBSqlite / SnowFlake）
└─ GameHallPro/                # 客户端（VS 工程）
   ├─ src/                     # ImGui UI、TcpClient、各玩法模块
   └─ src/Imgui/               # Dear ImGui 及其扩展
```

---

## 编译与运行

- 环境：Windows + Visual Studio 2022（工程为 `GameHallServerPro.sln` / `GameHallPro.sln`）
- 服务端监听端口 **9999**（见 `GameHallServerPro/src/main.cpp`）
- 启动顺序：先启动服务端，看到 `服务端 监听:0.0.0.0 端口:9999` 后启动客户端；运行目录需放置 `XNet.lib` 与 `GameServer.db`

```bash
# 客户端连接地址在 UI 中填写，服务端默认监听
服务端 监听:0.0.0.0 端口:9999
```

---

## 设计要点

- **Select 事件驱动**：单线程内完成 accept / recv / send 与逻辑派发，无锁竞争，逻辑天然串行
- **映射表路由**：`cmd → std::function` 动态分发，新增指令只需注册一个处理函数
- **逻辑与 IO 分离**：DB 操作异步投递到独立事件循环，结果回投主循环，网络线程不被阻塞
- **多态玩法扩展**：`GameBase` 抽象 + `GameFactory` 工厂，新增游戏类型只需派生子类并注册
- **schema 版本管理**：`dbver` 表记录数据库版本，未初始化时自动建表，便于后续升级迁移

## License

[GNU General Public License v3.0](./LICENSE)
