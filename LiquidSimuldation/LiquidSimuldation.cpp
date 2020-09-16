#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include "Particle.h"
#include "Line.h"
#include "FluidProcessor.h"

int window_width = 1000;
int window_height = 800;

void createWalls(sf::RenderWindow& window, std::vector<Line*>& walls);

int main()
{
    sf::RenderWindow window(sf::VideoMode(1000, 800), "SFML");

    auto desktop = sf::VideoMode::getDesktopMode();
    window.setPosition(sf::Vector2i(desktop.width / 2 - window.getSize().x / 2, desktop.height / 2 - window.getSize().y / 2));

    sf::Vector2f center(-window_width / 2, -window_height / 2);
    sf::Vector2f size(window_width, window_height);
    sf::View view(sf::FloatRect(center, size));
    window.setView(view);


    std::vector<Line*> walls;
    createWalls(window, walls);

    std::vector<Particle*> particles;

    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 20; y++) {
            sf::Vector2f position(x * 50 - 225, y * 10 - 100);
            Particle* particle = new Particle(window, position, 10);
            //particle->velosity = sf::Vector2f(rand() % 10 - 5, rand() % 10 - 5);
            particle->acceleration = sf::Vector2f(0, 0.3);
            particles.emplace_back(particle);
        }
    }

    FluidProcessor fluidProcessor(window);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        fluidProcessor.wallCollicionHandling(particles, walls);
        fluidProcessor.particlesGravity(particles);

        for (auto& particle : particles) {
            particle->update();
            particle->draw();
        }

        for (auto& wall : walls) {
            wall->draw();
        }

        window.display();

        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

void createWalls(sf::RenderWindow& window, std::vector<Line*>& walls) {
    int wall_width = window_width - 100;
    int wall_height = window_height - 100;

    sf::Vector2f point_a(-wall_width / 2, -wall_height / 2);
    sf::Vector2f point_b(wall_width / 2, -wall_height / 2);
    sf::Vector2f point_c(wall_width / 2, wall_height / 2);
    sf::Vector2f point_d(-wall_width / 2, wall_height / 2);

    walls.emplace_back(new Line(window, point_a, point_b));
    walls.emplace_back(new Line(window, point_b, point_c));
    walls.emplace_back(new Line(window, point_c, point_d));
    walls.emplace_back(new Line(window, point_d, point_a));
}