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

constexpr sf::Color LOBBY_OUTLINE_COLOR(120, 125, 140); // Define lobby box outline color (light grey)
constexpr sf::FloatRect LOBBY_BOX({20.f, 100.f}, {260.f, 460.f}); // Define lobby box position/size
constexpr sf::Vector2f LOBBY_LABEL_POS(LOBBY_BOX.position.x, LOBBY_BOX.position.y - 30.f);
                                       // Lobby label: 30px above the box
constexpr float ROW_HEIGHT = 22.f; // Define vertical spacing between patient rows
constexpr sf::Vector2f ROW_ORIGIN(LOBBY_BOX.position.x + 10.f, LOBBY_BOX.position.y + 10.f);
                                  // First patient row: 10px inside the lobby box
constexpr sf::Vector2f LOOKUP_POS(LOBBY_BOX.position.x, LOBBY_BOX.position.y + LOBBY_BOX.size.y + 20.f);
                                  // Lookup prompt: 20px below the lobby box
constexpr sf::Vector2f LOOKUP_RESULT_POS(LOOKUP_POS.x, LOOKUP_POS.y + 30.f);
                                         // Lookup result: 30px below the prompt
constexpr sf::Color LOOKUP_RESULT_COLOR(120, 200, 255); // Define lookup result color (light blue)

App::App() : window(sf::VideoMode({1000, 640}), "Nota Hospital Simulation") {
    window.setFramerateLimit(60);
    font.openFromFile(DATA_DIR "/assets/JetBrainsMonoNerdFontMono-Regular.ttf");
    sim.loadData(DATA_DIR "/data/patients.csv", "");
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
            } else if (const auto* t = event->getIf<sf::Event::TextEntered>()) {
                if (t->unicode == 8) {
                    if (!lookupText.empty()) lookupText.pop_back(); // backspace
                } else if (t->unicode >= 32 && t->unicode < 127 && lookupText.size() < 20) {
                    lookupText += static_cast<char>(t->unicode); // printable ASCII (max 20 chars)
                }
            } else if (const auto* k = event->getIf<sf::Event::KeyPressed>()) {
                if (k->code == sf::Keyboard::Key::Right) sim.nextTick(); // Right arrow = Next Tick
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

    sf::RectangleShape lobby(LOBBY_BOX.size); // Set lobby box shape
    lobby.setPosition(LOBBY_BOX.position); // Set lobby box position
    lobby.setFillColor(sf::Color::Transparent); // No fill
    lobby.setOutlineColor(LOBBY_OUTLINE_COLOR); // Set lobby box outline color
    lobby.setOutlineThickness(2.f); // Set lobby box outline thickness
    window.draw(lobby); // Draw lobby box

    sf::Text lobbyLabel(font, "Lobby:", 18); // Set lobby label text
    lobbyLabel.setPosition(LOBBY_LABEL_POS); // Place label above lobby box
    window.draw(lobbyLabel); // Draw lobby label

    auto waiting = sim.getWaiting();
    for (std::size_t i = 0; i < waiting.size(); ++i) {
        const auto& p = waiting[i];
        sf::Text row(font, p->getName() + " " + std::to_string(p->getCondition()) + " " + std::to_string(p->getDeadline()), 14);
        row.setPosition({ROW_ORIGIN.x, ROW_ORIGIN.y + ROW_HEIGHT * i});
        window.draw(row);
    }

    sf::Text lookupPrompt(font, "Lookup DOB (YYYY-MM-DD): " + lookupText + "_", 16); // Prompt with cursor
    lookupPrompt.setPosition(LOOKUP_POS); // Place prompt
    window.draw(lookupPrompt); // Draw prompt

    if (!lookupText.empty()) {
        std::string result = "No match"; // Default when nothing is found
        if (auto p = sim.findPatient(lookupText)) // BTree search by DOB
            result = "Patient " + p->getName() + ", age " + std::to_string(p->getAge()) +
                     ", condition " + std::to_string(p->getCondition()) +
                     ", arrives tick " + std::to_string(p->getArrivalTick());
        sf::Text lookupResult(font, result, 16); // Set result text
        lookupResult.setFillColor(LOOKUP_RESULT_COLOR); // Set result color (light blue)
        lookupResult.setPosition(LOOKUP_RESULT_POS); // Place result below prompt
        window.draw(lookupResult); // Draw result
    }

    window.display(); // Display window
}
