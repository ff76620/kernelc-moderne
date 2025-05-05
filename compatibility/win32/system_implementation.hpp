
        #pragma once
        #include 
        #include "../core/kernel.hpp"
        #include "../process/ProcessManager.hpp"

        namespace Win32Compat {
        class SystemImplementation {
        private:
            ProcessManager* processManager;
            MemoryManager* memoryManager;
            
        public:
            // Win32 API Layer
            HANDLE CreateProcessW(
                const ProcessCreationParams& params
            ) {
                return processManager->createProcess(params);
            }

            // NT Kernel Services  
            NTSTATUS NtCreateProcess(
                const NTProcessParams& params
            ) {
                return processManager->createNTProcess(params);
            }

            // Process Management
            HANDLE CreateThread(
                const ThreadParams& params
            ) {
                return processManager->createThread(params);
            }

            // Memory Management
            LPVOID VirtualAlloc(
                const MemoryParams& params
            ) {
                return memoryManager->allocateMemory(params);
            }
        };
        }