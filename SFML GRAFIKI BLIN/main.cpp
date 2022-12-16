

#include <iostream>
#include <sstream>
#include <array>
#include <vector>
#include <SFML\Graphics.hpp>
#include <Windows.h>

const int WX = 1000;
const int WY = 600;
const int MAX_VERTEX = 1000;

struct Ball { //структура для шара
    double x, y, r = 10;
    double vy, ay, vx, ax;
    double dt = 0.01, t = 0;
    double m = 5, c = 0.01;
};

struct Windxy { //структура для графического отображения
    double maxx, maxy;
    double minx = 0, miny = 100;
    double ppm = 10;//количество пикселей в одном метре
};

struct ListNode { //связный список, используется для реализации хвоста
    float val1, val2;
    ListNode* next;
    ListNode() : val1(0), val2(0), next(nullptr) {}
    ListNode(float x, float y) : val1(x), val2(y), next(nullptr) {}
    ListNode(float x, float y, ListNode* next) : val1(x), val2(y), next(next) {}
};

sf::RectangleShape addPixel(sf::Vector2f position, sf::Uint8 red, sf::Uint8 green, sf::Uint8 blue);

//функции для перевода метрических координат в экранные и наоборот
sf::Vector2f coordtopix(sf::Vector2f ball, Windxy &window);
sf::Vector2f pixtocoord(sf::Vector2f xy, Windxy& window);

std::vector <sf::Vertex> makeGPlot(Windxy window, float grid_size);
//функции для работы с шаром
Ball updateball(Ball ball);
bool ismovable(Ball& ball, Windxy& window);
void makeRandomBallxy(Ball& ball, sf::Vector2f xy);
sf::Text ballData(sf::Font& font, Ball& ball, sf::Vector2f pixcoord, const char* label);



int main() {
    sf::Font font;//загрузка шрифта
    font.loadFromFile("GothamMedium.ttf");

    sf::RenderWindow window(sf::VideoMode(WX, WY), "SQUARES");
    
    //создание поля для моделирования
    Windxy plot;
    plot.maxx = WX - 10;
    plot.maxy = WY - 100;
    plot.miny = 100;
    plot.minx = 10;

    //создание графической части поля
    sf::VertexBuffer graph_plot{ sf::Lines, sf::VertexBuffer::Static };
    graph_plot.create(MAX_VERTEX);
    graph_plot.update(makeGPlot(plot, 5).data());

    Ball ball = { 0 };
    ball.x = 0;
    ball.y = 9;
    ball.vy = 0;
    ball.vx = 0;
    ball.ax = 0;
    ball.ay = -9;

    Ball startdata = ball;

    sf::CircleShape tball(10.f);
    
    ListNode* track_head = new ListNode, * track_node = track_head; // создание следа траектории шара

    int sz = float(1)/ball.dt;
    window.setFramerateLimit(int(1 / ball.dt)*100);
    for (int i = 0; i < sz; ++i) {
        track_node->next = new ListNode;
        track_node->val1 = 0;
        track_node->val2 = 0;
        track_node = track_node->next;
    }

    while (window.isOpen())
    {
        sf::Event event;
        if (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (event.type == sf::Event::MouseButtonPressed) {//Создание нового шара со случайными ускорениями и скоростями
                makeRandomBallxy(ball, pixtocoord(sf::Vector2f(sf::Mouse::getPosition(window)), plot));
                startdata = ball;
            }
        }

       
        if (ismovable(ball, plot)) {
            ball = updateball(ball);
            tball.setPosition(coordtopix(sf::Vector2f(ball.x, ball.y), plot));

            track_head = track_head->next; //удаление точки с конца следа
            track_node = track_head;

            while (track_node->next != NULL) {//отрисовка 
                window.draw(addPixel(sf::Vector2f(track_node->val1 , track_node->val2), 255, 5, 5));
                track_node = track_node->next;

                if (track_node->next == NULL) { //добавление точки в начало следа
                    track_node->next = new ListNode;
                    track_node = track_node->next;
                    track_node->val2 = tball.getPosition().y;
                    track_node->val1 = tball.getPosition().x;
                    track_node = track_head;
                    break;
                }
            }
            
            ball = updateball(ball);
            tball.setPosition(coordtopix(sf::Vector2f(ball.x, ball.y), plot));
            tball.setPosition(sf::Vector2f(tball.getPosition().x - tball.getRadius(), tball.getPosition().y - tball.getRadius()));
        }
        else {
            while (track_node->next != NULL) {
                window.draw(addPixel(sf::Vector2f(track_node->val1, track_node->val2), 255, 5, 5));
                track_node = track_node->next;
            }
            track_node = track_head;
        }
        
        sf::Vertex pix;
        pix.color = sf::Color::Cyan;
        pix.position = sf::Vector2f(400, 300);

        //Отрисовка
        window.draw(graph_plot);
        window.draw(tball);
        window.draw(ballData(font, startdata, sf::Vector2f(10, 10), "Start data:\n"));
        window.draw(ballData(font, ball, sf::Vector2f(150, 10), "End data:\n"));
        window.draw(&pix, 1, sf::Points);
        
        window.display();
        window.clear();
    }

    return 0;
}


Ball updateball(Ball ball) {
    double y = ball.y, vy = ball.vy, x = ball.x, vx = ball.vx;
    double ay = ball.ay, ax = ball.ax, dt = ball.dt;

    //double ymid = (y + vy * 0.5 * dt);
    double vymid = (vy + ay * 0.5 * dt);
    double aymid = ay;

    ball.y += vymid*dt;
    ball.vy += aymid*dt;

    //double xmid = (x + vx * 0.5 * dt);
    double vxmid = (vx + ax * 0.5 * dt);
    double axmid = ax;

    ball.x += vxmid * dt;
    ball.vx += axmid * dt;

    ball.t += dt;
    //Сопротивление воздуха

    /*тут ничего нет*/

    return ball;
}

bool ismovable(Ball &ball, Windxy &window) {
    double x = coordtopix(sf::Vector2f(ball.x, ball.y), window).x, y = coordtopix(sf::Vector2f(ball.x, ball.y), window).y, r = ball.r * window.ppm;
    return (x <= window.maxx) && (x >= window.minx) && (y <= window.maxy) && (y >= window.miny);
}

void makeRandomBallxy(Ball &ball, sf::Vector2f xy) {
    ball.x = xy.x;
    ball.y = xy.y;

    ball.vx = rand() % 50 - 20;
    ball.vy = rand() % 50 - 20;
    ball.ay = -9;
    ball.ax = rand() % 10 - 5;
    ball.t = 0;
}

sf::RectangleShape addPixel(sf::Vector2f position, sf::Uint8 red, sf::Uint8 green, sf::Uint8 blue)
{
    sf::RectangleShape pixel;
    pixel.setSize({ 1.f, 1.f });
    pixel.setFillColor({ red, green, blue });
    pixel.setPosition(position);
    return pixel;
}


sf::Vector2f coordtopix(sf::Vector2f ball, Windxy &window) {
    return sf::Vector2f(ball.x * window.ppm + window.minx, window.maxy - ball.y * window.ppm);
}

sf::Vector2f pixtocoord(sf::Vector2f xy, Windxy &window) {
    return sf::Vector2f((xy.x - window.minx)/window.ppm, (window.maxy - xy.y)/window.ppm);
}

sf::Text ballData(sf::Font &font, Ball &ball, sf::Vector2f pixcoord, const char* label) {
    std::ostringstream parameters;
    sf::Text text("", font, 10);
    text.setPosition(pixcoord);
    parameters << "x: " << ball.x << "\ny: " << ball.y << "\nt = " << ball.t << "\nVx = " << ball.vx << "\nVy = " << ball.vy<< "\nax = " << ball.ax << "\nay = " << ball.ay;
    text.setString(label + parameters.str());

    return text;
}

std::vector <sf::Vertex> makeGPlot(Windxy window, float grid_size) {
    std::vector <sf::Vertex> plot(8);

    plot.at(0).position = sf::Vector2f(window.maxx, window.maxy);
    plot.at(1).position = sf::Vector2f(window.maxx, window.miny);
    plot.at(2).position = sf::Vector2f(window.minx, window.miny);
    plot.at(3).position = sf::Vector2f(window.minx, window.maxy);

    plot.at(4).position = sf::Vector2f(window.maxx, window.maxy);
    plot.at(6).position = sf::Vector2f(window.maxx, window.miny);
    plot.at(7).position = sf::Vector2f(window.minx, window.miny);
    plot.at(5).position = sf::Vector2f(window.minx, window.maxy);
    
    for (int i = 0; i < (window.maxx - window.minx) / window.ppm; i += grid_size) {
        sf::Vertex bufv;
        bufv.color = sf::Color(100, 100, 100);

        bufv.position = coordtopix(sf::Vector2f(i, 0), window);
        plot.push_back(bufv);
        
        bufv.position = coordtopix(sf::Vector2f(i, (window.maxy - window.miny) / window.ppm), window);
        plot.push_back(bufv);

    }

    for (int i = 0; i < (window.maxy - window.miny) / window.ppm; i += grid_size) {
        sf::Vertex bufv;
        bufv.color = sf::Color(100, 100, 100);

        bufv.position = coordtopix(sf::Vector2f(0, i), window);
        plot.push_back(bufv);

        bufv.position = coordtopix(sf::Vector2f((window.maxx - window.minx) / window.ppm, i), window);
        plot.push_back(bufv);
    }

    return plot;

}