#include "kernel/ui/WindowManager.hpp"
#include 

namespace UI {

WindowManager* WindowManager::instance = nullptr;

WindowManager::WindowManager() 
    : displayServer(nullptr)
    , activeWindow(nullptr)
    , focusedWidget(nullptr)
    , isInitialized(false) {
}

WindowManager& WindowManager::getInstance() {
    if (!instance) {
        instance = new WindowManager();
    }
    return *instance;
}

bool WindowManager::initialize() {
    if (!isInitialized) {
        displayServer = std::make_unique();
        isInitialized = displayServer->initialize();
    }
    return isInitialized;
}

void WindowManager::shutdown() {
    windows.clear();
    widgets.clear();
    if (displayServer) {
        displayServer->shutdown();
    }
    displayServer.reset();
    isInitialized = false;
}

std::shared_ptr WindowManager::createWindow(int x, int y, int width, int height) {
    auto window = std::make_shared(x, y, width, height);
    windows.push_back(window);
    return window;
}

void WindowManager::destroyWindow(std::shared_ptr window) {
    if (window == activeWindow) {
        activeWindow = nullptr;
    }
    
    auto it = std::find(windows.begin(), windows.end(), window);
    if (it != windows.end()) {
        windows.erase(it);
    }
}

void WindowManager::focusWindow(std::shared_ptr window) {
    if (activeWindow) {
        activeWindow->unfocus();
    }
    
    activeWindow = window;
    if (window) {
        window->focus();
        bringToFront(window);
    }
}

std::shared_ptr WindowManager::getActiveWindow() const {
    return activeWindow;
}

const std::vector>& WindowManager::getWindows() const {
    return windows;
}

std::shared_ptr WindowManager::createWidget(const std::string& type) {
    auto widget = std::make_shared(type);
    widgets[widget->getId()] = widget;
    return widget;
}

void WindowManager::destroyWidget(const std::string& widgetId) {
    if (widgets[widgetId] == focusedWidget) {
        focusedWidget = nullptr;
    }
    widgets.erase(widgetId);
}

std::shared_ptr WindowManager::getWidget(const std::string& widgetId) const {
    auto it = widgets.find(widgetId);
    return (it != widgets.end()) ? it->second : nullptr;
}

void WindowManager::setFocusedWidget(std::shared_ptr widget) {
    if (focusedWidget) {
        focusedWidget->handleBlur();
    }
    
    focusedWidget = widget;
    if (widget) {
        widget->handleFocus();
    }
}

std::shared_ptr WindowManager::getFocusedWidget() const {
    return focusedWidget;
}

void WindowManager::processEvents() {
    if (!isInitialized) return;
    
    displayServer->processEvents();
    
    Event event;
    while (displayServer->getEventQueue()->pop(event, false)) {
        switch (event.type) {
            case EventType::MouseMove:
                handleMouseMove(event.x, event.y);
                break;
            case EventType::MouseButton:
                handleMouseButton(event.button, true, event.x, event.y);
                break;
            case EventType::KeyPress:
            case EventType::KeyRelease:
                handleKeyEvent(event.keycode, event.type == EventType::KeyPress);
                break;
            case EventType::WindowResize:
                handleResize(event.x, event.y);
                break;
        }
    }

    // Process window-specific events
    for (auto& window : windows) {
        window->processEvents();
    }
}

void WindowManager::handleMouseMove(int x, int y) {
  // Handle mouse movement and hover detection
  for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
    auto window = *it;
    int wx, wy, ww, wh;
    window->getBounds(wx, wy, ww, wh);
    
    // Check if mouse is over window
    if (x >= wx && x < wx + ww && y >= wy && y < wy + wh) {
      // Transform coords to window space
      int localX = x - wx;
      int localY = y - wy;

      // Send hover event
      window->handleHover(localX, localY);
      
      // Send mouse move to window
      window->handleMouseMove(localX, localY); 
      
      // Only handle for topmost window
      break;
    }
  }
}

void WindowManager::handleMouseButton(int button, bool pressed, int x, int y) {
  // Handle mouse clicks and window focusing
  for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
    auto window = *it;
    int wx, wy, ww, wh;
    window->getBounds(wx, wy, ww, wh);
    
    if (x >= wx && x < wx + ww && y >= wy && y < wy + wh) {
      // Click is within window bounds
      
      if (pressed) {
        // Focus window on mouse down
        focusWindow(window);
        
        // Start possible drag
        if (button == MOUSE_LEFT) {
          window->startDrag(x - wx, y - wy);
        }
      } else {
        // Handle mouse up
        if (button == MOUSE_LEFT) {
          window->endDrag();
        }
      }
      
      // Forward click to window
      window->handleMouseButton(button, pressed, x - wx, y - wy);
      break;
    }
  }
}

void WindowManager::handleKeyEvent(int keycode, bool pressed) {
  // Forward key events to focused window
  if (activeWindow) {
    // Handle global hotkeys first
    if (handleGlobalHotkey(keycode, pressed)) {
      return;
    }
    
    // Forward to active window
    activeWindow->handleKeyEvent(keycode, pressed);
    
    // Forward to focused widget if any
    if (focusedWidget) {
      focusedWidget->handleKeyEvent(keycode, pressed);
    }
  }
}

void WindowManager::handleResize(int width, int height) {
  if (!isInitialized) return;
  
  // Update display server resolution
  displayServer->setResolution(width, height);
  
  // Calculate new work area
  int workX = 0;
  int workY = 0;
  int workWidth = width;
  int workHeight = height;
  
  // Notify all windows of resize
  for (auto& window : windows) {
    // Get current window bounds
    int wx, wy, ww, wh;
    window->getBounds(wx, wy, ww, wh);
    
    // Keep window within work area
    wx = std::max(workX, std::min(wx, workX + workWidth - ww));
    wy = std::max(workY, std::min(wy, workY + workHeight - wh));
    
    // Update window position
    window->setPosition(wx, wy);
    
    // Notify window of resize
    window->handleResize();
  }
}

void WindowManager::render() {
  if (!isInitialized) return;
  
  // Begin frame
  displayServer->beginFrame();
  
  // Setup render state
  setupRenderState();
  
  // Draw background
  renderBackground();
  
  // Render windows back to front
  for (auto& window : windows) {
    if (window->isVisible()) {
      // Draw window shadow
      renderWindowShadow(window);
      
      // Draw window frame
      window->renderFrame();
      
      // Draw window contents
      window->render();
      
      // Draw window decorations
      if (window == activeWindow) {
        renderActiveWindowDecorations(window);
      }
    }
  }
  
  // Draw overlay elements
  renderOverlays();
  
  // End frame
  displayServer->endFrame();
}

void WindowManager::updateScreen() {
    if (isInitialized) {
        displayServer->swapBuffers();
    }
}

DisplayServer* WindowManager::getDisplayServer() const {
    return displayServer.get();
}

void WindowManager::bringToFront(std::shared_ptr window) {
    auto it = std::find(windows.begin(), windows.end(), window);
    if (it != windows.end()) {
        windows.erase(it);
        windows.push_back(window);
    }
}

void WindowManager::sendToBack(std::shared_ptr window) {
    auto it = std::find(windows.begin(), windows.end(), window);
    if (it != windows.end()) {
        windows.erase(it);
        windows.insert(windows.begin(), window);
    }
}

WindowManager::~WindowManager() {
    shutdown();
    instance = nullptr;
}

} // namespace UI