//Header file
#define SCREEN_WIDTH	240
#define SCREEN_HEIGHT	160

typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;

#define MEM_IO			0x04000000
#define VRAM			0x06000000

#define VIDEOMODE		0x0003
#define BGMODE			0x0400

#define REG_DISPLAY			(*(volatile uint32 *) (MEM_IO))
#define REG_DISPLAY_VCOUNT	(*(volatile uint32 *) (MEM_IO + 0x0006))
#define REG_KEY_INPUT		(*(volatile uint32 *) (MEM_IO + 0x0130))

#define A		1
#define B		2
#define SELECT	4
#define START	8
#define RIGHT	16
#define LEFT	32
#define UP		64
#define DOWN	128
#define R 		256
#define L 		512

#define SCREENBUFFER	((volatile uint16 *) VRAM)

#define TILE_SIZE	10
#define HEIGHT		SCREEN_HEIGHT / TILE_SIZE
#define WIDTH		SCREEN_WIDTH / TILE_SIZE

// COLOR VARIABLES
static uint16 aliveColor;
static uint16 foodColor;
static uint16 nomColor;
static uint16 deadColor;
static uint16 tongueColor;
static uint16 black;
static uint16 white;
static uint16 winColor;

// GAMEPLAY VARIABLES
uint8 dead;
uint8 justAte;
uint16 length;
uint16 dir;
uint16 lastDir;
uint8 odo;
uint8 popup;

// BASIC DRAW UNIT
struct Rect 
{
	uint8 x, y;
} 
food, snek[(HEIGHT * WIDTH) - 1];

// METHOD DECLARATIONS
void titleScreen();
void mainGame();
void sync();
void getInput();
void pause();
void moveSnek();
void hasEaten();
uint8 isDead();
uint8 hitWall();
uint8 hitSelf();

uint16 makeColor(uint8 r, uint8 g, uint8 b);
void drawSnek();
void drawOther(struct Rect r, uint16 color, uint8 border);
void drawHead(uint16 color);
void drawBiggerEyes();
void drawTongue(uint16 color);
void drawSegment(uint16 color);
void drawLeft(uint16 color);
void drawRight(uint16 color);
void drawDown(uint16 color);
void drawUp(uint16 color);
void drawTail();
void drawQ1();
void drawQ2();
void drawQ3();
void drawQ4();
void fadeTileArray(struct Rect r[], uint16 len, uint16 toColor);
void fade(struct Rect r, uint8 density, uint16 toColor);
void drawDead();
void drawWin();
void clearScreen();
