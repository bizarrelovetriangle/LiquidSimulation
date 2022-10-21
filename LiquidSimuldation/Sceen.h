#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include "Particle.h"
#include "Line.h"
#include "FluidProcessor.h"

class Sceen {
public:
    Sceen() = delete;

    Sceen(bool isFullScreen)
    {
        if (isFullScreen) {
            _windowSize = sf::Vector2i(1920, 1080);
            _window.create(sf::VideoMode(_windowSize.x, _windowSize.y), "SFML", sf::Style::Fullscreen);
        }
        else {
            _windowSize = sf::Vector2i(1000, 800);
            _window.create(sf::VideoMode(_windowSize.x, _windowSize.y), "SFML");
        }

        _fluidProcessor = std::make_unique<FluidProcessor>(_windowSize);

        auto desktop = sf::VideoMode::getDesktopMode();
        _window.setPosition(sf::Vector2i(desktop.width / 2 - _window.getSize().x / 2, desktop.height / 2 - _window.getSize().y / 2));

        sf::Vector2f center(-_windowSize / 2);
        sf::Vector2f size(_windowSize);
        sf::View view(sf::FloatRect(center, size));
        _window.setView(view);

        createWalls(_window, _walls);
    }

    void Start() {
        sf::Clock clock;
        while (_window.isOpen())
        {
            HandleEvent();

            float interval = 1. / 100;
            double time = clock.getElapsedTime().asSeconds();
            if (time < interval) sf::sleep(sf::seconds(interval - time));
            _deltaTime = clock.getElapsedTime().asSeconds();
            clock.restart();

            Update();
            Draw();
        }
    }
    
    void Update() {
        _fluidProcessor->Update(_walls, _expectedDeltaTime);
    }

    void Draw() {
        _window.clear();

        _fluidProcessor->Draw();

        for (auto& wall : _walls) {
            wall.draw();
        }

        _window.display();
    }

    sf::Vector2f mousePosition;
    void HandleEvent() {
        sf::Event event;
        while (_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                _window.close();
            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2i mousePositionInt = sf::Mouse::getPosition(_window) - _windowSize / 2;
                mousePosition = sf::Vector2f(mousePositionInt.x, mousePositionInt.y);
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Q) {
                    _fluidProcessor->createParticle(_window, mousePosition);
                }
                if (event.key.code == sf::Keyboard::Space) {
                    createParticles(_window, mousePosition);
                }
            }
        }
    }

private:
    float _interactionRange;
    sf::Vector2i _windowSize;
    sf::RenderWindow _window;
    std::unique_ptr<FluidProcessor> _fluidProcessor;
    std::vector<Line> _walls;

    float _expectedDeltaTime = 1. / 60;
    float _deltaTime = 0;

    void createParticles(sf::RenderWindow& window, sf::Vector2f position) {
        int distance = 10;
        int width = 20;
        int height = 20;

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                sf::Vector2f particlePosition(x - width / 2, y - height / 2);
                _fluidProcessor->createParticle(window, particlePosition * (float)distance + position);
            }
        }
    }

    void createWalls(sf::RenderWindow& window, std::vector<Line>& walls) {
        int wall_width = _windowSize.x - 100;
        int wall_height = _windowSize.y - 100;

        sf::Vector2f point_a(-wall_width / 2, -wall_height / 2);
        sf::Vector2f point_b(wall_width / 2, -wall_height / 2);
        sf::Vector2f point_c(wall_width / 2, wall_height / 2);
        sf::Vector2f point_d(-wall_width / 2, wall_height / 2);

        walls.emplace_back(Line(window, point_a, point_b));
        walls.emplace_back(Line(window, point_b, point_c));
        walls.emplace_back(Line(window, point_c, point_d));
        walls.emplace_back(Line(window, point_d, point_a));
        //walls.emplace_back(Line(window, sf::Vector2f(-100, -100), sf::Vector2f(100, 100)));
    }
};