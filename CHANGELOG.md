# Changelog

All notable changes to the x64dbg MCP Server Plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - TBD

### Added

#### Dual Architecture Support
- **Multi-Architecture Build System**: Plugin now supports both x64 and x86 architectures
  - Build for x64dbg (64-bit) with `.\build.bat` or `.\build.bat --arch x64`
  - Build for x32dbg (32-bit) with `.\build.bat --arch x86`
  - Separate output files: `x64dbg_mcp.dp64` and `x32dbg_mcp.dp32`
- **Architecture-Aware Register Handling**
  - x64: RAX, RBX, RCX, RDX, RSI, RDI, RSP, RBP, RIP, R8-R15
  - x86: EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP, EIP
- **Unified SDK Structure**: Both architectures share the same header files from `include/x64dbg-pluginsdk`

#### Dump & Unpacking Module
- **Comprehensive Memory Dumping and Automatic Unpacking Capabilities**
  - `dump.module`: Dump executable modules with automatic PE reconstruction
    - Import table fixing (IAT reconstruction)
    - Relocation table handling
    - Entry point (OEP) fixing
    - PE header rebuilding
    - Section alignment
  - `dump.memory_region`: Dump arbitrary memory regions to file
  - `dump.auto_unpack`: Automatic unpacking with OEP detection
    - Support for multi-layer packed executables
    - Iterative unpacking with configurable max iterations
    - Automatic packer detection
  - `dump.analyze_module`: Detect packers and analyze module structure
    - UPX detection
    - ASPack detection
    - Generic packer detection via heuristics
  - `dump.detect_oep`: Original Entry Point detection using multiple strategies
    - Entropy-based detection
    - Pattern-based detection (function prologue scanning)
    - Execution trace-based detection
    - Custom AI-driven detection strategies
  - `dump.get_dumpable_regions`: Enumerate all dumpable memory regions
  - `dump.fix_imports`: Standalone import table reconstruction
    - Standard IAT fixing
    - Scylla-style advanced IAT rebuilding
  - `dump.rebuild_pe`: Standalone PE header reconstruction
- **DumpManager**: Core business logic for dump operations
- **DumpHandler**: JSON-RPC handler for dump methods

#### Script Execution API
- **Execute x64dbg Commands Programmatically**
  - `script.execute`: Run single x64dbg command
  - `script.execute_batch`: Execute multiple commands with error handling
  - `script.get_last_result`: Get last command execution result

#### Context Snapshot API
- **Capture and Compare Debugging State**
  - `context.get_snapshot`: Full debugging context snapshot
  - `context.get_basic`: Quick register + state snapshot
  - `context.compare_snapshots`: Compare two snapshots to find differences

### Features
- AI-customizable unpacking strategies via callback system
- Progress callbacks for long-running dump operations
- Automatic PE structure validation and repair
- Support for both raw binary dumps and PE-fixed dumps
- Batch region dumping capabilities
- Integration with existing breakpoint and debug control systems
- Conditional compilation using `XDBG_ARCH_X64` and `XDBG_ARCH_X86` macros
- Stack operations use architecture-specific pointer sizes (8 bytes for x64, 4 bytes for x86)
- Thread context retrieval adapted for both 32-bit and 64-bit environments

### Technical
- New `DumpManager` class for centralized dump operations
- PE format manipulation utilities
- Memory region enumeration and validation
- Packer signature database (extensible)
- OEP detection algorithm suite
- IAT reconstruction engine
- CMake build system now uses `XDBG_ARCH` variable to select architecture
- Build scripts (`build.bat`, `configure.bat`, `build.sh`) accept `--arch` parameter
- Replaced all `X64DBG_SDK_AVAILABLE` macros with `XDBG_SDK_AVAILABLE`
- Added `duint` type definition: `uint64_t` for x64, `uint32_t` for x86
- Modified `RegisterManager`, `StackManager`, `ThreadManager` for architecture compatibility
- Updated CMake to link correct SDK libraries based on architecture:
  - x64: `x64bridge.lib`, `x64dbg.lib`
  - x86: `x32bridge.lib`, `x32dbg.lib`

### Documentation
- Added `UPDATE.md` and `UPDATE_CN.md` for version feature highlights
- Updated `docs/Protocol.md` with 14 new methods (8 dump.*, 3 script.*, 3 context.*)
- Updated `README.md` to highlight all new features
- Added comprehensive usage examples for dump, script, and context APIs
- Comprehensive dump demo script (`examples/dump_demo.py`)

### Security
- Dump operations require write permission
- Path validation for output files
- Memory protection respect
- Size limits to prevent resource exhaustion

## [1.0.0] - 2025-11-20
## [1.0.0] - 2025-11-18

### Added
- Initial release with complete core functionality
- JSON-RPC 2.0 protocol implementation
- HTTP server with SSE support for event streaming
- 50+ debugging methods across multiple categories:
  - **Debug Control**: run, pause, step_into, step_over, step_out, run_to, restart, stop, get_state
  - **Memory Operations**: read, write, search, get_info, enumerate, allocate, free, get_protection
  - **Register Operations**: get, set, list, get_batch (50+ registers)
  - **Breakpoint Management**: set, delete, enable, disable, toggle, list, get, delete_all, set_condition, set_log
  - **Disassembly**: at, range, function
  - **Symbol Resolution**: resolve, from_address, search, list, modules
  - **Comment/Label Management**: set_comment, get_comment, set_label
  - **Module Information**: list, get, get_main, find
  - **Thread Management**: list, switch, get_current, suspend, resume
  - **Stack Operations**: get_trace, read_frame, get_pointers, is_on_stack
- Permission-based access control system
- Configuration file support (config.json)
- Multi-level logging system
- Event notification system via SSE
- Comprehensive error handling with JSON-RPC error codes
- Python client examples

### Technical Details
- Built with C++17
- Uses x64dbg Plugin SDK
- Dependencies: nlohmann/json via vcpkg
- Supports x64dbg on Windows (x64)
- Plugin size: ~445 KB

---

## Version History Summary

| Version | Release Date | Key Features |
|---------|-------------|--------------|
| 1.1.0 | TBD | Dual architecture, Dump & unpacking, Script execution, Context snapshots |
| 1.0.0 | 2025-11-18 | Initial public release |

---

## Future Roadmap

### Planned for v1.2.0
- Performance optimization and benchmarking
- Enhanced security features (Token/API Key authentication)
- WebSocket protocol support
- Multi-client connection support

### Planned for v2.0.0
- AI-assisted analysis features
- Automated pattern recognition
- Call graph generation
- CFG (Control Flow Graph) export
- Advanced memory visualization

---

## Upgrade Notes

### Upgrading to 1.1.0 from 1.0.0
- **Breaking Changes**: None
- **New Features**: 
  - Dual architecture support (x64/x86)
  - 14 new methods (8 dump.*, 3 script.*, 3 context.*)
  - AI-driven unpacking capabilities
- **Configuration Updates**: 
  - Update `config.json` to include `dump.*`, `script.*`, `context.*` in allowed_methods if using permission restrictions
  - For x86 builds, use `x32dbg_mcp.dp32` instead of `x64dbg_mcp.dp64`
- **Backward Compatibility**: Existing client code will continue to work without modifications

---

For more information, see:
- [README.md](README.md) - Main documentation
- [UPDATE.md](UPDATE.md) - Version update highlights
- [QUICKSTART.md](QUICKSTART.md) - Quick start guide
- [Plan.md](Plan.md) - Development roadmap
- [examples/](examples/) - Code examples
