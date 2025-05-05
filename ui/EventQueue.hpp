#ifndef EVENT_QUEUE_HPP
#define EVENT_QUEUE_HPP

#include 
#include 
#include 

namespace UI {

enum class EventType {
    MouseMove,
    MouseButton,
    KeyPress,
    KeyRelease,
    WindowResize,
    WindowClose,
    Focus,
    Blur
};

struct Event {
    EventType type;
    int x;
    int y;
    int button;
    int keycode;
    void* windowHandle;
    
    Event(EventType t) : type(t), x(0), y(0), button(0), keycode(0), windowHandle(nullptr) {}
};

class EventQueue {
private:
    std::queue events;
    mutable std::mutex mutex;
    std::condition_variable condition;
    bool shutdown;

public:
    EventQueue() : shutdown(false) {}
    
    void push(const Event& event) {
        std::lock_guard lock(mutex);
        events.push(event);
        condition.notify_one();
    }
    
    bool pop(Event& event, bool wait = true) {
        std::unique_lock lock(mutex);
        
        if (wait) {
            condition.wait(lock, [this] { 
                return !events.empty() || shutdown; 
            });
        }
        
        if (events.empty() || shutdown) {
            return false;
        }
        
        event = events.front();
        events.pop();
        return true;
    }
    
    void clear() {
        std::lock_guard lock(mutex);
        std::queue empty;
        std::swap(events, empty);
    }
    
    void stop() {
        std::lock_guard lock(mutex);
        shutdown = true;
        condition.notify_all();
    }
    
    bool isEmpty() const {
        std::lock_guard lock(mutex);
        return events.empty();
    }
};

} // namespace UI

#endif // EVENT_QUEUE_HPP