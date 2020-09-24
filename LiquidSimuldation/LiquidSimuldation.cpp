#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include "Particle.h"
#include "Line.h"
#include "FluidProcessor.h"

sf::Vector2i window_size(1000, 800);

void createParticles(sf::RenderWindow& window, ParticleGrid& particleGrid, sf::Vector2f position);
void createParticle(sf::RenderWindow& window, ParticleGrid& particleGrid, sf::Vector2f position);
void createWalls(sf::RenderWindow& window, std::vector<Line>& walls);

int main()
{
    sf::RenderWindow window(sf::VideoMode(1000, 800), "SFML");

    auto desktop = sf::VideoMode::getDesktopMode();
    window.setPosition(sf::Vector2i(desktop.width / 2 - window.getSize().x / 2, desktop.height / 2 - window.getSize().y / 2));

    sf::Vector2f center(-window_size / 2);
    sf::Vector2f size(window_size);
    sf::View view(sf::FloatRect(center, size));
    window.setView(view);

    std::vector<Line>* walls = new std::vector<Line>;
    createWalls(window, *walls);

    float interactionRange = 30;

    ParticleGrid particleGrid(window_size, interactionRange);
    FluidProcessor fluidProcessor(window, particleGrid, interactionRange);

    //createParticles(window, *particles, sf::Vector2f());

    sf::Vector2f mousePosition;

    int counter = 0;
    sf::Clock clock;
    sf::Clock clockWise;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2i mousePositionInt = sf::Mouse::getPosition(window) - window_size / 2;
                mousePosition = sf::Vector2f(mousePositionInt.x, mousePositionInt.y);
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Q) {
                    createParticle(window, particleGrid, mousePosition);
                }
                if (event.key.code == sf::Keyboard::Space) {
                    createParticles(window, particleGrid, mousePosition);
                }
            }
        }

        window.clear();

        float initialAndClear = clock.restart().asSeconds();

        fluidProcessor.wallCollicionHandling(*walls);

        float wallCollicionHandling = clock.restart().asSeconds();

        for (auto& particles : particleGrid.GridCells.data()) {
            for (auto& particle : particles) {
                particle.position_prev = particle.position;
            }
        }

        particleGrid.updateParticleNeighbours();

        float updateParticleNeighbours = clock.restart().asSeconds();

        fluidProcessor.particlesGravity();

        float particlesGravity = clock.restart().asSeconds();

        for (auto& particles : particleGrid.GridCells.data()) {
            for (auto& particle : particles) {
                particle.update();
            }
        }

        for (auto& particles : particleGrid.GridCells.data()) {
            for (auto& particle : particles) {
                particle.velosity = particle.position - particle.position_prev;
            }
        }

        for (auto& particles : particleGrid.GridCells.data()) {
            for (auto& particle : particles) {
                particle.draw();
            }
        }

        for (auto& wall : *walls) {
            wall.draw();
        }

        window.display();

        float drawAndDisplay = clock.restart().asSeconds();

        float currentTime = clock.restart().asSeconds();
        float fps = 1.f / currentTime;

        if (counter++ % 100 == 0 && false) {
            std::cout <<
                "fps: '" + std::to_string(fps) + "'," << std::endl <<
                "initialAndClear: '" + std::to_string(initialAndClear) + "'," << std::endl <<
                "wallCollicionHandling: '" + std::to_string(wallCollicionHandling) + "'," << std::endl <<
                "updateParticleNeighbours: '" + std::to_string(updateParticleNeighbours) + "'," << std::endl <<
                "particlesGravity: '" + std::to_string(particlesGravity) + "'," << std::endl <<
                "drawAndDisplaystd: '" + std::to_string(drawAndDisplay) << std::endl << std::endl << std::endl;
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}

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
    //particle.acceleration = sf::Vector2f(0, 0.03);
    particleGrid.addParticle(particle); 
}

void createWalls(sf::RenderWindow& window, std::vector<Line>& walls) {
    int wall_width = window_size.x - 100;
    int wall_height = window_size.y - 100;

    sf::Vector2f point_a(-wall_width / 2, -wall_height / 2);
    sf::Vector2f point_b(wall_width / 2, -wall_height / 2);
    sf::Vector2f point_c(wall_width / 2, wall_height / 2);
    sf::Vector2f point_d(-wall_width / 2, wall_height / 2);

    walls.emplace_back(Line(window, point_a, point_b));
    walls.emplace_back(Line(window, point_b, point_c));
    walls.emplace_back(Line(window, point_c, point_d));
    walls.emplace_back(Line(window, point_d, point_a));
}