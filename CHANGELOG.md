# Changelog

All notable changes to the x64dbg MCP Server Plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-11-20

### Added
- **Script Execution API**: Execute x64dbg commands programmatically
  - `script.execute`: Execute a single x64dbg command
  - `script.execute_batch`: Execute multiple commands in sequence with configurable error handling
  - `script.get_last_result`: Retrieve the result of the last executed command
- **Context Snapshot API**: Capture and compare debugging state
  - `context.get_snapshot`: Capture full debugging context (registers, threads, stack, modules, breakpoints)
  - `context.get_basic`: Quick snapshot of registers and basic state
  - `context.compare_snapshots`: Compare two snapshots to identify differences
- Advanced demo script (`examples/advanced_features_demo.py`) showcasing script execution and context snapshots
- Configurable snapshot components (optional inclusion of threads, stack, modules, breakpoints)

### Changed
- Updated configuration file to include `context.*` methods in allowed methods
- Plugin size increased to ~611 KB due to new features
- JSON-RPC method count increased from 50+ to 55+

### Improved
- Enhanced automation capabilities through batch script execution
- Better debugging workflow support with state comparison
- More flexible debugging context capture

## [1.0.1] - 2025-11-19

### Added
- Complete thread management API
  - `thread.list`: List all threads
  - `thread.get_current`: Get current thread information
  - `thread.get`: Get specific thread details
  - `thread.switch`: Switch active thread
  - `thread.suspend`: Suspend thread execution
  - `thread.resume`: Resume thread execution
  - `thread.get_count`: Get thread count
- Complete stack trace API
  - `stack.get_trace`: Get full stack trace
  - `stack.read_frame`: Read specific stack frame
  - `stack.get_pointers`: Get stack pointers (ESP/RSP, EBP/RBP)
  - `stack.is_on_stack`: Check if address is on stack

### Changed
- Improved error handling across all handlers
- Enhanced logging system with better formatting

### Fixed
- Thread switching reliability
- Stack frame reading accuracy

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

## [0.3.0] - 2025-11-15 (Internal)

### Added
- Hardware breakpoint support (DR0-DR3)
- Memory breakpoint implementation
- Conditional breakpoint system
- Logging breakpoint feature
- Memory search functionality
- Batch register read operations

## [0.2.0] - 2025-11-12 (Internal)

### Added
- Complete breakpoint management
- Symbol resolution system
- Comment and label management
- Event callback handlers
- Permission checker system
- Configuration management

## [0.1.0] - 2025-11-10 (Internal)

### Added
- Basic plugin framework
- HTTP server implementation
- JSON-RPC protocol parser
- Core debugging methods
- Memory and register operations
- Initial documentation

---

## Version History Summary

| Version | Release Date | Key Features |
|---------|-------------|--------------|
| 1.1.0 | 2025-11-20 | Script execution, Context snapshots |
| 1.0.1 | 2025-11-19 | Thread & stack management |
| 1.0.0 | 2025-11-18 | Initial public release |
| 0.3.0 | 2025-11-15 | Advanced breakpoints |
| 0.2.0 | 2025-11-12 | Symbols & permissions |
| 0.1.0 | 2025-11-10 | Core framework |

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

### Upgrading to 1.1.0 from 1.0.x
- No breaking changes
- New methods are backward compatible
- Update `config.json` to include `context.*` in allowed_methods if using permission restrictions
- Existing client code will continue to work without modifications

### Upgrading to 1.0.1 from 1.0.0
- No breaking changes
- New thread and stack methods are additions only
- Configuration file format unchanged

---

For more information, see:
- [README.md](README.md) - Main documentation
- [QUICKSTART.md](QUICKSTART.md) - Quick start guide
- [Plan.md](Plan.md) - Development roadmap
- [examples/](examples/) - Code examples
