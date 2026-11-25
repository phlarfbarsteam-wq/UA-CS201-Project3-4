#include "App.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>

constexpr sf::Color BG_COLOR(25, 28, 36); // Define background color (dark grey)
constexpr sf::Color BUTTON_COLOR(60, 110, 200); // Define button color (blue)
constexpr sf::FloatRect NEXT_BUTTON({20.f, 20.f}, {140.f, 40.f}); // Define Next button position/size
constexpr sf::Vector2f BUTTON_CENTER(NEXT_BUTTON.position.x + NEXT_BUTTON.size.x / 2.f,
                                     NEXT_BUTTON.position.y + NEXT_BUTTON.size.y / 2.f);
                                     // Center of Next button (label anchor)

constexpr sf::Vector2f TICK_TEXT_POS(NEXT_BUTTON.position.x + NEXT_BUTTON.size.x + 20.f,
                                     NEXT_BUTTON.position.y + 10.f);
                                     // Tick text: 20px right of button, 10px below its top

App::App() : window(sf::VideoMode({1000, 640}), "Nota Hospital Simulation") {
    window.setFramerateLimit(60);
    font.openFromFile(DATA_DIR "/assets/JetBrainsMonoNerdFontMono-Regular.ttf");
}

void App::run() {
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* m = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (m->button == sf::Mouse::Button::Left && NEXT_BUTTON.contains(sf::Vector2f(m->position))) {
                    sim.nextTick();
                }
            }
        }
        draw();
    }
}

void App::draw() {
    window.clear(BG_COLOR); // Dark grey

    sf::RectangleShape button(NEXT_BUTTON.size); // Set Next button shape
    button.setPosition(NEXT_BUTTON.position); // Set Next button position
    button.setFillColor(BUTTON_COLOR); // Set Next button color (blue)
    window.draw(button); // Draw Next button
 
    sf::Text tickLabel (font, "Next Tick", 18); // Set Next button text
    sf::FloatRect b = tickLabel.getLocalBounds(); // Get bounds of text
    tickLabel.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f}); // Set origin to center of text
    tickLabel.setPosition(BUTTON_CENTER); // Center text on button
    window.draw(tickLabel); // Draw Next button text

    sf::Text tickText(font, "Tick " + std::to_string(sim.getTick()), 16); // Fetch current tick
    tickText.setPosition(TICK_TEXT_POS); // Place text next to the button
    window.draw(tickText); // Draw text

    window.display(); // Display window
}
