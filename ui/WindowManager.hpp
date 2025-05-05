#ifndef WINDOW_MANAGER_HPP
#define WINDOW_MANAGER_HPP

#include 
#include 
#include 
#include 
#include "../ui/DisplayServer.hpp"
#include "../ui/Window.hpp"
#include "../ui/Widget.hpp"

namespace UI {

class WindowManager {
private:
    static WindowManager* instance;
    std::unique_ptr displayServer;
    std::vector> windows;
    std::map> widgets;
    std::shared_ptr activeWindow;
    std::shared_ptr focusedWidget;
    bool isInitialized;

    WindowManager();

public:
    static WindowManager& getInstance();
    
    bool initialize();
    void shutdown();
    
    // Window Management
    std::shared_ptr createWindow(int x, int y, int width, int height);
    void destroyWindow(std::shared_ptr window);
    void focusWindow(std::shared_ptr window);
    std::shared_ptr getActiveWindow() const;
    const std::vector>& getWindows() const;
    
    // Widget Management
    std::shared_ptr createWidget(const std::string& type);
    void destroyWidget(const std::string& widgetId);
    std::shared_ptr getWidget(const std::string& widgetId) const;
    void setFocusedWidget(std::shared_ptr widget);
    std::shared_ptr getFocusedWidget() const;
    
    // Event Handling
    void processEvents();
    void handleResize(int width, int height);
    void handleMouseMove(int x, int y);
    void handleMouseButton(int button, bool pressed, int x, int y);
    void handleKeyEvent(int keycode, bool pressed);
    
    // Rendering
    void render();
    void updateScreen();
    DisplayServer* getDisplayServer() const;

    // Window ordering
    void bringToFront(std::shared_ptr window);
    void sendToBack(std::shared_ptr window);
    
    ~WindowManager();

    // Prevent copying
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
};

} // namespace UI

#endif // WINDOW_MANAGER_HPP