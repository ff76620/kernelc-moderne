#ifndef TREE_VIEW_HPP
#define TREE_VIEW_HPP

#include "../ui/Widget.hpp"
#include 
#include 
#include 

namespace UI {

class TreeNode {
public:
    std::string text;
    std::vector> children;
    bool expanded;
    
    TreeNode(const std::string& text) 
        : text(text), expanded(false) {}
};

class TreeView : public Widget {
private:
    std::vector> rootNodes;
    uint32_t textColor;
    uint32_t backgroundColor;
    uint32_t selectionColor;
    int selectedIndex;
    
public:
    TreeView() : Widget("treeview"), selectedIndex(-1) {
        setSize(200, 400);
        textColor = 0x000000;
        backgroundColor = 0xFFFFFF;
        selectionColor = 0xA0A0FF;
    }
    
    void addRootNode(const std::string& text) {
        rootNodes.push_back(std::make_shared(text));
    }
    
    void addChildNode(TreeNode* parent, const std::string& text) {
        if (parent) {
            parent->children.push_back(std::make_shared(text));
        }
    }
    
    virtual void render(Canvas* canvas) override {
        int x, y, width, height;
        getBounds(x, y, width, height);

        // Draw background
        canvas->drawRect(x, y, width, height, backgroundColor);

        // Initialize rendering position
        int currentY = y + 5;
        const int indentWidth = 20;
        const int nodeHeight = 20;

        // Helper function to render a node and its children recursively
        std::function&, int)> renderNode = 
            [&](const std::shared_ptr& node, int level) {
                if (!node) return;

                // Calculate node position
                int nodeX = x + (level * indentWidth) + 5;
                
                // Draw expansion indicator (+ or -)
                if (!node->children.empty()) {
                    std::string indicator = node->expanded ? "-" : "+";
                    canvas->drawText(nodeX - 15, currentY + 6, indicator, textColor);
                }

                // Draw node background if selected
                if (selectedIndex == currentY) {
                    canvas->drawRect(nodeX, currentY, width - nodeX + x - 5, nodeHeight, selectionColor);
                }

                // Draw node text
                canvas->drawText(nodeX, currentY + 6, node->text, textColor);
                
                currentY += nodeHeight;

                // Recursively render children if expanded
                if (node->expanded) {
                    for (const auto& child : node->children) {
                        renderNode(child, level + 1);
                    }
                }
            };

        // Render all root nodes
        for (const auto& node : rootNodes) {
            renderNode(node, 0);
        }
    }
};

} // namespace UI

#endif