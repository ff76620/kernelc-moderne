#ifndef FS_MANAGER_HPP
#define FS_MANAGER_HPP

#include 
#include 
#include 
#include "../filesystem/VirtualFileSystem.hpp"
#include "../filesystem/FileCache.hpp"

namespace Kernel {

class FileSystemManager {
public:
    static FileSystemManager& getInstance();

    bool initialize();
    void shutdown();

    bool registerFileSystem(const std::string& fsType, std::unique_ptr fs);
    bool unregisterFileSystem(const std::string& fsType);

    VirtualFileSystem* getFileSystem(const std::string& path);
    bool mount(const std::string& device, const std::string& mountPoint, const std::string& fsType);
    bool unmount(const std::string& mountPoint);

    void syncFileSystems();
    bool checkFileSystems();
    
    // Monitoring and Statistics
    struct FSStats {
        uint64_t totalSpace;
        uint64_t freeSpace;
        uint64_t inodes;
        uint64_t freeInodes;
    };

    FSStats getStats(const std::string& mountPoint);
    void printMountPoints() const;

private:
    FileSystemManager() = default;
    ~FileSystemManager() = default;

    std::map> fileSystems;
    std::map mountPoints;
};

} // namespace Kernel

#endif