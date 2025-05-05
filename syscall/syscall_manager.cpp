#include "kernel/syscall/syscall_manager.hpp"
#include 
#include 

class SystemCallManager {
private:
    using SyscallHandler = std::function&)>;
    std::unordered_map syscallTable;
    std::atomic syscallCounter{0};

public:
    SystemCallManager() {
        initializeSyscallTable();
    }

    int64_t handleSyscall(uint32_t syscallNumber, const std::vector& params) {
        try {
            auto it = syscallTable.find(syscallNumber);
            if (it == syscallTable.end()) {
                throw std::runtime_error("Invalid syscall number");
            }

            syscallCounter++;
            return it->second(params);
        } catch (const std::exception& e) {
            logError(syscallNumber, e.what());
            return -1;
        }
    }

    bool registerSyscall(uint32_t number, SyscallHandler handler) {
        return syscallTable.emplace(number, std::move(handler)).second;
    }

    uint64_t getSyscallCount() const {
        return syscallCounter.load();
    }

private:
    void initializeSyscallTable() {
        registerSyscall(0x01, [this](const auto& params) { 
            return handleFileOpen(params); 
        });
        
        registerSyscall(0x02, [this](const auto& params) { 
            return handleProcessCreate(params); 
        });
        
        registerSyscall(0x03, [this](const auto& params) { 
            return handleMemoryAlloc(params); 
        });
        
        registerSyscall(0x04, [this](const auto& params) { 
            return handleNetworkIO(params); 
        });
    }

    int64_t handleFileOpen(const std::vector& params) {
        if (params.size() < 2) return -1;
        
        const char* path = reinterpret_cast(params[0]);
        uint32_t mode = static_cast(params[1]);
        
        if (!path || strlen(path) == 0) {
            return -EINVAL;
        }

        if (!SecurityManager::getInstance().checkAccess(path, getCurrentProcessId(), mode)) {
            return -EACCES; 
        }

        int flags = 0;
        if (mode & FILE_READ) flags |= O_RDONLY;
        if (mode & FILE_WRITE) flags |= O_WRONLY;
        if (mode & FILE_APPEND) flags |= O_APPEND;
        if (mode & FILE_CREATE) flags |= O_CREAT;

        int fd = open(path, flags, 0644);
        if (fd < 0) {
            return -errno;
        }

        if (!FileManager::getInstance().registerFd(getCurrentProcessId(), fd)) {
            close(fd);
            return -EMFILE;
        }

        return fd;
    }

    int64_t handleProcessCreate(const std::vector& params) {
        if (params.empty()) return -1;

        const ProcessCreationParams* createParams = 
            reinterpret_cast(params[0]);

        if (!createParams) {
            return -EINVAL;
        }

        if (!createParams->path || strlen(createParams->path) == 0) {
            return -EINVAL;
        }

        if (!SecurityManager::getInstance().checkAccess(
                createParams->path, 
                getCurrentProcessId(),
                EXEC_PERMISSION)) {
            return -EACCES;
        }

        ProcessAttributes attrs;
        attrs.name = createParams->name;
        attrs.priority = createParams->priority;
        attrs.parentId = getCurrentProcessId();

        ProcessID newPid = ProcessManager::getInstance().createProcess(attrs);
        if (newPid == INVALID_PROCESS_ID) {
            return -EAGAIN;
        }

        if (!ProcessLoader::getInstance().loadExecutable(newPid, createParams->path)) {
            ProcessManager::getInstance().terminateProcess(newPid);
            return -ENOEXEC;
        }

        if (!ProcessManager::getInstance().initializeProcess(newPid)) {
            ProcessManager::getInstance().terminateProcess(newPid);
            return -ENOMEM;
        }

        return newPid;
    }

    int64_t handleMemoryAlloc(const std::vector& params) {
        if (params.empty()) return -1;
        
        size_t size = static_cast(params[0]);
        
        if (size == 0 || size > MAX_ALLOCATION_SIZE) {
            return reinterpret_cast(nullptr);
        }

        ProcessID pid = getCurrentProcessId();
        if (!ProcessManager::getInstance().checkMemoryLimit(pid, size)) {
            return reinterpret_cast(nullptr);
        }

        void* ptr = MemoryManager::getInstance().allocateMemory(
            size,
            MemoryFlags::USER_ACCESSIBLE
        );

        if (!ptr) {
            return reinterpret_cast(nullptr);
        }

        if (!ProcessManager::getInstance().registerMemoryAllocation(pid, ptr, size)) {
            MemoryManager::getInstance().freeMemory(ptr);
            return reinterpret_cast(nullptr);
        }

        return reinterpret_cast(ptr);
    }

    int64_t handleNetworkIO(const std::vector& params) {
        if (params.size() < 3) return -1;

        int sockfd = static_cast(params[0]);
        void* buffer = reinterpret_cast(params[1]); 
        size_t length = static_cast(params[2]);

        if (!buffer || length == 0) {
            return -EINVAL;
        }

        ProcessID pid = getCurrentProcessId();
        if (!NetworkManager::getInstance().validateSocket(pid, sockfd)) {
            return -EBADF;
        }

        if (!ProcessManager::getInstance().checkMemoryAccess(
                pid, buffer, length, MemoryAccess::READ_WRITE)) {
            return -EFAULT;
        }

        if (!SecurityManager::getInstance().checkNetworkAccess(pid)) {
            return -EACCES;
        }

        ssize_t result = NetworkManager::getInstance().performIO(
            sockfd,
            buffer,
            length,
            NetworkIOFlags::NONBLOCK
        );

        if (result < 0) {
            return -errno;
        }

        return result;
    }
};
    