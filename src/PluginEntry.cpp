#include "core/Logger.h"
#include "core/ConfigManager.h"
#include "core/MethodDispatcher.h"
#include "core/PermissionChecker.h"
#include "core/MCPToolRegistry.h"
#include "core/MCPResourceRegistry.h"
#include "core/MCPPromptRegistry.h"
#include "core/X64DBGBridge.h"
#include "handlers/DebugHandler.h"
#include "handlers/RegisterHandler.h"
#include "handlers/MemoryHandler.h"
#include "handlers/BreakpointHandler.h"
#include "handlers/DisassemblyHandler.h"
#include "handlers/StackHandler.h"
#include "handlers/ThreadHandler.h"
#include "handlers/ModuleHandler.h"
#include "handlers/EventCallbackHandler.h"
#include "handlers/ScriptHandler.h"
#include "handlers/ContextHandler.h"
#include "handlers/DumpHandler.h"
#include "communication/MCPHttpServer.h"
#include "ui/ConfigEditor.h"
#include <windows.h>
#include <filesystem>
#include <fstream>

// 鎻掍欢鐗堟湰淇℃伅
#define PLUGIN_VERSION "1.0.1"

// 鍙敱 CMake 瑕嗙洊锛歅LUGIN_DISPLAY_NAME, PLUGIN_DIR_NAME
#ifndef PLUGIN_DISPLAY_NAME
#define PLUGIN_DISPLAY_NAME "x64dbg MCP Server"
#endif

#ifndef PLUGIN_DIR_NAME
#define PLUGIN_DIR_NAME "x64dbg-mcp"
#endif

#ifndef PLUGIN_NAME
#define PLUGIN_NAME PLUGIN_DISPLAY_NAME
#endif

// 鍏ㄥ眬鍙橀噺
static int g_pluginHandle = 0;
static int g_menuHandle = 0;
static std::unique_ptr<MCP::MCPHttpServer> g_mcpHttpServer;
static HMODULE g_hModule = NULL;  // 鎻掍欢妯″潡鍙ユ焺

// 鑿滃崟鍛戒护ID - 浣跨敤鍞竴鐨処D鍊?
enum MenuCommands {
    MENU_START_MCP_HTTP = 1,
    MENU_STOP_MCP_HTTP = 2,
    MENU_EDIT_CONFIG = 3,
    MENU_SHOW_CONFIG = 4,
    MENU_ABOUT = 5
};

namespace MCP {

/**
 * @brief x64dbg 鍥炶皟: 鍒濆鍖栬皟璇?
 */
static void CB_InitDebug(CBTYPE cbType, void* callbackInfo) {
    Logger::Info("Debug session started");
}

/**
 * @brief x64dbg 鍥炶皟: 鍋滄璋冭瘯
 */
static void CB_StopDebug(CBTYPE cbType, void* callbackInfo) {
    Logger::Info("Debug session stopped");
}

/**
 * @brief x64dbg 鍥炶皟: 鏂偣鍛戒腑
 */
static void CB_Breakpoint(CBTYPE cbType, void* callbackInfo) {
    // TODO: 闇€瑕佹牴鎹疄闄匰DK缁撴瀯瀹炵幇
    MCP::Logger::Info("Breakpoint hit");
}

/**
 * @brief x64dbg 鍥炶皟: 寮傚父
 */
static void CB_Exception(CBTYPE cbType, void* callbackInfo) {
    // TODO: 闇€瑕佹牴鎹疄闄匰DK缁撴瀯瀹炵幇
    MCP::Logger::Info("Exception occurred");
}

/**
 * @brief x64dbg 鍥炶皟: 妯″潡鍔犺浇
 */
static void CB_LoadDll(CBTYPE cbType, void* callbackInfo) {
    // TODO: 闇€瑕佹牴鎹疄闄匰DK缁撴瀯瀹炵幇
    MCP::Logger::Info("DLL loaded");
}

/**
 * @brief x64dbg 鍥炶皟: 妯″潡鍗歌浇
 */
static void CB_UnloadDll(CBTYPE cbType, void* callbackInfo) {
    // TODO: 闇€瑕佹牴鎹疄闄匰DK缁撴瀯瀹炵幇
    MCP::Logger::Info("DLL unloaded");
}

/**
 * @brief x64dbg 鍥炶皟: 杩涚▼鍒涘缓
 */
static void CB_CreateProcess(CBTYPE cbType, void* callbackInfo) {
    EventCallbackHandler::OnCreateProcess();
}

/**
 * @brief x64dbg 鍥炶皟: 杩涚▼閫€鍑?
 */
static void CB_ExitProcess(CBTYPE cbType, void* callbackInfo) {
    EventCallbackHandler::OnExitProcess();
}

/**
 * @brief x64dbg 鍥炶皟: 鑿滃崟椤圭偣鍑?
 */
static void CB_MenuEntry(CBTYPE cbType, void* callbackInfo) {
    auto* info = static_cast<PLUG_CB_MENUENTRY*>(callbackInfo);
    
    try {
        if (info->hEntry == MENU_EDIT_CONFIG) {
            // 缂栬緫閰嶇疆
            _plugin_logputs("[MCP] Opening config editor...");
            
            auto& config = ConfigManager::Instance();
            
            // 鏋勫缓閰嶇疆鏂囦欢璺緞
            std::string configPath = config.GetConfigPath();
            if (configPath.empty()) {
                // 濡傛灉閰嶇疆鏈姞杞?浣跨敤榛樿璺緞
                configPath = std::string(PLUGIN_DIR_NAME) + "\\config.json";
                _plugin_logprintf("[MCP] Using default config path: %s\n", configPath.c_str());
            }
            
            // 鑾峰彇x64dbg涓荤獥鍙ｅ彞鏌?
            HWND hwndDlg = GuiGetWindowHandle();
            
            if (MCP::ConfigEditor::Show(g_hModule, hwndDlg, configPath)) {
                _plugin_logputs("[MCP] Configuration updated");
                // 閲嶆柊鍔犺浇閰嶇疆
                config.Load(configPath);
                MCP::PermissionChecker::Instance().Initialize();
                
                // 濡傛灉鏈嶅姟鍣ㄦ鍦ㄨ繍琛?鎻愮ず闇€瑕侀噸鍚?
                if (g_mcpHttpServer && g_mcpHttpServer->IsRunning()) {
                    MessageBoxA(hwndDlg, 
                        "Configuration saved. Please restart MCP HTTP Server for changes to take effect.", 
                        "Config Updated", 
                        MB_OK | MB_ICONINFORMATION);
                }
            } else {
                _plugin_logputs("[MCP] Config editor cancelled or failed");
            }
        }
        else if (info->hEntry == MENU_SHOW_CONFIG) {
            // 鏄剧ず閰嶇疆
            auto& config = ConfigManager::Instance();
            
            _plugin_logputs("[MCP] Current Configuration:");
            _plugin_logprintf("  Address: %s\n", config.Get<std::string>("server.address", "127.0.0.1").c_str());
            _plugin_logprintf("  Port: %d\n", config.Get<int>("server.port", 3000));
        }
        else if (info->hEntry == MENU_START_MCP_HTTP) {
            // 鍚姩 MCP HTTP 鏈嶅姟鍣?
            if (!g_mcpHttpServer) {
                g_mcpHttpServer = std::make_unique<MCP::MCPHttpServer>();
            }
            
            if (g_mcpHttpServer->IsRunning()) {
                _plugin_logputs("[MCP] HTTP Server is already running");
                return;
            }
            
            auto& config = ConfigManager::Instance();
            std::string configPath = config.GetConfigPath();
            if (configPath.empty()) {
                configPath = std::string(PLUGIN_DIR_NAME) + "\\config.json";
            }

            // Ensure manual edits in config file are applied before startup.
            if (!config.Load(configPath)) {
                _plugin_logprintf("[MCP] Failed to reload config from: %s\n", configPath.c_str());
            }
            MCP::PermissionChecker::Instance().Initialize();

            std::string address = config.GetServerAddress();
            int port = static_cast<int>(config.GetServerPort());

            if (g_mcpHttpServer->Start(address, port)) {
                _plugin_logprintf("[MCP] HTTP Server started on http://%s:%d\n", address.c_str(), port);
                _plugin_logputs("[MCP] Configure VSCode/Claude to connect via SSE");
            } else {
                _plugin_logputs("[MCP] Failed to start HTTP Server");
            }
        }
        else if (info->hEntry == MENU_STOP_MCP_HTTP) {
            // 鍋滄 MCP HTTP 鏈嶅姟鍣?
            if (!g_mcpHttpServer || !g_mcpHttpServer->IsRunning()) {
                _plugin_logputs("[MCP] HTTP Server is not running");
                return;
            }
            
            g_mcpHttpServer->Stop();
            _plugin_logputs("[MCP] HTTP Server stopped");
        }
        else if (info->hEntry == MENU_ABOUT) {
            // 鍏充簬
            char aboutMsg[256];
            sprintf_s(aboutMsg, "[MCP] %s Plugin v1.0.1", PLUGIN_DISPLAY_NAME);
            _plugin_logputs(aboutMsg);
            _plugin_logputs("[MCP] Provides JSON-RPC debugging interface");
            _plugin_logputs("[MCP] https://github.com/SetsunaYukiOvO/x64dbg-mcp");
        }
    } catch (const std::exception& e) {
        _plugin_logprintf("[MCP] Menu callback error: %s\n", e.what());
    }
}

} // namespace MCP

/**
 * @brief 娉ㄥ唽鎵€鏈?JSON-RPC 鏂规硶
 */
static void RegisterAllMethods() {
    // 娉ㄥ唽 MCP 宸ュ叿瀹氫箟
    MCP::MCPToolRegistry::Instance().RegisterDefaultTools();
    
    // 娉ㄥ唽 MCP 璧勬簮鍜屾彁绀鸿瘝
    MCP::MCPResourceRegistry::Instance().RegisterDefaultResources();
    MCP::MCPPromptRegistry::Instance().RegisterDefaultPrompts();
    
    // 娉ㄥ唽 JSON-RPC 鏂规硶澶勭悊鍣?
    MCP::DebugHandler::RegisterMethods();
    MCP::RegisterHandler::RegisterMethods();
    MCP::MemoryHandler::RegisterMethods();
    MCP::BreakpointHandler::RegisterMethods();
    MCP::DisassemblyHandler::RegisterMethods();
    MCP::SymbolHandler::RegisterMethods();
    MCP::StackHandler::RegisterMethods();
    MCP::ThreadHandler::RegisterMethods();
    MCP::ModuleHandler::RegisterMethods();
    MCP::DumpHandler::RegisterMethods();
    
    // Register script execution methods
    auto& dispatcher = MCP::MethodDispatcher::Instance();
    dispatcher.RegisterMethod("script.execute", [](const json& params) -> json {
        return ScriptHandler::execute(params);
    });
    dispatcher.RegisterMethod("script.execute_batch", [](const json& params) -> json {
        return ScriptHandler::executeBatch(params);
    });
    dispatcher.RegisterMethod("script.get_last_result", [](const json& params) -> json {
        return ScriptHandler::getLastResult(params);
    });
    
    // Register context snapshot methods
    dispatcher.RegisterMethod("context.get_snapshot", [](const json& params) -> json {
        return ContextHandler::getSnapshot(params);
    });
    dispatcher.RegisterMethod("context.get_basic", [](const json& params) -> json {
        return ContextHandler::getBasicContext(params);
    });
    dispatcher.RegisterMethod("context.compare_snapshots", [](const json& params) -> json {
        return ContextHandler::compareSnapshots(params);
    });
    
    MCP::Logger::Info("All JSON-RPC methods registered");
}

/**
 * @brief 娉ㄥ唽鎵€鏈夊洖璋?
 */
static void RegisterCallbacks() {
    _plugin_registercallback(g_pluginHandle, CB_INITDEBUG, MCP::CB_InitDebug);
    _plugin_registercallback(g_pluginHandle, CB_STOPDEBUG, MCP::CB_StopDebug);
    _plugin_registercallback(g_pluginHandle, CB_BREAKPOINT, MCP::CB_Breakpoint);
    _plugin_registercallback(g_pluginHandle, CB_EXCEPTION, MCP::CB_Exception);
    _plugin_registercallback(g_pluginHandle, CB_CREATEPROCESS, MCP::CB_CreateProcess);
    _plugin_registercallback(g_pluginHandle, CB_EXITPROCESS, MCP::CB_ExitProcess);
    _plugin_registercallback(g_pluginHandle, CB_LOADDLL, MCP::CB_LoadDll);
    _plugin_registercallback(g_pluginHandle, CB_UNLOADDLL, MCP::CB_UnloadDll);
    _plugin_registercallback(g_pluginHandle, CB_MENUENTRY, MCP::CB_MenuEntry);  // 娉ㄥ唽鑿滃崟鍥炶皟
    
    MCP::Logger::Info("All callbacks registered");
}

// ==================== x64dbg 鎻掍欢瀵煎嚭鍑芥暟 ====================

/**
 * @brief 鎻掍欢鍒濆鍖?
 * @return true琛ㄧず鍒濆鍖栨垚鍔燂紝false琛ㄧず澶辫触
 */
extern "C" __declspec(dllexport) bool pluginit(PLUG_INITSTRUCT* initStruct) {
    // 鉁?蹇呴』濉啓鎻掍欢淇℃伅锛屽惁鍒檟64dbg鏃犳硶璇嗗埆鎻掍欢
    initStruct->sdkVersion = PLUG_SDKVERSION;
    initStruct->pluginVersion = 1;  // 鎻掍欢鐗堟湰鍙?
    strcpy_s(initStruct->pluginName, PLUGIN_NAME);  // 鎻掍欢鍚嶇О
    
    g_pluginHandle = initStruct->pluginHandle;
    
    try {
        // 鑾峰彇褰撳墠妯″潡璺緞锛岀敤浜庢棩蹇楁枃浠?
        char logPath[MAX_PATH] = {0};
        HMODULE hModule = NULL;
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               (LPCSTR)&pluginit, &hModule)) {
            GetModuleFileNameA(hModule, logPath, MAX_PATH);
            // 鏇挎崲鏂囦欢鍚嶄负鏃ュ織鏂囦欢鍚?
            char* lastSlash = strrchr(logPath, '\\');
            if (lastSlash) {
                std::string logName = std::string(PLUGIN_DIR_NAME) + ".log";
                strcpy_s(lastSlash + 1, MAX_PATH - (lastSlash - logPath + 1), logName.c_str());
            }
        }
        
        // 濡傛灉鑾峰彇璺緞澶辫触锛屼娇鐢ㄩ粯璁よ矾寰?
        if (logPath[0] == 0) {
            std::string logName = std::string(PLUGIN_DIR_NAME) + ".log";
            strcpy_s(logPath, logName.c_str());
        }
        
        // 鍒濆鍖栨棩蹇楃郴缁?
        MCP::Logger::Initialize(logPath, MCP::LogLevel::Info, true);
        char initMsg[256];
        sprintf_s(initMsg, "%s Plugin v1.0.1 initializing...", PLUGIN_DISPLAY_NAME);
        MCP::Logger::Info(initMsg);
        _plugin_logprintf("[MCP] Log file: %s\n", logPath);
        
        _plugin_logprintf("[MCP] %s v%s\n", PLUGIN_DISPLAY_NAME, PLUGIN_VERSION);
        _plugin_logputs("[MCP] https://github.com/SetsunaYukiOvO/x64dbg-mcp");
        
        // 鍔犺浇閰嶇疆锛堝鏋滀笉瀛樺湪鍒欏垱寤洪粯璁ら厤缃級
        try {
            auto& config = MCP::ConfigManager::Instance();
            std::string configPath = std::string(PLUGIN_DIR_NAME) + "\\config.json";
            
            // 妫€鏌ラ厤缃枃浠舵槸鍚﹀瓨鍦?
            if (!std::filesystem::exists(configPath)) {
                MCP::Logger::Warning("Config file not found, creating default config.json");
                _plugin_logputs("[MCP] Creating default config.json...");
                
                // 鍒涘缓鐩綍
                std::filesystem::create_directories(PLUGIN_DIR_NAME);
                
                // 鍒涘缓榛樿閰嶇疆
                std::ofstream configFile(configPath);
                if (configFile.is_open()) {
                    configFile << R"({
  "version": "1.0.1",
  "server": {
    "address": "127.0.0.1",
    "port": 3000
  },
  "permissions": {
    "allow_memory_write": true,
    "allow_register_write": true,
    "allow_script_execution": true,
    "allow_breakpoint_modification": true,
    "allowed_methods": [
      "debug.*",
      "register.*",
      "memory.*",
      "breakpoint.*",
      "disasm.*",
      "disassembly.*",
      "module.*",
      "symbol.*",
      "thread.*",
      "stack.*",
      "comment.*",
      "script.*",
      "context.*",
      "dump.*"
    ]
  },
    "logging": {
        "enabled": true,
        "level": "info",
        "file": "plugin.log",
        "max_file_size_mb": 10,
        "console_output": true
    },
  "timeout": {
    "request_timeout_ms": 30000,
    "step_timeout_ms": 10000,
    "memory_read_timeout_ms": 5000
  },
  "features": {
    "enable_notifications": true,
    "enable_heartbeat": true,
    "heartbeat_interval_seconds": 30,
    "enable_batch_requests": true
  }
})";
                    configFile.close();
                    MCP::Logger::Info("Default config.json created successfully");
                    _plugin_logputs("[MCP] Default config.json created");
                } else {
                    MCP::Logger::Error("Failed to create config.json");
                    _plugin_logputs("[MCP] Failed to create config.json");
                }
            }
            
            config.Load(configPath);
            
            // 璁剧疆鏃ュ織绾у埆
            std::string logLevel = config.Get<std::string>("logging.level", "info");
            // 杞崲瀛楃涓插埌鏋氫妇
            MCP::LogLevel level = MCP::LogLevel::Info;
            if (logLevel == "trace") level = MCP::LogLevel::Trace;
            else if (logLevel == "debug") level = MCP::LogLevel::Debug;
            else if (logLevel == "warning") level = MCP::LogLevel::Warning;
            else if (logLevel == "error") level = MCP::LogLevel::Error;
            else if (logLevel == "critical") level = MCP::LogLevel::Critical;
            MCP::Logger::SetLevel(level);
        } catch (const std::exception& e) {
            MCP::Logger::Warning("Failed to load config: {}, using defaults", e.what());
        }
        
        // 鍒濆鍖栨潈闄愭鏌ュ櫒
        try {
            MCP::PermissionChecker::Instance().Initialize();
            MCP::Logger::Info("PermissionChecker initialized");
        } catch (const std::exception& e) {
            MCP::Logger::Error("Failed to initialize PermissionChecker: {}", e.what());
            _plugin_logprintf("[MCP] Failed to initialize PermissionChecker: %s\n", e.what());
        }
        
        // 娉ㄥ唽鎵€鏈?JSON-RPC 鏂规硶
        try {
            RegisterAllMethods();
        } catch (const std::exception& e) {
            MCP::Logger::Error("Failed to register methods: {}", e.what());
            _plugin_logprintf("[MCP] Failed to register methods: %s\n", e.what());
        }
        
        // 娉ㄥ唽 x64dbg 鍥炶皟
        try {
            RegisterCallbacks();
        } catch (const std::exception& e) {
            MCP::Logger::Error("Failed to register callbacks: {}", e.what());
            _plugin_logprintf("[MCP] Failed to register callbacks: %s\n", e.what());
        }
        
        // 鍒濆鍖栦簨浠跺鐞嗗櫒
        try {
            MCP::EventCallbackHandler::Instance().Initialize();
        } catch (const std::exception& e) {
            MCP::Logger::Error("Failed to initialize event handler: {}", e.what());
            _plugin_logprintf("[MCP] Failed to initialize event handler: %s\n", e.what());
        }
        
        MCP::Logger::Info("Plugin initialized successfully");
        _plugin_logputs("[MCP] Plugin initialized successfully");
        return true;  // 鉁?杩斿洖true琛ㄧず鍒濆鍖栨垚鍔?
    } catch (const std::exception& e) {
        // 鎹曡幏鎵€鏈夋湭澶勭悊鐨勫紓甯革紝闃叉宕╂簝
        _plugin_logprintf("[MCP] FATAL ERROR during initialization: %s\n", e.what());
        return false;  // 鉁?杩斿洖false琛ㄧず鍒濆鍖栧け璐?
    } catch (...) {
        _plugin_logputs("[MCP] FATAL ERROR: Unknown exception during initialization");
        return false;  // 鉁?杩斿洖false琛ㄧず鍒濆鍖栧け璐?
    }
}

/**
 * @brief 鎻掍欢鍋滄
 */
extern "C" __declspec(dllexport) void plugstop() {
    MCP::Logger::Info("Plugin stopping...");
    
    // 鍋滄 MCP HTTP 鏈嶅姟鍣?
    try {
        if (g_mcpHttpServer && g_mcpHttpServer->IsRunning()) {
            g_mcpHttpServer->Stop();
        }
        g_mcpHttpServer.reset();
    } catch (...) {}
    
    // 娓呯悊浜嬩欢澶勭悊鍣?
    MCP::EventCallbackHandler::Instance().Cleanup();
    
    MCP::Logger::Info("Plugin stopped");
    MCP::Logger::Shutdown();
}

/**
 * @brief 鎻掍欢璁剧疆锛堝垱寤鸿彍鍗曪級
 * @return true琛ㄧず璁剧疆鎴愬姛锛宖alse琛ㄧず澶辫触
 */
extern "C" __declspec(dllexport) bool plugsetup(PLUG_SETUPSTRUCT* setupStruct) {
    _plugin_logputs("[MCP] Setting up plugin menu...");
    
    // 娣诲姞鎻掍欢鑿滃崟
    g_menuHandle = _plugin_menuadd(setupStruct->hMenu, "&MCP Server");
    
    if (g_menuHandle) {
        // 鉁?浣跨敤 _plugin_menuaddentry 娣诲姞鍙偣鍑荤殑鑿滃崟椤?
        _plugin_menuaddentry(g_menuHandle, MENU_START_MCP_HTTP, "Start &MCP HTTP Server");
        _plugin_menuaddentry(g_menuHandle, MENU_STOP_MCP_HTTP, "Stop M&CP HTTP Server");
        _plugin_menuaddseparator(g_menuHandle);  // 鍒嗛殧绗?
        _plugin_menuaddentry(g_menuHandle, MENU_EDIT_CONFIG, "&Edit Config");
        _plugin_menuaddentry(g_menuHandle, MENU_SHOW_CONFIG, "Show &Config");
        _plugin_menuaddseparator(g_menuHandle);  // 鍒嗛殧绗?
        _plugin_menuaddentry(g_menuHandle, MENU_ABOUT, "&About");
        
        _plugin_logputs("[MCP] Plugin menu created successfully");
        _plugin_logprintf("[MCP] Menu handle: %d\n", g_menuHandle);
        _plugin_logprintf("[MCP] Menu entries added\n");
    } else {
        _plugin_logputs("[MCP] ERROR: Failed to create plugin menu!");
        return false;  // 鉁?鑿滃崟鍒涘缓澶辫触
    }
    
    // 浣跨敤x64dbg鐨勬棩蹇楃郴缁燂紝鍥犱负鑷畾涔塋ogger鍙兘杩樻湭鍒濆鍖?
    try {
        MCP::Logger::Debug("Plugin menu created");
    } catch (...) {
        // 濡傛灉Logger鏈垵濮嬪寲锛屽拷鐣ラ敊璇?
    }
    
    return true;  // 鉁?杩斿洖true琛ㄧず璁剧疆鎴愬姛
}

/**
 * @brief DLL 涓诲嚱鏁?
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            g_hModule = hModule;  // 淇濆瓨妯″潡鍙ユ焺
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
