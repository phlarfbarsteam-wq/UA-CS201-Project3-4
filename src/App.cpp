#include "App.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <optional>
#include <string>

App::App() : window(sf::VideoMode({1000, 640}), "Nota Hospital Simulation") {
    window.setFramerateLimit(60);
    font.openFromFile(DATA_DIR "/assets/JetBrainsMonoNerdFontMono-Regular.ttf");
}

void App::run() {
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }
        draw();
    }
}

void App::draw() {
    window.clear(sf::Color(25, 28, 36)); // Dark grey
    sf::Text text(font, "Tick " + std::to_string(sim.getTick()), 16); // Fetch current tick
    text.setPosition({20.f, 20.f}); // Place text at the top-left corner
    window.draw(text); // Draw text
    window.display(); // Display window
}
