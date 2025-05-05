// Virtual Memory Manager Header
#ifndef VMM_HPP
#define VMM_HPP

#include 
#include 
#include 

namespace kernel::memory {

class VirtualMemoryManager {
public:
    static VirtualMemoryManager& getInstance();
    
    void initialize();
    void* allocatePages(size_t count);
    void freePages(void* address, size_t count);
    void mapPage(void* virtualAddr, void* physicalAddr);
    void unmapPage(void* virtualAddr);

private:
    VirtualMemoryManager() = default;
    void setupPageTables();
    
    std::vector<uint64_t> pageDirectory;
    std::vector<uint64_t> pageFrames;
};

} // namespace kernel::memory

#endif // VMM_HPP