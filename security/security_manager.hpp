// security_manager.hpp
#ifndef SECURITY_MANAGER_HPP
#define SECURITY_MANAGER_HPP

#include "../include/types.hpp"
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include <chrono>
#include <filesystem>

namespace Kernel {
namespace Security {

enum class SecurityLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

struct Permission {
    std::string name;
    uint32_t flags;
    SecurityLevel minLevel;
};

struct Role {
    std::string name;
    std::vector<Permission> permissions;
    uint32_t priority_level;
    SecurityLevel securityLevel;
};

class SecurityManager {
private:
    // Singleton implementation
    static SecurityManager* instance;
    SecurityManager();
    
    // Core components
    std::mutex mutex;
    bool initialized;
    std::unique_ptr<HardwareAccelerator> hw_accelerator;
    KeyStore key_store;
    std::vector<std::string> securityLog;

    // Access Control Components
    class ACL {
        private:
            std::unordered_map<UserId, std::vector<Permission>> user_acls;
            std::unordered_map<GroupId, std::vector<Permission>> group_acls;
            std::map<std::string, SecurityPolicy> securityPolicies;
            
        public:
            bool checkAccess(const UserId& user, const Permission& perm);
            bool validatePolicy(const SecurityPolicy& policy);
            bool setSecurityPolicy(const std::string& processId, const SecurityPolicy& policy);
    } acl;

    // Quota Management
    class QuotaManager {
        private:
            struct QuotaInfo {
                size_t soft_limit;
                size_t hard_limit;
                size_t current_usage;
                std::chrono::system_clock::time_point grace_period;
            };
            std::unordered_map<UserId, QuotaInfo> quotas;
            
        public:
            bool checkQuota(const UserId& user, size_t requested_size);
            void updateUsage(const UserId& user, size_t delta);
            void setLimits(const UserId& user, size_t soft, size_t hard);
    } quotaManager;

public:
    static SecurityManager& getInstance();
    
    bool initialize();
    void shutdown();
    
    // Security policy management
    bool setSecurityPolicy(const std::string& processId, const SecurityPolicy& policy);
    bool getSecurityPolicy(const std::string& processId, SecurityPolicy& policy);
    
    // Access control
    bool checkAccess(const std::string& processId, const std::string& resource);
    bool validateOperation(const std::string& processId, const std::string& operation);
    
    // Encryption services
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data, const std::string& key);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data, const std::string& key);
    void encryptDirectory(const std::filesystem::path& dir_path);
    void encryptFile(const std::filesystem::path& file_path);
    
    // Quota management
    bool enforceQuota(const UserId& user, size_t size);
    void setUserQuota(const UserId& user, size_t soft_limit, size_t hard_limit);
    
    // Audit logging
    void logSecurityEvent(const std::string& event, SecurityLevel severity);
    std::vector<std::string> getSecurityLog() const;

    // Role management
    void assignRole(const UserId& user, const Role& role);
    bool checkPermission(const UserId& user, const Permission& permission);

    // Prevent copying
    SecurityManager(const SecurityManager&) = delete;
    SecurityManager& operator=(const SecurityManager&) = delete;
};

} // namespace Security
} // namespace Kernel

#endif // SECURITY_MANAGER_HPP