#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include "Particle.h"
#include "Line.h"
#include "FluidProcessor.h"

sf::Vector2i window_size(1000, 800);

void createWalls(sf::RenderWindow& window, std::vector<Line*>& walls);

int main()
{
    sf::RenderWindow window(sf::VideoMode(1000, 800), "SFML");

    auto desktop = sf::VideoMode::getDesktopMode();
    window.setPosition(sf::Vector2i(desktop.width / 2 - window.getSize().x / 2, desktop.height / 2 - window.getSize().y / 2));

    sf::Vector2f center(-window_size / 2);
    sf::Vector2f size(window_size);
    sf::View view(sf::FloatRect(center, size));
    window.setView(view);

    std::vector<Line*> walls;
    createWalls(window, walls);

    std::vector<Particle*> particles;

    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 20; y++) {
            sf::Vector2f position(x * 10 - 300, y * 10 - 100);
            Particle* particle = new Particle(window, position, 4);
            //particle->velosity = sf::Vector2f(rand() % 10 - 5, rand() % 10 - 5);
            //particle->acceleration = sf::Vector2f(0, 0.03);
            particles.emplace_back(particle);
        }
    }
    
    float interactionRange = 30;

    ParticleGrid particleGrid(particles, window_size, interactionRange);
    FluidProcessor fluidProcessor(window, particles, particleGrid, interactionRange);
    
    sf::Vector2f mousePosition;

    sf::Clock clock;
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
                    particles[0]->position = mousePosition;
                }
            }
        }

        window.clear();

        fluidProcessor.wallCollicionHandling(walls);

        for (auto particle : particles) {
            particle->position_prev = particle->position;
        }

        particleGrid.updateParticleNeighbours();

        fluidProcessor.particlesGravity();

        for (auto& particle : particles) {
            particle->update();
        }

        for (auto particle : particles) {
            particle->velosity = particle->position - particle->position_prev;
        }

        for (auto& particle : particles) {
            particle->draw();
        }

        for (auto& wall : walls) {
            wall->draw();
        }

        window.display();
        
        float currentTime = clock.restart().asSeconds();
        float fps = 1.f / currentTime;

        std::cout << fps << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}

void createWalls(sf::RenderWindow& window, std::vector<Line*>& walls) {
    int wall_width = window_size.x - 100;
    int wall_height = window_size.y - 100;

    sf::Vector2f point_a(-wall_width / 2, -wall_height / 2);
    sf::Vector2f point_b(wall_width / 2, -wall_height / 2);
    sf::Vector2f point_c(wall_width / 2, wall_height / 2);
    sf::Vector2f point_d(-wall_width / 2, wall_height / 2);

    walls.emplace_back(new Line(window, point_a, point_b));
    walls.emplace_back(new Line(window, point_b, point_c));
    walls.emplace_back(new Line(window, point_c, point_d));
    walls.emplace_back(new Line(window, point_d, point_a));
}