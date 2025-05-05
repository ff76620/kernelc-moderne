#include "kernel/boot/bootscreen.hpp"

BootScreen::BootScreen(sf::RenderWindow& window) : window_(window) {
  startTime_ = std::chrono::steady_clock::now();
  initializeGraphics();
  
  if (!glowShader_.loadFromMemory(R"(
    uniform sampler2D texture;
    uniform vec2 blur_radius;
    void main() {
      vec2 pos = gl_TexCoord[0].xy;
      vec4 color = vec4(0.0);
      color += texture2D(texture, pos) * 0.4;
      color += texture2D(texture, pos + blur_radius) * 0.15;
      color += texture2D(texture, pos - blur_radius) * 0.15;
      color += texture2D(texture, pos + vec2(blur_radius.x, -blur_radius.y)) * 0.15;
      color += texture2D(texture, pos - vec2(blur_radius.x, -blur_radius.y)) * 0.15;
      gl_FragColor = color;
    })", sf::Shader::Fragment)) {
    throw std::runtime_error("Failed to load glow shader");
  }
  
  renderTexture_.create(window.getSize().x, window.getSize().y);
}

void BootScreen::initializeGraphics() {
  if (!font_.loadFromFile("arial.ttf")) {
    throw std::runtime_error("Failed to load font");
  }
  
  createHexagon(outerHexagon_, 300.f, sf::Color(0, 102, 204));
  createHexagon(innerHexagon_, 250.f, sf::Color(0, 153, 255));
  createTriangle(outerTriangle_, sf::Color(0, 153, 255));
  createTriangle(innerTriangle_, sf::Color(0, 153, 255));
  
  text_.setFont(font_);
  text_.setString("Ancestral");
  text_.setCharacterSize(24);
  text_.setFillColor(sf::Color(0, 153, 255));
  
  sf::Vector2u windowSize = window_.getSize();
  sf::Vector2f center(windowSize.x / 2.f, windowSize.y / 2.f);
  
  outerHexagon_.setPosition(center);
  innerHexagon_.setPosition(center);
  outerTriangle_.setPosition(center);
  innerTriangle_.setPosition(center);
  
  sf::FloatRect textBounds = text_.getLocalBounds();
  text_.setOrigin(textBounds.width / 2.f, textBounds.height / 2.f);
  text_.setPosition(center.x, center.y + 150.f);
}

void BootScreen::update() {
  auto now = std::chrono::steady_clock::now();
  float elapsed = std::chrono::duration(now - startTime_).count();
  animationProgress_ = std::min(elapsed / ANIMATION_DURATION, 1.0f);
}

void BootScreen::render() {
  renderTexture_.clear(sf::Color::Black);
  
  renderTexture_.draw(outerHexagon_);
  renderTexture_.draw(innerHexagon_);
  renderTexture_.draw(outerTriangle_);
  renderTexture_.draw(innerTriangle_);
  renderTexture_.draw(text_);
  
  renderTexture_.display();
  
  sf::Sprite sprite(renderTexture_.getTexture());
  glowShader_.setUniform("texture", sf::Shader::CurrentTexture);
  glowShader_.setUniform("blur_radius", sf::Vector2f(2.0f, 2.0f));
  
  window_.clear(sf::Color::Black);
  window_.draw(sprite, &glowShader_);
  window_.display();
}

bool BootScreen::isComplete() const {
  return animationProgress_ >= 1.0f;
}

void BootScreen::createHexagon(sf::ConvexShape& shape, float size, const sf::Color& color) {
  shape.setPointCount(6);
  for (int i = 0; i < 6; i++) {
    float angle = i * 3.14159f * 2 / 6;
    shape.setPoint(i, sf::Vector2f(size * std::cos(angle), size * std::sin(angle)));
  }
  shape.setFillColor(color);
  shape.setOutlineColor(color);
  shape.setOutlineThickness(2);
}

void BootScreen::createTriangle(sf::ConvexShape& shape, const sf::Color& color) {
  shape.setPointCount(3);
  shape.setPoint(0, sf::Vector2f(-50, -50));
  shape.setPoint(1, sf::Vector2f(50, -50));
  shape.setPoint(2, sf::Vector2f(0, 50));
  shape.setFillColor(color);
  shape.setOutlineColor(color);
  shape.setOutlineThickness(2);
}