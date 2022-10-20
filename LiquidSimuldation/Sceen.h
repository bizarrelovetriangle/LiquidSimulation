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
        : _interactionRange(30),
        _fluidProcessor(_particleGrid, _interactionRange)
    {
        _window.close();

        if (isFullScreen) {
            _windowSize = sf::Vector2i(1920, 1080);
            _window.create(sf::VideoMode(_windowSize.x, _windowSize.y), "SFML", sf::Style::Fullscreen);
        }
        else {
            _windowSize = sf::Vector2i(1000, 800);
            _window.create(sf::VideoMode(_windowSize.x, _windowSize.y), "SFML");
        }

        _particleGrid.Init(_windowSize, _interactionRange);

        auto desktop = sf::VideoMode::getDesktopMode();
        _window.setPosition(sf::Vector2i(desktop.width / 2 - _window.getSize().x / 2, desktop.height / 2 - _window.getSize().y / 2));

        sf::Vector2f center(-_windowSize / 2);
        sf::Vector2f size(_windowSize);
        sf::View view(sf::FloatRect(center, size));
        _window.setView(view);

        createWalls(_window, _walls);
    }

    void Start() {
        sf::Clock clockWise;
        while (_window.isOpen())
        {
            HandleEvent();

            int fps = 60;
            double time = clockWise.getElapsedTime().asSeconds();
            if (1. / fps > time) sf::sleep(sf::seconds(1. / fps - time));
            //std::cout << 1000. / clockWise.restart().asMilliseconds() << std::endl;

            Update();
            Draw();
        }
    }
    
    int counter = 0;

    void Update() {
        sf::Clock clock;
        clock.restart();

        _particleGrid.updateParticleNeighbours();
        float updateParticleNeighbours = clock.restart().asSeconds();

        _fluidProcessor.wallCollicionHandling(_walls, _frameExpectedInterval);
        float wallCollicionHandling = clock.restart().asSeconds();

        _fluidProcessor.createPairs();
        float createPairs = clock.restart().asSeconds();

        _fluidProcessor.applyViscosity(_frameExpectedInterval);
        float applyViscosity = clock.restart().asSeconds();

        _fluidProcessor.particlesGravity(_frameExpectedInterval);
        float particlesGravity = clock.restart().asSeconds();

        for (auto& particles : _particleGrid.GridCells.data()) {
            for (auto& particle : particles) {
                particle.update(_frameExpectedInterval);
            }
        }
        float particlesUpdate = clock.restart().asSeconds();

        float overall = updateParticleNeighbours + wallCollicionHandling + createPairs + applyViscosity + particlesGravity + particlesUpdate;

        if (counter++ % 100 == 0) {
            std::cout <<
                "updateParticleNeighbours: '" + std::to_string(updateParticleNeighbours / overall) + "'," << std::endl <<
                "wallCollicionHandling: '" + std::to_string(wallCollicionHandling / overall) + "'," << std::endl <<
                "createPairs: '" + std::to_string(createPairs / overall) + "'," << std::endl <<
                "applyViscosity: '" + std::to_string(applyViscosity / overall) + "'," << std::endl <<
                "particlesGravity: '" + std::to_string(particlesGravity / overall) + "'," << std::endl <<
                "particlesUpdate: '" + std::to_string(particlesUpdate / overall) + "'," << std::endl << std::endl;
        }
    }

    void Draw() {
        _window.clear();

        for (auto& particles : _particleGrid.GridCells.data()) {
            for (auto& particle : particles) {
                particle.draw();
            }
        }

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
                    createParticle(_window, _particleGrid, mousePosition);
                }
                if (event.key.code == sf::Keyboard::Space) {
                    createParticles(_window, _particleGrid, mousePosition);
                }
            }
        }
    }

private:
    float _interactionRange;
    sf::Vector2i _windowSize;
    sf::RenderWindow _window;
    ParticleGrid _particleGrid;
    FluidProcessor _fluidProcessor;
    std::vector<Line> _walls;

    int _expectedFps = 60;
    float _frameExpectedInterval = 1.f / _expectedFps;

    void createParticles(sf::RenderWindow& window, ParticleGrid& particleGrid, sf::Vector2f position) {
        int distance = 10;
        int width = 20;
        int height = 20;

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                sf::Vector2f particlePosition(x - width / 2, y - height / 2);
                createParticle(window, particleGrid, particlePosition * (float)distance + position);
            }
        }
    }

    void createParticle(sf::RenderWindow& window, ParticleGrid& particleGrid, sf::Vector2f position) {
        Particle particle(window, position, 4);
        particle.acceleration = sf::Vector2f(0, 200);
        particleGrid.addParticle(particle);
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
        walls.emplace_back(Line(window, sf::Vector2f(-100, -100), sf::Vector2f(100, 100)));
    }
};