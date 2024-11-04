#include <iostream>
#include <vector>

#include "generate.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>

#define SCREEN_WIDTH 1600 
#define SCREEN_HEIGHT 960 
#define BLOCK_SIZE 16

SDL_Texture* wall_sprite;
SDL_Texture* player_sprite;
SDL_Texture* end_sprite;

struct Rect
{
	int x1,y1,x2,y2;
};

class Collider
{
private:
	Rect rect1,rect2;

public:
	/*
    x1 = rect.x
    y1 = rect.y
    x2 = rect.x + rect.w
    y2 = rect.y + rect.h
	*/
	
	bool rect_collision(SDL_Rect _rect1, SDL_Rect _rect2)
	{
		rect1 = {_rect1.x,_rect1.y,_rect1.x+_rect1.w,_rect1.y+_rect1.h};
		rect2 = {_rect2.x,_rect2.y,_rect2.x+_rect2.w,_rect2.y+_rect2.h};
		if ((rect1.y1 >= rect2.y1 && rect1.y1 <= rect2.y2) && ((rect1.x1 >= rect2.x1 && rect1.x1 <= rect2.x2) || (rect1.x2 >= rect2.x1 && rect1.x2 <= rect2.x2)))
			return true;
		if ((rect1.y2 >= rect2.y1 && rect1.y2 <= rect2.y2) && ((rect1.x1 >= rect2.x1 && rect1.x1 <= rect2.x2) || (rect1.x2 >= rect2.x1 && rect1.x2 <= rect2.x2)))
			return true;
		return false;
	}
};

class Entity
{
public:
    SDL_Texture* sprite;
    SDL_Rect rect;

    int half_height, half_width;
};

class Wall : public Entity
{
public:
    Wall(SDL_Texture* _sprite, SDL_Rect _rect)
    {
        sprite = _sprite;
        rect = _rect;

        half_height = rect.h / 2;
		half_width = rect.w / 2;
    }
};

class Player : public Entity
{
private:
    Collider collider;

public:
	// x = 1 = forward x = -1 = backward | y = 1 = down y = 1 = up  
	int dir_x = 0, dir_y = 0;
	int vel_x = 2, vel_y = 2;

    Player(SDL_Texture* _sprite ,SDL_Rect _rect)
    {
        sprite = _sprite;
        rect = _rect;

        half_height = rect.h / 2;
		half_width = rect.w / 2;
    }

	void movement()
	{
		rect.x += vel_x * dir_x;
		rect.y += vel_y * dir_y;
	}

	bool win(Entity entity)
	{
		if(collider.rect_collision(rect, entity.rect))
			return true;
		return false;
	}

    void rigid_collision(Entity entity2)
	{
		if (collider.rect_collision(rect, entity2.rect))
		{
			int e1_centerx = rect.x + half_width;
			int e1_centery = rect.y + half_height;

			int e2_centerx = entity2.rect.x + entity2.half_width;
			int e2_centery = entity2.rect.y + entity2.half_height;

			int dx = e1_centerx - e2_centerx; // current distance of entity1 from entity2 on x axis
			int dy = e1_centery - e2_centery; // current distance of entity1 from entity2 on y axis

			int mindistx = half_width + entity2.half_width; // min dist on x axis for collision between 2 entities
			int mindisty = half_height + entity2.half_height; // min dist on y axis for collision between 2 entities

			int depthx = dx > 0 ? mindistx - dx : -mindistx - dx; // depth of object after collision in x axis
			int depthy = dy > 0 ? mindisty - dy : -mindisty - dy; // depth of object after collision in y axis

			if (depthx != 0 && depthy != 0)
			{
				if (abs(depthx) < abs(depthy))
				{
					if (depthx > 0)
					{
						rect.x = entity2.rect.x + entity2.rect.w; // right
					}
					else
					{
						rect.x = entity2.rect.x - rect.w; // left
					}
				}
				else
				{
					if (depthy > 0)
					{
						rect.y = entity2.rect.y + entity2.rect.h; // bottom

					}
					else
					{
						rect.y = entity2.rect.y - rect.h; // top
					}
				}
			}
		}
	}
};

class End : public Entity
{
public:
    End(SDL_Texture* _sprite, SDL_Rect _rect)
    {
        sprite = _sprite;
        rect = _rect;
    }
};

class RenderWindow
{
public:
	SDL_Window* window;
	SDL_Renderer* renderer;

	void init_system()
	{
		// error check
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			std::cout << "SDL_Init HAS FAILED. SDL_Error: " << SDL_GetError() << std::endl;

		// error check
		if (!(IMG_Init(IMG_INIT_PNG)))
			std::cout << "IMG_Init HAS FAILED. SDL_Error: " << SDL_GetError() << std::endl;

		// create window
		window = SDL_CreateWindow("Test", SCREEN_WIDTH, SCREEN_HEIGHT, 0);

		// error check
		if (!window)
			std::cout << "FAILED TO CREATE WINDOW. SDL_Error:" << SDL_GetError() << std::endl;

		// create renderer
		renderer = SDL_CreateRenderer(window, NULL);
		SDL_SetRenderVSync(renderer, 1);

		// error check
		if (!renderer)
			std::cout << "FAILED TO CREATE RENDERER. SDL_Error:" << SDL_GetError() << std::endl;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	}

	SDL_Texture* loadTexture(const char* p_filePath)
	{
		SDL_Texture* texture = NULL;
		SDL_IOStream* file = SDL_IOFromFile(p_filePath, "rb");
		texture = SDL_CreateTextureFromSurface(renderer, IMG_Load_IO(file, true));
		return texture;
	}

    void renderTexture(Entity entity)
	{
		SDL_FRect temp = {0};
		SDL_RectToFRect(&entity.rect, &temp);
		SDL_RenderTexture(renderer, entity.sprite, NULL, &temp);
	}

	void cleanup(void)
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void clear(void)
	{
		SDL_RenderClear(renderer);
	}

	void display(void) //render
	{
		SDL_RenderPresent(renderer);
	}
};

std::vector<Entity> generate_world(RenderWindow _window, std::vector<std::vector<char>> _grid, int height, int width)
{
    std::vector<Entity> _world;
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            if(_grid[i][j] == '1')
            {
                Wall temp = Wall(wall_sprite, {j*BLOCK_SIZE, i*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE});
                _world.push_back(temp);
            }
        }
    }
    return _world;
}

void load_textures(RenderWindow window)
{
    wall_sprite = window.loadTexture("wall.png");
    player_sprite = window.loadTexture("player.png");
    end_sprite = window.loadTexture("end.png");
}

int main(int argc, char* argv[])
{
    int height = 59;
    int width = 99;
	int change_rate = 10;

    RenderWindow window;
	window.init_system();

    load_textures(window);

    Maze maze(height, width);
    std::vector<std::vector<char>> grid = maze.generate_maze();
    std::vector<Entity> world = generate_world(window, grid, height, width);

    Player player = Player(player_sprite, {16,16,BLOCK_SIZE,BLOCK_SIZE});
	End end = End(end_sprite, {(width - 2)*BLOCK_SIZE,(height - 2)*BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE});
	
	SDL_Event event;

	int counter = 0;
	int timer = 0;

	unsigned int initial = SDL_GetTicks();
	unsigned int now;
	unsigned int reserved_delta_time = 3;
	int fps;
	while(1)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				exit(0);
				break;

			case SDL_EVENT_KEY_DOWN:
				switch (event.key.key) 
				{
					case SDLK_D:
						player.dir_x = 1;
						break;
					case SDLK_A:
						player.dir_x = -1;
						break;
					case SDLK_W:
						player.dir_y = -1;
						break;
					case SDLK_S:
						player.dir_y = 1;
						break;
				}
				break;

			case SDL_EVENT_KEY_UP:
				switch (event.key.key) 
				{
					case SDLK_D:
						player.dir_x = 0;
						break;
					case SDLK_A:
						player.dir_x = 0;
						break;
					case SDLK_W:
						player.dir_y = 0;
						break;
					case SDLK_S:
						player.dir_y = 0;
						break;
				}
				break;
			}
		}
		
		if(player.win(end))
			break;

		player.movement();
		
		for(const Entity& i : world)
			player.rigid_collision(i);

		window.clear();

		window.renderTexture(player);

		for(const Entity& i : world)
			window.renderTexture(i);

		window.renderTexture(end);

		window.display();

		// fps counter
		now = SDL_GetTicks();
		if(now - initial != 0)
		{
			reserved_delta_time = now - initial;
			fps = 1000/(now - initial);
		}
		else
			fps = 1000/(reserved_delta_time);
		initial = now;

		// timer counter
		counter++;
		// if fps is zero the program crashes
		if(fps != 0)
		{
			if(counter % fps == 0)
			{
				timer++;
				counter = 0;
			}
		}

		// maze change timer
		if(timer % change_rate == 0 && timer > 0)
		{
			Maze temp(height, width);
			grid = temp.generate_maze();
			world = generate_world(window, grid, height, width);
			timer = 0;
		}
	}

	window.cleanup();
	std::cout << "YOU WON!!!" << std::endl;
	std::cout << "Press enter to exit...";
	std::cin.get();
	return 0;
}
