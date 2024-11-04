#pragma once
#include <random>
#include <vector>

class Point
{
public:
    int x, y;

    Point(){}
    Point(int _x, int _y)
    {
        x = _x;
        y = _y;
    }

    bool operator ==(Point _val)
    {
        if(x == _val.x && y == _val.y)
            return true;
        return false;
    }

    void operator +(Point _val)
    {
        x += _val.x;
        y += _val.y;
    }
};

class Maze
{
private:
    std::mt19937 generator = std::mt19937(std::random_device{}());

    const Point UP = Point(0,-1);
    const Point DOWN = Point(0,1);
    const Point LEFT = Point(-1,0);
    const Point RIGHT = Point(1,0);

    int HEIGHT;
    int WIDTH;
    
    char WALL = '1';
    char PATH = '0';

    std::vector<std::vector<char>> grid;
    std::vector<Point> visited;

    Point choice(std::vector<Point> _val)
    {
        std::uniform_int_distribution<int> randir(0, _val.capacity() - 1);
        return _val[randir(generator)];
    }

    bool notIn(std::vector<Point> _vec, Point _val)
    {
        for(int i = 0; i < _vec.capacity(); i++)
        {
            if(_vec[i] == _val)
                return false;
        }
        return true;
    }

    void init_grid()
    {
        grid = std::vector(HEIGHT, std::vector<char>(WIDTH, WALL));
    }

    void walk(int x,int y)
    {
        Point current_cell = Point(x,y);
        grid[y][x] = PATH;
        visited.push_back(current_cell);

        while(1)
        {
            std::vector<Point> neighbours;

            if(current_cell.y > 1 && notIn(visited,Point(current_cell.x,current_cell.y - 2)))
                neighbours.push_back(UP);
            if(current_cell.y < HEIGHT - 2 && notIn(visited,Point(current_cell.x,current_cell.y + 2)))
                neighbours.push_back(DOWN);
            if(current_cell.x > 1 && notIn(visited,Point(current_cell.x - 2,current_cell.y)))
                neighbours.push_back(LEFT);
            if(current_cell.x < WIDTH - 2 && notIn(visited,Point(current_cell.x + 2,current_cell.y)))
                neighbours.push_back(RIGHT);

            if(neighbours.capacity() == 0)
                return;

            Point direction = choice(neighbours);
            current_cell + direction;
            grid[current_cell.y][current_cell.x] = PATH;
            current_cell + direction;
            walk(current_cell.x,current_cell.y);
        }
    }

    int convert_to_odd(int _val)
    {
        if(_val % 2 != 0)
            return _val;
        return _val + 1;
    }
public:
    Maze(int height, int width)
    {
        HEIGHT = height;
        WIDTH = width;
        init_grid();
    }

    std::vector<std::vector<char>> generate_maze()
    {
        std::uniform_int_distribution<int> rand_height(1, HEIGHT - 2);
        std::uniform_int_distribution<int> rand_width(1, WIDTH - 2);
        walk(convert_to_odd(rand_width(generator)),convert_to_odd(rand_height(generator)));
        return grid;
    }
};