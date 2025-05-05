#ifndef SIGNAL_MANAGER_HPP
#define SIGNAL_MANAGER_HPP

#include 
#include 
#include "../process/process.hpp"

namespace Kernel {

enum class Signal {
    SIGTERM,
    SIGKILL,
    SIGINT,
    SIGUSR1,
    SIGUSR2,
    SIGCHLD
};

class SignalManager {
public:
    static SignalManager& getInstance();

    void registerHandler(Signal sig, std::function handler);
    void unregisterHandler(Signal sig);
    void sendSignal(pid_t pid, Signal sig);
    void handlePendingSignals();

private:
    SignalManager() = default;
    ~SignalManager() = default;

    struct SignalInfo {
        Signal signal;
        pid_t sender;
        pid_t receiver;
    };

    std::map> handlers;
    std::vector pendingSignals;
};

} // namespace Kernel

#endif