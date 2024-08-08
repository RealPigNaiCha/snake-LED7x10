#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoMatrix RGBmatrix = Adafruit_NeoMatrix(10, 7, 9, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG, NEO_GRB + NEO_KHZ800);

const int buttonPins[] = {5, 4, 6, 8, 1}; // 按钮5用于前进，按钮6用于后退，按钮8用于左转，按钮4用于右转，按钮1用于重启游戏
const int buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);

int snakeX[70], snakeY[70]; // 贪吃蛇的X和Y坐标
int snakeLength = 3; // 初始蛇的长度
int foodX, foodY; // 食物的位置
int direction = 1; // 初始方向（0=上, 1=右, 2=下, 3=左）
int lastDirection = 1; // 记录上一次的方向，防止蛇反向移动导致自撞

unsigned long lastMoveTime = 0; // 上一次蛇移动的时间
int moveInterval = 300; // 蛇移动的时间间隔（毫秒），初始值为300ms

uint32_t snakeColor = RGBmatrix.Color(255, 0, 0); // 蛇身颜色，初始为红色

bool gameOver = false; // 游戏结束标志

void setup() {
  RGBmatrix.begin();
  RGBmatrix.setBrightness(10);

  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP); // 设置引脚为输入，并开启内部上拉电阻
  }

  snakeX[0] = 5; snakeY[0] = 3; // 初始蛇头位置
  snakeX[1] = 4; snakeY[1] = 3;
  snakeX[2] = 3; snakeY[2] = 3;

  placeFood();
}

void loop() {
  if (gameOver) {
    handleGameOver();
  } else {
    updateDirection();

    // 仅在一定时间间隔后移动蛇
    if (millis() - lastMoveTime > moveInterval) {
      lastMoveTime = millis();
      moveSnake();
      checkCollision();
      drawSnakeAndFood();
    }
  }
}

void updateDirection() {
  if (isButtonPressed(0) && lastDirection != 2) direction = 0; // 上，避免反向
  if (isButtonPressed(1) && lastDirection != 3) direction = 1; // 右，避免反向
  if (isButtonPressed(2) && lastDirection != 0) direction = 2; // 下，避免反向
  if (isButtonPressed(3) && lastDirection != 1) direction = 3; // 左，避免反向

  lastDirection = direction; // 记录当前方向
}

bool isButtonPressed(int buttonIndex) {
  const int debounceDelay = 20; // 去抖动延迟
  static unsigned long lastDebounceTime[buttonCount] = {0};

  if (digitalRead(buttonPins[buttonIndex]) == LOW) {
    if (millis() - lastDebounceTime[buttonIndex] > debounceDelay) {
      lastDebounceTime[buttonIndex] = millis();
      return true;
    }
  }
  return false;
}

void moveSnake() {
  // 更新蛇的身体位置
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  // 更新蛇头位置
  switch (direction) {
    case 0: snakeY[0]--; break; // 上
    case 1: snakeX[0]++; break; // 右
    case 2: snakeY[0]++; break; // 下
    case 3: snakeX[0]--; break; // 左
  }

  // 检查边界并环绕
  if (snakeX[0] < 0) snakeX[0] = 9;
  if (snakeX[0] > 9) snakeX[0] = 0;
  if (snakeY[0] < 0) snakeY[0] = 6;
  if (snakeY[0] > 6) snakeY[0] = 0;
}

void checkCollision() {
  // 检查蛇头是否吃到食物
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeLength++;
    placeFood();
    adjustSpeedAndColor(); // 吃到食物后调整速度和颜色
  }

  // 检查蛇头是否碰到身体
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver = true; // 游戏结束
      RGBmatrix.clear();
      RGBmatrix.show();
      return;
    }
  }
}

void adjustSpeedAndColor() {
  // 根据蛇的长度调整移动间隔和蛇身颜色
  moveInterval = 300 - ((snakeLength / 15) * 50); // 每增加15长度减少50ms
  if (moveInterval < 100) moveInterval = 100; // 设置最小间隔为100ms，避免速度过快

  // 根据长度调整蛇身颜色
  if (snakeLength >= 10 && snakeLength < 20) {
    snakeColor = RGBmatrix.Color(255, 255, 0); // 长度10-19时变为黄色
  } else if (snakeLength >= 20) {
    snakeColor = RGBmatrix.Color(173, 216, 230); // 长度20及以上变为淡蓝色
  } else {
    snakeColor = RGBmatrix.Color(255, 0, 0); // 默认红色
  }
}

void placeFood() {
  bool validPosition;
  do {
    validPosition = true;
    foodX = random(0, 10);
    foodY = random(0, 7);

    for (int i = 0; i < snakeLength; i++) {
      if (snakeX[i] == foodX && snakeY[i] == foodY) {
        validPosition = false;
        break;
      }
    }
  } while (!validPosition);
}

void drawSnakeAndFood() {
  RGBmatrix.clear();

  // 绘制食物
  RGBmatrix.drawPixel(foodX, foodY, RGBmatrix.Color(0, 255, 0)); // 绿色食物

  // 绘制蛇
  RGBmatrix.drawPixel(snakeX[0], snakeY[0], RGBmatrix.Color(255, 0, 255)); // 粉色蛇头
  for (int i = 1; i < snakeLength; i++) {
    RGBmatrix.drawPixel(snakeX[i], snakeY[i], snakeColor); // 根据蛇长度变化的蛇身颜色
  }

  RGBmatrix.show();
}

void handleGameOver() {
  while (!isButtonPressed(4)) { // 持续显示烟花，直到按钮1被按下
    drawFireworks();
  }
  resetGame();
}

void drawFireworks() {
  RGBmatrix.clear();
  for (int i = 0; i < 20; i++) {
    int x = random(0, 10);
    int y = random(0, 7);
    RGBmatrix.drawPixel(x, y, RGBmatrix.Color(random(255), random(255), random(255)));
  }
  RGBmatrix.show();
  delay(100); // 控制烟花效果的刷新速度
}

void resetGame() {
  gameOver = false;
  snakeLength = 3; // 重置蛇的长度
  snakeX[0] = 5; snakeY[0] = 3; // 重置蛇头位置
  snakeX[1] = 4; snakeY[1] = 3;
  snakeX[2] = 3; snakeY[2] = 3;
  moveInterval = 300; // 重置速度
  snakeColor = RGBmatrix.Color(255, 0, 0); // 重置蛇身颜色为红色
  placeFood();
  RGBmatrix.clear();
  RGBmatrix.show();
}
