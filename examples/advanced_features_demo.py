#!/usr/bin/env python3
"""
x64dbg MCP Server - Advanced Features Demo
Demonstrates script execution, context snapshots, and batch operations
"""

import socket
import json
import sys
from typing import Dict, Any, Optional

class X64DBGMCPClient:
    def __init__(self, host='127.0.0.1', port=3000):
        self.host = host
        self.port = port
        self.sock = None
        self.request_id = 1
        
    def connect(self):
        """Connect to MCP server"""
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))
        print(f"✓ Connected to {self.host}:{self.port}")
        
    def disconnect(self):
        """Disconnect from server"""
        if self.sock:
            self.sock.close()
            self.sock = None
            print("✓ Disconnected")
            
    def send_request(self, method: str, params: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """Send JSON-RPC request and get response"""
        request = {
            "jsonrpc": "2.0",
            "id": self.request_id,
            "method": method,
            "params": params or {}
        }
        self.request_id += 1
        
        # Send request
        request_json = json.dumps(request)
        self.sock.sendall(request_json.encode('utf-8') + b'\n')
        
        # Receive response
        response_data = b''
        while b'\n' not in response_data:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise ConnectionError("Connection closed")
            response_data += chunk
            
        response = json.loads(response_data.decode('utf-8'))
        return response
        
    def call_method(self, method: str, params: Optional[Dict[str, Any]] = None):
        """Call a method and print the result"""
        print(f"\n{'='*60}")
        print(f"Method: {method}")
        if params:
            print(f"Params: {json.dumps(params, indent=2)}")
        
        response = self.send_request(method, params)
        
        if 'result' in response:
            print(f"✓ Success:")
            print(json.dumps(response['result'], indent=2))
            return response['result']
        else:
            print(f"✗ Error:")
            print(json.dumps(response.get('error', 'Unknown error'), indent=2))
            return None


def demo_script_execution(client: X64DBGMCPClient):
    """Demonstrate script execution features"""
    print("\n" + "="*60)
    print("SCRIPT EXECUTION DEMO")
    print("="*60)
    
    # Execute single command
    print("\n1. Execute single x64dbg command")
    client.call_method("script.execute", {
        "command": "log \"Hello from MCP Server!\""
    })
    
    # Execute batch commands
    print("\n2. Execute batch commands")
    client.call_method("script.execute_batch", {
        "commands": [
            "log \"Starting batch execution...\"",
            "log \"Command 2\"",
            "log \"Command 3\"",
            "log \"Batch execution complete!\""
        ],
        "stop_on_error": True
    })
    
    # Get last result
    print("\n3. Get last command result")
    client.call_method("script.get_last_result", {})


def demo_context_snapshots(client: X64DBGMCPClient):
    """Demonstrate context snapshot features"""
    print("\n" + "="*60)
    print("CONTEXT SNAPSHOT DEMO")
    print("="*60)
    
    # Get basic context
    print("\n1. Get basic context (registers + state)")
    basic_context = client.call_method("context.get_basic", {})
    
    # Get full snapshot
    print("\n2. Get full context snapshot")
    snapshot1 = client.call_method("context.get_snapshot", {
        "include_memory": True,
        "include_stack": True,
        "include_modules": True,
        "include_breakpoints": True,
        "include_threads": True
    })
    
    # Execute some steps
    print("\n3. Execute a step")
    client.call_method("debug.step_over", {})
    
    # Get another snapshot
    print("\n4. Get another snapshot after step")
    snapshot2 = client.call_method("context.get_snapshot", {
        "include_memory": False,
        "include_stack": True,
        "include_modules": False,
        "include_breakpoints": False,
        "include_threads": False
    })
    
    # Compare snapshots
    if snapshot1 and snapshot2:
        print("\n5. Compare two snapshots")
        client.call_method("context.compare_snapshots", {
            "snapshot1": snapshot1,
            "snapshot2": snapshot2
        })


def demo_combined_workflow(client: X64DBGMCPClient):
    """Demonstrate a combined workflow using multiple features"""
    print("\n" + "="*60)
    print("COMBINED WORKFLOW DEMO")
    print("="*60)
    
    # 1. Get initial snapshot
    print("\n1. Capture initial state")
    initial_snapshot = client.call_method("context.get_snapshot", {
        "include_memory": False,
        "include_stack": True,
        "include_modules": False,
        "include_breakpoints": True,
        "include_threads": True
    })
    
    # 2. Set a breakpoint using script
    print("\n2. Set breakpoint using script")
    client.call_method("script.execute", {
        "command": "bp 0x401000"
    })
    
    # 3. Run to breakpoint
    print("\n3. Run to breakpoint")
    client.call_method("debug.run", {})
    
    # 4. Capture state at breakpoint
    print("\n4. Capture state at breakpoint")
    bp_snapshot = client.call_method("context.get_snapshot", {
        "include_memory": False,
        "include_stack": True,
        "include_modules": False,
        "include_breakpoints": True,
        "include_threads": True
    })
    
    # 5. Execute batch analysis commands
    print("\n5. Execute batch analysis commands")
    client.call_method("script.execute_batch", {
        "commands": [
            "log \"=== Analysis at Breakpoint ===\"",
            "log \"Registers:\"",
            "r",
            "log \"Stack:\"",
            "k",
            "log \"Disassembly:\"",
            "u"
        ]
    })
    
    # 6. Compare snapshots
    if initial_snapshot and bp_snapshot:
        print("\n6. Compare initial state vs breakpoint state")
        client.call_method("context.compare_snapshots", {
            "snapshot1": initial_snapshot,
            "snapshot2": bp_snapshot
        })


def demo_error_handling(client: X64DBGMCPClient):
    """Demonstrate error handling"""
    print("\n" + "="*60)
    print("ERROR HANDLING DEMO")
    print("="*60)
    
    # Invalid command
    print("\n1. Test invalid command (should fail gracefully)")
    client.call_method("script.execute", {
        "command": "invalid_command_xyz"
    })
    
    # Batch with error
    print("\n2. Test batch with errors")
    client.call_method("script.execute_batch", {
        "commands": [
            "log \"Command 1\"",
            "invalid_command",
            "log \"Command 3 (should not execute if stop_on_error=true)\""
        ],
        "stop_on_error": True
    })
    
    # Missing parameters
    print("\n3. Test missing parameters")
    client.call_method("context.compare_snapshots", {
        "snapshot1": {}
        # Missing snapshot2
    })


def main():
    """Main demo function"""
    if len(sys.argv) > 1:
        host = sys.argv[1]
    else:
        host = '127.0.0.1'
        
    if len(sys.argv) > 2:
        port = int(sys.argv[2])
    else:
        port = 3000
    
    client = X64DBGMCPClient(host, port)
    
    try:
        client.connect()
        
        # Test connection
        print("\n" + "="*60)
        print("Testing connection...")
        print("="*60)
        result = client.call_method("system.ping")
        
        if not result or not result.get('pong'):
            print("✗ Connection test failed!")
            return
            
        # Check debugger state
        state = client.call_method("debug.get_state")
        if not state or not state.get('is_debugging'):
            print("\n⚠ Warning: Debugger is not active!")
            print("Please start debugging a target in x64dbg before running these demos.")
            print("\nYou can still test script execution methods without an active target.")
            
            # Only run script demo without active debugger
            demo_script_execution(client)
        else:
            print("\n✓ Debugger is active and ready!")
            
            # Run all demos
            choice = input("\nSelect demo:\n"
                          "1. Script Execution\n"
                          "2. Context Snapshots\n"
                          "3. Combined Workflow\n"
                          "4. Error Handling\n"
                          "5. Run All\n"
                          "Choice (1-5): ").strip()
            
            if choice == '1':
                demo_script_execution(client)
            elif choice == '2':
                demo_context_snapshots(client)
            elif choice == '3':
                demo_combined_workflow(client)
            elif choice == '4':
                demo_error_handling(client)
            elif choice == '5':
                demo_script_execution(client)
                demo_context_snapshots(client)
                demo_combined_workflow(client)
                demo_error_handling(client)
            else:
                print("Invalid choice!")
                
    except ConnectionRefusedError:
        print(f"\n✗ Failed to connect to {host}:{port}")
        print("Make sure x64dbg is running and MCP Server is started.")
    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()
    finally:
        client.disconnect()


if __name__ == '__main__':
    main()
