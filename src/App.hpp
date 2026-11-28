#pragma once
#include <SFML/Graphics.hpp>
#include <cstddef>
#include <string>

#include "Simulation.hpp"

class App {
    Simulation sim;
    sf::RenderWindow window;
    sf::Font font;
    std::string lookupText;
    // First visible row in the scrollable name-match panel.
    std::size_t nameResultOffset = 0;

    void draw();

public:
    App();
    void run();
};
