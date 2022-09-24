#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include "snake.h"

#define DEFAULT_FPS 11

Body  *snake = NULL;
Apple *apple = NULL;
int score = 0;

void quit(void)
{
	endwin();
	exit(0);
}


void generate_apple(int win_width, int win_height) 
{
	int randx, randy;

	if(!apple)
	{
		apple = (Apple*) malloc(sizeof(Apple));
		if (apple == NULL) {
			printf("Failed to allocate memory, exiting.");
			quit();
		}
	}

	srand(time(NULL));
	randy = rand() % win_height;
	randx = rand() % win_width;

	randy++;
	randx++;

	if(randy == win_height || 
	   randy == win_height - 1)
		randy--;

	if(randx >= win_width)
		randx--;

	apple->posx = randx;
	apple->posy = randy;
}

void add_body_start(int posx, int posy)
{
	Body *node = (Body*) malloc(sizeof(Body));

	if(node == NULL) {
		printf("Failed to allocate memory, exiting.");
		quit();
	}

	node->posx = posx;
	node->posy = posy;

	node->next = snake;
	snake = node;
}

void add_body_last(void)
{
	Body *node = (Body*) malloc(sizeof(Body));

	if(node == NULL) {
		endwin();
		printf("Failed to allocate memory, exiting.");
		exit(0);
	}


	node->posx = 0;
	node->posy = 0;
	node->next = NULL;

	Body *temp = snake;

	while(1) {
		if(temp->next == NULL) {
			temp->next = node;
			break;
		}

		temp = temp->next;			
	}
}


void del_last_body(void)
{
    Body *sec_last = snake;

    if (sec_last->next == NULL) {
        free(sec_last);
		sec_last = NULL;
        return;
    }

    while(sec_last->next->next != NULL)
        sec_last = sec_last->next;

    free(sec_last->next);
    sec_last->next = NULL;
}

void update_snake(Direction *dir)
{
	int newx = snake->posx + dir->dx;
	int newy = snake->posy + dir->dy;
	add_body_start(newx, newy);
	del_last_body();
}


int kbhit(WINDOW *game_window)
{
    int c = wgetch(game_window);

    if (c != ERR) {
        ungetch(c);
        return 1;
    } else {
        return 0;
    }
}

WINDOW *create_game_window(int rows, int cols)
{
	WINDOW *game_window;

	int smaller;
	if(rows > cols){
		smaller = cols;
	} else {
		smaller = rows;
	}

	int starty = (rows - smaller) / 2;
	int startx = (cols - smaller * 2) / 2;

	game_window = newwin(smaller, smaller * 2, starty, startx);
	box(game_window, 0, 0);

	wrefresh(game_window);

	return game_window;
}

void draw_objects(WINDOW *game_window) {
	wclear(game_window);

	Body *temp = snake;
	
	/* Iterate over LL and draw Snake */
	while(1) {
		mvwaddch(game_window, temp->posy, temp->posx, '#');

		if(temp->next == NULL)
			break;
		temp = temp->next;			
	}

	/* Draw Apple */
	mvwaddch(game_window, apple->posy, apple->posx, 'A');
}

void draw_game_over(WINDOW *game_window, int win_width, int win_height) 
{
	char game_over[] = "GAME OVER";
	char  sub_text[] = "Press Space to retry.";
	
	attron(A_BLINK);
	mvwprintw(game_window, 
		  win_height / 2,
		  (win_width - strlen(game_over)) / 2,
		  "%s",
		  game_over);

	attroff(A_BLINK);

	mvwprintw(game_window,
		  win_height / 2 + 2,
		  (win_width - strlen(sub_text)) / 2,
		  "%s",
		  sub_text);
}

int check_collission(int win_width, int win_height) {

	Body *head = snake;

	/* Wall Collission */
	if(head->posx < 1 ||
	   head->posx >= (win_width-1) ||
	   head->posy < 1 ||
	   head->posy >= (win_height-1))
		return 1;

	Body *temp = snake;
	
	/* Snake Collission */
	while((temp = temp->next) != NULL) {
		if(head->posx == temp->posx && head->posy == temp->posy)
			return 1;
	}
	
	/* Apple Collossion */
	if(head->posx == apple->posx && head->posy == apple->posy)
	{
		score++;
		add_body_last();
		generate_apple(win_width, win_height);
	}

	return 0;
}

void init_game(int win_width, int win_height)
{
	snake = NULL;
	apple = NULL;
	score = 0;

	add_body_start(win_width / 2, win_height / 2);
}

void main_loop(void)
{
	int chr, height, width;
	WINDOW *game_window;
	unsigned int fps;
	Direction *dir = (Direction*) malloc(sizeof(Direction));
	
	dir->dx = 1;
	dir->dy = 0;
	fps = DEFAULT_FPS;

	getmaxyx(stdscr, height, width);

	game_window = create_game_window(height, width);
	keypad(game_window, TRUE);
	nodelay(game_window, TRUE);
	getmaxyx(game_window, height, width);

	init_game(width, height);
	generate_apple(width, height);

	int break_loop = 0;
	int game_over  = 0;
	int paused     = 0;
	while(!break_loop) {
		if(kbhit(game_window)) {
			chr = wgetch(game_window);

			switch(chr)
			{
				case KEY_DOWN:
					if(dir->dy == -1)
						break;
					dir->dx =  0;
					dir->dy =  1;
					break;

				case KEY_UP:
					if(dir->dy == 1)
						break;
					dir->dx =  0;
					dir->dy = -1;
					break;

				case KEY_RIGHT:
					if(dir->dx == -1)
						break;
					dir->dx = 1;
					dir->dy = 0;
					break;

				case KEY_LEFT:
					if(dir->dx == 1)
						break;
					dir->dx = -1;
					dir->dy =  0;
					break;

				case 113:
					quit();
					break;

				case 119:
					fps++;
					break;

				case 115:
					if(fps == 0)
						break;
					fps--;
					break;
				
				case 32:
					if(game_over)
						break_loop = 1;
					break;
				
				case 112:
					if(!game_over)
						paused = !paused;

				default:
					break;
			}	
		}


		if(!paused) {
			update_snake(dir);
			game_over = check_collission(width, height);
			
			if(game_over) {
				draw_game_over(game_window, width, height);
			} else {
				draw_objects(game_window);
			}
		} else {
			char paused_txt[] = "PAUSED";
			mvwprintw(game_window,
				  height / 2,
				  (width - strlen(paused_txt)) / 2,
				  "%s", paused_txt);
		}

		box(game_window, 0, 0);
		mvwprintw(game_window, 0, 1, "Score: %d", score);
		mvwprintw(game_window, 0, width - 8, "FPS: %d", fps);
		mvwprintw(game_window,
			  height - 1,
			  1,
			  "Q... quit | P... pause | W... fps up | S... fps down");
		wrefresh(game_window);

		napms((int) (1000 / fps));
	}

	main_loop();

}

int main(void) 
{

	/* Initalize ncurses */
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	nodelay(stdscr, TRUE);
	curs_set(0);
	refresh();

	/* Start Game */
	main_loop();

	return 0;
}
