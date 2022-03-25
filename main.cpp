#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <math.h>

using namespace std;

constexpr float WIDTH = 750.0f, HEIGHT=750.0f;
constexpr float partDis = 5.0f;
constexpr float buttonRadius = 10.0f;
constexpr int particPerRow = 40;

struct vec2f {
    float x, y;
};

struct particle {
    vec2f pos;
    vec2f pastPos;
    vec2f acc = {0.0f, 9.8f};
    particle(vec2f p) {
        pos = p;
        pastPos = p;
    }
};

void simulateVerlet(vector<vector<particle>> &rope);
void relaxConstraint(particle &p1, particle &p2, float dis);
float length(vec2f &p1, vec2f &p2);
void jakobsen(vector<vector<particle>> &particles, int n);


int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "GravSim");
    window.setFramerateLimit(150);

    vector<vector<particle>> rope;
    for (int j=0; j < particPerRow; j++) {
        vector<particle> ropeRow;
        for (int i=0; i < particPerRow; i++) {
            ropeRow.push_back(particle({WIDTH/3.0f+i*partDis, HEIGHT/3.0f+j*partDis}));
        }
        rope.push_back(ropeRow);
    }

    vector<sf::CircleShape> buttons;
    buttons.push_back(sf::CircleShape(buttonRadius));
    buttons.push_back(sf::CircleShape(buttonRadius));

    vector<sf::VertexArray> clothVisualHor;
    for (int i=0; i < rope.size(); i++) {
        sf::VertexArray ropeVisual(sf::LineStrip, rope[i].size());
        for (int j=0; j < rope[i].size(); j++) {
            ropeVisual[j].position = {rope[i][j].pos.x, rope[i][j].pos.y};
        }
        clothVisualHor.push_back(ropeVisual);
    }

    vector<sf::VertexArray> clothVisualVert;
    for (int i=0; i < rope.size(); i++) {
        sf::VertexArray ropeVisual(sf::LineStrip, rope[i].size());
        for (int j=0; j < rope[i].size(); j++) {
            ropeVisual[j].position = {rope[j][i].pos.x, rope[j][i].pos.y};
        }
        clothVisualVert.push_back(ropeVisual);
    }

    rope[0][0].acc = {0.0f, 0.0f};
    rope[0][particPerRow-1].acc = {0.0f, 0.0f};
    rope[0][particPerRow-1].pos.x = rope[0][particPerRow-1].pastPos.x+10;
    
    while (window.isOpen()) {
        simulateVerlet(rope);
        jakobsen(rope, 2);
        sf::Event event;
        bool dragEnabled = false;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            if (event.type == sf::Event::MouseButtonPressed) {
                if (abs(event.mouseButton.x-rope[0][0].pos.x) <= buttonRadius && abs(event.mouseButton.y-rope[0][0].pos.y) <= buttonRadius) {
                    dragEnabled = true;
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                dragEnabled = false;
            }
            if (event.type == sf::Event::MouseMoved) {
                rope[0][0].pastPos = {float(event.mouseMove.x), float(event.mouseMove.y)};
            }
        }
        rope[0][0].pos = rope[0][0].pastPos;
        rope[0][particPerRow-1].pos = rope[0][particPerRow-1].pastPos;
        

        for (int i=0; i < rope.size(); i++) {
            for (int j=0; j < rope[j].size(); j++) {
                clothVisualHor[i][j].position = {rope[i][j].pos.x, rope[i][j].pos.y};
                clothVisualVert[i][j].position = {rope[j][i].pos.x, rope[j][i].pos.y};
            }
        }

        buttons[0].setPosition(rope[0][0].pos.x-buttonRadius, rope[0][0].pos.y-buttonRadius);
        buttons[1].setPosition(rope[0][rope[0].size()-1].pos.x-buttonRadius, rope[0][rope[0].size()-1].pos.y-buttonRadius);

        window.clear();
        for(auto& ropeVisual : clothVisualHor) {
            window.draw(ropeVisual);
        }
        for(auto& ropeVisual : clothVisualVert) {
            window.draw(ropeVisual);
        }
        for(auto& button : buttons) {
            window.draw(button);
        }
        window.display();
    }
}


float length(vec2f &p1, vec2f &p2) {
    return float(sqrt(pow(p1.x-p2.x, 2) + pow(p1.y-p2.y, 2)));
}


void simulateVerlet(vector<vector<particle>> &particles) {
    //borde implementera dt
    float dt = 1.0f/20.0f;
    for (auto& partRow : particles) {
        for(auto& p : partRow) {
            vec2f positionCopy = p.pos;
            p.pos.x = 2.0f*p.pos.x - p.pastPos.x + p.acc.x*dt*dt;
            p.pos.y = 2.0f*p.pos.y - p.pastPos.y + p.acc.y*dt*dt;
            p.pastPos = positionCopy;
        }
    }
}

void relaxConstraint(particle &p1, particle &p2, float dis, bool anchor) {
    vec2f dir = {(p2.pos.x-p1.pos.x)/length(p1.pos, p2.pos), (p2.pos.y-p1.pos.y)/length(p1.pos, p2.pos)};
    float deltaDis = length(p1.pos, p2.pos) - dis;
    if(!anchor) {
        p1.pos.x = p1.pos.x + deltaDis*dir.x/2.0f;
        p1.pos.y = p1.pos.y + deltaDis*dir.y/2.0f;
        p2.pos.x = p2.pos.x - deltaDis*dir.x/2.0f;
        p2.pos.y = p2.pos.y - deltaDis*dir.y/2.0f;
    } else {
        p2.pos.x = p2.pos.x - deltaDis*dir.x;
        p2.pos.y = p2.pos.y - deltaDis*dir.y;
    }
}

void jakobsen(vector<vector<particle>> &particles, int n) {
    for(int k=0; k < n; k++) {
        for (int i=0; i < particles.size(); i++) {
            for (int j=0; j < particles[i].size()-1; j++) {
                if (j==0 && i == 0) {
                    relaxConstraint(particles[i][j], particles[i][j+1], partDis, true);
                }/*   else if (j==particles[i].size()-2 && i == 0) {
                    relaxConstraint(particles[i][j], particles[i][j+1], partDis, true);
                }*/ else {
                    relaxConstraint(particles[i][j], particles[i][j+1], partDis, false);
                }
            }
        }
        for (int i=0; i < particles.size(); i++) {
            for (int j=0; j < particles[i].size()-1; j++) {
                if (j==0 && i == 0) {
                    relaxConstraint(particles[j][i], particles[j+1][i], partDis, true);
                } else if (j==0 && i == particles[i].size()-1) {
                    relaxConstraint(particles[j][i], particles[j+1][i], partDis, true);
                } else {
                    relaxConstraint(particles[j][i], particles[j+1][i], partDis, false);
                }
            }
        }
    }
}