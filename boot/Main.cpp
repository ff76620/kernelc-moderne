#include 
#include "kernel/boot/bootloader.hpp"
#include "kernel/boot/bootscreen.hpp"
#include 

int main() {
  Kernel::Bootloader& bootLoader = Kernel::Bootloader::getInstance();
  
  sf::RenderWindow window(sf::VideoMode(800, 600), "Ancestral");
  window.setVerticalSyncEnabled(true);
  
  BootScreen bootScreen(window);
  
  if (!bootLoader.initialize()) {
    std::cerr << "Boot process failed" << std::endl;
    return 1;
  }
  
  while (window.isOpen() && !bootLoader.isBootComplete()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }
    
    bootScreen.update();
    bootScreen.render();
  }
  
  if (bootLoader.isBootComplete()) {
    bootLoader.startKernel();
  }
  
  return 0;
}