# 版本更新说明

本文档介绍 x64dbg MCP Server Plugin 的新功能和改进。

**当前开发版本**: v1.1.0

---

## v1.1.0 (开发中) - 综合调试与脱壳平台

**发布日期**: 待定

### 概述

版本 1.1.0 是一个重大版本,将 x64dbg MCP Server 转变为综合调试和自动化脱壳平台,具有多架构支持、脚本执行、上下文快照和 AI 驱动的脱壳能力。

### 🎯 内存转储与自动脱壳

全面的转储和脱壳系统,专为 AI 驱动的自动化逆向工程工作流设计。

#### 核心能力

**8 个新的 JSON-RPC 方法**:
- `dump.module` - 转储可执行模块并自动重建 PE 结构
- `dump.memory_region` - 转储任意内存区域到文件
- `dump.auto_unpack` - 自动脱壳并检测 OEP
- `dump.analyze_module` - 检测加壳器并分析模块结构
- `dump.detect_oep` - 使用多种策略检测原始入口点
- `dump.get_dumpable_regions` - 枚举所有可转储的内存区域
- `dump.fix_imports` - 独立的导入表重建
- `dump.rebuild_pe` - 独立的 PE 头重建

#### 主要特性

**1. 智能模块转储**
```json
{
  "method": "dump.module",
  "params": {
    "module_name": "packed.exe",
    "output_path": "C:\\dumps\\unpacked.exe",
    "fix_iat": true,
    "fix_relocations": true,
    "fix_oep": true
  }
}
```
- 自动 PE 头重建
- 导入地址表 (IAT) 修复
- 重定位表处理
- 入口点 (OEP) 修正
- 节区对齐和验证

**2. 自动脱壳**
```json
{
  "method": "dump.auto_unpack",
  "params": {
    "module_name": "packed.exe",
    "output_path": "C:\\dumps\\unpacked.exe",
    "max_iterations": 5,
    "oep_detection_method": "entropy"
  }
}
```
- 支持多层加壳
- 可配置的迭代次数限制
- 多种 OEP 检测策略
- 自动加壳器检测
- AI 可自定义的脱壳工作流

**3. 加壳器检测**
```json
{
  "method": "dump.analyze_module",
  "params": {
    "module_name": "packed.exe"
  }
}
```
可检测常见加壳器:
- **UPX** (Ultimate Packer for eXecutables)
- **ASPack** (Alexey Solodovnikov's Packer)
- **通用加壳器** 通过启发式分析
  - 高熵节区
  - 可疑节区名称 (.upx, .aspack, .enigma)
  - 导入表异常

**4. OEP 检测策略**
```json
{
  "method": "dump.detect_oep",
  "params": {
    "module_name": "packed.exe",
    "strategy": "pattern"
  }
}
```
三种检测方法:
- **熵分析**: 识别具有正常熵的代码节区
- **模式匹配**: 扫描函数序言 (PUSH EBP, MOV EBP ESP)
- **执行跟踪**: 分析执行流以找到原始入口点
- **自定义 AI 策略**: 用于 AI 驱动检测的可扩展回调系统

**5. 导入表重建**
```json
{
  "method": "dump.fix_imports",
  "params": {
    "module_base": "0x400000",
    "output_path": "C:\\dumps\\fixed.exe",
    "use_scylla": true
  }
}
```
- 针对常见加壳器的标准 IAT 修复
- Scylla 风格的高级重建 (待实现)
- 自动 API 解析
- 导入目录重建

#### 技术架构

**新增组件**:
- `DumpManager` - 核心业务逻辑类
  - PE 格式操作工具
  - 内存区域枚举和验证
  - 加壳器特征数据库
  - OEP 检测算法套件
  - IAT 重建引擎

- `DumpHandler` - JSON-RPC 接口层
  - 8 个静态处理方法
  - 基于权限的访问控制
  - 进度回调支持
  - 错误处理和验证

**集成**:
- 与现有 `MemoryManager` 无缝集成
- 与 `BreakpointManager` 兼容,用于自动化工作流
- 使用 `DebugController` 进行进程状态管理

#### 使用场景

**自动化恶意软件分析**:
```python
# 1. 检测加壳器
analysis = client.analyze_module("malware.exe")
if analysis["is_packed"]:
    print(f"检测到: {analysis['packer_type']}")
    
    # 2. 自动脱壳
    result = client.auto_unpack(
        module_name="malware.exe",
        output_path="unpacked.exe",
        max_iterations=3
    )
    
    # 3. 分析脱壳后的代码
    if result["success"]:
        print(f"OEP: 0x{result['oep']:X}")
```

**自定义脱壳工作流**:
```python
# 1. 在疑似 OEP 处设置断点
client.set_breakpoint(address=0x401000)

# 2. 运行到断点
client.run()

# 3. 检测 OEP
oep = client.detect_oep(strategy="execution")

# 4. 修复导入表
client.fix_imports(module_base=0x400000, output_path="fixed.exe")

# 5. 转储模块
client.dump_module(
    module_name="target.exe",
    output_path="dumped.exe",
    fix_iat=True,
    fix_oep=True
)
```

#### 安全性与限制

**安全性**:
- 所有转储操作需要 `write` 权限
- 路径验证防止目录遍历
- 遵守内存保护标志
- 大小限制防止资源耗尽

**当前限制**:
- Scylla IAT 重建尚未完全实现 (标记为 TODO)
- 基于执行的 OEP 检测需要跟踪分析 (TODO)
- 加壳器特征数据库有限 (可扩展)

#### 文档资源

- **API 参考**: 参见 `docs/Protocol.md` 获取完整方法规范
- **Python 示例**: 参见 `examples/dump_demo.py` 获取使用演示
- **发布说明**: 参见 `RELEASE_NOTES_v1.3.0.md` 获取详细变更日志

---

### 🏗️ 双架构支持

支持 64 位 (x64dbg) 和 32 位 (x32dbg) 架构,使插件能够无缝地在两个调试器中工作。

#### 构建系统增强

**灵活的架构选择**:
```powershell
# 为 x64 构建 (64 位 x64dbg) - 默认
.\build.bat
.\build.bat --arch x64

# 为 x86 构建 (32 位 x32dbg)
.\build.bat --arch x86
```

**独立的输出文件**:
- x64 构建: `x64dbg_mcp.dp64` (用于 x64dbg)
- x86 构建: `x32dbg_mcp.dp32` (用于 x32dbg)

#### 架构感知组件

**1. 寄存器处理**

基于架构自动选择寄存器集:

**x64 架构** (64 位):
- 通用寄存器: RAX, RBX, RCX, RDX, RSI, RDI, RSP, RBP, RIP
- 扩展寄存器: R8, R9, R10, R11, R12, R13, R14, R15
- 段寄存器: CS, DS, ES, FS, GS, SS
- 标志寄存器: RFLAGS
- SSE/AVX: XMM0-XMM15, YMM0-YMM15

**x86 架构** (32 位):
- 通用寄存器: EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP, EIP
- 段寄存器: CS, DS, ES, FS, GS, SS
- 标志寄存器: EFLAGS
- SSE/AVX: XMM0-XMM7, YMM0-YMM7

**2. 内存操作**

指针大小自适应:
- x64: 8 字节指针 (`uint64_t`)
- x86: 4 字节指针 (`uint32_t`)

栈操作自动调整:
```cpp
// x64: 每个栈条目读取 8 字节
// x86: 每个栈条目读取 4 字节
```

**3. 类型定义**

统一的 `duint` 类型:
```cpp
#if defined(XDBG_ARCH_X64)
    typedef uint64_t duint;  // 64 位
#else
    typedef uint32_t duint;  // 32 位
#endif
```

#### CMake 构建系统更新

**架构变量**:
```cmake
set(XDBG_ARCH "x64" CACHE STRING "Target architecture (x64 or x86)")
```

**条件编译**:
```cpp
#if defined(XDBG_ARCH_X64)
    // x64 特定代码
#elif defined(XDBG_ARCH_X86)
    // x86 特定代码
#endif
```

**SDK 库链接**:
- x64: `x64bridge.lib`, `x64dbg.lib`
- x86: `x32bridge.lib`, `x32dbg.lib`

#### 更新的组件

**修改的管理器**:
- `RegisterManager` - 架构感知的寄存器访问
- `StackManager` - 指针大小感知的栈操作
- `ThreadManager` - 32/64 位上下文检索
- `MemoryManager` - 地址空间处理

**构建脚本**:
- `build.bat` - Windows 批处理脚本,带 `--arch` 参数
- `build.sh` - Linux/WSL bash 脚本,带 `--arch` 参数
- `configure.bat` - CMake 配置助手

#### 向后兼容性

所有现有 API 保持不变:
- JSON-RPC 方法在两种架构上的工作方式完全相同
- 客户端代码无需修改
- 寄存器名称已规范化 (例如,"rax" 对 RAX/EAX 都有效)

#### 技术细节

**宏迁移**:
- 旧: `X64DBG_SDK_AVAILABLE`
- 新: `XDBG_SDK_AVAILABLE` (架构无关)

**线程上下文**:
```cpp
#if defined(XDBG_ARCH_X64)
    CONTEXT64 context;
#else
    CONTEXT32 context;
#endif
```

---

### ✨ 脚本执行 API

以编程方式执行 x64dbg 命令以实现强大的自动化:

**3 个新的 JSON-RPC 方法**:
- `script.execute` - 运行单个 x64dbg 命令
- `script.execute_batch` - 执行多个命令并进行错误处理
- `script.execute_async` - 异步执行命令

**功能特性**:
- 直接访问 x64dbg 的原生命令系统
- 具有类似事务行为的批量执行
- 命令历史和结果跟踪
- 错误处理和验证

**使用示例**:
```json
{
  "method": "script.execute",
  "params": {
    "command": "bp CreateFileW"
  }
}
```

```python
# 批量执行
result = client.execute_batch([
    "bp CreateFileW",
    "bp WriteFile",
    "run"
])
```

---

### 🔍 上下文快照 API

捕获和比较调试状态以进行差分分析:

**3 个新的 JSON-RPC 方法**:
- `context.get_snapshot` - 完整的调试上下文快照 (寄存器、栈、内存)
- `context.get_basic` - 快速寄存器 + 状态快照
- `context.compare_snapshots` - 比较两个快照以找出差异

**功能特性**:
- 完整状态捕获 (寄存器、标志、栈、内存区域)
- 快照之间的差分分析
- 用于频繁捕获的轻量级基本快照
- 与自动化工作流集成

**使用示例**:
```python
# 操作前捕获状态
snapshot1 = client.get_snapshot(include_stack=True, stack_depth=50)

# 执行操作
client.step_over()

# 操作后捕获状态
snapshot2 = client.get_snapshot(include_stack=True, stack_depth=50)

# 比较以找出变化
diff = client.compare_snapshots(snapshot1, snapshot2)
print(f"变化的寄存器: {diff['registers']}")
```

---

## 更多资源

- **完整变更日志**: 参见 `CHANGELOG.md` 了解所有版本变更
- **API 文档**: 参见 `docs/Protocol.md` 获取完整方法参考
- **快速入门**: 参见 `QUICKSTART.md` 获取设置指南
- **架构指南**: 参见 `docs/Architecture.md` 了解技术细节
- **示例**: 参见 `examples/` 目录获取使用演示

---

**有问题或反馈?**

- GitHub Issues: https://github.com/SetsunaYukiOvO/x64dbg-mcp/issues
- 文档: 参见 `docs/` 目录
- 贡献指南: 参见 `CONTRIBUTING.md`
