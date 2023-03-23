// imports the SPI library (needed to communicate with Gamebuino's screen)
#include <SPI.h>
// importe the Gamebuino library
#include <Gamebuino.h>
// creates a Gamebuino object named gb
Gamebuino gb;

const byte logo[] PROGMEM = {
    64,
    30,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
    B00111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00001000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00001000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00011100,B00000000,B00001000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00011100,B00000000,B00001000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00011100,B00000000,B00001000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B11001000,
    B00000000,B00000011,B10000000,B00000000,B00000000,B00000000,B01111101,B00001000,
    B00000001,B11100110,B00000000,B00000000,B00000000,B00111110,B00111101,B00001000,
    B00000111,B11000010,B00000000,B00000000,B00001111,B00001110,B00011110,B00001000,
    B00000001,B11110010,B00000000,B00000001,B11110011,B10001110,B00011110,B00001000,
    B00000000,B01110100,B00000001,B11100000,B11100011,B10001110,B00001111,B00001000,
    B00000000,B00111100,B00000011,B00111000,B11100011,B10001110,B00010111,B00001000,
    B00000000,B00111100,B00000011,B00111000,B11100011,B10001110,B00010111,B11001000,
    B00000000,B00011100,B00000111,B00111000,B11100011,B10001110,B00110011,B10001000,
    B00000000,B00011111,B00000111,B00111000,B11100011,B11001111,B10000000,B00001000,
    B00000000,B00101111,B00000111,B00011000,B11111011,B11100000,B00000000,B00001000,
    B00000000,B01100011,B11000001,B10011000,B11110000,B00000000,B00000000,B00001000,
    B00000000,B01000001,B11100000,B11110000,B00000000,B00000010,B00100010,B00001000,
    B00000000,B01000001,B11110000,B00000000,B00000000,B00000001,B00100100,B00001000,
    B00000000,B11100011,B00000000,B00000000,B00000000,B00000000,B11111000,B00001000,
    B00000001,B10000000,B00000000,B00000000,B00000000,B00000001,B10001000,B00001000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B10001111,B11111000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000001,B10001000,B00000000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B11111000,B00000000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000001,B00100100,B00000000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000010,B00100010,B00000000,
    B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
};

const byte heart[] PROGMEM = {
    5,
    4,
    B11011000,
    B11111000,
    B01110000,
    B00100000,
};

const byte line[] PROGMEM = {
    2,
    2,
    B10000000,
    B01000000,
};

typedef struct
{
  int x;
  int y;
} coords;

bool gameOver = false;
bool won = false;
bool paused = false;
bool levelup = false;

coords player;
coords player_velocity;
int health = 3;

int playerSpeed = 1;
int enemysSpeed = 1;

#define MAX_ENEMYS 10
coords enemys[MAX_ENEMYS];
coords enemys_velocity[MAX_ENEMYS];
int enemysCount = 1;

#define CELL_EMPTY 0
#define CELL_CLAIMED 1
#define CELL_LINE 2
#define CELL_CAN_CLAIM 3

#define CELLSIZE 2
#define BOARDWIDTH ((LCDWIDTH - 7) / CELLSIZE)
#define BOARDHEIGHT (LCDHEIGHT / CELLSIZE)
#define LEVELCLEARRATE (BOARDWIDTH * BOARDHEIGHT / 100 * 75)
char board[BOARDWIDTH][BOARDHEIGHT];

// movement speed
unsigned long prevTime = 0; // for movement delay (ms)
const int delayTime = 200;  // move speed in ms

void setup()
{
  Serial.begin(9600);
  gb.begin(); // initialize the Gamebuino object
  gb.battery.show = false;
  titleMenu(true);
}

void loop()
{
  if (gb.update())
  {
    if (gb.buttons.pressed(BTN_C))
    {
      titleMenu(false);
    }

    if (!gameOver && !won)
    {
      if (gb.buttons.pressed(BTN_B))
      {
        paused = !paused;
        levelup = false;
      }
      if (!paused)
      {
        checkInput();

        unsigned long currentTime = millis();
        if (currentTime - prevTime >= delayTime)
        {
          move();
          prevTime = currentTime;
        }
      }
    }
    else
    {
      if (gb.buttons.pressed(BTN_A))
      {
        initGame();
      }
    }

    drawField();
    drawScore();
    updatePopup();

    if (paused || levelup || gameOver || won)
    {
      gb.display.cursorY = 0;
      gb.display.setColor(WHITE);
      gb.display.fillRect(0, 0, LCDWIDTH, gb.display.fontHeight + 1);
      gb.display.setColor(BLACK);
      if (won) printCentered(F(" You Won! "));
      else if (gameOver) printCentered(F(" Game over "));
      else if (levelup) printCentered(F(" Level Up "));
      else if (paused) printCentered(F(" PAUSE "));
    }
  }
}

void titleMenu(bool firstRun)
{
  gb.titleScreen(F("Xonix by TnxEc2"), logo);
  initGame();
}

void initGame()
{
  gb.pickRandomSeed();

  paused = false;
  gameOver = false;
  won = false;
  health = 3;
  enemysCount = 1;

  initPlayer();
  clearBoard();
  initEnemys();
  gb.sound.playOK();
}

void initPlayer()
{
  player.x = BOARDWIDTH / 2;
  player.y = 0;
  player_velocity.x = 0;
  player_velocity.y = 0;
}

void clearBoard()
{
  for (int row = 0; row < BOARDHEIGHT; row++)
  {
    board[0][row] = CELL_CLAIMED;
    board[BOARDWIDTH - 1][row] = CELL_CLAIMED;
    for (int col = 1; col < BOARDWIDTH - 1; col++)
    {
      board[col][row] = (row == 0 || row == BOARDHEIGHT - 1) ? CELL_CLAIMED : CELL_EMPTY;
    }
  }
}

void initEnemys()
{
  for (int i = 0; i < MAX_ENEMYS; i++)
  {
    if (i >= enemysCount)
    {
      enemys[i] = {-1, -1};
      enemys_velocity[i] = {0, 0};
    }
    else
    {
      enemys[i] = {random(10, BOARDWIDTH - 10), random(10, BOARDHEIGHT - 10)};
      enemys_velocity[i] = {random(40) > 20 ? -1 : 1, random(40) > 20 ? -1 : 1};
    }
  }
}

void checkInput()
{
  if (gb.buttons.pressed(BTN_RIGHT))
  {
    if (board[player.x][player.y] == CELL_CLAIMED || player_velocity.x != -1) {
      player_velocity.x = 1;
      player_velocity.y = 0;
    }
  }
  else if (gb.buttons.pressed(BTN_LEFT))
  {
    if (board[player.x][player.y] == CELL_CLAIMED || player_velocity.x != 1) {
      player_velocity.x = -1;
      player_velocity.y = 0;
    }
  }
  else if (gb.buttons.pressed(BTN_DOWN))
  {
    if (board[player.x][player.y] == CELL_CLAIMED || player_velocity.y != -1) {
      player_velocity.y = 1;
      player_velocity.x = 0;
    }
  }
  else if (gb.buttons.pressed(BTN_UP))
  {
    if (board[player.x][player.y] == CELL_CLAIMED || player_velocity.y != 1) {
      player_velocity.y = -1;
      player_velocity.x = 0;
    }
  }
}

void move()
{

  char oldPlayerStat = board[player.x][player.y];

  if (board[player.x][player.y] == CELL_EMPTY)
    board[player.x][player.y] = CELL_LINE;

  // move player
  if (player_velocity.x == 1 && player.x < BOARDWIDTH - 1)
    player.x++;
  else if (player_velocity.x == -1 && player.x > 0)
    player.x--;
  if (player_velocity.y == 1 && player.y < BOARDHEIGHT - 1)
    player.y++;
  else if (player_velocity.y == -1 && player.y > 0)
    player.y--;

  bool playerAttacked = false;
  // move enemys
  for (int i = 0; i < enemysCount; i++)
  {
    if (board[enemys[i].x + enemys_velocity[i].x][enemys[i].y] == CELL_CLAIMED)
      enemys_velocity[i].x = -(enemys_velocity[i].x);
    if (board[enemys[i].x][enemys[i].y + enemys_velocity[i].y] == CELL_CLAIMED)
      enemys_velocity[i].y = -(enemys_velocity[i].y);
    enemys[i].x += enemys_velocity[i].x;
    enemys[i].y += enemys_velocity[i].y;

    if (player.x == enemys[i].x && player.y == enemys[i].y)
      playerAttacked = true;
    if (board[enemys[i].x][enemys[i].y] == CELL_LINE)
      playerAttacked = true;
  }

  if (playerAttacked || board[player.x][player.y] == CELL_LINE)
  {
    initPlayer();
    clearLine();
    gb.sound.playCancel();
    health--;
    if (health <= 0)
      gameOver = true;
  }

  if (!gameOver && (player_velocity.x != 0 || player_velocity.y != 0))
  {
    if (oldPlayerStat != CELL_CLAIMED && board[player.x][player.y] == CELL_CLAIMED)
    {
      claimBoard();
    }
    if ((oldPlayerStat != CELL_CLAIMED && board[player.x][player.y] == CELL_CLAIMED))
    {
      player_velocity.x = 0;
      player_velocity.y = 0;
    }
  }
}

void clearLine()
{
  for (int row = 0; row < BOARDHEIGHT; row++)
  {
    for (int col = 0; col < BOARDWIDTH; col++)
    {
      if (board[col][row] == CELL_LINE)
        board[col][row] = CELL_EMPTY;
    }
  }
}

void claimBoard()
{
  gb.sound.playTick();

  // claim line
  for (int row = 0; row < BOARDHEIGHT; row++)
  {
    for (int col = 0; col < BOARDWIDTH; col++)
    {
      if (board[col][row] == CELL_LINE)
        board[col][row] = CELL_CLAIMED;
      else if (board[col][row] == CELL_EMPTY)
        board[col][row] = CELL_CAN_CLAIM;
    }
  }

  // clear enemy cells for bucker-fill
  for (int i = 0; i < enemysCount; i++)
  {
    board[enemys[i].x][enemys[i].y] = CELL_EMPTY;
  }

  // Bucket-fill
  bool changed = true;
  while (changed)
  {
    changed = false;
    for (int row = 0; row < BOARDHEIGHT; ++row)
    {
      for (int col = 0; col < BOARDWIDTH; ++col)
      {
        if (board[col][row] != CELL_CAN_CLAIM)
          continue;

        if ((row > 1 && board[col][row - 1] == CELL_EMPTY) ||
            (row < BOARDHEIGHT - 1 && board[col][row + 1] == CELL_EMPTY) ||
            (col > 1 && board[col - 1][row] == CELL_EMPTY) ||
            (col < BOARDWIDTH - 1 && board[col + 1][row] == CELL_EMPTY))
        {
          if (board[col][row] == CELL_CAN_CLAIM)
            board[col][row] = CELL_EMPTY;
          changed = true;
        }
      }
    }
  }

  // claim
  int score = 0;
  for (int row = 0; row < BOARDHEIGHT; row++)
  {
    for (int col = 0; col < BOARDWIDTH; col++)
    {
      if (board[col][row] == CELL_CAN_CLAIM)
        board[col][row] = CELL_CLAIMED;
      if (board[col][row] == CELL_CLAIMED)
        score++;
    }
  }

  if (score >= LEVELCLEARRATE)
  {
    levelUp();
  }
}

void levelUp()
{
  if (enemysCount >= MAX_ENEMYS)
  {
    gb.sound.playOK();
    won = true;
    return;
  }
  initPlayer();
  clearBoard();
  enemysCount++;
  if (health < 3) health++;
  initEnemys();
  gb.sound.playOK();
  levelup = true;
  paused = true;
}

void drawField()
{
  // draw board
  gb.display.setColor(BLACK);
  for (int row = 0; row < BOARDHEIGHT; row++)
  {
    for (int col = 0; col < BOARDWIDTH; col++)
    {
      if (board[col][row] == CELL_CLAIMED)
      {
        gb.display.fillRect(col * CELLSIZE, row * CELLSIZE, CELLSIZE, CELLSIZE);
      }
      if (board[col][row] == CELL_LINE)
      {
        gb.display.drawBitmap(col * CELLSIZE, row * CELLSIZE, line);
      }
    }
  }

  // draw player
  gb.display.setColor(board[player.x][player.y] == CELL_EMPTY ? BLACK : WHITE);
  gb.display.fillRect(player.x * CELLSIZE, player.y * CELLSIZE, CELLSIZE, CELLSIZE);

  // draw enemys
  gb.display.setColor(BLACK);
  for (int i = 0; i < enemysCount; i++)
  {
    gb.display.fillRect(enemys[i].x * CELLSIZE, enemys[i].y * CELLSIZE, CELLSIZE, CELLSIZE);
  }
}

void drawScore()
{
  gb.display.setColor(BLACK);
  gb.display.fillRect(BOARDWIDTH * CELLSIZE, 0, LCDWIDTH - BOARDWIDTH * CELLSIZE, LCDHEIGHT);
  gb.display.setColor(WHITE);
  for (int i = 0; i < health; i++)
  {
    gb.display.drawBitmap(LCDWIDTH - 7, 2 + (i * 5), heart);
  }
  gb.display.fontSize = 1;
  gb.display.cursorX = LCDWIDTH - 7;
  gb.display.cursorY = LCDHEIGHT - gb.display.fontHeight - 2;
  gb.display.print(enemysCount);
}
