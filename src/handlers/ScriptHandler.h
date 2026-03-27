#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <mutex>

using json = nlohmann::json;

class ScriptHandler {
public:
    // Execute x64dbg script command
    static json execute(const json& params);
    
    // Execute multiple commands in sequence
    static json executeBatch(const json& params);
    
    // Get last command result
    static json getLastResult(const json& params);

private:
    static void UpdateLastResult(bool success, const std::string& resultText);

    static std::string lastResult;
    static bool lastSuccess;
    static std::mutex resultMutex;
};
