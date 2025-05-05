#ifndef TAB_CONTROL_HPP
#define TAB_CONTROL_HPP

#include "../ui/Widget.hpp"
#include 
#include 

namespace UI {

class TabControl : public Widget {
private:
    struct Tab {
        std::string title;
        std::vector> widgets;
        bool active;
    };
    
    std::vector tabs;
    int activeTab;
    uint32_t backgroundColor;
    uint32_t textColor;
    uint32_t activeColor;
    
public:
    TabControl() : Widget("tabcontrol"), activeTab(0) {
        setSize(400, 300);
    }
    
    void addTab(const std::string& title) {
        tabs.push_back({title, {}, tabs.empty()});
    }
    
    void addWidgetToTab(int tabIndex, std::shared_ptr widget) {
        if (tabIndex >= 0 && tabIndex < tabs.size()) {
            tabs[tabIndex].widgets.push_back(widget);
        }
    }
    
    void setActiveTab(int index) {
        if (index >= 0 && index < tabs.size()) {
            tabs[activeTab].active = false;
            activeTab = index;
            tabs[activeTab].active = true;
        }
    }
    
    virtual void render(Canvas* canvas) override {
        int x, y, width, height;
        getBounds(x, y, width, height);

        // Draw tab background
        canvas->drawRect(x, y, width, height, backgroundColor);

        // Draw tab headers
        int tabX = x;
        int tabWidth = width / tabs.size();
        int tabHeight = 30;

        for (size_t i = 0; i < tabs.size(); i++) {
            // Draw tab header background
            uint32_t tabColor = (i == activeTab) ? activeColor : backgroundColor;
            canvas->drawRect(tabX, y, tabWidth, tabHeight, tabColor);

            // Draw tab border
            canvas->drawRect(tabX, y, tabWidth, 1, textColor);
            canvas->drawRect(tabX, y, 1, tabHeight, textColor);
            canvas->drawRect(tabX + tabWidth - 1, y, 1, tabHeight, textColor);

            // Draw tab title
            int textX = tabX + (tabWidth - tabs[i].title.length() * 8) / 2;
            int textY = y + (tabHeight - 8) / 2;
            canvas->drawText(textX, textY, tabs[i].title, textColor);

            tabX += tabWidth;
        }

        // Draw active tab content area
        if (activeTab >= 0 && activeTab < tabs.size()) {
            int contentY = y + tabHeight;
            int contentHeight = height - tabHeight;
            canvas->drawRect(x, contentY, width, contentHeight, backgroundColor);

            // Draw widgets in active tab
            for (auto& widget : tabs[activeTab].widgets) {
                widget->render(canvas);
            }
        }
    }
};

} // namespace UI

#endif // TAB_CONTROL_HPP