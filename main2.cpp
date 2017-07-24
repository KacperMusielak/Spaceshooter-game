#include <ncurses.h>
#include <string>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>

struct bullet
{
	int posY, posX;
	bool is_moving;
};

struct asteroid
{
	int posY, posX, balancer;
	bool is_moving, start;
};

struct readThreadParams 
{
	asteroid ast;
	int id;
};

pthread_mutex_t	mutex;

int test = 0;
bool quit = false;
char map[25][80]; 
int posY = 11, posX = 5; 
bullet bullets[10]; 
asteroid asteroids[100]; 
int asteroidspeed = 3; 
int asteroid_index = 0;
int asteroid_spawnrate = 100; 
int asteroid_spawnbalancer = 0;
int asteroids_destroyed = 0;

static const int num_threads = 100;


void *call_from_thread(void *args) {
	//asteroid *asteroids = (asteroid *) args;
	struct readThreadParams *read = (readThreadParams *) args;

	if(read->ast.posX == 1)
	{
		pthread_mutex_lock(&mutex);
		usleep(220);

		quit = true;
		test = read->id;
		pthread_mutex_unlock(&mutex);
	}    	

    	return NULL;
}


int main()
{	
    	pthread_t t[num_threads];
	pthread_mutex_init(&mutex, NULL);
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	srand(time(0));



	for(int i=0; i < 10; ++i)
	{
		bullets[i].is_moving = false;
		bullets[i].posX = 0;
		bullets[i].posY = 0;
	}

	for(int i=0; i < 100; ++i)
	{
		asteroids[i].is_moving = false;
		asteroids[i].balancer = 0;
	}

	init_pair(1, COLOR_BLUE, COLOR_BLUE);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	init_pair(3, COLOR_YELLOW, COLOR_BLUE);
	init_pair(4, COLOR_BLACK, COLOR_BLUE);
	init_pair(5, COLOR_RED, COLOR_BLACK);
	init_pair(6, COLOR_RED, COLOR_BLUE);
	init_pair(7, COLOR_RED, COLOR_RED);

	for(int i=0; i < 80; ++i)
		map[0][i] = ' ';

	for(int i=0; i < 80; ++i)
	{
		map[1][i] = '#';
		map[23][i] = '#';
	}
	for(int a=2; a < 23; ++a)
	{
		for(int b=0; b < 80; ++b)
		{
			map[a][b] = ' ';
		}

	}
	for(int i=0; i < 80; ++i)
		map[24][i] = ' ';

	map[posY][posX] = 'P';

	char text3[] = "Press any key to start";
	int maxX, maxY;
	getmaxyx(stdscr, maxY, maxX);
	attron(COLOR_PAIR(5));

	attron(A_BOLD);
	mvprintw((maxY / 2) + 4, (maxX - sizeof(text3)) / 2, "%s", text3);
	attroff(A_BOLD);

	getch();
	
	curs_set(0);

	int ch, bullet_stabilizer = 10;
	int bulletIndex = 0;
	

	while(quit == false)
	{
  	for (int i = 0; i < num_threads; ++i) {
		struct readThreadParams readParams;
		readParams.ast = asteroids[i];
		readParams.id = i;
        	pthread_create(&t[i], NULL, call_from_thread, (void *) &readParams);
    	} 
		usleep(30000);
		++bullet_stabilizer;
		++asteroid_spawnbalancer;

		timeout(0);
		ch = getch();
		switch(ch)
		{
		case KEY_UP:
			if(posY - 1 > 1)
			{
				map[posY][posX] = ' ';
				map[posY][posX+1] = ' ';
				--posY;
			}
			break;
		case KEY_DOWN:
			if(posY + 1 < 23)
			{
				map[posY][posX] = ' ';
				map[posY][posX+1] = ' ';
				++posY;
			}
			break;
		case ' ':
			if(bullet_stabilizer >= 10)
			{
				bullets[bulletIndex].is_moving = true;
				bullets[bulletIndex].posX = posX + 2;
				bullets[bulletIndex].posY = posY;
			
				if(bulletIndex == 9) bulletIndex = 0;
				else ++bulletIndex;

				bullet_stabilizer = 0;
			}
			break;
		case 27:
			return 0;
			break;
		default:
			break;
		}
		map[posY][posX] = 'P';
	
		for(int i=0; i < 10; ++i)
		{
			if(bullets[i].is_moving == true)
			{
				map[bullets[i].posY][bullets[i].posX] = ' ';
				++bullets[i].posX;
				map[bullets[i].posY][bullets[i].posX] = '*';
			}
			if(bullets[i].posX == maxX-1)
			{
				bullets[i].is_moving = false;
				map[bullets[i].posY][bullets[i].posX] = ' ';
			}

		}
		for(int i=0; i < 100; ++i)
		{
			if(asteroids[i].is_moving == true && asteroids[i].balancer >= asteroidspeed)
			{
				map[asteroids[i].posY][asteroids[i].posX] = ' ';
				--asteroids[i].posX;
				map[asteroids[i].posY][asteroids[i].posX] = '@';
				asteroids[i].balancer = 0;
			}
			if(asteroids[i].start == true)
			{
				asteroids[i].posX = maxX-1;
				asteroids[i].posY = rand() % 20 + 3;
				asteroids[i].is_moving = true;
				asteroids[i].start = false;

			}
			


			for(int a=0; a < 10; ++a)
			{
				if(asteroids[i].posX > 0 && asteroids[i].is_moving == true &&
				((asteroids[i].posY == bullets[a].posY && asteroids[i].posX == bullets[a].posX) ||
				(asteroids[i].posY == bullets[a].posY && asteroids[i].posX == bullets[a].posX+1)))
				{
					bullets[a].is_moving = false;
					asteroids[i].is_moving = false;
					map[bullets[a].posY][bullets[a].posX] = ' ';
					map[asteroids[i].posY][asteroids[i].posX] = ' ';
					bullets[a].posY = 0;
					++asteroids_destroyed;
				}
			}
		}
		if(asteroid_spawnbalancer >= asteroid_spawnrate)
		{
			asteroid_spawnbalancer = 0;
			asteroids[(rand()%99)].start = true;
			asteroids[(rand()%99)].start = true;
		}

		for(int i=0; i < maxX - 1; ++i)
			map[0][i] = ' ';

		move(0, 0);
		for(int a=0; a < 25; ++a)
			for(int b=0; b < 80; ++b)
			{
				if(map[a][b] == ' ') attron(COLOR_PAIR(1) | A_BOLD);
				else if(map[a][b] == 'P') attron(COLOR_PAIR(3) | A_BOLD);
				else if(map[a][b] == '*') attron(COLOR_PAIR(4));
				else if(map[a][b] == '@') attron(COLOR_PAIR(6));
				else attron(COLOR_PAIR(2) | A_BOLD);
				addch(map[a][b]);
				standend();
			}
			attron(COLOR_PAIR(2) | A_BOLD);
			mvprintw(maxY-1, 0, "asteroids destroyed: %i", asteroids_destroyed);
			attroff(COLOR_PAIR(2) | A_BOLD);

		for(int i=0; i < 100; ++i)
			++asteroids[i].balancer;
    	for (int i = 0; i < num_threads; ++i) {
        	pthread_join(t[i], NULL);
    	}
	}
	char text[] = " You lost... ";
	attron(COLOR_PAIR(7) | A_BOLD);
	mvprintw(maxY / 2, (maxX - sizeof(text)) / 2, "%s , killed by asteroid.", text);
	attroff(COLOR_PAIR(7) | A_BOLD);

	timeout(-1);
	do
	{
		ch = getch();
	} while(ch != 10);

	endwin();
	pthread_mutex_destroy(&mutex);
	return 0;
}
