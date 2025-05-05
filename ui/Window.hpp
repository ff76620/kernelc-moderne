#ifndef WINDOW_HPP
#define WINDOW_HPP

#include 
#include 
#include 
#include 

namespace UI {

class Widget;
class Canvas;

class Window {
private:
    int x, y, width, height;
    std::string title;
    bool visible;
    bool focused;
    std::vector> widgets;
    std::unique_ptr