#include "ServerManager.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "PermissionChecker.h"
#include "JSONRPCParser.h"
#include "ResponseBuilder.h"
#include "Exceptions.h"

namespace MCP {

ServerManager& ServerManager::Instance() {
    static ServerManager instance;
    return instance;
}

ServerManager::~ServerManager() {
    Shutdown();
}

bool ServerManager::Initialize(const std::string& configPath) {
    if (m_initialized) {
        Logger::Warning("ServerManager already initialized");
        return true;
    }
    
    Logger::Info("Initializing MCP Server...");
    
    // 鍔犺浇閰嶇疆
    auto& config = ConfigManager::Instance();
    if (!config.Load(configPath)) {
        Logger::Error("Failed to load configuration from: {}", configPath);
        return false;
    }
    
    // 鍒濆鍖栨棩蹇楃郴缁?
    if (config.IsLoggingEnabled()) {
        std::string logFile = config.GetLogFile();
        std::string logLevelStr = config.GetLogLevel();
        
        LogLevel logLevel = LogLevel::Info;
        if (logLevelStr == "trace") logLevel = LogLevel::Trace;
        else if (logLevelStr == "debug") logLevel = LogLevel::Debug;
        else if (logLevelStr == "warning") logLevel = LogLevel::Warning;
        else if (logLevelStr == "error") logLevel = LogLevel::Error;
        
        if (!Logger::Initialize(logFile, logLevel, true)) {
            return false;
        }
    }
    
    // 鍒濆鍖栨潈闄愭鏌ュ櫒
    PermissionChecker::Instance().Initialize();
    
    // 娉ㄥ唽榛樿鏂规硶
    MethodDispatcher::Instance().RegisterDefaultMethods();
    
    // 鍒涘缓 TCP 鏈嶅姟鍣?
    m_tcpServer = std::make_unique<TCPServer>();
    
    // 璁剧疆鍥炶皟
    m_tcpServer->SetMessageHandler(
        std::bind(&ServerManager::OnMessageReceived, this, 
                 std::placeholders::_1, std::placeholders::_2)
    );
    
    m_tcpServer->SetConnectionHandler(
        std::bind(&ServerManager::OnConnectionChanged, this,
                 std::placeholders::_1, std::placeholders::_2)
    );
    
    m_initialized = true;
    Logger::Info("MCP Server initialized successfully");
    return true;
}

bool ServerManager::Start() {
    if (!m_initialized) {
        Logger::Error("ServerManager not initialized");
        return false;
    }
    
    if (m_running) {
        Logger::Warning("ServerManager already running");
        return true;
    }
    
    Logger::Info("Starting MCP Server...");
    
    auto& config = ConfigManager::Instance();
    std::string address = config.GetServerAddress();
    uint16_t port = config.GetServerPort();
    
    // 鍚姩 TCP 鏈嶅姟鍣?
    if (!m_tcpServer->Start(address, port)) {
        Logger::Error("Failed to start TCP server");
        return false;
    }
    
    // 鍚姩蹇冭烦鐩戞帶锛堝鏋滃惎鐢級
    if (config.Get<bool>("features.enable_heartbeat", true)) {
        Logger::Warning("Heartbeat monitor is currently disabled in TCP mode");
    }
    
    m_running = true;
    Logger::Info("MCP Server started on {}:{}", address, port);
    return true;
}

void ServerManager::Stop() {
    if (!m_running) {
        return;
    }
    
    Logger::Info("Stopping MCP Server...");
    
    // 鍋滄蹇冭烦鐩戞帶
    if (m_heartbeatMonitor) {
        m_heartbeatMonitor->Stop();
        m_heartbeatMonitor.reset();
    }
    
    // 鍋滄 TCP 鏈嶅姟鍣?
    if (m_tcpServer) {
        m_tcpServer->Stop();
    }
    
    m_running = false;
    Logger::Info("MCP Server stopped");
}

bool ServerManager::IsRunning() const {
    return m_running;
}

void ServerManager::Shutdown() {
    Stop();
    
    if (m_tcpServer) {
        m_tcpServer.reset();
    }
    
    Logger::Shutdown();
    m_initialized = false;
}

void ServerManager::SendNotification(const std::string& method, const json& params) {
    if (!m_running || !m_tcpServer) {
        return;
    }
    
    std::string notification = ResponseBuilder::CreateNotification(method, params);
    m_tcpServer->BroadcastMessage(notification);
}

void ServerManager::OnMessageReceived(ClientId clientId, const std::string& message) {
    Logger::Trace("Received message from client {}: {}", clientId, message);
    ProcessRequest(clientId, message);
}

void ServerManager::OnConnectionChanged(ClientId clientId, bool connected) {
    if (connected) {
        Logger::Info("Client {} connected", clientId);
    } else {
        Logger::Info("Client {} disconnected", clientId);
    }
}

void ServerManager::ProcessRequest(ClientId clientId, const std::string& message) {
    try {
        // 瑙ｆ瀽璇锋眰
        JSONRPCRequest request = JSONRPCParser::ParseRequest(message);
        
        // 鍒嗗彂璇锋眰
        JSONRPCResponse response = MethodDispatcher::Instance().Dispatch(request);
        
        // 鍙戦€佸搷搴旓紙閫氱煡娑堟伅涓嶉渶瑕佸搷搴旓級
        if (!request.IsNotification()) {
            std::string responseStr = ResponseBuilder::Serialize(response);
            m_tcpServer->SendMessage(clientId, responseStr);
        }
        
    } catch (const ::MCP::ParseErrorException&) {
        Logger::Error("Parse error from client {}", clientId);
        JSONRPCResponse errorResponse = ResponseBuilder::CreateErrorResponse(
            RequestId(), -32700, "Parse error"
        );
        std::string responseStr = ResponseBuilder::Serialize(errorResponse);
        m_tcpServer->SendMessage(clientId, responseStr);
        
    } catch (const ::std::exception&) {
        Logger::Error("Exception processing request from client {}", clientId);
    }
}

} // namespace MCP
