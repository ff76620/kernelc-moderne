#ifndef WIDGET_HPP
#define WIDGET_HPP

#include 
#include 
#include 

namespace UI {

class Canvas;

class Widget {
private:
    std::string id;
    std::string type;
    int x, y, width, height;
    bool visible;
    bool enabled;
    
    std::function onClick;
    std::function onFocus;
    std::function onBlur;

public:
    Widget(const std::string& type);
    virtual ~Widget();

    // Properties
    const std::string& getId() const;
    const std::string& getType() const;
    void setPosition(int x, int y);
    void setSize(int width, int height);
    void getBounds(int& x, int& y, int& width, int& height) const;

    // State
    void show();
    void hide();
    void enable();
    void disable();
    bool isVisible() const;
    bool isEnabled() const;

    // Event handlers
    void setOnClick(std::function handler);
    void setOnFocus(std::function handler);
    void setOnBlur(std::function handler);

    // Event processing
    virtual void handleClick();
    virtual void handleFocus();
    virtual void handleBlur();

    // Rendering
    virtual void render(Canvas* canvas);
    virtual void update();

    virtual void handleDrop(const struct DragData& data) {}
    virtual void handleDragComplete(Widget* target) {}
};

} // namespace UI

#endif // WIDGET_HPP