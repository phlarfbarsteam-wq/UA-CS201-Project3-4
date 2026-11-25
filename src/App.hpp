#pragma once
#include <SFML/Graphics.hpp>
#include <string>

#include "Simulation.hpp"

class App {
    Simulation sim;
    sf::RenderWindow window;
    sf::Font font;
    std::string lookupText;

    void draw();

public:
    App();
    void run();
};
