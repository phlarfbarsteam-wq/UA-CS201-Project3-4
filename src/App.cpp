#include "App.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <optional>
#include <string>
#include <stdexcept>

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
// Move the existing panels down to make room for the tree viewer.
constexpr sf::FloatRect LOBBY_BOX({20.f, 280.f}, {260.f, 460.f}); // Define lobby box position/size
constexpr sf::Vector2f LOBBY_LABEL_POS(LOBBY_BOX.position.x, LOBBY_BOX.position.y - 30.f);
                                       // Lobby label: 30px above the box

// Reserve a full-width strip for the patient-tree visualization.
constexpr sf::FloatRect TREE_PANEL({20.f, 100.f}, {960.f, 150.f});
constexpr sf::Vector2f TREE_LABEL_POS(TREE_PANEL.position.x,
                                      TREE_PANEL.position.y - 30.f);

constexpr float ROW_HEIGHT = 22.f; // Define vertical spacing between patient rows
constexpr sf::Color TEXT_COLOR(230, 230, 230); // Define main text color (white)
constexpr sf::Color DIM_COLOR(150, 155, 165); // Define dim text color (grey)
constexpr sf::Color CONDITION_LOW(120, 210, 120); // Define low condition color (green)
constexpr sf::Color CONDITION_MID(240, 160, 50); // Define mid condition color (orange)
constexpr sf::Color CONDITION_CRITICAL(230, 60, 60); // Define critical condition color (red)
constexpr sf::Vector2f ROW_ORIGIN(LOBBY_BOX.position.x + 10.f, LOBBY_BOX.position.y + 10.f);
                                  // First patient row: 10px inside the lobby box

constexpr sf::FloatRect DOCTOR_BOX({580.f, 280.f}, {400.f, 460.f}); // Define doctor box position/size
constexpr sf::Vector2f DOCTOR_LABEL_POS(DOCTOR_BOX.position.x, DOCTOR_BOX.position.y - 30.f);
                                        // Doctor label: 30px above the box

constexpr sf::Vector2f DOCTOR_ROW_ORIGIN(DOCTOR_BOX.position.x + 10.f, DOCTOR_BOX.position.y + 10.f);
                                         // First doctor row: 10px inside the doctor box
constexpr float DOCTOR_ROW_HEIGHT = 34.f; // Define vertical spacing between doctor entries (two lines each)
constexpr sf::Vector2f LOOKUP_POS(LOBBY_BOX.position.x, 
                                  LOBBY_BOX.position.y + LOBBY_BOX.size.y + 20.f);
                                  // Lookup prompt: 20px below the lobby box

constexpr sf::Vector2f LOOKUP_RESULT_POS(LOOKUP_POS.x, LOOKUP_POS.y + 30.f);
                                         // Lookup result: 30px below the prompt

// Use the center column so name results stay within the window.
constexpr sf::FloatRect NAME_RESULTS_BOX({300.f, 280.f}, {260.f, 460.f});
constexpr std::size_t NAME_RESULTS_VISIBLE_COUNT = 17;
constexpr float NAME_RESULTS_ROW_HEIGHT = 24.f;

constexpr sf::Color LOOKUP_RESULT_COLOR(120, 200, 255); // Define lookup result color (light blue)

constexpr sf::Color conditionColor(int condition) {
    if (condition >= 10) return CONDITION_CRITICAL;
    if (condition >= 6) return CONDITION_MID;
    return CONDITION_LOW;
}

// Extra height accommodates the tree viewer above the existing panels.
App::App() : window(sf::VideoMode({1000, 820}), "Nota Hospital Simulation") {
    window.setFramerateLimit(60);
    if (!font.openFromFile(DATA_DIR "/assets/JetBrainsMonoNerdFontMono-Regular.ttf"))
        throw std::runtime_error("Failed to load font in assets/");
    sim.loadData(DATA_DIR "/data/patients.csv", DATA_DIR "/data/doctors.csv");
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
            } else if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                // Scroll the result list only when the pointer is over its panel.
                if (NAME_RESULTS_BOX.contains(sf::Vector2f(wheel->position))) {
                    if (wheel->delta < 0.f) {
                        ++nameResultOffset;
                    } else if (wheel->delta > 0.f && nameResultOffset > 0) {
                        --nameResultOffset;
                    }
                }
            } else if (const auto* t = event->getIf<sf::Event::TextEntered>()) {
                if (t->unicode == 8) {
                    if (!lookupText.empty()) {
                        lookupText.pop_back(); // backspace
                        nameResultOffset = 0;
                    }
                } else if (t->unicode >= 32 && t->unicode < 127 && lookupText.size() < 64) {
                    lookupText += static_cast<char>(t->unicode); // printable ASCII
                    nameResultOffset = 0;
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

    sf::Text tickText(font, "Tick " + std::to_string(sim.getTick()) + "  (hour " + std::to_string(sim.getTick() % 24) + ")   [Right arrow = next]", 16); // Fetch current tick and hour
    tickText.setPosition(TICK_TEXT_POS); // Place text next to the button
    window.draw(tickText); // Draw text

    sf::RectangleShape treePanel(TREE_PANEL.size);
    treePanel.setPosition(TREE_PANEL.position);
    treePanel.setFillColor(sf::Color::Transparent);
    treePanel.setOutlineColor(LOBBY_OUTLINE_COLOR);
    treePanel.setOutlineThickness(2.f);
    window.draw(treePanel);

    sf::Text treeLabel(font, "Patient DOB B-Tree:", 18);
    treeLabel.setPosition(TREE_LABEL_POS);
    window.draw(treeLabel);

    // Refresh the layout snapshot each frame so tree changes appear immediately.
    const auto treeLevels = sim.getPatientTreeLevels();
    if (treeLevels.empty()) {
        sf::Text emptyTree(font, "No patient keys", 14);
        emptyTree.setPosition({TREE_PANEL.position.x + 10.f,
                               TREE_PANEL.position.y + 12.f});
        window.draw(emptyTree);
    } else {
        const float contentLeft = TREE_PANEL.position.x + 10.f;
        const float contentWidth = TREE_PANEL.size.x - 20.f;
        const float contentTop = TREE_PANEL.position.y + 10.f;
        const float contentHeight = TREE_PANEL.size.y - 20.f;
        const float rowStep =
            contentHeight / static_cast<float>(treeLevels.size());
        const float nodeHeight = std::min(22.f, rowStep - 4.f);
        std::vector<std::vector<float>> nodeCenters(treeLevels.size());

        for (std::size_t level = 0; level < treeLevels.size(); ++level) {
            const auto nodeCount = treeLevels[level].size();
            const float slotWidth = contentWidth / static_cast<float>(nodeCount);
            nodeCenters[level].reserve(nodeCount);
            for (std::size_t node = 0; node < nodeCount; ++node) {
                nodeCenters[level].push_back(
                    contentLeft + slotWidth * (static_cast<float>(node) + 0.5f));
            }
        }

        // Children of each parent are consecutive in the next level snapshot.
        for (std::size_t level = 0; level + 1 < treeLevels.size(); ++level) {
            const float parentY = contentTop + rowStep * static_cast<float>(level) +
                                  (rowStep - nodeHeight) / 2.f + nodeHeight;
            const float childY = contentTop + rowStep * static_cast<float>(level + 1) +
                                 (rowStep - nodeHeight) / 2.f;
            std::size_t childIndex = 0;
            for (std::size_t node = 0; node < treeLevels[level].size(); ++node) {
                const std::size_t childCount = treeLevels[level][node].size() + 1;
                for (std::size_t child = 0; child < childCount; ++child) {
                    sf::Vertex edge[2];
                    edge[0].position = {nodeCenters[level][node], parentY};
                    edge[0].color = LOBBY_OUTLINE_COLOR;
                    edge[1].position = {
                        nodeCenters[level + 1][childIndex + child], childY};
                    edge[1].color = LOBBY_OUTLINE_COLOR;
                    window.draw(edge, 2, sf::PrimitiveType::Lines);
                }
                childIndex += childCount;
            }
        }

        for (std::size_t level = 0; level < treeLevels.size(); ++level) {
            const auto nodeCount = treeLevels[level].size();
            const float slotWidth = contentWidth / static_cast<float>(nodeCount);
            const float rowY = contentTop + rowStep * static_cast<float>(level) +
                               (rowStep - nodeHeight) / 2.f;
            for (std::size_t node = 0; node < nodeCount; ++node) {
                std::string keys;
                for (const auto& key : treeLevels[level][node]) {
                    if (!keys.empty()) keys += " | ";
                    keys += key;
                }

                unsigned int characterSize = 9;
                sf::Text keyText(font, keys, characterSize);
                while (characterSize > 4 &&
                       keyText.getLocalBounds().size.x > slotWidth - 10.f) {
                    keyText.setCharacterSize(--characterSize);
                }
                const float boxWidth =
                    std::min(slotWidth - 4.f,
                             keyText.getLocalBounds().size.x + 8.f);
                const float boxX = nodeCenters[level][node] - boxWidth / 2.f;
                sf::RectangleShape keyBox({boxWidth, nodeHeight});
                keyBox.setPosition({boxX, rowY});
                keyBox.setFillColor(BG_COLOR);
                keyBox.setOutlineColor(TEXT_COLOR);
                keyBox.setOutlineThickness(1.f);
                window.draw(keyBox);

                keyText.setPosition({boxX + 4.f, rowY + 2.f});
                window.draw(keyText);
            }
        }
    }

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
        row.setFillColor(conditionColor(p->getCondition())); // Color by condition
        row.setPosition({ROW_ORIGIN.x, ROW_ORIGIN.y + ROW_HEIGHT * i});
        window.draw(row); // Draw patient row
    }

    sf::RectangleShape doctorBox(DOCTOR_BOX.size); // Set doctor box shape
    doctorBox.setPosition(DOCTOR_BOX.position); // Set doctor box position
    doctorBox.setFillColor(sf::Color::Transparent); // No fill
    doctorBox.setOutlineColor(LOBBY_OUTLINE_COLOR); // Set doctor box outline
    doctorBox.setOutlineThickness(2.f); // Set doctor box outline thickness
    window.draw(doctorBox); // Draw doctor box

    sf::Text doctorLabel(font, "Doctors:", 18); // Set doctor label text
    doctorLabel.setPosition(DOCTOR_LABEL_POS); // Place label above doctor box
    window.draw(doctorLabel); // Draw doctor label
    
    auto doctors = sim.getDoctors();
    for (std::size_t i = 0; i < doctors.size(); ++i) {
        const auto& d = doctors[i];
        bool onShift = d->isOnShift(sim.getTick()); // true if on shift this tick
        sf::Text info(font, d->getName() + "  skill " + std::to_string(d->getSkill()) +
                      "  shift " + std::to_string(d->getShiftStart()) + "-" +
                      std::to_string(d->getShiftEnd()) + (onShift ? "  ON" : "  OFF"), 14); // Set doctor info line
        info.setFillColor(onShift ? TEXT_COLOR : DIM_COLOR); // White when on shift, grey when off
        info.setPosition({DOCTOR_ROW_ORIGIN.x, DOCTOR_ROW_ORIGIN.y + DOCTOR_ROW_HEIGHT * i});
        window.draw(info); // Draw doctor info line

        sf::Text statusLine(font, d->getCurrent() ? "treating " : "idle", 14); // Set doctor status line
        statusLine.setFillColor(DIM_COLOR); // Grey
        statusLine.setPosition({DOCTOR_ROW_ORIGIN.x, DOCTOR_ROW_ORIGIN.y + DOCTOR_ROW_HEIGHT * i + 16.f}); // 16px below info line
        window.draw(statusLine); // Draw doctor status line

        if (d->getCurrent()) {
            sf::Text patientName(font, d->getCurrent()->getName(), 14); // Set treated patient name
            patientName.setFillColor(conditionColor(d->getCurrent()->getCondition())); // Color by condition
            patientName.setPosition(statusLine.findCharacterPos(statusLine.getString().getSize())); // Right after "treating "
            window.draw(patientName); // Draw treated patient name

            sf::Text untilTick(font, " until tick " + std::to_string(d->getBusyUntil()), 14); // Set end tick text
            untilTick.setFillColor(DIM_COLOR); // Grey
            untilTick.setPosition(patientName.findCharacterPos(patientName.getString().getSize())); // Right after the name
            window.draw(untilTick); // Draw end tick text
        }
    }

    sf::Text lookupPrompt(font, "Lookup DOB or patient name: " + lookupText + "_", 16); // Prompt with cursor
    lookupPrompt.setPosition(LOOKUP_POS); // Set prompt position (below lobby box)
    window.draw(lookupPrompt); // Draw prompt
    if (!lookupText.empty()) {
        std::string result = "No match"; // Default when nothing is found
        std::vector<Patient*> nameMatches;
        if (auto p = sim.findPatient(lookupText)) // BTree search by DOB
            result = "Patient " + p->getName() + ", age " + std::to_string(p->getAge()) +
                     ", condition " + std::to_string(p->getCondition()) +
                     ", arrives tick " + std::to_string(p->getArrivalTick());
        else if (auto d = sim.findDoctor(lookupText)) // BTree search by DOB
            result = "Doctor " + d->getName() + ", age " + std::to_string(d->getAge()) +
                     ", skill " + std::to_string(d->getSkill()) + ", shift " +
                     std::to_string(d->getShiftStart()) + "-" + std::to_string(d->getShiftEnd());
        else {
            // Fall back to patient-name lookup after the DOB indexes miss.
            nameMatches = sim.findPatientsByName(lookupText);
            if (!nameMatches.empty()) {
                result.clear();
            }
        }

        if (nameMatches.empty()) {
            sf::Text lookupResult(font, result, 16); // Set result text
            lookupResult.setFillColor(LOOKUP_RESULT_COLOR); // Set result color (light blue)
            lookupResult.setPosition(LOOKUP_RESULT_POS); // Place result below prompt
            window.draw(lookupResult); // Draw result
        } else {
            sf::RectangleShape resultsBox(NAME_RESULTS_BOX.size);
            resultsBox.setPosition(NAME_RESULTS_BOX.position);
            resultsBox.setFillColor(sf::Color::Transparent);
            resultsBox.setOutlineColor(LOBBY_OUTLINE_COLOR);
            resultsBox.setOutlineThickness(2.f);
            window.draw(resultsBox);

            sf::Text resultsTitle(
                font, "Matches (" + std::to_string(nameMatches.size()) +
                          ") - scroll",
                14);
            resultsTitle.setPosition({NAME_RESULTS_BOX.position.x + 10.f,
                                      NAME_RESULTS_BOX.position.y + 8.f});
            window.draw(resultsTitle);

            const std::size_t maxOffset =
                nameMatches.size() > NAME_RESULTS_VISIBLE_COUNT
                    ? nameMatches.size() - NAME_RESULTS_VISIBLE_COUNT
                    : 0;
            nameResultOffset = std::min(nameResultOffset, maxOffset);
            const std::size_t end =
                std::min(nameResultOffset + NAME_RESULTS_VISIBLE_COUNT,
                         nameMatches.size());
            for (std::size_t i = nameResultOffset; i < end; ++i) {
                const Patient* patient = nameMatches[i];
                const float y = NAME_RESULTS_BOX.position.y + 32.f +
                                NAME_RESULTS_ROW_HEIGHT *
                                    static_cast<float>(i - nameResultOffset);
                sf::Text row(
                    font,
                    patient->getDob() + " age " +
                        std::to_string(patient->getAge()) + " cond " +
                        std::to_string(patient->getCondition()),
                    11);
                row.setPosition({NAME_RESULTS_BOX.position.x + 10.f, y});
                window.draw(row);
            }
        }
    }

    window.display(); // Display window
}
