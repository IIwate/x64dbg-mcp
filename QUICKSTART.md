# Quick Start Guide

English | [中文](docs/QUICKSTART_CN.md)

Get up and running with x64dbg MCP Server in minutes.

## Prerequisites

- Windows 10/11 (x64 or x86)
- x64dbg or x32dbg debugger installed
- Visual Studio 2022 with C++ Desktop Development workload
- CMake 3.15+
- vcpkg package manager

## Installation

### Option 1: Pre-built Release (Recommended)

1. Download the latest release from [GitHub Releases](https://github.com/SetsunaYukiOvO/x64dbg-mcp/releases)
2. Choose the appropriate version:
   - `x64dbg_mcp.dp64` for x64dbg (64-bit)
   - `x32dbg_mcp.dp32` for x32dbg (32-bit)
3. Extract to your debugger's plugins directory:
   - x64dbg: `x64dbg\x64\plugins\`
   - x32dbg: `x64dbg\x32\plugins\`
4. Copy `config.json` to `plugins/x64dbg-mcp/` (or `plugins/x32dbg-mcp/`)
5. Restart the debugger

### Option 2: Build from Source

#### 1. Install vcpkg

```powershell
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg

# Bootstrap vcpkg
C:\vcpkg\bootstrap-vcpkg.bat

# Set environment variable (optional)
setx VCPKG_ROOT "C:\vcpkg"
```

#### 2. Build the Plugin

```powershell
# Clone repository
git clone https://github.com/SetsunaYukiOvO/x64dbg-mcp.git
cd x64dbg-mcp

# Build for x64 (default)
.\build.bat

# Build for x86 (32-bit)
.\build.bat --arch x86

# Or build with specific options
.\build.bat --clean          # Clean build
.\build.bat --arch x86 --clean  # Clean x86 build
.\build.bat --debug          # Debug build
.\build.bat --help           # Show all options
```

The build script will:
- Automatically detect vcpkg installation
- Download and compile dependencies (nlohmann_json)
- Build the plugin for the selected architecture using Visual Studio
- Optionally install to the debugger's plugins directory

**Output files:**
- x64: `build\bin\Release\x64dbg_mcp.dp64`
- x86: `build\bin\Release\x32dbg_mcp.dp32`

#### 3. Manual Build (Advanced)

```powershell
# Configure for x64
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
    -DXDBG_ARCH=x64

# Or configure for x86
cmake -B build -G "Visual Studio 17 2022" -A Win32 ^
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
    -DXDBG_ARCH=x86

# Compile
cmake --build build --config Release

# Output: 
# - x64: build\bin\Release\x64dbg_mcp.dp64
# - x86: build\bin\Release\x32dbg_mcp.dp32
```

## 1. Install Plugin

If you didn't use the automatic installation in the build script:

```powershell
# For x64dbg
copy build\bin\Release\x64dbg_mcp.dp64 C:\x64dbg\x64\plugins\

# For x32dbg
copy build\bin\Release\x32dbg_mcp.dp32 C:\x64dbg\x32\plugins\

# Copy config (adjust path based on architecture)
mkdir C:\x64dbg\x64\plugins\x64dbg-mcp
copy config.json C:\x64dbg\x64\plugins\x64dbg-mcp\

# Or for x32
mkdir C:\x64dbg\x32\plugins\x32dbg-mcp
copy config.json C:\x64dbg\x32\plugins\x32dbg-mcp\
```

## 2. Start Server

1. Launch x64dbg or x32dbg
2. Load a target program for debugging
3. Menu: **Plugins → MCP Server → Start MCP HTTP Server**
4. Server starts on port 3000 (configurable in config.json)
5. Verify server is running by accessing http://127.0.0.1:3000 in a browser

## 3. Connect Client

### Python Example

```python
import requests
import json

class MCPClient:
    def __init__(self, host='127.0.0.1', port=3000):
        self.base_url = f"http://{host}:{port}"
        self.request_id = 1
    
    def call(self, method, params=None):
        request = {
            "jsonrpc": "2.0",
            "id": self.request_id,
            "method": method,
            "params": params or {}
        }
        self.request_id += 1
        
        response = requests.post(
            f"{self.base_url}/rpc",
            json=request,
            headers={"Content-Type": "application/json"}
        )
        return response.json()

# Use the client
client = MCPClient()
print(client.call("initialize"))
print(client.call("tools/list"))
```

### MCP Protocol Example

```python
# Initialize MCP session
init_response = client.call("initialize", {
    "protocolVersion": "2024-11-05",
    "capabilities": {},
    "clientInfo": {
        "name": "my-client",
        "version": "1.0.0"
    }
})

# Send initialized notification
client.call("notifications/initialized")

# List available tools
tools = client.call("tools/list")
print(tools)
```

## Common Operations

### Get System Info
```python
response = client.call("system.info")
```

### Read Register
```python
response = client.call("register.get", {"name": "rax"})
value = response["result"]["value"]
```

### Read Memory
```python
response = client.call("memory.read", {
    "address": "0x140001000",
    "size": 100
})
data = response["result"]["data"]  # hex string
```

### Set Breakpoint
```python
response = client.call("breakpoint.set", {
    "address": "0x140001000",
    "type": "software"
})
```

### Disassemble
```python
response = client.call("disassembly.at", {
    "address": "0x140001000",
    "count": 10
})
instructions = response["result"]["instructions"]
```

## Configuration

Edit `config.json` to customize:

```json
{
  "version": "1.0.1",
  "server": {
    "address": "127.0.0.1",
    "port": 3000
  },
  "permissions": {
    "allow_memory_write": true,
    "allow_register_write": true,
    "allow_script_execution": true,
    "allow_breakpoint_modification": true
  },
  "logging": {
    "enabled": true,
    "level": "info",
    "file": "x64dbg_mcp.log"
  }
}
```

## Next Steps

- See [README.md](README.md) for complete API reference
- Use `system.methods` API call to discover all available methods
- Explore [examples/](examples/) for more client implementations

## Troubleshooting

### Build Issues

**CMake cannot find vcpkg**
- Ensure `VCPKG_ROOT` environment variable is set
- Or use the full path in CMAKE_TOOLCHAIN_FILE
- Default location: `C:\vcpkg`

**Link errors during build**
- Make sure x64dbg SDK libraries exist in `include/x64dbg-pluginsdk/`
- Try clean rebuild: `.\build.bat --clean`
- Verify you're building for x64 architecture

**vcpkg not found**
- Install vcpkg: `git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg`
- Bootstrap: `C:\vcpkg\bootstrap-vcpkg.bat`

### Runtime Issues

**Plugin won't load**
- Ensure plugin file is named `x64dbg_mcp.dp64`
- Check x64dbg version (requires 64-bit version)
- View x64dbg log for error messages

**Server won't start**
- Check port 3000 is not in use
- Verify config.json is valid JSON
- Ensure you have a program loaded in x64dbg
- Check x64dbg log for detailed error messages

**Connection refused**  
- Ensure HTTP server is started via plugin menu ("Start MCP HTTP Server")
- Check firewall settings for port 3000
- Verify client connects to http://127.0.0.1:3000
- Test with browser: http://127.0.0.1:3000

## Build Script Options

The `build.bat` script supports the following options:

```powershell
build.bat [OPTIONS]

Options:
  --debug         Build in Debug mode (default: Release)
  --clean         Clean build directory before building
  --help          Show help message

Examples:
  build.bat                    # Release build
  build.bat --debug            # Debug build
  build.bat --clean            # Clean and rebuild
  build.bat --clean --debug    # Clean debug build
```

## Development Tips

### Quick Rebuild Cycle

```powershell
# Make code changes...

# Rebuild (faster, incremental)
.\build.bat

# x64dbg must be restarted to reload the plugin
```

### Debug Build for Development

```powershell
# Build with debug symbols
.\build.bat --debug

# Debug output: build\bin\Debug\x64dbg_mcp.dp64
```

## Advanced Features (v1.1.0+)

### Script Execution

Execute x64dbg commands programmatically:

```python
# Execute single command
response = client.send_request("script.execute", {
    "command": "bp MessageBoxA"
})

# Execute batch commands
response = client.send_request("script.execute_batch", {
    "commands": [
        "log \"Starting analysis...\"",
        "bp kernel32.CreateFileW",
        "bp kernel32.WriteFile",
        "run"
    ],
    "stop_on_error": True  # Stop if any command fails
})

# Get last command result
response = client.send_request("script.get_last_result")
```

### Context Snapshots

Capture and compare debugging state:

```python
# Capture initial state
snapshot1 = client.send_request("context.get_snapshot", {
    "include_stack": True,
    "include_threads": True,
    "include_modules": True,
    "include_breakpoints": True
})

# Execute some steps
client.send_request("debug.step_over")

# Capture new state
snapshot2 = client.send_request("context.get_snapshot", {
    "include_stack": True,
    "include_threads": False,  # Skip for faster capture
    "include_modules": False,
    "include_breakpoints": False
})

# Compare snapshots to see what changed
diff = client.send_request("context.compare_snapshots", {
    "snapshot1": snapshot1["result"],
    "snapshot2": snapshot2["result"]
})

print("Changes:", diff["result"]["has_differences"])
print("Register changes:", diff["result"]["differences"].get("registers", []))
```

### Quick Context Check

```python
# Get basic context (registers + state only)
context = client.send_request("context.get_basic")
print("Registers:", context["result"]["registers"])
print("Is debugging:", context["result"]["state"]["is_debugging"])
```

### Automated Analysis Workflow

```python
# 1. Set up environment
client.send_request("script.execute_batch", {
    "commands": [
        "bp VirtualAlloc",
        "bp VirtualProtect",
        "run"
    ]
})

# 2. Capture state at breakpoint
bp_snapshot = client.send_request("context.get_snapshot", {
    "include_stack": True
})

# 3. Analyze with scripts
client.send_request("script.execute_batch", {
    "commands": [
        "log \"VirtualAlloc called!\"",
        "? rcx",
        "? rdx"
    ]
})

# 4. Continue and compare
client.send_request("debug.run")
after = client.send_request("context.get_snapshot")
diff = client.send_request("context.compare_snapshots", {
    "snapshot1": bp_snapshot["result"],
    "snapshot2": after["result"]
})
```

### Example Scripts

Check the `examples/` directory:

- `python_client_http.py` - Basic HTTP client
- `advanced_features_demo.py` - v1.1.0+ features demo

Run the demo:

```powershell
cd examples
python advanced_features_demo.py
```

## Next Steps
