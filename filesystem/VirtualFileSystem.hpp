#ifndef VIRTUAL_FILE_SYSTEM_HPP
#define VIRTUAL_FILE_SYSTEM_HPP

#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include "./types.hpp"
#include "./FileCache.hpp"

namespace Kernel {
namespace FileSystem {

class FileSystemDriver;
class JournalManager;
class SecurityManager;

class VirtualFileSystem {
private:
    static VirtualFileSystem* instance;
    static std::mutex instanceMutex;
    
    FileCache cache;
    std::unique_ptr journalManager;
    std::unique_ptr securityManager;
    
    struct FileDescriptor {
        uint32_t id;
        std::string path;
        uint32_t flags;
        size_t position;
    };
    
    std::mutex mutex;
    uint32_t nextFileDescriptorId;
    std::vector openFiles;
    std::unordered_map> fsDrivers;

public:
    static VirtualFileSystem& getInstance() {
        std::lock_guard lock(instanceMutex);
        if (!instance) {
            instance = new VirtualFileSystem();
        }
        return *instance;
    }

    Result openFile(const std::string& path, uint32_t flags) {
        if (!validatePath(path)) {
            return Result::error("Invalid path");
        }

        auto cachedFile = cache.get(path);
        if (cachedFile) {
            auto fd = createFileDescriptor(path, flags);
            return Result::ok(fd);
        }

        auto driver = getDriverForPath(path);
        if (!driver) {
            return Result::error("No driver found");
        }

        auto result = driver->openFile(path, flags);
        if (!result) {
            return Result::error(result.error());
        }

        auto fd = createFileDescriptor(path, flags);
        cache.store(path, result.value());
        return Result::ok(fd);
    }

    FileSystemDriver* getDriverForPath(const std::string& path) {
        // Get mount point from path
        std::string mountPoint = getMountPoint(path);
        if (mountPoint.empty()) {
            return nullptr;
        }

        // Get appropriate driver based on filesystem type
        try {
            auto& driver = fsDrivers.at(mountPoint);
            if (!driver->isInitialized()) {
                if (!driver->initialize()) {
                    return nullptr;
                }
            }
            return driver.get();
        } catch (const std::out_of_range&) {
            return nullptr;
        }
    }

    bool validatePath(const std::string& path) {
        try {
            // Check for null bytes
            if (path.find('\0') != std::string::npos) {
                return false;
            }

            // Check maximum path length
            const size_t MAX_PATH_LENGTH = 4096;
            if (path.length() > MAX_PATH_LENGTH) {
                return false;
            }

            // Check for invalid characters
            static const std::string invalidChars = "<>:\"|?*";
            if (path.find_first_of(invalidChars) != std::string::npos) {
                return false;
            }

            // Normalize path
            std::string normalized = normalizePath(path);
            
            // Check path components
            std::stringstream ss(normalized);
            std::string component;
            while (std::getline(ss, component, '/')) {
                if (component.empty()) continue;

                // Check component length
                const size_t MAX_NAME_LENGTH = 255;
                if (component.length() > MAX_NAME_LENGTH) {
                    return false;
                }

                // Check for reserved names
                static const std::vector reserved = {
                    "CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3",
                    "LPT1", "LPT2", "LPT3", "CLOCK$"
                };

                std::string upper = component;
                std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                if (std::find(reserved.begin(), reserved.end(), upper) != reserved.end()) {
                    return false;
                }
            }

            // Check for directory traversal
            if (normalized.find("../") != std::string::npos) {
                if (!isWithinPermittedBoundaries(normalized)) {
                    return false;
                }
            }

            return true;

        } catch (const std::exception&) {
            return false;
        }
    }

private:
    // Private implementation methods
    FileDescriptor* createFileDescriptor(const std::string& path, uint32_t flags) {
        std::lock_guard lock(mutex);
        FileDescriptor fd;
        fd.id = nextFileDescriptorId++;
        fd.path = path;
        fd.flags = flags;
        fd.position = 0;
        openFiles.push_back(fd);
        return &openFiles.back();
    }

    Result<void> closeFileDescriptor(uint32_t id) {
        std::lock_guard lock(mutex);
        auto it = std::find_if(openFiles.begin(), openFiles.end(),
            [id](const FileDescriptor& fd) { return fd.id == id; });
        if (it == openFiles.end()) {
            return Result<void>::error("Invalid file descriptor");
        }
        openFiles.erase(it);
        return Result<void>::ok();
    }

    bool isWithinPermittedBoundaries(const std::string& path) {
        for (const auto& mount : fsDrivers) {
            if (path.find(mount.first) == 0) {
                return securityManager->checkAccess(path);
            }
        }
        return false;
    }

    std::string getMountPoint(const std::string& path) {
        std::string bestMatch;
        size_t bestLength = 0;
        for (const auto& mount : fsDrivers) {
            if (path.find(mount.first) == 0 && mount.first.length() > bestLength) {
                bestMatch = mount.first;
                bestLength = mount.first.length();
            }
        }
        return bestMatch;
    }

    std::string normalizePath(const std::string& path) {
        std::vector components;
        std::stringstream ss(path);
        std::string item;

        while (std::getline(ss, item, '/')) {
            if (item.empty() || item == ".") continue;
            if (item == "..") {
                if (!components.empty()) components.pop_back();
            } else {
                components.push_back(item);
            }
        }

        std::string result = path[0] == '/' ? "/" : "";
        for (size_t i = 0; i < components.size(); ++i) {
            if (i > 0) result += "/";
            result += components[i];
        }
        return result.empty() ? "/" : result;
    }
};

} // namespace FileSystem
} // namespace Kernel

#endif
void>void>void>