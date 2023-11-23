#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <windows.h>
#include "termcolor.h"

using namespace std;

array<array<int, 28>, 31> map;

int score = 0;

char pacmanIcon = '@';
int pacmanX = 13;
int pacmanY = 23;

char coinIcon = (char)249u;
char ghostIcon = (char)253u;

enum DIRECTION {UP, DOWN, RIGHT, LEFT, ELSE};
enum LOCATION {LEFTPORT, RIGHTPORT, EMPTY, COIN, SUPERCOIN, WALL, PACMAN, REDGHOST, ORANGEGHOST, BLUEGHOST, PINKGHOST};

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
                else if (elem == 'r') map[i][j] = LOCATION::REDGHOST;
                else if (elem == 'b') map[i][j] = LOCATION::BLUEGHOST;
                else if (elem == 'p') map[i][j] = LOCATION::PINKGHOST;
                else if (elem == 'o') map[i][j] = LOCATION::ORANGEGHOST;
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

int main() {
    //set location
    loadMapFromFile("map.txt");
    map[pacmanY][pacmanX] = LOCATION::PACMAN;
    DIRECTION dir, curDir;
    dir = DIRECTION::LEFT;

    cout << "Press Enter\n";
    while (!(GetAsyncKeyState(VK_RETURN) & 0x8000)){}//wait enter 
    
    while (1) {
        show();
        //set direction and move
        curDir = keyHandler();
        if (dir != curDir && curDir != DIRECTION::ELSE) {
            if (!movePacmanTo(curDir)) movePacmanTo(dir);
            else dir = curDir;
        } else movePacmanTo(dir);
        cout << "Score: " << score << "\n";
    }
    return 0;
}
