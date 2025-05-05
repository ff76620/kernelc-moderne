#ifndef MENU_SYSTEM_HPP
#define MENU_SYSTEM_HPP

#include "../ui/Widget.hpp"
#include 
#include 
#include 

namespace UI {

struct MenuItem {
    std::string text;
    std::function action;
    std::vector subItems;
    bool enabled;
    
    MenuItem(const std::string& text) 
        : text(text), enabled(true) {}
};

class MenuBar : public Widget {
private:
    std::vector items;
    uint32_t backgroundColor;
    uint32_t textColor;
    uint32_t highlightColor;
    int activeMenu;
    
public:
    MenuBar() : Widget("menubar"), activeMenu(-1) {
        setSize(800, 25);
    }
    
    void addMenuItem(const MenuItem& item) {
        items.push_back(item);
    }
    
    virtual void render(Canvas* canvas) override {
        int x, y, width, height;
        getBounds(x, y, width, height);

        // Draw menu bar background
        canvas->drawRect(x, y, width, height, backgroundColor);

        // Draw menu items
        int itemX = x + 5;
        const int itemPadding = 20;
        const int itemHeight = height - 4;

        for (size_t i = 0; i < items.size(); i++) {
            const auto& item = items[i];
            
            // Calculate item width based on text
            int itemWidth = item.text.length() * 8 + itemPadding;
            
            // Draw item background if active
            if (static_cast(i) == activeMenu) {
                canvas->drawRect(itemX, y + 2, itemWidth, itemHeight, highlightColor);
            }
            
            // Draw item text
            int textY = y + (height - 8) / 2;
            canvas->drawText(itemX + itemPadding/2, textY, item.text, textColor);
            
            // Draw submenu indicator if has children
            if (!item.subItems.empty()) {
                canvas->drawText(itemX + itemWidth - 12, textY, "▼", textColor);
            }
            
            // Draw disabled state
            if (!item.enabled) {
                uint32_t disabledColor = 0x808080;
                for (int dy = 2; dy < itemHeight; dy += 2) {
                    for (int dx = 0; dx < itemWidth; dx += 2) {
                        canvas->drawPixel(itemX + dx, y + dy, disabledColor);
                    }
                }
            }
            
            itemX += itemWidth;
        }

        // Draw submenu if active
        if (activeMenu >= 0 && activeMenu < static_cast(items.size())) {
            const auto& activeItem = items[activeMenu];
            if (!activeItem.subItems.empty()) {
                int submenuX = x;
                int submenuY = y + height;
                int submenuWidth = 150;
                int submenuItemHeight = 25;
                int submenuHeight = activeItem.subItems.size() * submenuItemHeight;
                
                // Draw submenu background
                canvas->drawRect(submenuX, submenuY, submenuWidth, submenuHeight, backgroundColor);
                canvas->drawRect(submenuX, submenuY, submenuWidth, 1, textColor);
                canvas->drawRect(submenuX, submenuY, 1, submenuHeight, textColor);
                canvas->drawRect(submenuX + submenuWidth - 1, submenuY, 1, submenuHeight, textColor);
                canvas->drawRect(submenuX, submenuY + submenuHeight - 1, submenuWidth, 1, textColor);
                
                // Draw submenu items
                int itemY = submenuY;
                for (const auto& subItem : activeItem.subItems) {
                    if (subItem.enabled) {
                        canvas->drawText(submenuX + 10, itemY + 8, subItem.text, textColor);
                    } else {
                        canvas->drawText(submenuX + 10, itemY + 8, subItem.text, 0x808080);
                    }
                    itemY += submenuItemHeight;
                }
            }
        }
    }
};

} // namespace UI

#endif