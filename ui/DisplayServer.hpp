#ifndef DISPLAY_SERVER_HPP
#define DISPLAY_SERVER_HPP

#include 
#include 

namespace UI {

class RenderContext;
class EventQueue;

class DisplayServer {
private:
    std::unique_ptr renderContext;
    std::unique_ptr eventQueue;
    int screenWidth;
    int screenHeight;
    bool vsyncEnabled;

public:
    DisplayServer();
    ~DisplayServer();

    // Initialization
    bool initialize();
    void shutdown();

    // Display properties
    void setResolution(int width, int height);
    void getResolution(int& width, int& height) const;
    void setVSync(bool enabled);
    bool isVSyncEnabled() const;

    // Frame management
    void beginFrame();
    void endFrame();
    void swapBuffers();

    // Event handling
    void processEvents();
    void flushEvents();

    // Screen management
    void resize(int width, int height);
    void refreshScreen();
    
    // Rendering context
    RenderContext* getRenderContext();

    // Device capabilities
    bool supportsHardwareAcceleration() const;
    bool supportsMultipleDisplays() const;
};

} // namespace UI

#endif // DISPLAY_SERVER_HPP