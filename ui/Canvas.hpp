#ifndef CANVAS_HPP
#define CANVAS_HPP

#include 
#include 
#include 

namespace UI {

class Canvas {
private:
    int width, height;
    std::vector buffer;
    bool dirty;

public:
    Canvas(int width, int height) 
        : width(width), height(height), buffer(width * height), dirty(false) {}

    void clear(uint32_t color = 0x00000000) {
        std::fill(buffer.begin(), buffer.end(), color);
        dirty = true;
    }

    void drawPixel(int x, int y, uint32_t color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            buffer[y * width + x] = color;
            dirty = true;
        }
    }

    void drawRect(int x, int y, int w, int h, uint32_t color) {
        for (int i = x; i < x + w; i++) {
            for (int j = y; j < y + h; j++) {
                drawPixel(i, j, color);
            }
        }
    }

    void drawText(int x, int y, const std::string& text, uint32_t color) {
        // Font dimensions
        const int CHAR_WIDTH = 6;
        const int CHAR_HEIGHT = 8;
        
        int currentX = x;
        for (char c : text) {
            // Get font bitmap for character
            const uint8_t* charBitmap = getFontBitmap(c);
            
            // Render character pixel by pixel
            for (int dy = 0; dy < CHAR_HEIGHT; dy++) {
                for (int dx = 0; dx < CHAR_WIDTH; dx++) {
                    // Check if pixel should be drawn based on font bitmap
                    if (charBitmap[dy] & (1 << (CHAR_WIDTH-1-dx))) {
                        drawPixel(currentX + dx, y + dy, color);
                    }
                }
            }
            
            currentX += CHAR_WIDTH + 1; // Add 1 pixel spacing between chars
        }
    }

    const uint8_t* getFontBitmap(char c) {
        // Basic 6x8 pixel font bitmap data
        // Each char is represented by 8 bytes, each byte represents one row
        static const uint8_t fontData[][8] = {
            // Space
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
            
            // Letters A-Z
            {0x1C, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x00}, // A
            {0x3C, 0x22, 0x22, 0x3C, 0x22, 0x22, 0x3C, 0x00}, // B
            {0x1C, 0x22, 0x20, 0x20, 0x20, 0x22, 0x1C, 0x00}, // C
            // ... Add remaining letters
            
            // Numbers 0-9  
            {0x1C, 0x22, 0x26, 0x2A, 0x32, 0x22, 0x1C, 0x00}, // 0
            {0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00}, // 1
            // ... Add remaining numbers
            
            // Basic punctuation
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00}, // .
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x10}, // ,
            {0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00}, // :
            // ... Add other punctuation
        };

        // Map ASCII character to font bitmap index
        int index = 0;
        if (c == ' ') index = 0;
        else if (c >= 'A' && c <= 'Z') index = 1 + (c - 'A');
        else if (c >= '0' && c <= '9') index = 27 + (c - '0');
        else if (c == '.') index = 37;
        else if (c == ',') index = 38;
        else if (c == ':') index = 39;
        // Add more character mappings as needed
        
        return fontData[index];
    }

    bool isDirty() const { return dirty; }
    void clearDirty() { dirty = false; }
    
    const uint32_t* getBuffer() const { return buffer.data(); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    void drawChar(int x, int y, char c, uint32_t color) {
        const uint8_t* bitmap = getFontBitmap(c);
        
        // Draw character pixel by pixel based on bitmap
        for (int dy = 0; dy < 8; dy++) {
            for (int dx = 0; dx < 6; dx++) {
                if (bitmap[dy] & (1 << (5-dx))) {
                    drawPixel(x + dx, y + dy, color);
                }
            }
        }
    }
};

} // namespace UI

#endif // CANVAS_HPP