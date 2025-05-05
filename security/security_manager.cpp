#include "kernel/security/security_manager.hpp"
#include "kernel/loggin/EventLogger.hpp"
#include 
#include 
#include 
#include 
#include 
#include 

namespace Kernel {
namespace Security {

// Define security enums and structs
enum class SecurityLevel {
  LOW = 0,
  MEDIUM = 1,
  HIGH = 2, 
  CRITICAL = 3
};

struct AccessControlEntry {
  uint32_t userId;
  uint32_t groupId; 
  std::bitset<32> permissions;
};

struct AuditRecord {
  uint32_t userId;
  std::string action;
  std::chrono::system_clock::time_point timestamp;
  bool success;
};

struct SecurityPolicy {
  SecurityLevel level;
  bool allowFileAccess;
  bool allowNetworking;
  std::vector allowedPaths;
};

class SecurityManager {
private:
    bool initialized;
    std::mutex mutex;
    std::mutex aclMutex;
    std::mutex auditMutex;
    std::unordered_map> acls;
    std::vector auditTrail;
    std::unordered_map securityPolicies;
    std::vector securityLog;

public:
    static SecurityManager& getInstance() {
        static SecurityManager instance;
        return instance;
    }

    SecurityManager() : initialized(false) {}
    
    ~SecurityManager() {
        shutdown();
    }

    bool initialize() {
        std::lock_guard lock(mutex);
        
        if (initialized) {
            return true;
        }
        
        EventLogger::log("Initializing Security Manager...");
        
        try {
            setupSecurityPolicies();
            initializePermissions();
            setupAccessControl();
            
            securityPolicies.clear();
            securityLog.clear();
            
            initialized = true;
            EventLogger::log("Security Manager initialized successfully");
            return true;
        }
        catch (const std::exception& e) {
            EventLogger::log("Error initializing Security Manager: " + std::string(e.what()));
            return false;
        }
    }

    bool setPermissions(const std::string& resource, uint32_t userId,
                       uint32_t groupId, const std::bitset<32>& permissions) {
        std::lock_guard lock(aclMutex);
        
        auto& entries = acls[resource];
        auto it = std::find_if(entries.begin(), entries.end(),
            [userId, groupId](const AccessControlEntry& ace) {
                return ace.userId == userId && ace.groupId == groupId;
            });

        if (it != entries.end()) {
            it->permissions = permissions;
        } else {
            entries.push_back({userId, groupId, permissions});
        }

        logAuditEvent(userId, "SET_PERMISSIONS", true);
        return true;
    }

    bool checkAccess(const std::string& resource, uint32_t userId,
                    uint32_t groupId, uint32_t requestedPermission) {
        std::lock_guard lock(aclMutex);
        
        if (!initialized) return false;
        
        auto it = acls.find(resource);
        if (it == acls.end()) {
            logAuditEvent(userId, "ACCESS_DENIED", false);
            return false;
        }

        for (const auto& ace : it->second) {
            if ((ace.userId == userId || ace.userId == 0xFFFFFFFF) &&
                (ace.groupId == groupId || ace.groupId == 0xFFFFFFFF)) {
                bool hasAccess = ace.permissions.test(requestedPermission);
                logAuditEvent(userId, "ACCESS_CHECK", hasAccess);
                return hasAccess;
            }
        }

        return false;
    }

    bool setSecurityPolicy(const std::string& processId, const SecurityPolicy& policy) {
        if (!initialized) return false;
        
        std::lock_guard lock(mutex);
        
        if (!validatePolicy(policy)) {
            EventLogger::log("Invalid security policy rejected for process: " + processId);
            return false;
        }
        
        securityPolicies[processId] = policy;
        logSecurityEvent("Security policy set for process: " + processId, policy.level);
        return true;
    }

    void shutdown() {
        std::lock_guard lock(mutex);
        
        if (!initialized) {
            return;
        }
        
        EventLogger::log("Shutting down Security Manager...");
        securityPolicies.clear();
        securityLog.clear();
        initialized = false;
    }

    std::vector getSecurityLog() {
        std::lock_guard lock(mutex);
        return securityLog;
    }

private:
    void logAuditEvent(uint32_t userId, const std::string& action, bool success) {
        std::lock_guard lock(auditMutex);
        auditTrail.push_back({
            userId,
            action,
            std::chrono::system_clock::now(),
            success
        });
    }

    void logSecurityEvent(const std::string& event, SecurityLevel severity) {
        std::string timestamp = std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count()
        );
        
        std::string logEntry = timestamp + " [" + std::to_string(static_cast(severity)) + "] " + event;
        securityLog.push_back(logEntry);
        
        if (severity >= SecurityLevel::HIGH) {
            EventLogger::log("SECURITY ALERT: " + event);
        }
    }

    bool validatePolicy(const SecurityPolicy& policy) {
        if (policy.level < SecurityLevel::LOW || policy.level > SecurityLevel::CRITICAL) {
            return false;
        }
        
        for (const auto& path : policy.allowedPaths) {
            if (path.empty()) {
                return false;
            }
        }
        
        return true;
    }
};

} // namespace Security 
} // namespace Kernel