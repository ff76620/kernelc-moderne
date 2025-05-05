#ifndef VFS_HPP
#define VFS_HPP

#include 
#include 
#include 
#include 

namespace Kernel {

class FileSystem;
class File;
class Directory;

struct FilePermissions {
    bool read;
    bool write;
    bool execute;
    
    FilePermissions(bool r = false, bool w = false, bool x = false)
        : read(r), write(w), execute(x) {}
};

class FileNode {
public:
    enum Type {
        FILE_TYPE,
        DIRECTORY_TYPE,
        SYMLINK_TYPE
    };

    FileNode(const std::string& name, Type type);
    virtual ~FileNode() = default;

    const std::string& getName() const { return name; }
    Type getType() const { return type; }
    FilePermissions& getPermissions() { return permissions; }
    
    virtual size_t getSize() const = 0;
    virtual time_t getModificationTime() const = 0;

protected:
    std::string name;
    Type type;
    FilePermissions permissions;
    time_t modificationTime;
};

class VirtualFileSystem {
public:
    VirtualFileSystem();
    ~VirtualFileSystem();

    // Mounting operations
    bool mount(const std::string& path, std::shared_ptr fs);
    bool unmount(const std::string& path);

    // File operations
    std::shared_ptr openFile(const std::string& path, int flags);
    bool closeFile(std::shared_ptr file);
    bool createFile(const std::string& path);
    bool deleteFile(const std::string& path);

    // Directory operations
    std::shared_ptr openDirectory(const std::string& path);
    bool createDirectory(const std::string& path);
    bool deleteDirectory(const std::string& path);

    // Path operations
    std::string normalizePath(const std::string& path);
    bool exists(const std::string& path);
    
    // Permission operations
    bool setPermissions(const std::string& path, const FilePermissions& permissions);
    FilePermissions getPermissions(const std::string& path);

private:
    struct MountPoint {
        std::string path;
        std::shared_ptr filesystem;
    };

    std::shared_ptr root;
    std::vector mountPoints;
    std::map> openFiles;

    std::shared_ptr resolvePath(const std::string& path);
    std::shared_ptr getResponsibleFS(const std::string& path);
};

} // namespace Kernel

#endif // VFS_HPP