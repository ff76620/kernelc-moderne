#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "../ui/Widget.hpp"

namespace UI {

class Button : public Widget {
private:
    std::string label;
    uint32_t backgroundColor;
    uint32_t textColor;
    bool pressed;

public:
    Button(const std::string& label) 
        : Widget("button")
        , label(label)
        , backgroundColor(0x333333)
        , textColor(0xFFFFFF)
        , pressed(false) {
        setSize(100, 30); // Default size
    }

    void setLabel(const std::string& newLabel) {
        label = newLabel;
    }

    void setColors(uint32_t bg, uint32_t text) {
        backgroundColor = bg;
        textColor = text;
    }

    virtual void render(Canvas* canvas) override {
        int x, y, width, height;
        getBounds(x, y, width, height);
        
        uint32_t currentBg = pressed ? 0x444444 : backgroundColor;
        
        // Draw button background
        canvas->drawRect(x, y, width, height, currentBg);
        
        // Draw button label
        int textX = x + (width - label.length() * 8) / 2;
        int textY = y + (height - 8) / 2;
        canvas->drawText(textX, textY, label, textColor);
    }

    virtual void handleClick() override {
        pressed = true;
        Widget::handleClick();
        pressed = false;
    }
};

} // namespace UI

#endif // BUTTON_HPP