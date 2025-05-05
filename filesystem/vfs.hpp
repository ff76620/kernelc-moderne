#ifndef VIRTUAL_FILE_SYSTEM_HPP
#define VIRTUAL_FILE_SYSTEM_HPP

#include 
#include 
#include "../include/types.hpp"

namespace Kernel {

class VirtualFileSystem {
public:
    virtual ~VirtualFileSystem() = default;

    struct FileDescriptor {
        uint32_t id;
        std::string path;
        uint32_t flags;
        size_t position;
    };

    virtual bool mount(const std::string& device, const std::string& mountPoint, const std::string& fsType) = 0;
    virtual bool unmount(const std::string& mountPoint) = 0;
    
    virtual std::unique_ptr open(const std::string& path, uint32_t flags) = 0;
    virtual bool close(const FileDescriptor& fd) = 0;
    
    virtual ssize_t read(const FileDescriptor& fd, void* buffer, size_t count) = 0;
    virtual ssize_t write(const FileDescriptor& fd, const void* buffer, size_t count) = 0;

    // Common filesystem operations
    virtual bool mkdir(const std::string& path, uint32_t mode);
    virtual bool rmdir(const std::string& path);
    virtual bool rename(const std::string& oldPath, const std::string& newPath);
    virtual bool link(const std::string& target, const std::string& linkpath);
    virtual bool unlink(const std::string& path);
    virtual bool chmod(const std::string& path, uint32_t mode);
    virtual bool chown(const std::string& path, uint32_t uid, uint32_t gid);

    // File attributes
    struct FileAttributes {
        uint32_t mode;
        uint32_t uid;
        uint32_t gid;
        size_t size;
        std::chrono::system_clock::time_point accessTime;
        std::chrono::system_clock::time_point modifyTime;
        std::chrono::system_clock::time_point createTime;
    };

    virtual bool getAttributes(const std::string& path, FileAttributes& attrs);
    virtual bool setAttributes(const std::string& path, const FileAttributes& attrs);

protected:
    VirtualFileSystem() = default;
};

} // namespace Kernel

#endif