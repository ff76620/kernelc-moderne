#ifndef DRAG_DROP_MANAGER_HPP
#define DRAG_DROP_MANAGER_HPP

#include 
#include "../ui/Widget.hpp"

namespace UI {

class DragDropManager {
private:
    Widget* dragSource;
    Widget* dropTarget;
    bool isDragging;
    int dragX, dragY;
    
public:
    DragDropManager() : dragSource(nullptr), dropTarget(nullptr), isDragging(false) {}
    
    void startDrag(Widget* source, int x, int y) {
        dragSource = source;
        dragX = x;
        dragY = y;
        isDragging = true;
    }
    
    void updateDrag(int x, int y) {
        if (isDragging) {
            dragX = x;
            dragY = y;
        }
    }
    
    void endDrag() {
        if (isDragging && dropTarget) {
            // Notify source and target widgets about the drag & drop operation
            if (dragSource && dropTarget) {
                // Get drag source bounds
                int srcX, srcY, srcWidth, srcHeight;
                dragSource->getBounds(srcX, srcY, srcWidth, srcHeight);

                // Get drop target bounds  
                int targetX, targetY, targetWidth, targetHeight;
                dropTarget->getBounds(targetX, targetY, targetWidth, targetHeight);

                // Create drag data structure
                struct DragData {
                    int sourceX, sourceY;
                    int dragX, dragY; 
                    Widget* source;
                };

                DragData dragData = {
                    srcX, srcY,
                    dragX, dragY,
                    dragSource
                };

                // Notify drop target about the incoming drop
                dropTarget->handleDrop(dragData);

                // Notify drag source that drag completed successfully
                dragSource->handleDragComplete(dropTarget);

                // Reset drag source position if needed
                dragSource->setPosition(dragX - srcWidth/2, dragY - srcHeight/2);
            }
        }
        isDragging = false;
        dragSource = nullptr;
        dropTarget = nullptr;
    }
};

} // namespace UI

#endif