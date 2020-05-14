#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#include <stdbool.h>

LCDKeypad lcd;

#define MAX_LEVELS 8 // memory cannot support more than 9-10 levels

bool maze[26][26];
uint8_t level = 0;
uint8_t MAZE_SIZE = 5;
uint8_t wallsAmount = 0;
bool gameOver = true, isAtEdgeOfY = 1;

struct Point {
  uint8_t row, col;

  Point() : row(0), col(1) {}

  Point(uint8_t row_, uint8_t col_) : row(row_), col(col_) {}
};
struct Point walls[174][2];
struct Player {
  struct Point coordinates;
  
  byte character[8] = {
    B00000,
    B00100,
    B01010,
    B01010,
    B10101,
    B00100,
    B01010,
    B00000,
  };
  
} player;

byte crystal[8] = {
  B00100,
  B01010,
  B10001,
  B10101,
  B10101,
  B10001,
  B01010,
  B00100,
};
byte select[8] = {
  B00100,
  B01110,
  B11111,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
};
void setup()
{
  Serial.begin(9600);
  randomSeed(10);

  lcd.begin(16, 2);
  lcd.createChar(1, player.character);
  lcd.createChar(2, crystal);
  lcd.createChar(3, select);
}

void loop()
{
  if (!gameOver) {
    
    drawMaze(player.coordinates);
    int btn = waitButton();
    movePlayerOnBtn(btn);
    
  } else {
      if(level < MAX_LEVELS){
        lcd.setCursor(0, 0);
        lcd.noBlink();
        lcd.print("TMZ -- level:");
        lcd.print(level+1);
        delay(500);
        lcd.setCursor(0, 1);
        lcd.print("-> ");
        lcd.setCursor(3, 1);
        lcd.write(2);
        lcd.setCursor(4, 1);
        lcd.print(" <- to start");
        lcd.blink();
        int btn = waitButton();
        if(btn == KEYPAD_SELECT){
          level++;
          MAZE_SIZE += 2;
          player.coordinates = Point();
          isAtEdgeOfY = true;
          gameOver = false;
          generateMaze();
          drawMaze(player.coordinates);
        }
      } else {
        lcd.setCursor(0, 0);
        lcd.noBlink();
        lcd.print("Congratulations!");
        delay(500);
        lcd.setCursor(2, 1);
        lcd.noBlink();
        lcd.print("- You won! -");
      }
  }
}
void drawMaze(Point p) {
  lcd.clear();
  lcd.setCursor(8, isAtEdgeOfY);
  lcd.noBlink();
  lcd.write(1);
  for (int row = p.row; row < p.row + 2; row++) {
    for (int col = p.col - 8; col < p.col + 8; col++) {
      lcd.setCursor(col - p.col + 8, row - p.row);
      lcd.noBlink();
      if (col < 0 || col > MAZE_SIZE - 1) {
        lcd.write(32);
      }
      else if (!maze[row][col]) {
        lcd.write(255);
      } else if (col == MAZE_SIZE - 1 && row == MAZE_SIZE - 2) {
        lcd.write(2);
      }
    }
  }
}

void movePlayerOnBtn(int button) {
  struct Point* p = &(player.coordinates);
  switch (button) {
 
    case KEYPAD_LEFT:
      if (p->col <= 0 || !maze[p->row + isAtEdgeOfY][p->col - 1]) {
        break;
      }
      p->col--;
      break;
    case KEYPAD_RIGHT:
      if (p->col >= MAZE_SIZE - 1 || !maze[p->row + isAtEdgeOfY][p->col + 1]) {
        break;
      }
      p->col++;
      break;
    case KEYPAD_UP:
      if (p->row <= 0) {
        break;
      }
      if (isAtEdgeOfY) {
        if (maze[p->row][p->col]) {
          isAtEdgeOfY = false;
        }
        break;
      }
      if (!maze[p->row - 1][p->col]) {
        if (!isAtEdgeOfY) {
          p->row--;
          isAtEdgeOfY = true;
        }
        break;
      }
      p->row--;
      break;
    case KEYPAD_DOWN:
      if (isAtEdgeOfY) {
        isAtEdgeOfY = false;
        p->row++;
        break;
      }
      if (p->row >= MAZE_SIZE - 1 || !maze[p->row + 1][p->col]) {
        break;
      }
      p->row++;
      break;
    default:
      break;
  }
  if (p->col == MAZE_SIZE - 1 && p->row == MAZE_SIZE - 2) {
    lcd.clear();
    gameOver = true;
  }
}
void generateMaze() {
  
  for (uint8_t row = 0; row < MAZE_SIZE; row++) {
    for (uint8_t col = 0; col < MAZE_SIZE; col++) {
      maze[row][col] = false;
    }
  }
  for(uint8_t i = 0; i < wallsAmount; i++){
    walls[i][0] = walls[i][1] = Point();
  }
  wallsAmount = 0;
  
  maze[1][1] = true;
  addWalls(Point(1, 1));
  while (wallsAmount != 0) {
    uint8_t pos = rand() % wallsAmount + 1;
    struct Point wallP = walls[pos - 1][0];
    struct Point cellP = walls[pos - 1][1];
    for (uint8_t i = pos - 1; i < wallsAmount - 1; i++)
    {
      walls[i][0] = walls[i + 1][0];
      walls[i][1] = walls[i + 1][1];
    }
    wallsAmount--;
    if (maze[wallP.row][wallP.col] || maze[cellP.row][cellP.col]) {
      continue;
    }
    maze[wallP.row][wallP.col] = maze[cellP.row][cellP.col] = true;
    addWalls(cellP);
  }
  maze[MAZE_SIZE - 2][MAZE_SIZE - 1] = true;
}
void addWalls(Point p)
{
  short dir[4][2] = {{0, 1}, {1, 0}, {0, -1}, { -1, 0}};

  for (int i = 0; i < 4; i++) {
    uint8_t wallRow = p.row + dir[i][0], wallCol = p.col + dir[i][1];
    uint8_t cellRow = wallRow + dir[i][0], cellCol = wallCol + dir[i][1];
    struct Point wallP(wallRow, wallCol), cellP(cellRow, cellCol);

    if (!(inMaze(wallP)) || !(inMaze(cellP))) {
      continue;
    }

    walls[wallsAmount][0] = wallP;
    walls[wallsAmount][1] = cellP;
    wallsAmount++;
  }
}
bool inMaze(Point p) {
  return p.col > 0 && p.col < MAZE_SIZE - 1 && p.row > 0 && p.row < MAZE_SIZE - 1;
}

int waitButton()
{
  int buttonPressed;
  waitReleaseButton();
  while ((buttonPressed = lcd.button()) == KEYPAD_NONE);
  delay(200);
  return buttonPressed;
}

void waitReleaseButton()
{
  delay(200);
  while (lcd.button() != KEYPAD_NONE) {};
  delay(200);
}
