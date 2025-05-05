#pragma once
#include 
#include 
#include 
#include 

class BootScreen {
public:
  BootScreen(sf::RenderWindow& window);
  void update();
  void render();
  bool isComplete() const;

private:
  void initializeGraphics();
  void createHexagon(sf::ConvexShape& shape, float size, const sf::Color& color);
  void createTriangle(sf::ConvexShape& shape, const sf::Color& color);
  
  sf::RenderWindow& window_;
  sf::ConvexShape outerHexagon_;
  sf::ConvexShape innerHexagon_;
  sf::ConvexShape outerTriangle_;
  sf::ConvexShape innerTriangle_;
  sf::Text text_;
  sf::Font font_;
  
  float animationProgress_{0.0f};
  std::chrono::steady_clock::time_point startTime_;
  static constexpr float ANIMATION_DURATION{8.0f};
  
  sf::Shader glowShader_;
  sf::RenderTexture renderTexture_;
};
