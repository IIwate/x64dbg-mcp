# Version Updates

This document describes new features and improvements in x64dbg MCP Server Plugin.

**Current Development Version**: v1.1.0

---

## v1.1.0 (Development) - Comprehensive Debugging & Unpacking Platform

**Release Date**: TBD

### Overview

Version 1.1.0 is a major release that transforms x64dbg MCP Server into a comprehensive debugging and automated unpacking platform with multi-architecture support, script execution, context snapshots, and AI-driven unpacking capabilities.

### 🎯 Memory Dumping & Automatic Unpacking

A comprehensive dump and unpacking system designed for AI-driven automated reverse engineering workflows.

#### Core Capabilities

**8 New JSON-RPC Methods**:
- `dump.module` - Dump executable modules with automatic PE reconstruction
- `dump.memory_region` - Dump arbitrary memory regions to file
- `dump.auto_unpack` - Automatic unpacking with OEP detection
- `dump.analyze_module` - Detect packers and analyze module structure
- `dump.detect_oep` - Original Entry Point detection using multiple strategies
- `dump.get_dumpable_regions` - Enumerate all dumpable memory regions
- `dump.fix_imports` - Standalone import table reconstruction
- `dump.rebuild_pe` - Standalone PE header reconstruction

#### Key Features

**1. Intelligent Module Dumping**
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
- Automatic PE header reconstruction
- Import Address Table (IAT) fixing
- Relocation table handling
- Entry point (OEP) correction
- Section alignment and validation

**2. Automatic Unpacking**
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
- Multi-layer unpacking support
- Configurable iteration limits
- Multiple OEP detection strategies
- Automatic packer detection
- AI-customizable unpacking workflows

**3. Packer Detection**
```json
{
  "method": "dump.analyze_module",
  "params": {
    "module_name": "packed.exe"
  }
}
```
Detects common packers:
- **UPX** (Ultimate Packer for eXecutables)
- **ASPack** (Alexey Solodovnikov's Packer)
- **Generic packers** via heuristic analysis
  - High entropy sections
  - Suspicious section names (.upx, .aspack, .enigma)
  - Import table anomalies

**4. OEP Detection Strategies**
```json
{
  "method": "dump.detect_oep",
  "params": {
    "module_name": "packed.exe",
    "strategy": "pattern"
  }
}
```
Three detection methods:
- **Entropy Analysis**: Identifies code sections with normal entropy
- **Pattern Matching**: Scans for function prologues (PUSH EBP, MOV EBP ESP)
- **Execution Trace**: Analyzes execution flow to find original entry point
- **Custom AI Strategies**: Extensible callback system for AI-driven detection

**5. Import Table Reconstruction**
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
- Standard IAT fixing for common packers
- Scylla-style advanced reconstruction (TODO)
- Automatic API resolution
- Import directory rebuilding

#### Technical Architecture

**New Components**:
- `DumpManager` - Core business logic class
  - PE format manipulation utilities
  - Memory region enumeration and validation
  - Packer signature database
  - OEP detection algorithm suite
  - IAT reconstruction engine

- `DumpHandler` - JSON-RPC interface layer
  - 8 static handler methods
  - Permission-based access control
  - Progress callback support
  - Error handling and validation

**Integration**:
- Seamless integration with existing `MemoryManager`
- Compatible with `BreakpointManager` for automated workflows
- Uses `DebugController` for process state management

#### Use Cases

**Automated Malware Analysis**:
```python
# 1. Detect packer
analysis = client.analyze_module("malware.exe")
if analysis["is_packed"]:
    print(f"Detected: {analysis['packer_type']}")
    
    # 2. Auto-unpack
    result = client.auto_unpack(
        module_name="malware.exe",
        output_path="unpacked.exe",
        max_iterations=3
    )
    
    # 3. Analyze unpacked code
    if result["success"]:
        print(f"OEP: 0x{result['oep']:X}")
```

**Custom Unpacking Workflow**:
```python
# 1. Set breakpoint at suspected OEP
client.set_breakpoint(address=0x401000)

# 2. Run to breakpoint
client.run()

# 3. Detect OEP
oep = client.detect_oep(strategy="execution")

# 4. Fix imports
client.fix_imports(module_base=0x400000, output_path="fixed.exe")

# 5. Dump module
client.dump_module(
    module_name="target.exe",
    output_path="dumped.exe",
    fix_iat=True,
    fix_oep=True
)
```

#### Security & Limitations

**Security**:
- All dump operations require `write` permission
- Path validation prevents directory traversal
- Memory protection flags are respected
- Size limits prevent resource exhaustion

**Current Limitations**:
- Scylla IAT reconstruction not fully implemented (marked TODO)
- Execution-based OEP detection needs trace analysis (TODO)
- Limited packer signature database (extensible)

#### Documentation

- **API Reference**: See `docs/Protocol.md` for complete method specifications
- **Python Examples**: See `examples/dump_demo.py` for usage demonstrations
- **Release Notes**: See `RELEASE_NOTES_v1.3.0.md` for detailed changelog

---

### 🏗️ Dual Architecture Support

Support for both 64-bit (x64dbg) and 32-bit (x32dbg) architectures, allowing the plugin to work seamlessly with both debuggers.

#### Build System Enhancements

**Flexible Architecture Selection**:
```powershell
# Build for x64 (64-bit x64dbg) - Default
.\build.bat
.\build.bat --arch x64

# Build for x86 (32-bit x32dbg)
.\build.bat --arch x86
```

**Separate Output Files**:
- x64 build: `x64dbg_mcp.dp64` (for x64dbg)
- x86 build: `x32dbg_mcp.dp32` (for x32dbg)

#### Architecture-Aware Components

**1. Register Handling**

Automatic register set selection based on architecture:

**x64 Architecture** (64-bit):
- General Purpose: RAX, RBX, RCX, RDX, RSI, RDI, RSP, RBP, RIP
- Extended: R8, R9, R10, R11, R12, R13, R14, R15
- Segment: CS, DS, ES, FS, GS, SS
- Flags: RFLAGS
- SSE/AVX: XMM0-XMM15, YMM0-YMM15

**x86 Architecture** (32-bit):
- General Purpose: EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP, EIP
- Segment: CS, DS, ES, FS, GS, SS
- Flags: EFLAGS
- SSE/AVX: XMM0-XMM7, YMM0-YMM7

**2. Memory Operations**

Pointer size adaptation:
- x64: 8-byte pointers (`uint64_t`)
- x86: 4-byte pointers (`uint32_t`)

Stack operations automatically adjust:
```cpp
// x64: reads 8 bytes per stack entry
// x86: reads 4 bytes per stack entry
```

**3. Type Definitions**

Unified `duint` type:
```cpp
#if defined(XDBG_ARCH_X64)
    typedef uint64_t duint;  // 64-bit
#else
    typedef uint32_t duint;  // 32-bit
#endif
```

#### CMake Build System Updates

**Architecture Variable**:
```cmake
set(XDBG_ARCH "x64" CACHE STRING "Target architecture (x64 or x86)")
```

**Conditional Compilation**:
```cpp
#if defined(XDBG_ARCH_X64)
    // x64-specific code
#elif defined(XDBG_ARCH_X86)
    // x86-specific code
#endif
```

**SDK Library Linking**:
- x64: `x64bridge.lib`, `x64dbg.lib`
- x86: `x32bridge.lib`, `x32dbg.lib`

#### Updated Components

**Modified Managers**:
- `RegisterManager` - Architecture-aware register access
- `StackManager` - Pointer-size-aware stack operations
- `ThreadManager` - 32/64-bit context retrieval
- `MemoryManager` - Address space handling

**Build Scripts**:
- `build.bat` - Windows batch script with `--arch` parameter
- `build.sh` - Linux/WSL bash script with `--arch` parameter
- `configure.bat` - CMake configuration helper

#### Backward Compatibility

All existing APIs remain unchanged:
- JSON-RPC methods work identically on both architectures
- Client code doesn't need modification
- Register names are normalized (e.g., "rax" works for both RAX/EAX)

#### Technical Details

**Macro Migration**:
- Old: `X64DBG_SDK_AVAILABLE`
- New: `XDBG_SDK_AVAILABLE` (architecture-agnostic)

**Thread Context**:
```cpp
#if defined(XDBG_ARCH_X64)
    CONTEXT64 context;
#else
    CONTEXT32 context;
#endif
```

---

### ✨ Script Execution API

Execute x64dbg commands programmatically for powerful automation:

**3 New JSON-RPC Methods**:
- `script.execute` - Run single x64dbg command
- `script.execute_batch` - Execute multiple commands with error handling
- `script.execute_async` - Execute commands asynchronously

**Features**:
- Direct access to x64dbg's native command system
- Batch execution with transaction-like behavior
- Command history and result tracking
- Error handling and validation

**Example Usage**:
```json
{
  "method": "script.execute",
  "params": {
    "command": "bp CreateFileW"
  }
}
```

```python
# Batch execution
result = client.execute_batch([
    "bp CreateFileW",
    "bp WriteFile",
    "run"
])
```

---

### 🔍 Context Snapshot API

Capture and compare debugging state for differential analysis:

**3 New JSON-RPC Methods**:
- `context.get_snapshot` - Full debugging context snapshot (registers, stack, memory)
- `context.get_basic` - Quick register + state snapshot
- `context.compare_snapshots` - Compare two snapshots to find differences

**Features**:
- Complete state capture (registers, flags, stack, memory regions)
- Differential analysis between snapshots
- Lightweight basic snapshots for frequent captures
- Integration with automation workflows

**Example Usage**:
```python
# Capture state before operation
snapshot1 = client.get_snapshot(include_stack=True, stack_depth=50)

# Execute operation
client.step_over()

# Capture state after operation
snapshot2 = client.get_snapshot(include_stack=True, stack_depth=50)

# Compare to find changes
diff = client.compare_snapshots(snapshot1, snapshot2)
print(f"Changed registers: {diff['registers']}")
```

---

## Additional Resources

- **Full Changelog**: See `CHANGELOG.md` for all version changes
- **API Documentation**: See `docs/Protocol.md` for complete method reference
- **Quick Start**: See `QUICKSTART.md` for setup guide
- **Architecture Guide**: See `docs/Architecture.md` for technical details
- **Examples**: See `examples/` directory for usage demonstrations

---

**Questions or Issues?**

- GitHub Issues: https://github.com/SetsunaYukiOvO/x64dbg-mcp/issues
- Documentation: See `docs/` directory
- Contributing: See `CONTRIBUTING.md`
