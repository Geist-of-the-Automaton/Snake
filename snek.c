// Copyright 2019 Auden Childress

#include "snek.h"
#include <time.h>
#include <stdlib.h>

int main() 
{
	REG_DISPLAY = VIDEOMODE | BGMODE;
	aliveColor = makeColor(0x13, 0x18, 0x05);
	nomColor = makeColor(0x15, 0x1b, 0x08);
	foodColor = makeColor(0x1f, 0x1f, 0x14);
	deadColor = makeColor(0x12, 0x07, 0);
	tongueColor = makeColor(0x1f, 0, 0);
	white = makeColor(0x1f, 0x1f, 0x1f);
	black = 0;
	srand(time(NULL));
	while (1) 
	{
		titleScreen();
		mainGame();
	}
}

void titleScreen()
{
	odo = 0;
	food.y = 15;
	// draw snek title to screen
	for (food.x = 0; food.x < WIDTH; ++food.x)
		drawOther(food, makeColor(31 - food.y, 31, food.x + 6), 0);
	for (food.y = 0; food.y < 5; ++food.y) 
	{
		for (food.x = 0; food.x < WIDTH; ++food.x) 
		{
			drawOther(food, makeColor(31 - food.y, 31, food.x + 6), 0);
			food.y = 14 - food.y;
			drawOther(food, makeColor(31 - food.y, 31, food.x + 6), 0);
			food.y = 14 - food.y;
		}
		sync();
	}
	for (food.x = 7; food.x < 18; food.x += 2) 
	{
		if (food.x == 11 || food.x == 15)
			food.x += 2;
		for (food.y = 6; food.y < 9; ++food.y)
			drawOther(food, aliveColor, 0);
	}
	for (uint8 y = 0; y < 3 * TILE_SIZE; ++y)
	{
		for (uint8 x = 0; (y < 7 || (y > 11 && y < 19) || y > 22) && x < 2 * TILE_SIZE; ++x)
			SCREENBUFFER[(6 * TILE_SIZE + y) * SCREEN_WIDTH + 14 * TILE_SIZE + x] = aliveColor;
	}
	for (uint8 y = TILE_SIZE * 6; y < TILE_SIZE * 9; ++y)
		for (uint8 x = TILE_SIZE / 2; x < TILE_SIZE; ++x)
			SCREENBUFFER[y * SCREEN_WIDTH + 4 * TILE_SIZE + y / 2 + x] = aliveColor;
	for (uint8 y = TILE_SIZE * 7; y > TILE_SIZE * 6; --y)
		for (uint8 x = TILE_SIZE / 2; x < 4 * TILE_SIZE / 3; ++x)
			SCREENBUFFER[y * SCREEN_WIDTH + 21 * TILE_SIZE - y / 2 + x] = aliveColor;
	for (uint8 y = TILE_SIZE * 7; y < TILE_SIZE * 9; ++y)
		for (uint8 x = TILE_SIZE / 2; x < 4 * TILE_SIZE / 3; ++x)
			SCREENBUFFER[y * SCREEN_WIDTH + 14 * TILE_SIZE + y / 2 + x] = aliveColor;
	for (uint8 y = 6; y < 10; y += 3)
		for (uint8 x = 0; x < SCREEN_WIDTH; ++x)
			SCREENBUFFER[(y * TILE_SIZE - (y == 9)) * SCREEN_WIDTH + x] = black;
	food.x = 11;
	food.y = 7;
	// draw first food
	drawOther(food, foodColor, 1);
	// draw the snek moving into the position of 'S'
	length = 6;
	dead = 0;
	dir = RIGHT;
	justAte = 0;
	for (uint16 i = 0; i < length; ++i) 
	{
		snek[i].y = 8;
		snek[i].x = 0;
	}
	drawSnek();
	uint16 dirs[] = { RIGHT, RIGHT, RIGHT, RIGHT, RIGHT, UP, LEFT, UP, RIGHT };
	for (uint8 i = 0; i < 9; ++i) 
	{
		sync();
		lastDir = dir;
		drawHead(aliveColor);
		if (i > 4)
			drawOther(snek[length - 1], black, 0);
		for (uint16 j = length - 1; j > 0; --j)
			snek[j] = snek[j - 1];
		dir = dirs[i];
		drawSegment(nomColor);
		if (dir == DOWN)
			++snek[0].y;
		else if (dir == UP)
			--snek[0].y;
		else if (dir == LEFT)
			--snek[0].x;
		else if (dir == RIGHT)
			++snek[0].x;
		if (i > 5)
			drawTail();
		drawSnek();
	}
	lastDir = dir;
	// wait for input
	while ((REG_KEY_INPUT & UP) && (REG_KEY_INPUT & RIGHT));
	// clear screen of border and logo
	for (food.y = 0; food.y < HEIGHT; ++food.y)
		for (food.x = 0; (food.y < 6 || food.y > 8) && food.x < WIDTH; ++food.x)
			drawOther(food, black, 0);
	for (food.x = 7; food.x < 22; ++food.x) 
	{
		if (food.x == 10)
			food.x += 3;
		for (food.y = 6; food.y < 9; ++food.y)
			drawOther(food, black, 0);
	}
	// reset modified vars
	food.x = 11;
	food.y = 7;
	odo = 0;
}

void mainGame() 
{
	while (!dead) 
	{
		getInput();
		if (!isDead()) 
		{
			moveSnek();
			hasEaten();
		}
	}
}

void sync() 
{
	for (uint8 i = 0; i < 10; ++i) 
	{
		while (REG_DISPLAY_VCOUNT >= 160);
		while (REG_DISPLAY_VCOUNT < 160);
	}
}

void getInput() 
{
	uint16 tempDir = lastDir = dir;
	for (uint8 i = 0; i < 10; ++i) 
	{
		while (REG_DISPLAY_VCOUNT >= 160);
		while (REG_DISPLAY_VCOUNT < 160);
		// drawing the food slowly appearing
		if (popup)
			drawOther(food, foodColor, 5 - (i / 4 + 2 * (1 - justAte)));
		/*
		allow input over the frame wait. A temp direction is used as the user can pick an adjcent direction to the snake's current
		movement then select the opposite direction of the initial direction, thus killing themself.
		*/
		if (!(REG_KEY_INPUT & DOWN) && dir != UP)
			tempDir = DOWN;
		if (!(REG_KEY_INPUT & UP) && dir != DOWN)
			tempDir = UP;
		if (!(REG_KEY_INPUT & LEFT) && dir != RIGHT)
			tempDir = LEFT;
		if (!(REG_KEY_INPUT & RIGHT) && dir != LEFT)
			tempDir = RIGHT;
	}
	if (popup)
		--popup;
	// pause was not included in the loop as it would cause extraordinary delay times
	if (!(REG_KEY_INPUT & START))
		pause();
	drawHead(justAte ? nomColor : aliveColor);
	if (dir != tempDir)
		drawTongue(black);
	dir = tempDir;
	drawSegment(nomColor);
}

void pause()
{
	for (uint8 i = 0; i < 5; ++i)
		sync();
	while (1)
		if (!(REG_KEY_INPUT & START))
			break;
}

void moveSnek() 
{
	++odo;
	odo %= 21;
	drawOther(snek[length - 1], black, 0);
	for (uint16 i = length - 1; i > 0; --i)
		snek[i] = snek[i - 1];
	if (dir == DOWN)
		++snek[0].y;
	else if (dir == UP)
		--snek[0].y;
	else if (dir == LEFT)
		--snek[0].x;
	else if (dir == RIGHT)
		++snek[0].x;
	drawTail();
}

void hasEaten() 
{
	if (snek[0].x == food.x && snek[0].y == food.y) 
	{
		// needs to be check for if you are the longest you can be. if so move snek forward one to appear as though it go longer and display win screen
		snek[length] = snek[length - 1];
		++length;
		uint8 flag = 1;
		// sets a random height and width and progresses forward until the food is not on top of the snake
		for (uint8 Y = rand(); flag; ++Y)
			for (uint8 X = rand(); flag; ++X) 
			{
				Y %= HEIGHT - 1;
				X %= WIDTH - 1;
				for (uint16 i = 0; i < length; ++i)
					if (X == snek[i].x && Y == snek[i].y) 
					{
						flag = 0;
						break;
					}
				if (flag) 
				{
					flag = 0;
					food.x = X;
					food.y = Y;
				}
				else
					flag = 1;
			}
		justAte = 1;
		popup = 2;
		drawOther(snek[0], nomColor, 0);
		// draw the eyes farther apart than normal - snakes stretch they jaw when eating and this attempts to aesthetic flavor by visualizing that.
		if (dir == UP || dir == DOWN) 
		{
			SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2 + 2] = 0;
			SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2 - 3] = 0;
		}
		else 
		{
			SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2 + 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2] = 0;
			SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2 - 3) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2] = 0;
		}
	}
	else
		drawSnek();
}

uint8 isDead() 
{
	if (hitWall() || hitSelf()) 
	{
		drawTongue(black);
		drawOther(food, 0, 0);
		dead = 1;
		sync();
		for (uint16 i = 0; i < length; ++i)
			drawOther(snek[i], i % 2 == 0 ? deadColor & nomColor : deadColor, 0);
		for (uint8 i = 0; i < 15; ++i)
			sync();
		// make the snake fade out like an old-school rpg boss
		fadeTileArray(snek, length, black);
		for (uint8 i = 0; i < 5; ++i)
			sync();
	}
	return dead;
}

uint8 hitWall() 
{
	return (dir == DOWN && snek[0].y == HEIGHT - 1) || (dir == UP && snek[0].y == 0) || (dir == LEFT && snek[0].x == 0) || (dir == RIGHT && snek[0].x == WIDTH - 1);
}

uint8 hitSelf() 
{
	for (uint16 i = 1; i < length; ++i)
		if (snek[0].x == snek[i].x && snek[0].y == snek[i].y)
			return 1;
	return 0;
}

// ALL METHODS BELOW ARE GRAPHICAL

uint16 makeColor(uint8 r, uint8 g, uint8 b) 
{
	return (r & 0x1f) | ((g & 0x1f) << 5) | ((b & 0x1f) << 10);
}

void drawSnek() 
{
	int xStart = 0, yStart = 0;
	int xEnd = TILE_SIZE, yEnd = TILE_SIZE;
	if (dir != DOWN)
		++yStart;
	if (dir != RIGHT)
		++xStart;
	if (dir != UP)
		--yEnd;
	if (dir != LEFT)
		--xEnd;
	int Y = snek[0].y * TILE_SIZE;
	int X = snek[0].x * TILE_SIZE;
	for (uint8 y = yStart; y < yEnd; ++y)
		for (uint8 x = xStart; x < xEnd; ++x)
			SCREENBUFFER[(Y + y) * SCREEN_WIDTH + X + x] = justAte ? nomColor : aliveColor;
	drawHead(black);
	drawTongue(tongueColor);
}

void drawOther(struct Rect r, uint16 color, uint8 border) 
{
	for (uint8 y = border; y < TILE_SIZE - border; ++y)
		for (uint8 x = border; x < TILE_SIZE - border; ++x)
			SCREENBUFFER[(r.y * TILE_SIZE + y) * SCREEN_WIDTH + r.x * TILE_SIZE + x] = color;
}

void drawHead(uint16 color)
{
	// curve the head and draw the eyes
	if (dir == DOWN) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2 + (1 + justAte)] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2 - (2 + justAte)] = color;
	}
	else if (dir == LEFT) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2 + (1 + justAte)) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2 - (2 + justAte)) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2] = color;
	}
	else if (dir == UP) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2 + (1 + justAte)] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2 - (2 + justAte)] = color;
	}
	else 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2 + (1 + justAte)) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2] = color;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE / 2 - (2 + justAte)) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE / 2] = color;
	}
}

void drawTongue(uint16 color) 
{
	if (odo == 20) 
	{
		if (snek[0].y != HEIGHT - 1 && snek[0].y != 0 && snek[0].x != 0 && snek[0].x != WIDTH - 1 && !justAte) 
		{
			uint8 height = dir == LEFT || dir == RIGHT ? 2 : TILE_SIZE / 2;
			uint8 width = height == 2 ? TILE_SIZE / 2 : 2;
			uint8 x, y;
			if (dir == UP) 
			{
				y = snek[0].y - 1;
				x = snek[0].x;
			}
			else if (dir == DOWN) 
			{
				y = snek[0].y + 1;
				x = snek[0].x;
			}
			else if (dir == LEFT) 
			{
				y = snek[0].y;
				x = snek[0].x - 1;
			}
			else 
			{
				y = snek[0].y;
				x = snek[0].x + 1;
			}
			short yOffset = (TILE_SIZE / 2 - 1) + (dir == UP ? 2 : (dir == DOWN ? -5 : 0));
			short xOffset = (TILE_SIZE / 2 - 1) + (dir == LEFT ? 2 : (dir == RIGHT ? -5 : 0));
			for (uint8 Y = 0; Y < height; ++Y)
				for (uint8 X = 0; X < width; ++X)
					SCREENBUFFER[(y * TILE_SIZE + yOffset + Y) * SCREEN_WIDTH + x * TILE_SIZE + xOffset + X] = color;
		}
	}
}

void drawSegment(uint16 color) 
{
	if (justAte) 
	{
		if (dir != lastDir)
		{
			// curve body when changing direction
			if ((dir == DOWN && lastDir == RIGHT) || (dir == LEFT && lastDir == UP))
				SCREENBUFFER[(snek[0].y * TILE_SIZE) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 1] = black;
			else if ((dir == DOWN && lastDir == LEFT) || (dir == RIGHT && lastDir == UP))
				SCREENBUFFER[(snek[0].y * TILE_SIZE) * SCREEN_WIDTH + snek[0].x * TILE_SIZE] = black;
			else if ((dir == UP && lastDir == LEFT) || (dir == RIGHT && lastDir == DOWN))
				SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE] = black;
			else
				SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 1] = black;
		}
		justAte = 0;
	}
	else
	{
		if (dir == UP)
			drawUp(color);
		else if (dir == DOWN)
			drawDown(color);
		else if (dir == LEFT)
			drawLeft(color);
		else
			drawRight(color);
	}
}

void drawLeft(uint16 color) 
{
	// curve body when changing direction
	for (uint8 y = 1; y < TILE_SIZE - 1; ++y)
		SCREENBUFFER[(snek[0].y * TILE_SIZE + y) * SCREEN_WIDTH + snek[0].x * TILE_SIZE] = color;
	if (lastDir == DOWN) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 3] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 3) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE) * SCREEN_WIDTH + snek[0].x * TILE_SIZE] = color;
	}
	else if (lastDir == UP) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 3] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE] = color;
	}
}

void drawRight(uint16 color) 
{
	// curve body when changing direction
	for (uint8 y = 1; y < TILE_SIZE - 1; ++y)
		SCREENBUFFER[(snek[0].y * TILE_SIZE + y) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 1] = color;
	if (lastDir == DOWN) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 3) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 1] = color;
	}
	else if (lastDir == UP) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 1] = color;
	}
}

void drawDown(uint16 color) 
{
	// curve body when changing direction
	for (uint8 x = 1; x < TILE_SIZE - 1; ++x)
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + x] = color;
	if (lastDir == RIGHT) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 3] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE] = color;
	}
	else if (lastDir == LEFT) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 1) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 1] = color;
	}
}

void drawUp(uint16 color) 
{
	// curve body when changing direction
	for (uint8 x = 1; x < TILE_SIZE - 1; ++x)
		SCREENBUFFER[(snek[0].y * TILE_SIZE) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + x] = color;
	if (lastDir == RIGHT) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 3] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 3) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE) * SCREEN_WIDTH + snek[0].x * TILE_SIZE] = color;
	}
	else if (lastDir == LEFT) 
	{
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 2) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 2] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE + TILE_SIZE - 3) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + 1] = black;
		SCREENBUFFER[(snek[0].y * TILE_SIZE) * SCREEN_WIDTH + snek[0].x * TILE_SIZE + TILE_SIZE - 1] = color;
	}
}

void drawTail()
{
	if (snek[length - 1].x > snek[length - 2].x)
	{
		drawQ1();
		drawQ4();
	}
	else if (snek[length - 1].x < snek[length - 2].x)
	{
		drawQ2();
		drawQ3();
	}
	else if (snek[length - 1].y > snek[length - 2].y)
	{
		drawQ3();
		drawQ4();
	}
	else
	{
		drawQ1();
		drawQ2();
	}
}

void drawQ1() 
{
	// draw a inverted triangle in the first quadrant of the tile
	uint8 i = ((TILE_SIZE - 1) / 2);
	for (uint8 y = i; y > 0; --y)
		for (uint8 x = y; x > 0; --x)
			SCREENBUFFER[(snek[length - 1].y * TILE_SIZE + (i - y) - 1) * SCREEN_WIDTH + snek[length - 1].x * TILE_SIZE + TILE_SIZE - (1 + (y - x))] = black;
}

void drawQ2() 
{
	// draw a inverted triangle in the second quadrant of the tile
	uint8 i = ((TILE_SIZE - 1) / 2) - 1;
	for (uint8 y = i; y > 0; --y)
		for (uint8 x = y; x > 0; --x)
			SCREENBUFFER[(snek[length - 1].y * TILE_SIZE + (i - y)) * SCREEN_WIDTH + snek[length - 1].x * TILE_SIZE + (y - x)] = black;
}

void drawQ3() 
{
	// draw a inverted triangle in the third quadrant of the tile
	uint8 i = ((TILE_SIZE - 1) / 2) - 1;
	for (uint8 y = i; y > 0; --y)
		for (uint8 x = y; x > 0; --x)
			SCREENBUFFER[(snek[length - 1].y * TILE_SIZE + TILE_SIZE - (1 + (i - y))) * SCREEN_WIDTH + snek[length - 1].x * TILE_SIZE + (y - x)] = black;
}

void drawQ4() 
{
	// draw a inverted triangle in the fourth quadrant of the tile
	uint8 i = ((TILE_SIZE - 1) / 2);
	for (uint8 y = i; y > 0; --y)
		for (uint8 x = y; x > 0; --x)
			SCREENBUFFER[(snek[length - 1].y * TILE_SIZE + TILE_SIZE - (1 + (i - y))) * SCREEN_WIDTH + snek[length - 1].x * TILE_SIZE + TILE_SIZE - (y - x)] = black;
}

void fadeTileArray(struct Rect r[], uint16 len, uint16 toColor) 
{
	// fades tiles given in an array. can be used for columns and rows if wanted
	uint8 k = 0;
	for (short i = len - 1; i >= -(TILE_SIZE / 2); --i) 
	{
		for (uint8 j = TILE_SIZE / 2; j > 0; --j)
		{
			++k;
			uint8 c = 2 * (j - 1);
			if (i >= 0 && i < len)
				fade(r[i], j, toColor | makeColor(c, c, c));
			++i;
		}
		sync();
		i -= k;
		k = 0;
	}
}

void fade(struct Rect r, uint8 density, uint16 toColor) 
{
	// fades a tiles by a pixel density
	for (uint8 y = 0; y < TILE_SIZE; ++y)
		for (uint8 x = 0; x < TILE_SIZE; ++x) 
		{
			uint16 pos = (r.y * TILE_SIZE + y) * SCREEN_WIDTH + r.x * TILE_SIZE + x;
			if (!(y % density || x % density))
				SCREENBUFFER[pos] = SCREENBUFFER[pos] & toColor;
		}
}

// UNUSED

void invert(struct Rect r) {
	for (int y = 0; y < TILE_SIZE; ++y)
		for (int x = 0; x < TILE_SIZE; ++x) {
			uint32 pos = (r.y * TILE_SIZE + y) * SCREEN_WIDTH + r.x * TILE_SIZE + x;
			SCREENBUFFER[pos] = white - SCREENBUFFER[pos];
		}
}
