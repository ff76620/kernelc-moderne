
#include "kernel/filesystem/vfs.hpp"
#include "kernel/filesystem/FileCache.hpp"
#include "kernel/include/types.hpp"
#include 
#include 
#include 
#include 
#include 
#include 

namespace Kernel {

class FileSystem;
class FileNode;

struct FilePermissions {
    bool read;
    bool write;
    bool execute;
    
    FilePermissions(bool r = false, bool w = false, bool x = false)
        : read(r), write(w), execute(x) {}
};

class FileNode {
public:
    enum Type { FILE_TYPE, DIRECTORY_TYPE, SYMLINK_TYPE };
    
    FileNode(const std::string& name, Type type)
        : name(name)
        , type(type)
        , permissions(true, true, false)  // Default RW permissions
        , modificationTime(std::time(nullptr))
    {}

    const std::string& getName() const { return name; }
    Type getType() const { return type; }
    FilePermissions& getPermissions() { return permissions; }
    std::time_t getModificationTime() const { return modificationTime; }

private:
    std::string name;
    Type type;
    FilePermissions permissions;
    std::time_t modificationTime;
};

struct MountPoint {
    std::string path;
    std::shared_ptr filesystem;
};

class VirtualFileSystem {
public:
    VirtualFileSystem() {
        root = std::make_shared("/", FileNode::DIRECTORY_TYPE);
        initialize();
    }

    void mountRootFS() {
        // Create a root filesystem instance
        auto rootFS = std::make_shared();
        
        // Initialize essential root directories
        rootFS->createDirectory("/bin");
        rootFS->createDirectory("/boot");
        rootFS->createDirectory("/dev");
        rootFS->createDirectory("/etc");
        rootFS->createDirectory("/home");
        rootFS->createDirectory("/lib");
        rootFS->createDirectory("/mnt");
        rootFS->createDirectory("/proc");
        rootFS->createDirectory("/root");
        rootFS->createDirectory("/sbin");
        rootFS->createDirectory("/tmp");
        rootFS->createDirectory("/usr");
        rootFS->createDirectory("/var");
        
        // Set appropriate permissions
        rootFS->setPermissions("/", FilePermissions(true, false, true)); // r-x
        rootFS->setPermissions("/tmp", FilePermissions(true, true, true)); // rwx
        
        // Mount the root filesystem
        mount("/", rootFS);
    }

    void mountFilesystems() {
        // Mount the root filesystem first
        mountRootFS();
        
        // Setup additional mount points
        setupMountPoints();
        
        // Mount virtual filesystems
        mountVirtualFilesystems();
    }

    void setupMountPoints() {
        // Setup standard mount points
        createMountPoint("/dev", "devfs");
        createMountPoint("/proc", "procfs");
        createMountPoint("/sys", "sysfs");
        createMountPoint("/tmp", "tmpfs");
        
        // Setup user home directories mount point
        createMountPoint("/home", "userfs");
        
        // Setup media mount points
        createMountPoint("/media", "mediafs");
        createMountPoint("/mnt", "mntfs");
    }

    std::shared_ptr resolvePath(const std::string& path) {
        std::string normalizedPath = normalizePath(path);
        
        // Handle root path specially
        if (normalizedPath == "/") {
            return root;
        }
        
        // Split path into components
        std::vector components;
        std::stringstream ss(normalizedPath);
        std::string component;
        
        while (std::getline(ss, component, '/')) {
            if (!component.empty()) {
                components.push_back(component);
            }
        }
        
        // Start from root
        std::shared_ptr current = root;
        
        // Traverse path components
        for (const auto& comp : components) {
            // Find the responsible filesystem
            auto fs = getResponsibleFS(normalizedPath);
            if (!fs) return nullptr;
            
            // Get the child node
            auto child = fs->getChildNode(current, comp);
            if (!child) return nullptr;
            
            current = child;
        }
        
        return current;
    }

private:
    void createMountPoint(const std::string& path, const std::string& fsType) {
        auto fs = std::make_shared();
        fs->setFilesystemType(fsType);
        mount(path, fs);
    }

    void mountVirtualFilesystems() {
        // Mount virtual filesystems like procfs, sysfs, etc.
        auto procFS = std::make_shared();
        procFS->setFilesystemType("procfs");
        mount("/proc", procFS);
        
        auto sysFS = std::make_shared();
        sysFS->setFilesystemType("sysfs");
        mount("/sys", sysFS);
    }

    std::shared_ptr root;
    std::vector mountPoints;
    std::map> openFiles;
};

} // namespace Kernel
    