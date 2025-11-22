# 更新日志

x64dbg MCP 服务器插件的所有重要更改都将记录在此文件中。

格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
本项目遵循 [语义化版本](https://semver.org/lang/zh-CN/spec/v2.0.0.html)。

## [1.1.0] - 2025-11-20

### 新增
- **脚本执行 API**：以编程方式执行 x64dbg 命令
  - `script.execute`：执行单个 x64dbg 命令
  - `script.execute_batch`：按顺序执行多个命令，支持可配置的错误处理
  - `script.get_last_result`：获取最后执行命令的结果
- **上下文快照 API**：捕获和比较调试状态
  - `context.get_snapshot`：捕获完整的调试上下文（寄存器、线程、栈、模块、断点）
  - `context.get_basic`：快速捕获寄存器和基本状态
  - `context.compare_snapshots`：比较两个快照以识别差异
- 高级演示脚本（`examples/advanced_features_demo.py`），展示脚本执行和上下文快照
- 可配置的快照组件（可选包含线程、栈、模块、断点）

### 变更
- 更新配置文件，在允许的方法中包含 `context.*` 方法
- 由于新功能，插件大小增加到约 611 KB
- JSON-RPC 方法数量从 50+ 增加到 55+

### 改进
- 通过批量脚本执行增强自动化能力
- 通过状态比较更好地支持调试工作流
- 更灵活的调试上下文捕获

## [1.0.1] - 2025-11-19

### 新增
- 完整的线程管理 API
  - `thread.list`：列出所有线程
  - `thread.get_current`：获取当前线程信息
  - `thread.get`：获取特定线程详情
  - `thread.switch`：切换活动线程
  - `thread.suspend`：挂起线程执行
  - `thread.resume`：恢复线程执行
  - `thread.get_count`：获取线程数量
- 完整的调用栈跟踪 API
  - `stack.get_trace`：获取完整的调用栈
  - `stack.read_frame`：读取特定栈帧
  - `stack.get_pointers`：获取栈指针（ESP/RSP、EBP/RBP）
  - `stack.is_on_stack`：检查地址是否在栈上

### 变更
- 改进了所有处理器的错误处理
- 增强了日志系统，格式化更好

### 修复
- 线程切换可靠性
- 栈帧读取准确性

## [1.0.0] - 2025-11-18

### 新增
- 初始发布，包含完整的核心功能
- JSON-RPC 2.0 协议实现
- 支持事件流的 HTTP 服务器和 SSE
- 跨多个类别的 50+ 调试方法：
  - **调试控制**：run、pause、step_into、step_over、step_out、run_to、restart、stop、get_state
  - **内存操作**：read、write、search、get_info、enumerate、allocate、free、get_protection
  - **寄存器操作**：get、set、list、get_batch（50+ 寄存器）
  - **断点管理**：set、delete、enable、disable、toggle、list、get、delete_all、set_condition、set_log
  - **反汇编**：at、range、function
  - **符号解析**：resolve、from_address、search、list、modules
  - **注释/标签管理**：set_comment、get_comment、set_label
  - **模块信息**：list、get、get_main、find
- 基于权限的访问控制系统
- 配置文件支持（config.json）
- 多级日志系统
- 通过 SSE 的事件通知系统
- 带有 JSON-RPC 错误代码的全面错误处理
- Python 客户端示例

### 技术细节
- 使用 C++17 构建
- 使用 x64dbg 插件 SDK
- 依赖项：通过 vcpkg 的 nlohmann/json
- 支持 Windows (x64) 上的 x64dbg
- 插件大小：约 445 KB

## [0.3.0] - 2025-11-15（内部版本）

### 新增
- 硬件断点支持（DR0-DR3）
- 内存断点实现
- 条件断点系统
- 日志断点功能
- 内存搜索功能
- 批量寄存器读取操作

## [0.2.0] - 2025-11-12（内部版本）

### 新增
- 完整的断点管理
- 符号解析系统
- 注释和标签管理
- 事件回调处理器
- 权限检查器系统
- 配置管理

## [0.1.0] - 2025-11-10（内部版本）

### 新增
- 基础插件框架
- HTTP 服务器实现
- JSON-RPC 协议解析器
- 核心调试方法
- 内存和寄存器操作
- 初始文档

---

## 版本历史摘要

| 版本 | 发布日期 | 主要功能 |
|---------|-------------|--------------|
| 1.1.0 | 2025-11-20 | 脚本执行、上下文快照 |
| 1.0.1 | 2025-11-19 | 线程和栈管理 |
| 1.0.0 | 2025-11-18 | 首次公开发布 |
| 0.3.0 | 2025-11-15 | 高级断点 |
| 0.2.0 | 2025-11-12 | 符号和权限 |
| 0.1.0 | 2025-11-10 | 核心框架 |

---

## 未来路线图

### v1.2.0 计划
- 性能优化和基准测试
- 增强的安全功能（Token/API Key 认证）
- WebSocket 协议支持
- 多客户端连接支持

### v2.0.0 计划
- AI 辅助分析功能
- 自动模式识别
- 调用图生成
- CFG（控制流图）导出
- 高级内存可视化

---

## 升级说明

### 从 1.0.x 升级到 1.1.0
- 无破坏性更改
- 新方法向后兼容
- 如果使用权限限制，请更新 `config.json` 在 allowed_methods 中包含 `context.*`
- 现有客户端代码将继续工作，无需修改

### 从 1.0.0 升级到 1.0.1
- 无破坏性更改
- 新的线程和栈方法仅是添加
- 配置文件格式未更改

---

更多信息，请参阅：
- [README_CN.md](docs/README_CN.md) - 主要文档
- [QUICKSTART_CN.md](docs/QUICKSTART_CN.md) - 快速入门指南
- [Plan.md](Plan.md) - 开发路线图
- [examples/](examples/) - 代码示例
