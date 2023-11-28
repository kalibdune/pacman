#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <windows.h>
#include <cmath>
#include <vector>
#include "termcolor.h"

using namespace std;

array<array<int, 28>, 31> map;

int score = 0;

char pacmanIcon = '@';
int pacmanX = 13;
int pacmanY = 23;

int rGhostX = 13;
int rGhostY = 11;

int oGhostX = 14;
int oGhostY = 14;

int bGhostX = 12;
int bGhostY = 14;

int pGhostX = 13;
int pGhostY = 14;

char coinIcon = (char)249u;
char ghostIcon = (char)253u;

enum DIRECTION {UP, DOWN, RIGHT, LEFT, ELSE};
enum LOCATION {LEFTPORT, RIGHTPORT, EMPTY, COIN, SUPERCOIN, PACMAN, WALL, REDGHOST, ORANGEGHOST, BLUEGHOST, PINKGHOST};

LOCATION varRGhost = LOCATION::EMPTY;
LOCATION varOGhost = LOCATION::EMPTY;
LOCATION varPGhost = LOCATION::EMPTY;
LOCATION varBGhost = LOCATION::EMPTY;
DIRECTION redGhostDir = DIRECTION::DOWN;

void updateScreen() {
    BOOL WINAPI WriteConsoleOutput(
        _In_     HANDLE hConsoleOutput,
        _In_     const CHAR_INFO * lpBuffer,
        _In_     COORD dwBufferSize,
        _In_     COORD dwBufferCoord,
        _Inout_  PSMALL_RECT lpWriteRegion
    );
}
void setcur(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void hidecursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}
void loadMapFromFile(string mapPath) {
    ifstream file(mapPath);
    if (file.is_open()) {
        for (int i = 0; i < 31; ++i) {
            string buffer;
            file >> buffer;
            for (int j = 0; j < 28; ++j) {
                char elem = buffer[j];
                if (elem == '.') map[i][j] = LOCATION::COIN;
                else if (elem == '0') map[i][j] = LOCATION::EMPTY;
                else if (elem == '$') map[i][j] = LOCATION::LEFTPORT;
                else if (elem == '%') map[i][j] = LOCATION::RIGHTPORT;
                else map[i][j] = LOCATION::WALL;
            }
        }
    }
}

void show() {
    setcur(0, 0);
    updateScreen();
    for (int i = 0; i < 31; ++i) {
        for (int j = 0; j < 28; ++j) {
            hidecursor();
            switch (map[i][j]) {
            case LOCATION::LEFTPORT:
            case LOCATION::RIGHTPORT:
            case LOCATION::EMPTY:
                cout << " ";
                break;
            case LOCATION::WALL:
                cout << termcolor::on_blue << "#" << termcolor::reset;
                break;
            case LOCATION::COIN:
                cout << termcolor::yellow << coinIcon << termcolor::reset;
                break;
            case LOCATION::PACMAN:
                cout << termcolor::bright_yellow << pacmanIcon << termcolor::reset;
                break;
            case LOCATION::REDGHOST:
                cout << termcolor::red << ghostIcon << termcolor::reset;
                break;
            case LOCATION::ORANGEGHOST:
                cout << termcolor::yellow << ghostIcon << termcolor::reset;
                break;
            case LOCATION::PINKGHOST:
                cout << termcolor::magenta << ghostIcon << termcolor::reset;
                break;
            case LOCATION::BLUEGHOST:
                cout << termcolor::blue << ghostIcon << termcolor::reset;
                break;
            }
        }
        cout << "\n";
    }
}

DIRECTION keyHandler() {
    if (GetAsyncKeyState(VK_UP) & 0x8000) return DIRECTION::UP;
    else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) return DIRECTION::RIGHT;
    else if (GetAsyncKeyState(VK_LEFT) & 0x8000) return DIRECTION::LEFT;
    else if (GetAsyncKeyState(VK_DOWN) & 0x8000) return DIRECTION::DOWN;
    else return DIRECTION::ELSE;
}

bool movePacmanTo(DIRECTION dir) {
    bool move = false;
    if (dir == DIRECTION::UP && pacmanY - 1 > 0 && map[pacmanY - 1][pacmanX] <= 4) {
        map[pacmanY][pacmanX] = LOCATION::EMPTY;
        --pacmanY;
        move = true;
    } else if (dir == DIRECTION::DOWN && pacmanY + 1 < 31 && map[pacmanY + 1][pacmanX] <= 4) {
        map[pacmanY][pacmanX] = LOCATION::EMPTY;
        ++pacmanY;
        move = true;
    } else if (dir == DIRECTION::RIGHT && pacmanX + 1 < 28 && map[pacmanY][pacmanX + 1] <= 4) {
        map[pacmanY][pacmanX] = LOCATION::EMPTY;
        ++pacmanX;
        move = true;
    } else if (dir == DIRECTION::LEFT && pacmanX - 1 >= 0 && map[pacmanY][pacmanX - 1] <= 4) {
        map[pacmanY][pacmanX] = LOCATION::EMPTY;
        --pacmanX;
        move = true;
    }
    else return false;
    if (move) {
        if (map[pacmanY][pacmanX] == LOCATION::COIN) score += 10;
        if (map[pacmanY][pacmanX] == LOCATION::LEFTPORT) pacmanX = 26;
        else if (map[pacmanY][pacmanX] == LOCATION::RIGHTPORT) pacmanX = 1;
        map[pacmanY][pacmanX] = LOCATION::PACMAN;
        return true;
    }
}

double distanceFromDot(int curX, int curY, int targetX, int targetY) {
    int vectorX = targetX - curX;
    int vectorY = targetY - curY;
    return sqrt(vectorX * vectorX + vectorY * vectorY);
}

void redGhostMove() {
    int minX = rGhostX;
    int minY = rGhostY;
    double minDistance = 1000;
    if (redGhostDir != DIRECTION::DOWN && rGhostY - 1 > 0 && map[rGhostY - 1][rGhostX] <= 5) {
        double distance = distanceFromDot(rGhostX, rGhostY - 1, pacmanX, pacmanY);
        if (distance < minDistance) {
            minX = rGhostX;
            minY = rGhostY - 1;
            minDistance = distance;
            redGhostDir = DIRECTION::UP;
        }
    }
    if (redGhostDir != DIRECTION::UP && rGhostY + 1 < 31 && map[rGhostY + 1][rGhostX] <= 5) {
        double distance = distanceFromDot(rGhostX, rGhostY + 1, pacmanX, pacmanY);
        if (distance < minDistance) {
            minX = rGhostX;
            minY = rGhostY + 1;
            minDistance = distance;
            redGhostDir = DIRECTION::DOWN;
        }
    }
    if (redGhostDir != DIRECTION::LEFT && rGhostX + 1 < 28 && map[rGhostY][rGhostX + 1] <= 5) {
        double distance = distanceFromDot(rGhostX + 1, rGhostY, pacmanX, pacmanY);
        if (distance < minDistance) {
            minX = rGhostX + 1;
            minY = rGhostY;
            minDistance = distance;
            redGhostDir = DIRECTION::RIGHT;
        }
    }
    if (redGhostDir != DIRECTION::RIGHT && rGhostX - 1 >= 0 && map[rGhostY][rGhostX - 1] <= 5) {
        double distance = distanceFromDot(rGhostX - 1, rGhostY, pacmanX, pacmanY);
        if (distance < minDistance) {
            minX = rGhostX - 1;
            minY = rGhostY;
            minDistance = distance;
            redGhostDir = DIRECTION::LEFT;
        }
    }
    map[rGhostY][rGhostX] = varRGhost;
    varRGhost = (LOCATION)map[minY][minX];
    map[minY][minX] = LOCATION::REDGHOST;
    rGhostX = minX;
    rGhostY = minY;
}



int main() {
    //set location
    loadMapFromFile("map.txt");
    map[pacmanY][pacmanX] = LOCATION::PACMAN;
    map[bGhostY][bGhostX] = LOCATION::BLUEGHOST;
    map[oGhostY][oGhostX] = LOCATION::ORANGEGHOST;
    map[pGhostY][pGhostX] = LOCATION::PINKGHOST;
    map[rGhostY][rGhostX] = LOCATION::REDGHOST;
    DIRECTION dir, curDir;
    dir = DIRECTION::LEFT;

    cout << "Press Enter\n";
    while (!(GetAsyncKeyState(VK_RETURN) & 0x8000)){}//wait enter 
    while (1) {
        show();
        //set direction and pacman move
        curDir = keyHandler();
        if (dir != curDir && curDir != DIRECTION::ELSE) {
            if (!movePacmanTo(curDir)) movePacmanTo(dir);
            else dir = curDir;
        } else movePacmanTo(dir);
        //ghost move
        redGhostMove();
        cout << "Score: " << score << "\n";
        cout << distanceFromDot(rGhostX, rGhostY, pacmanX, pacmanY) << "\n";
        cout << "current red direction: " << redGhostDir << "\n";
    }
    return 0;
}
