
            class GameProcess {
            public:
                HANDLE processHandle;
                std::string name;
                int priority;
                std::vector threads;
                
                void SetPriority(int newPriority) {
                    SetPriorityClass(processHandle, newPriority);
                    priority = newPriority;
                }
                
                void OptimizeThreads() {
                    for(auto thread : threads) {
                        SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
                    }
                }
            };

            class GameAwareScheduler {
            private:
                std::vector activeGames;
                std::unordered_set knownGameExecutables;
                std::map gameProfiles;
                ProcessPriorityManager priorityMgr;
                LatencyOptimizer latencyOpt; 
                CoreAffinity coreAffinity;
                std::string gameThreadPriority;
                int ioLatencyTarget;
                std::vector memoryPool;

                struct GameProfile {
                    std::string gameName;
                    int basePriority;
                    size_t reservedMemory;
                    bool useGPUBoost;
                    float cpuAllocation;
                    std::vector requiredServices;
                };

                bool isGameProcess(const std::string& processName) {
                    return knownGameExecutables.find(processName) != knownGameExecutables.end();
                }

            public:
                GameAwareScheduler() : gameThreadPriority("HIGH"), ioLatencyTarget(1) {
                    knownGameExecutables.insert("TS4_x64.exe");
                    knownGameExecutables.insert("Sims4.exe");
                    knownGameExecutables.insert("GTA5.exe");
                    knownGameExecutables.insert("RDR2.exe");
                    knownGameExecutables.insert("Cyberpunk2077.exe");
                    knownGameExecutables.insert("VALORANT.exe");
                    knownGameExecutables.insert("csgo.exe");
                    knownGameExecutables.insert("Overwatch.exe");
                    knownGameExecutables.insert("League of Legends.exe");
                    knownGameExecutables.insert("FortniteLauncher.exe");
                    
                    initScheduler();
                }

                void ScanForGames() {
                    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                    if (snapshot != INVALID_HANDLE_VALUE) {
                        PROCESSENTRY32 processEntry;
                        processEntry.dwSize = sizeof(processEntry);
                        
                        if (Process32First(snapshot, &processEntry)) {
                            do {
                                std::string processName = processEntry.szExeFile;
                                if (isGameProcess(processName)) {
                                    GameProcess game;
                                    game.name = processName;
                                    game.processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processEntry.th32ProcessID);
                                    activeGames.push_back(game);
                                }
                            } while (Process32Next(snapshot, &processEntry));
                        }
                        CloseHandle(snapshot);
                    }
                }

                void OptimizeForGaming() {
                    for(auto& game : activeGames) {
                        game.SetPriority(HIGH_PRIORITY_CLASS);
                        game.OptimizeThreads();
                        
                        HANDLE threadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
                        if (threadSnapshot != INVALID_HANDLE_VALUE) {
                            THREADENTRY32 threadEntry;
                            threadEntry.dwSize = sizeof(threadEntry);
                            
                            if (Thread32First(threadSnapshot, &threadEntry)) {
                                do {
                                    HANDLE threadHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry.th32ThreadID);
                                    if (threadHandle) {
                                        SetThreadPriority(threadHandle, THREAD_PRIORITY_HIGHEST);
                                        
                                        SYSTEM_INFO sysInfo;
                                        GetSystemInfo(&sysInfo);
                                        
                                        DWORD_PTR affinityMask = 0;
                                        for(DWORD i = 0; i < sysInfo.dwNumberOfProcessors/2; i++) {
                                            affinityMask |= (1ULL << i);
                                        }
                                        
                                        SetThreadAffinityMask(threadHandle, affinityMask);
                                        
                                        SetThreadPriorityBoost(threadHandle, FALSE);
                                        
                                        SetThreadIdealProcessor(threadHandle, MAXIMUM_PROCESSORS);
                                        
                                        game.threads.push_back(threadHandle);
                                    }
                                } while (Thread32Next(threadSnapshot, &threadEntry));
                            }
                            CloseHandle(threadSnapshot);
                        }
                        
                        DWORD_PTR processAffinityMask = 0;
                        DWORD_PTR systemAffinityMask = 0;
                        if(GetProcessAffinityMask(game.processHandle, &processAffinityMask, &systemAffinityMask)) {
                            SetProcessAffinityMask(game.processHandle, processAffinityMask);
                        }
                        
                        SetProcessPriorityBoost(game.processHandle, FALSE);
                        SetPriorityClass(game.processHandle, HIGH_PRIORITY_CLASS);
                        
                        game.OptimizeThreads();
                    }
                }

                void MonitorPerformance() {
                    for(const auto& game : activeGames) {
                        PROCESS_MEMORY_COUNTERS_EX pmc;
                        if (GetProcessMemoryInfo(game.processHandle, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
                            if (pmc.WorkingSetSize > 8ULL * 1024 * 1024 * 1024) { 
                                EmptyWorkingSet(game.processHandle);
                                
                                SetProcessWorkingSetSize(game.processHandle, 
                                    (SIZE_T)-1,  
                                    (SIZE_T)-1   
                                );
                                
                                SYSTEM_INFO sysInfo;
                                GetSystemInfo(&sysInfo);
                                
                                SIZE_T minWorkingSetSize = 4ULL * 1024 * 1024 * 1024; 
                                SIZE_T maxWorkingSetSize = 12ULL * 1024 * 1024 * 1024; 
                                
                                SetProcessWorkingSetSizeEx(game.processHandle,
                                    minWorkingSetSize,
                                    maxWorkingSetSize,
                                    QUOTA_LIMITS_HARDWS_MIN_ENABLE |
                                    QUOTA_LIMITS_HARDWS_MAX_DISABLE
                                );
                            }
                            
                            FILETIME createTime, exitTime, kernelTime, userTime;
                            if(GetProcessTimes(game.processHandle, &createTime, &exitTime, &kernelTime, &userTime)) {
                                ULARGE_INTEGER kernelTimeValue, userTimeValue;
                                kernelTimeValue.LowPart = kernelTime.dwLowDateTime;
                                kernelTimeValue.HighPart = kernelTime.dwHighDateTime;
                                userTimeValue.LowPart = userTime.dwLowDateTime;
                                userTimeValue.HighPart = userTime.dwHighDateTime;
                                
                                double cpuUsage = (kernelTimeValue.QuadPart + userTimeValue.QuadPart) / 10000000.0;
                                
                                if(cpuUsage > 80.0) { 
                                    game.SetPriority(HIGH_PRIORITY_CLASS);
                                } else {
                                    game.SetPriority(ABOVE_NORMAL_PRIORITY_CLASS);
                                }
                            }
                            
                            IO_COUNTERS ioCounters;
                            if(GetProcessIoCounters(game.processHandle, &ioCounters)) {
                                if(ioCounters.ReadOperationCount > 1000 || 
                                   ioCounters.WriteOperationCount > 1000) {
                                    SetPriorityClass(game.processHandle, 
                                        HIGH_PRIORITY_CLASS | PROCESS_MODE_BACKGROUND_BEGIN);
                                        
                                    HANDLE hToken;
                                    if(OpenProcessToken(game.processHandle, TOKEN_ADJUST_PRIVILEGES, &hToken)) {
                                        SetProcessIoPriority(game.processHandle, IO_PRIORITY_HIGH);
                                        
                                        SetSystemFileCacheSize(
                                            -1,  
                                            -1,  
                                            0    
                                        );
                                        
                                        SetFileSystemOptimizations(
                                            game.processHandle,
                                            FILE_SYSTEM_ENABLE_PREFETCHER |
                                            FILE_SYSTEM_ENABLE_SUPERFETCH |
                                            FILE_SYSTEM_ENABLE_CACHE_MANAGER
                                        );
                                        
                                        CloseHandle(hToken);
                                    }
                                    
                                    if(ioCounters.ReadTransferCount > 100 * 1024 * 1024) { 
                                        SetFileSystemReadAheadBuffering(
                                            game.processHandle,
                                            READ_AHEAD_BUFFER_LARGE
                                        );
                                    }
                                    
                                    OVERLAPPED_IO_CONFIG ioConfig = {};
                                    ioConfig.EnableAsyncIO = TRUE;
                                    ioConfig.ConcurrentRequestCount = 8;
                                    ioConfig.BufferAlignment = 4096;
                                    SetProcessIoConfiguration(game.processHandle, &ioConfig);
                                }
                            }
                        }
                    }
                }

                void setGamingMode(bool enabled) {
                    if(enabled) {
                        applyGameOptimizations();
                    }
                }

                void adjustPriorities() {
                    setGameThreadPriority();
                    priorityMgr.updatePriorities();
                }

                void optimizeLatency() {
                    latencyOpt.setTarget(ioLatencyTarget);
                    latencyOpt.optimize();
                }

            private:
                GameProfile createDefaultProfile(const std::string& gameName) {
                    GameProfile profile;
                    profile.gameName = gameName;
                    profile.basePriority = 8; // High priority
                    profile.reservedMemory = 4ULL * 1024 * 1024 * 1024; // 4GB default reservation
                    profile.useGPUBoost = true;
                    profile.cpuAllocation = 0.75f; // 75% CPU allocation
                    
                    // Add required services
                    profile.requiredServices = {
                        "AudioEndpointBuilder",
                        "Audiosrv", 
                        "DXGKrnl",
                        "GoogleChromeElevationService",
                        "DbxSvc",
                        "nvagent",
                        "Origin Web Helper Service",
                        "Steam Client Service"
                    };
                    
                    return profile;
                }

                void optimizeForGame(const std::string& gameName) {
                    GameProfile profile;
                    
                    // Look up existing profile or create default
                    auto it = gameProfiles.find(gameName);
                    if (it != gameProfiles.end()) {
                        profile = it->second;
                    } else {
                        profile = createDefaultProfile(gameName);
                        gameProfiles[gameName] = profile;
                    }

                    // Apply the optimizations
                    applyGameOptimizations(profile);
                }

                void initScheduler() {
                    // Set system-wide gaming optimizations
                    SetSystemPowerProfile("High Performance");
                    SetWin32PriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
                    
                    // Create default profiles for known games
                    for (const auto& game : knownGameExecutables) {
                        if (gameProfiles.find(game) == gameProfiles.end()) {
                            gameProfiles[game] = createDefaultProfile(game);
                        }
                    }
                    
                    // Start initial performance monitoring
                    std::thread perfThread(&GameAwareScheduler::monitorPerformance, this);
                    perfThread.detach();
                }

                void applyGameOptimizations(const GameProfile& profile) {
                    // Set process priority based on profile
                    setProcessPriority(profile.basePriority);
                    
                    // Reserve memory
                    reserveMemory(profile.reservedMemory);
                    
                    // Enable GPU optimizations if specified
                    if (profile.useGPUBoost) {
                        enableGPUBoost();
                    }
                    
                    // Allocate CPU resources
                    allocateCPUResources(profile.cpuAllocation);
                    
                    // Start required services
                    startRequiredServices(profile.requiredServices);
                    
                    // Apply Windows gaming optimizations
                    DWORD gameMode = PROCESS_POWER_THROTTLING_IGNORE_SYSTEM_THROTTLING;
                    SetProcessInformation(
                        GetCurrentProcess(),
                        ProcessPowerThrottling,
                        &gameMode,
                        sizeof(gameMode)
                    );
                    
                    // Configure process for gaming
                    SetProcessWorkingSetSize(
                        GetCurrentProcess(),
                        profile.reservedMemory,  // Minimum working set
                        profile.reservedMemory * 2 // Maximum working set
                    );
                    
                    SetProcessPriorityBoost(GetCurrentProcess(), TRUE);
                    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
                }

                void setProcessPriority(int priority) {
                    HANDLE process = GetCurrentProcess();
                    DWORD priorityClass;
                        
                    switch(priority) {
                        case 10:
                            priorityClass = REALTIME_PRIORITY_CLASS;
                            break;
                        case 8:
                            priorityClass = HIGH_PRIORITY_CLASS;
                            break; 
                        case 6:
                            priorityClass = ABOVE_NORMAL_PRIORITY_CLASS;
                            break;
                        case 4:
                            priorityClass = NORMAL_PRIORITY_CLASS;
                            break;
                        default:
                            priorityClass = NORMAL_PRIORITY_CLASS;
                    }
                    
                    SetPriorityClass(process, priorityClass);
                    
                    // Set I/O priority
                    HANDLE hToken;
                    if(OpenProcessToken(process, TOKEN_ADJUST_PRIVILEGES, &hToken)) {
                        SetProcessIoPriority(process, priority >= 8 ? 
                            IO_PRIORITY_HIGH : IO_PRIORITY_NORMAL);
                        CloseHandle(hToken);
                    }
                }

                void reserveMemory(size_t amount) {
                    // Reserve and commit memory 
                    void* reserved = VirtualAlloc(NULL, amount,
                        MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES,
                        PAGE_READWRITE);

                    if (reserved) {
                        // Add to tracked allocations
                        memoryPool.push_back(reserved);
                        
                        // Lock pages in physical memory
                        VirtualLock(reserved, amount);
                        
                        // Configure large pages if amount is over 2MB
                        if (amount >= (2ULL * 1024 * 1024)) {
                            HANDLE token;
                            if (OpenProcessToken(GetCurrentProcess(), 
                                TOKEN_ADJUST_PRIVILEGES, &token)) {
                                EnableLargePages(token);
                                CloseHandle(token); 
                            }
                        }
                        
                        // Set working set size
                        SetProcessWorkingSetSize(
                            GetCurrentProcess(),
                            amount,     // Minimum
                            amount * 2  // Maximum
                        );
                    }
                }

                std::vector GetGameThreads() {
                    std::vector gameThreads;
                    DWORD currentPid = GetCurrentProcessId();
                    
                    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
                    if (snapshot != INVALID_HANDLE_VALUE) {
                        THREADENTRY32 threadEntry;
                        threadEntry.dwSize = sizeof(THREADENTRY32);
                        
                        if (Thread32First(snapshot, &threadEntry)) {
                            do {
                                if (threadEntry.th32OwnerProcessID == currentPid) {
                                    HANDLE thread = OpenThread(
                                        THREAD_ALL_ACCESS, 
                                        FALSE,
                                        threadEntry.th32ThreadID
                                    );
                                    
                                    if (thread) {
                                        // Get thread processor affinity
                                        DWORD_PTR affinity = 0;
                                        GetThreadIdealProcessorEx(thread, (PPROCESSOR_NUMBER)&affinity);
                                        
                                        // Only include threads on game-allocated cores
                                        if (affinity < coreAffinity.getGameCoreCount()) {
                                            gameThreads.push_back(thread);
                                        }
                                    }
                                }
                            } while (Thread32Next(snapshot, &threadEntry));
                        }
                        CloseHandle(snapshot);
                    }
                    
                    return gameThreads;
                }

                void EnableLargePages(HANDLE token) {
                    TOKEN_PRIVILEGES tp;
                    LUID luid;

                    if (LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &luid)) {
                        tp.PrivilegeCount = 1;
                        tp.Privileges[0].Luid = luid;
                        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                        AdjustTokenPrivileges(
                            token,
                            FALSE,
                            &tp,
                            sizeof(TOKEN_PRIVILEGES),
                            NULL,
                            NULL
                        );
                    }
                }
            };
        