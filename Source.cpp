#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>

#ifdef _MSC_VER 
#define _CRT_SECURE_NO_DEPRECATE 
#pragma warning (disable : 4996)
#endif


#include"SDL2-2.0.10/include/SDL.h"
#include"SDL2-2.0.10/include/SDL_main.h"




#define SCREEN_WIDTH	790
#define SCREEN_HEIGHT	803
#define FROG_WIDTH 58
#define FROG_HEIGHT 55
#define FRAME_TICK 0.05
#define ROWS 13
#define COLUMNS 13
#define WATERFINISH 0
#define WATERSTART 6
#define FINISH_HEIGHT 325
#define FINISH_SLOTS 5
#define SUNK_TURTLEL 3
#define SUNK_TURTLER 7
#define LIVES 5






typedef enum {
	fUpJ, fRightJ, fDownJ, fLeftJ, fUp, fRight, fDown, fLeft
}frogAnims;
typedef enum {
	right, left, up, down
}uniOrient;

typedef enum {
	TheFrog, FastCar, SlowCar, LogShort, LogMed, LogLong, Croc, Turtle
}spriteType;

typedef enum {
	road, pavement, water, finish
}backgroundType;



typedef struct {//structure used for all objects/sprites
	int x, y;//center of the sprite
	SDL_Rect spriteRect, visibleRect;
	SDL_Surface* surface;
	bool inMotion, onWater;
	int dirSprites, animSprites, animFramesTotal, currAnimFrame, currSprite, totalSprites, speed, currentRow;
	spriteType name;
	uniOrient facingDir;
}Sprite;

typedef struct {//structure representing each row
	backgroundType background;
	uniOrient direction;
	spriteType spriteRowType;
	int spriteAmount, rowSpeed;
	Sprite* spriteArr;
	SDL_Rect field[COLUMNS];	
}Row;



void drawPixel(SDL_Surface* surface, int x, int y, Uint32 color);//function from template program, draws a single pixel
void drawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color);//function from template program, draws a line
void drawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset);//function from template program, draws a string to the screen
void drawSurface(SDL_Surface* screen, SDL_Surface* sprite, SDL_Rect* visibleRect, int x, int y, int spriteCount);// slightly modified template function, draws part of a given surface
void drawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor);//draws a rectangle
void changeOrientation(Sprite* givenSprite, SDL_Rect* SpriteRec, int direction, int height);//changes the "visible" part of the surface i.e. responsible for animations
void renderSprite(Sprite* givenSprite, int rowSpeed, uniOrient direction);//function that is respoinsible for managing movement and animation
void renderSpriteOnWater(Sprite* givenSprite, int rowSpeed, uniOrient direction);//funciton responsible for making the frog move with the given object it is on when on water
void initEverything();//sdl init
void initSettings(SDL_Window* window, SDL_Renderer* renderer);//more sdl initialisation
void genericInit(Sprite* givenSprite, spriteType type, Row givenRow, int crocChance);//initialisation for all sprites apart from the frog
bool frogAlive(Row* givenRow, Sprite* frog, bool filledSlots[], char filename[], double * worldTime, int * score);//checking whether the frog has not made a fatal move
void moveSprite(Sprite* givenSprite, int vX, int vY);//resposible for chanigng coordinates of a sprite
bool legalMove(Sprite* frog, uniOrient direction);//checking whether the given move will not move the frog out of bounds
bool checkLevelComplete(bool filledSlots[]);//checking if all the lilies are full
void drawFinishFrogs(SDL_Surface* screen, Sprite frog, bool filledSlots[]);//drawing the frogs on the lilies
SDL_Surface* getScreen();//returns main screen
SDL_Texture* getTexture(SDL_Renderer* renderer);//returns texture used throughout
void initFrog(Sprite* frog, char filename[]);//special initialisation for frog
bool gameOver(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Event* event, bool dead, int score);//displaying appropriate game over screen depending on whether the player ran out of lives or completed all available levels
bool menu(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Event* event);//displays a simple main menu
int getRectWidth(double worldTime, double maxTime);//determines the width of the timer bar



int main(int argc, char** argv) {
	int t1, t2, quit, frames, rc, frogLives, levelCounter = 1, maxTime, crocChance, score, highestRow;
	double delta, worldTime, fpsTimer, fps, animTime;
	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Surface* background;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	Sprite frog;
	FILE* file;
	bool filledSlots[5], completed, hardQuit, paused;
	char filename[17];	
	srand(time(NULL));
	//all needed SDL features are created and the background and charset are loaded as well as the frog
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};
	screen = getScreen();
	scrtex = getTexture(renderer);
	charset = SDL_LoadBMP_RW(SDL_RWFromFile("sprites/cs8x8.bmp", "rb"), 1);
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	background = SDL_LoadBMP_RW(SDL_RWFromFile("sprites/background.bmp", "rb"), 1);
	if (background == NULL) {
		printf("SDL_LoadBMP(background.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	frog.surface = SDL_LoadBMP_RW(SDL_RWFromFile("sprites/lefrog.bmp", "rb"), 1);
	if (frog.surface == NULL) {
		printf("SDL_LoadBMP(lefrog.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(background);
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);
	int black = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int green = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int red = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int blue = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	paused = false;
	hardQuit = false;	
	score = 0;
	while (!hardQuit)//main outer loop of program, when this loop ends the program closes
	{	
		
		snprintf(filename, 15, "saves/lvl%d.txt", levelCounter);		//opening next level		
		file = fopen(filename, "r");
		if (file == NULL) {//if no more levels, gameover is displayed
			hardQuit=gameOver(screen, renderer, charset, scrtex, &event, false,score);			
			levelCounter = 1;
			score = 0;
			if (hardQuit)
				break;
			else//if user has opted to replay, level 1 is loaded again
			{				
				snprintf(filename, 15, "saves/lvl%d.txt", levelCounter);				
				file = fopen(filename, "r");
			}
		}
		hardQuit = menu(screen, renderer, charset, scrtex, &event);
		if (hardQuit)
			break;//the user can opt to quit the program without ever playing the game
		else
			paused = false;
		for (int i = 0; i < FINISH_SLOTS; i++)
		{
			filledSlots[i] = false;//lily pads are cleared
		}				
		frogLives = LIVES;//lives are reset
		Row* boardRows = (Row*)malloc(sizeof(Row) * ROWS);//malloc for each row of the board
		if (boardRows == NULL)
		{
			printf("Malloc failed! \n");
			SDL_FreeSurface(frog.surface);
			SDL_FreeSurface(screen);
			SDL_DestroyTexture(scrtex);
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			SDL_Quit();
			return 0;
		}
		else
		{//frog coordinates are skipped from the start of the file, as they are only needed in initfrog and not here.
			char skiptemp[100];
			fgets(skiptemp, 100, file);//allowed time and 'bonus chance' i.e. crocodile chance are given at the top of the file
			if (!fscanf(file, "%u",&maxTime))
			{
				printf("error");
				break;
			}
			if (!fscanf(file, "%u", &(crocChance)))
			{
				printf("error");
				break;
			}
			for (int i = 0; i < ROWS; i++)
			{
				(boardRows + i)->spriteAmount = 0;

				if (i == 0)//rectangles are created for the top row in order to check for collisions with the lily pads.
				{//margin adjustment
					int rectX = 28, rectY = (SCREEN_HEIGHT - 6 - (FROG_HEIGHT * (ROWS - i)));
					for (int j = 0; j < COLUMNS; j++)
					{
						(boardRows + i)->field[j].w = FROG_WIDTH;
						(boardRows + i)->field[j].h = FROG_HEIGHT;
						(boardRows + i)->field[j].x = rectX;
						(boardRows + i)->field[j].y = rectY;
						rectX += (FROG_WIDTH - 2);//aesthetic adjustment
					}
				}
				switch (i)
				{
				case 0://row Type is assigned depending on the height of the row
					(boardRows + i)->background = finish;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					(boardRows + i)->background = water;
					break;
				case 6:
				case 12:
					(boardRows + i)->background = pavement;
					break;
				default:
					(boardRows + i)->background = road;
				}
				if ((i > 0 && i < 6) || (i > 6 && i < 12))//in order: each row is assigned features from the text file
				{
					if (!fscanf(file, "%u", &((boardRows + i)->spriteRowType)))
					{
						printf("error");
						break;
					}
					if (!fscanf(file, "%u", &((boardRows + i)->direction)))
					{
						printf("error");
						break;
					}
					if (!fscanf(file, "%u", &((boardRows + i)->spriteAmount)))
					{
						printf("error");
						break;
					}
					if (!fscanf(file, "%u", &((boardRows + i)->rowSpeed)))
					{
						printf("error");
						break;
					}					
					(boardRows + i)->spriteArr = (Sprite*)malloc(sizeof(Sprite) * (boardRows + i)->spriteAmount);
					for (int j = 0; j < (boardRows + i)->spriteAmount; j++)
					{
						if (!fscanf(file, "%u", &((boardRows + i)->spriteArr[j].x)))//each sprite in each row is assigned coordinates and initialised
						{
							printf("error");
							break;
						}
						(boardRows + i)->spriteArr[j].y = SCREEN_HEIGHT - 6 - (FROG_HEIGHT * (ROWS - i)) + (FROG_HEIGHT / 2);
						(boardRows + i)->spriteArr[j].currentRow = i;
						genericInit(&((boardRows + i)->spriteArr[j]), (boardRows + i)->spriteRowType, *(boardRows + i),crocChance);
					}
				}
			}
		}
		
		fclose(file);
				
		initFrog(&frog,filename);//frog is initialised
		char text[128];
		animTime = t1 = SDL_GetTicks();
		frames = 0;
		quit = 0;
		fpsTimer = 0;
		fps = 0;
		worldTime = 0;
		completed = false;	
		bool quitConfirm;
		quitConfirm = false;
		highestRow = 12;
		while (!quit && !completed && frogLives > 0)//main game loop, can be exited without program switching off
		{
					
			while (SDL_PollEvent(&event)) {//polling event loop
				switch (event.type) {
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{					
					case (SDLK_p):
						if (paused == true)
							paused = false;
						else paused = true;
						break;
					case (SDLK_y):
						if (quitConfirm == true && paused==true)
							quit = true;
						break;
					case (SDLK_n):
						if (quitConfirm == true && paused == true)
							quitConfirm = false;
						break;
					case (SDLK_q):
						if(paused == true)																	
							quitConfirm = true;					
						break;
					case (SDLK_UP):
						if (legalMove(&frog, up) && paused == false)
						{
							frog.currentRow--;
							if (frog.currentRow < highestRow)
							{
								highestRow--;
								score += 10;
							}
							frog.inMotion = true;
							changeOrientation(&frog, &frog.visibleRect, fUp, FROG_HEIGHT);
						}
						break;
					case (SDLK_DOWN):
						if (legalMove(&frog, down) && paused == false)
						{
							frog.currentRow++;							
							frog.inMotion = true;
							changeOrientation(&frog, &frog.visibleRect, fDown, FROG_HEIGHT);
						}
						break;
					case (SDLK_LEFT):
						if (legalMove(&frog, left) && paused == false)
						{
							frog.inMotion = true;
							changeOrientation(&frog, &frog.visibleRect, fLeft, FROG_HEIGHT);
						}
						break;
					case  (SDLK_RIGHT):
						if (legalMove(&frog, right) && paused == false)
						{
							frog.inMotion = true;
							changeOrientation(&frog, &frog.visibleRect, fRight, FROG_HEIGHT);
						}
						break;
					}
					break;
				case SDL_KEYUP:
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};			
			if (!paused)
			{
				completed = checkLevelComplete(filledSlots);
				if (completed)
				{
					levelCounter++;
				}
				t2 = SDL_GetTicks();
				delta = ((double)t2 - (double)t1) * 0.001;
				t1 = t2;
				worldTime += delta;				


				drawSurface(screen, background, NULL, SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 20, 1);
				drawFinishFrogs(screen, frog, filledSlots);
				if ((t2 * 0.001) - (animTime * 0.001) > FRAME_TICK)
				{
					if (frog.inMotion)//if frog is mid move, its animation is finished
						renderSprite(&frog, (boardRows + frog.currentRow)->rowSpeed, (boardRows + frog.currentRow)->direction);
					else if (frog.onWater)
						renderSpriteOnWater(&frog, (boardRows + frog.currentRow)->rowSpeed, (boardRows + frog.currentRow)->direction);// frog is moved along with log/turtle/crocodile if on water and not mid animation
					for (int i = 0; i < ROWS; i++)
					{
						for (int j = 0; j < (boardRows + i)->spriteAmount; j++)
						{							
							if ((boardRows + i)->spriteArr[j].inMotion == false)
							{
								(boardRows + i)->spriteArr[j].inMotion = true;
							}
							renderSprite(&(boardRows + i)->spriteArr[j], NULL, up);//all other objects are moved/rendered
						}
					}
					animTime = SDL_GetTicks();
				}
				if (maxTime - worldTime < 0)//if timer runs out, it counts as a death
				{
					frogLives--;
					initFrog(&frog, filename);
					worldTime = 0;
				}
				if (!frog.inMotion)
				{
					if (frogAlive((boardRows + frog.currentRow), &frog, filledSlots,filename,&worldTime,&score) == false)//checking if frog has had an accident and resetting if that is the case
					{
						highestRow = 12;
						frogLives--;
						initFrog(&frog,filename);
					}
				}
				for (int i = 0; i < ROWS; i++)//drawing sprites
				{
					for (int j = 0; j < (boardRows + i)->spriteAmount; j++)
					{
						drawSurface(screen, (boardRows + i)->spriteArr[j].surface, &((boardRows + i)->spriteArr[j].visibleRect), (boardRows + i)->spriteArr[j].x, (boardRows + i)->spriteArr[j].y, (boardRows + i)->spriteArr[j].totalSprites);
					}
				}
				drawSurface(screen, frog.surface, &frog.visibleRect, frog.x, frog.y, frog.totalSprites);//drawing frog
				fpsTimer += delta;
				if (fpsTimer > 0.5)
				{
					fps = double(frames) * 2;
					frames = 0;
					fpsTimer -= 0.5;//displaying fps
				}
				drawRectangle(screen, 0, 0, SCREEN_WIDTH, 40, blue,blue); //displaying information such as lives, time left and fps
				sprintf(text, "Level %u . fps: %.0lf time: ",levelCounter, fps);				
				drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);				
				if (maxTime-worldTime<10)
				{
					drawRectangle(screen, 490, 10, getRectWidth(worldTime, maxTime), 10, black, red);					
				}
				else
					drawRectangle(screen, 490, 10, getRectWidth(worldTime, maxTime), 10, black, green);
				sprintf(text, "p - pause, \030 - move up, \031 - move down \032 - move left \033 - move right");
				drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
				sprintf(text, "Lives left: %u", frogLives);
				drawString(screen, screen->w - strlen(text) * 8, SCREEN_HEIGHT - 10, text, charset);
				sprintf(text, "Score: %u", score);
				drawString(screen, 50, SCREEN_HEIGHT - 10, text, charset);
				SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
				SDL_RenderCopy(renderer, scrtex, NULL, NULL);
				SDL_RenderPresent(renderer);
				frames++;
			}
			else//game paused loop; allows the user to quit and unpause
			{
				drawRectangle(screen, SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2, 400, 100, green, black);
				if (quitConfirm == true)
				{
					sprintf(text, "QUIT? Y/N");
					drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 + 30, text, charset);
				}				
				sprintf(text, "Paused. Press p to resume, or q to quit ");
				drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2+10, text, charset);
				SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
				SDL_RenderCopy(renderer, scrtex, NULL, NULL);
				SDL_RenderPresent(renderer);
			}
		}
		if (frogLives == 0)
		{
			hardQuit = gameOver(screen, renderer, charset, scrtex, &event, true,score);//user can choose to quit program or go to main menu
		}		
		for (int i = 0; i < ROWS; i++)
		{
			if ((i > 0 && i < 6) || (i > 6 && i < 12))
			{
				if ((boardRows + i) != NULL)
				{
					for (int j = 0; j < (boardRows + i)->spriteAmount; j++)//each bitmap in each row is freed in order to ensure no memory leak
						SDL_FreeSurface((boardRows + i)->spriteArr[j].surface);
					free((boardRows + i)->spriteArr);
				}
			}
		}
		free(boardRows);
	}	
	SDL_FreeSurface(frog.surface);//all other items are cleared
	SDL_FreeSurface(background);
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	return 0;
}



void renderSprite(Sprite* givenSprite, int rowSpeed, uniOrient direction)
{
	double animTime;
	int frameCount = 0, delta = 0, orientation, vX = 0, vY = 0;
	frameCount = givenSprite->animFramesTotal;
	animTime = SDL_GetTicks();
	animTime *= 0.001;
	switch (givenSprite->name)
	{
	case TheFrog://the frog has a different animation sprite order in its bitmap to other sprites and therefore needs its own eum
		delta = ((givenSprite->visibleRect.h) - ((givenSprite->visibleRect.h) % frameCount)) / frameCount;//the amount the frog moves in each frame is determined by its height and the amount of frames it is assigned for each animation cycle
		switch (givenSprite->currSprite)
		{
		case fUpJ:
		case fUp:
			vY = -delta;
			vX = 0;
			break;
		case fDownJ:
		case fDown:
			vY = delta;
			vX = 0;
			break;
		case fLeftJ:
		case fLeft:
			vX = -delta;
			vY = 0;
			break;
		case fRightJ:
		case fRight:
			vX = delta;
			vY = 0;
			break;
		}
		break;
	default://all other sprites have a universal enum
		switch (givenSprite->facingDir)
		{
		case up:
			vY = -givenSprite->speed;
			vX = 0;
			break;
		case down:
			vY = givenSprite->speed;
			vX = 0;
			break;
		case left:
			vX = -givenSprite->speed;
			vY = 0;
			break;
		case right:
			vX = givenSprite->speed;
			vY = 0;
			break;
		}
		break;
	}
	moveSprite(givenSprite, vX, vY);//the sprite is moved the required amount of pixels;
	givenSprite->currAnimFrame--;//next animation frame is cycled to
	if (givenSprite->animSprites > 1)
	{
		if (givenSprite->name == TheFrog)//frog switches animations only twice as it has only 2 anim sprites, whilst all other sprites have 4, therefore needs its own animation handler
		{
			if ((givenSprite->currAnimFrame == 1 || givenSprite->currAnimFrame == frameCount - 1))
			{
				if ((givenSprite->currSprite - (givenSprite->totalSprites / 2)) >= 0 && (givenSprite->currSprite - (givenSprite->totalSprites / 2)) < givenSprite->totalSprites)
				{
					orientation = (givenSprite->currSprite - (givenSprite->totalSprites / 2));
				}
				else
					orientation = givenSprite->totalSprites + (givenSprite->currSprite - (givenSprite->totalSprites / 2));
				changeOrientation(givenSprite, &(givenSprite->visibleRect), orientation, givenSprite->visibleRect.h);
			}
		}
		else//all other sprite animations are universaly handled
		{
			if (givenSprite->currAnimFrame % (givenSprite->animFramesTotal / givenSprite->animSprites) == 0)
			{
				orientation = givenSprite->currSprite + 1;
				if (orientation == givenSprite->totalSprites || orientation == givenSprite->totalSprites / givenSprite->dirSprites)
					orientation -= givenSprite->totalSprites / givenSprite->dirSprites;
				changeOrientation(givenSprite, &(givenSprite->visibleRect), orientation, givenSprite->visibleRect.h);
			}
		}

	}
	if (givenSprite->currAnimFrame == 0)//if this is the last frame of an animation(or a sprite only has 1 anim frame ie no animation, the cycle is reset
	{
		if (givenSprite->name == TheFrog)
		{
			if (vY != 0)//if delta * frameCount did not equal the height(or width), the remainder is added here. frog is only thing that has to move in the set grid
			{
				givenSprite->y += (vY / delta) * (FROG_HEIGHT - (frameCount * delta));
			}
			if (vX != 0)
			{
				givenSprite->x += (vX / delta) * (FROG_WIDTH - (frameCount * delta));
			}
		}
		givenSprite->currAnimFrame = givenSprite->animFramesTotal;
		givenSprite->inMotion = false;
		if (givenSprite->currentRow > WATERFINISH&& givenSprite->currentRow < WATERSTART)
		{
			givenSprite->onWater = true;
		}
		else
		{
			givenSprite->onWater = false;
		}
	}
}



void renderSpriteOnWater(Sprite* givenSprite, int rowSpeed, uniOrient direction)//frog simply moves along with whatever object it is on with the same speed as the current row
{
	if (direction == left)
		moveSprite(givenSprite, -rowSpeed, NULL);
	else
		moveSprite(givenSprite, rowSpeed, NULL);
}



void changeOrientation(Sprite* givenSprite, SDL_Rect* spriteRec, int direction, int height)//the spriterect is responsible for which part of the surface is visible and draw to the screen
{
	givenSprite->currSprite = direction;
	spriteRec->y = height * direction;
}



void drawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};



void drawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		drawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};



void drawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};



void drawSurface(SDL_Surface* screen, SDL_Surface* sprite, SDL_Rect* visibleRect, int x, int y, int spriteCount) {
	SDL_Rect dest;
	if (visibleRect == NULL)
	{
		dest.x = x - sprite->w / 2;
		dest.y = y - sprite->h / 2;
		dest.w = sprite->w;
		dest.h = sprite->h;
		SDL_BlitSurface(sprite, NULL, screen, &dest);
	}
	else//if specified only part of a given surface is drawn
	{
		dest.x = x - sprite->w / 2;
		dest.y = y - sprite->h / (2 * spriteCount);
		dest.w = sprite->w;
		dest.h = sprite->h / spriteCount;
		SDL_BlitSurface(sprite, visibleRect, screen, &dest);

	}
};



void drawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	drawLine(screen, x, y, k, 0, 1, outlineColor);
	drawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	drawLine(screen, x, y, l, 1, 0, outlineColor);
	drawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		drawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};



void initEverything()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return;
	}
}



void initSettings(SDL_Window* window, SDL_Renderer* renderer)
{
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(window, "Frogger by Kacper Krzeminski");
	SDL_ShowCursor(SDL_DISABLE);
}



SDL_Surface* getScreen()
{
	return SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
}



SDL_Texture* getTexture(SDL_Renderer* renderer)
{
	return SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
}



void initFrog(Sprite* frog, char filename[])
{
	const int hitboxAdjust = 28, margyY = 6, margX = 18;
	frog->visibleRect.h = FROG_HEIGHT;
	frog->visibleRect.w = FROG_WIDTH;
	frog->visibleRect.x = 0;
	frog->visibleRect.y = 0;
	frog->dirSprites = 4;
	frog->animSprites = 4;
	frog->animFramesTotal = 5;
	frog->currSprite = 0;
	frog->inMotion = false;
	frog->name = TheFrog;
	frog->totalSprites = frog->dirSprites + frog->animSprites;
	frog->currAnimFrame = 5;	
	FILE* file;
	file=fopen(filename, "r");//frog x and y starting coordinates are read from the file
	if (file != NULL)
	{
		if (!fscanf(file, "%u", &(frog->x)))
		{
			printf("error");

		}
		if (!fscanf(file, "%u", &(frog->y)))
		{
			printf("error");
			return;
		}
		fclose(file);
	}
	else
	{
		printf("error");
		return;
	}
	frog->spriteRect.h = FROG_HEIGHT;//frog's hitbox is slightly smaller than its sprite in order to allow for more realistic collisions as sprites are smaller than their rectangles
	frog->spriteRect.w = FROG_WIDTH - hitboxAdjust;
	frog->spriteRect.x = frog->x - ((FROG_WIDTH - hitboxAdjust) / 2);
	frog->spriteRect.y = frog->y - (FROG_HEIGHT / 2);
	frog->currentRow = ROWS - 1;
	frog->onWater = false;
	//frog->waterTime = SDL_GetTicks();
	changeOrientation(frog, &frog->visibleRect, fUp, FROG_HEIGHT);
	SDL_SetColorKey(frog->surface, true, 0xFFFFFF);
}
void genericInit(Sprite* givenSprite, spriteType type, Row givenRow, int crocChance)
{		
	if ((type == LogShort || type == LogMed || type == LogLong) && crocChance!=0)
	{
		int temp = rand();
		if (temp % crocChance == 0)
		{
			type = Croc;
		}		
	}
	switch (type)
	{
	case Croc:
		givenSprite->surface = SDL_LoadBMP("sprites/croc.bmp");
		givenSprite->totalSprites = 2;
		givenSprite->animSprites = 1;
		givenSprite->animFramesTotal = 1;
		givenSprite->name = Croc;
		givenSprite->dirSprites = 2;
		givenSprite->facingDir = givenRow.direction;
		givenSprite->onWater = true;
		break;	
	case LogShort://big case statement loading all sprite bitmaps and settings depending on the rowType
		givenSprite->surface = SDL_LoadBMP("sprites/logShort.bmp");
		//(boardRows + i)->spriteArr[j].surface = SDL_LoadBMP_RW(SDL_RWFromFile("logShort.bmp", "rb"), 1);
		givenSprite->totalSprites = 1;
		givenSprite->animSprites = 1;
		givenSprite->dirSprites = 1;
		givenSprite->animFramesTotal = 1;
		givenSprite->name = LogShort;
		givenSprite->onWater = true;
		break;
	case Turtle:
		givenSprite->surface = SDL_LoadBMP("sprites/Turtle.bmp");
		givenSprite->totalSprites = 8;
		givenSprite->animSprites = 4;
		givenSprite->animFramesTotal = 64;
		givenSprite->name = Turtle;
		givenSprite->dirSprites = 2;
		givenSprite->facingDir = givenRow.direction;
		givenSprite->onWater = true;
		break;
	case LogMed:
		givenSprite->surface = SDL_LoadBMP("sprites/logMed.bmp");		
		givenSprite->totalSprites = 1;
		givenSprite->dirSprites = 1;
		givenSprite->animSprites = 1;
		givenSprite->animFramesTotal = 1;
		givenSprite->name = LogMed;
		givenSprite->onWater = true;
		break;
	case LogLong:
		givenSprite->surface = SDL_LoadBMP("sprites/logLong.bmp");		
		givenSprite->totalSprites = 1;
		givenSprite->animSprites = 1;
		givenSprite->dirSprites = 1;
		givenSprite->animFramesTotal = 1;
		givenSprite->name = LogLong;
		givenSprite->onWater = true;
		break;
	case SlowCar:
		givenSprite->surface = SDL_LoadBMP("sprites/SlowCar.bmp");		
		givenSprite->totalSprites = 8;
		givenSprite->animSprites = 4;
		givenSprite->animFramesTotal = 16;
		givenSprite->name = SlowCar;
		givenSprite->dirSprites = 2;
		givenSprite->onWater = false;
		break;
	case FastCar:
		givenSprite->surface = SDL_LoadBMP("sprites/FastCar.bmp");		
		givenSprite->totalSprites = 8;
		givenSprite->animSprites = 4;
		givenSprite->animFramesTotal = 16;
		givenSprite->name = FastCar;
		givenSprite->dirSprites = 2;
		givenSprite->onWater = false;
		break;
	}
	SDL_SetColorKey(givenSprite->surface, true, 0xFFFFFF);	
	givenSprite->speed = givenRow.rowSpeed;	
	givenSprite->currAnimFrame = givenSprite->animFramesTotal;
	givenSprite->inMotion = true;
	givenSprite->facingDir = givenRow.direction;	
	givenSprite->visibleRect.w = givenSprite->surface->w;
	givenSprite->visibleRect.h = FROG_HEIGHT;
	givenSprite->visibleRect.x = 0;
	givenSprite->spriteRect.h = FROG_HEIGHT;
	givenSprite->spriteRect.w = givenSprite->surface->w;	
	givenSprite->spriteRect.x = givenSprite->x - (givenSprite->spriteRect.w / 2);
	givenSprite->spriteRect.y = givenSprite->y - (givenSprite->spriteRect.h / 2);
	if (givenSprite->name == Croc)
	{
		givenSprite->spriteRect.w -= 124;
		if (givenSprite->facingDir == left)
			givenSprite->spriteRect.x += 124;
	}
	if (givenSprite->totalSprites == 1) {

		givenSprite->visibleRect.y = 0;
	}
	else
	{
		changeOrientation(givenSprite, &(givenSprite->visibleRect), (givenSprite->facingDir * givenSprite->animSprites), givenSprite->visibleRect.h);
	}
}
bool frogAlive(Row* givenRow, Sprite* frog, bool filledSlots[], char filename[], double * worldTime, int * score)
{
	SDL_Rect temp;
	switch (givenRow->background)
	{
	case water:
		for (int i = 0; i < givenRow->spriteAmount; i++)
		{
			temp = givenRow->spriteArr[i].spriteRect;
			temp.w -= 10;
			if (SDL_HasIntersection(&temp, &(frog->spriteRect)) && frog->x > 0 && frog->x < SCREEN_WIDTH)//log rectangles are slightly shrunk before checking for collisions to allow for more realistic collisions
			{
				if (givenRow->spriteRowType == Turtle)
				{
					if (givenRow->spriteArr[i].currSprite == SUNK_TURTLEL || givenRow->spriteArr[i].currSprite == SUNK_TURTLER)//checking if the turtle is currently in its sunk state ie the frog hasjumped on turtle that is submerged and died
						return false;
				}
				return true;
			}
		}
		return false;
		break;
	case road:

		for (int i = 0; i < givenRow->spriteAmount; i++)
		{
			if (SDL_HasIntersection(&(givenRow->spriteArr[i].spriteRect), &(frog->spriteRect)))//checking if the frog has hit a car
			{
				return false;
			}
		}
		break;
	case finish:
		for (int i = 0; i < FINISH_SLOTS; i++)
		{
			if (givenRow->field[3 * i].x <= frog->spriteRect.x + ((frog->spriteRect.w) / 2) && givenRow->field[3 * i].x + givenRow->field[3 * i].w >= frog->spriteRect.x + frog->spriteRect.w - ((frog->spriteRect.w) / 2) && filledSlots[i] == false)
			{//checking whether the frog has collided with a lily pad slot, these hitboxes are quite generous and a bit larger than the visible slot
				filledSlots[i] = true;
				initFrog(frog, filename);//frog is put back to the start
				*score += 50;
				*score += (10 * *worldTime);
				*worldTime = 0;				
				return true;
			}
		}
		return false;
		break;
	}
	return true;
}



void moveSprite(Sprite* givenSprite, int vX, int vY)//sprite is moved by given parameters
{
	if (((givenSprite->x + vX) < SCREEN_WIDTH * 2) && ((givenSprite->x + vX) > -(givenSprite->surface->w)))
	{
		givenSprite->x += vX;
		givenSprite->spriteRect.x += vX;
	}
	else if ((givenSprite->x + vX) >= SCREEN_WIDTH * 2)
	{
		givenSprite->x = -(givenSprite->surface->w);
		givenSprite->spriteRect.x = givenSprite->x - (givenSprite->spriteRect.w / 2);
	}
	else
	{
		givenSprite->x = SCREEN_WIDTH * 2;
		givenSprite->spriteRect.x = givenSprite->x - (givenSprite->spriteRect.w / 2);
	}
	givenSprite->y += vY;
	givenSprite->spriteRect.y += vY;
}



bool legalMove(Sprite* frog, uniOrient direction)
{
	const int yMarg = 6, xMarg = 18;
	switch (direction)
	{
	case up:
		if (!frog->inMotion)//all moves up are legal as the frog cannot go past the lily pad row and therefore either lands on a lily pad or dies
			return true;
		break;
	case down:
		if (!frog->inMotion && (frog->y < SCREEN_HEIGHT - (FROG_HEIGHT / 2) - yMarg))//frog cannot move below the bottom of the screen
			return true;
		break;
	case left:
		if (frog->inMotion == false)//frog cannot jump out of the screen left or right
			if (frog->x > (xMarg + FROG_WIDTH / 2))
				return true;
		break;
	case right:
		if (frog->inMotion == false)
			if (frog->x < (xMarg + FROG_WIDTH / 2) + (FROG_WIDTH * 12))
				return true;
		break;
	}
	return false;
}



bool checkLevelComplete(bool filledSlots[])//if all lilies are occupied level is complete
{
	for (int i = 0; i < FINISH_SLOTS; i++)
	{
		if (filledSlots[i] == false)
			return false;
	}
	return true;
}



void drawFinishFrogs(SDL_Surface* screen, Sprite frog, bool filledSlots[])//lily pad frogs are drawn
{
	SDL_Rect  dest;
	dest.w = FROG_WIDTH;
	dest.h = FROG_HEIGHT;
	dest.x = 0;
	dest.y = FINISH_HEIGHT;
	const int xMarg = 30, dx = -2, yMarg = 40, dy = FROG_HEIGHT;//used to perfect position of finish sprites;

	for (int i = 0; i < FINISH_SLOTS; i++)
	{
		if (filledSlots[i] == true)
			drawSurface(screen, frog.surface, &dest, xMarg + (FROG_WIDTH / 2) + (3 * i * (FROG_WIDTH + dx)), yMarg + dy, frog.totalSprites);
	}
}



bool menu(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Event* event)
{
	char text[128];
	while (true)
	{
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x22, 0x00));		
		sprintf(text, "Frogger by Kacper Krzeminski");
		drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		sprintf(text, "Press a to play next level or q to exit the game");
		drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 300, text, charset);
		sprintf(text, "use arrow keys to navigate 5 frogs to the lilies at the end. You have 3 lives.");
		drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 350, text, charset);
		sprintf(text, "Beware of the timer. Press p to pause at any time.");
		drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 360, text, charset);
		while (SDL_PollEvent(event))
		{
			switch (event->type)
			{
			case SDL_KEYDOWN:
				switch (event->key.keysym.sym)
				{
				case (SDLK_a):
					return false;
					break;
				case (SDLK_q):
					return true;
					break;
				}
				break;
			}

		};
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
	return true;
}



int getRectWidth(double worldTime, double maxTime)
{
	return ((maxTime - worldTime) * 4);
}



bool gameOver(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Event* event, bool dead, int score)
{
	char text[128];
	while (true)//looped until user chooses an option
	{
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x66, 0x00));//message is chosen depending on how the user got to this screen
		if (!dead)
			sprintf(text, "Game Over: all levels complete!");			
		else
			sprintf(text, "Game Over: out of lives!");
		drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		sprintf(text, "Score: %u",score);
		drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20, text, charset);
		sprintf(text, "Press a to go to main menu or q to exit the game");
		drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 300, text, charset);
		while (SDL_PollEvent(event))
		{
			switch (event->type)
			{
			case SDL_KEYDOWN:
				switch (event->key.keysym.sym)
				{
				case (SDLK_a):
					return false;
					break;
				case (SDLK_q):
					return true;
					break;
				}
				break;
			}

		};
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
	return true;
}
