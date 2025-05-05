#ifndef TEXTBOX_HPP
#define TEXTBOX_HPP

#include "../ui/Widget.hpp"
#include 

namespace UI {

class TextBox : public Widget {
private:
    std::string text;
    size_t cursorPos;
    uint32_t backgroundColor;
    uint32_t textColor;
    uint32_t borderColor;
    bool focused;

public:
    TextBox() 
        : Widget("textbox")
        , text("")
        , cursorPos(0)
        , backgroundColor(0xFFFFFF)
        , textColor(0x000000)
        , borderColor(0x666666)
        , focused(false) {
        setSize(200, 30); // Default size
    }

    void setText(const std::string& newText) {
        text = newText;
        cursorPos = text.length();
    }

    const std::string& getText() const {
        return text;
    }

    virtual void render(Canvas* canvas) override {
        int x, y, width, height;
        getBounds(x, y, width, height);
        
        // Draw background
        canvas->drawRect(x, y, width, height, backgroundColor);
        
        // Draw border
        canvas->drawRect(x, y, width, 1, borderColor);
        canvas->drawRect(x, y, 1, height, borderColor);
        canvas->drawRect(x + width - 1, y, 1, height, borderColor);
        canvas->drawRect(x, y + height - 1, width, 1, borderColor);
        
        // Draw text
        if (!text.empty()) {
            canvas->drawText(x + 5, y + (height - 8) / 2, text, textColor);
        }
        
        // Draw cursor if focused
        if (focused) {
            int cursorX = x + 5 + cursorPos * 8;
            canvas->drawRect(cursorX, y + 5, 1, height - 10, textColor);
        }
    }

    virtual void handleFocus() override {
        focused = true;
        Widget::handleFocus();
    }

    virtual void handleBlur() override {
        focused = false;
        Widget::handleBlur();
    }
};

} // namespace UI

#endif // TEXTBOX_HPP