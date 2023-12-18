#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <windows.h>
#include <cmath>
#include <vector>
#include "termcolor.h"

using namespace std;

const int width = 28;
const int height = 31;
int timer = 0;

array<array<int, width>, height> map;

int score = 0;
bool hideSeek = false;
bool isPacmanCaught = false;

char pacmanIcon = '@';
int pacmanX = 13;
int pacmanY = 23;
int pacmanBlock = 4;

int rGhostX = 13;
int rGhostY = 11;
bool redGhostTimeOut = false;

int oGhostX = 14;  
int oGhostY = 14;
bool oStarted = false;
bool orangeGhostTimeOut = false;

int bGhostX = 12;
int bGhostY = 14;
bool bStarted = false;
bool blueGhostTimeOut = false;

int pGhostX = 13;
int pGhostY = 14;
bool pStarted = false;
bool pinkGhostTimeOut = false;

char coinIcon = (char)249u;
char ghostIcon = (char)253u;

enum DIRECTION {UP, DOWN, RIGHT, LEFT, ELSE};
enum LOCATION {LEFTPORT, RIGHTPORT, EMPTY, COIN, SUPERCOIN, PACMAN, WALL, REDGHOST, ORANGEGHOST, BLUEGHOST, PINKGHOST, DOOR};

vector<LOCATION> pacmanRegular = {LOCATION::LEFTPORT, LOCATION::RIGHTPORT, LOCATION::EMPTY, LOCATION::COIN, LOCATION::SUPERCOIN};
vector<LOCATION> ghostRegular = {LOCATION::LEFTPORT, LOCATION::RIGHTPORT, LOCATION::EMPTY, LOCATION::COIN, LOCATION::SUPERCOIN, LOCATION::PACMAN, LOCATION::REDGHOST, LOCATION::ORANGEGHOST, LOCATION::BLUEGHOST, LOCATION::PINKGHOST };
vector<LOCATION> neutralObjects = {LOCATION::LEFTPORT, LOCATION::RIGHTPORT, LOCATION::EMPTY, LOCATION::COIN, LOCATION::SUPERCOIN};
vector<LOCATION> ghostsList = { LOCATION::REDGHOST, LOCATION::ORANGEGHOST, LOCATION::BLUEGHOST, LOCATION::PINKGHOST };
vector<vector<int>> startPinkPath = {
    vector<int>{13, 13},
    vector<int>{12, 13},
    vector<int>{11, 13}
};
vector<vector<int>> startBluePath = {
    vector<int>{13, 11},
    vector<int>{13, 13},
    vector<int>{12, 13},
    vector<int>{11, 13},
};
vector<vector<int>> startOrangePath = {
    vector<int>{13, 14},
    vector<int>{12, 14},
    vector<int>{11, 14}
};

LOCATION varRGhost = LOCATION::COIN;
LOCATION varOGhost = LOCATION::EMPTY;
LOCATION varPGhost = LOCATION::EMPTY;
LOCATION varBGhost = LOCATION::EMPTY;
DIRECTION rGhostDir = DIRECTION::DOWN;
DIRECTION oGhostDir = DIRECTION::LEFT;
DIRECTION pGhostDir = DIRECTION::LEFT;
DIRECTION bGhostDir = DIRECTION::LEFT;

bool checkPermission(int texture, vector<LOCATION> arr) {
    for (int i = 0; i < arr.size(); ++i) { 
        if (arr[i] == texture) return true;
    };
    return false;
}

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
        for (int i = 0; i < height; ++i) {
            string buffer;
            file >> buffer;
            for (int j = 0; j < 28; ++j) {
                char elem = buffer[j];
                if (elem == '.') map[i][j] = LOCATION::COIN;
                else if (elem == '0') map[i][j] = LOCATION::EMPTY;
                else if (elem == '$') map[i][j] = LOCATION::LEFTPORT;
                else if (elem == '%') map[i][j] = LOCATION::RIGHTPORT;
                else if (elem == '=') map[i][j] = LOCATION::DOOR;
                else if (elem == '*') map[i][j] = LOCATION::SUPERCOIN;
                else map[i][j] = LOCATION::WALL;
            }
        }
    }
}

void show() {
    setcur(0, 0);
    updateScreen();
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < 28; ++j) {
            hidecursor();
            switch (map[i][j]) {
            case LOCATION::DOOR:
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
            case LOCATION::SUPERCOIN:
                cout << termcolor::yellow << "*" << termcolor::reset;
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

bool pacmanMove(DIRECTION dir, vector<LOCATION> textureArr) {
    bool move = false;
    if (dir == DIRECTION::UP && pacmanY - 1 > 0 && checkPermission(map[pacmanY - 1][pacmanX], textureArr)) {
        map[pacmanY][pacmanX] = LOCATION::EMPTY;
        --pacmanY;
        move = true;
    } else if (dir == DIRECTION::DOWN && pacmanY + 1 < height && checkPermission(map[pacmanY + 1][pacmanX], textureArr)) {
        map[pacmanY][pacmanX] = LOCATION::EMPTY;
        ++pacmanY;
        move = true;
    } else if (dir == DIRECTION::RIGHT && pacmanX + 1 < 28 && checkPermission(map[pacmanY][pacmanX + 1], textureArr)) {
        map[pacmanY][pacmanX] = LOCATION::EMPTY;
        ++pacmanX;
        move = true;
    } else if (dir == DIRECTION::LEFT && pacmanX - 1 >= 0 && checkPermission(map[pacmanY][pacmanX - 1], textureArr)) {
        map[pacmanY][pacmanX] = LOCATION::EMPTY;
        --pacmanX;
        move = true;
    }
    else return false;
    if (move) {
        if (map[pacmanY][pacmanX] == LOCATION::COIN) score += 10;
        if (map[pacmanY][pacmanX] == LOCATION::SUPERCOIN) {
            score += 100;
            hideSeek = true;
        }
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

void ghostMove(int& ghostX, int& ghostY, int targetX, int targetY, DIRECTION& ghostDir, LOCATION& varGhost, LOCATION ghostType, vector<LOCATION> &permissionList) {
    int minX = ghostX;
    int minY = ghostY;
    DIRECTION localeDir = ghostDir;
    double minDistance = 1000;
    if (ghostDir != DIRECTION::DOWN && ghostY - 1 > 0 && checkPermission(map[ghostY - 1][ghostX], permissionList) ) {
        double distance = distanceFromDot(ghostX, ghostY - 1, targetX, targetY);
        if (distance < minDistance) {
            minX = ghostX;
            minY = ghostY - 1;
            minDistance = distance;
            localeDir = DIRECTION::UP;
        }
    }
    if (ghostDir != DIRECTION::UP && ghostY + 1 < height && checkPermission(map[ghostY + 1][ghostX], permissionList)) {
        double distance = distanceFromDot(ghostX, ghostY + 1, targetX, targetY);
        if (distance < minDistance) {
            minX = ghostX;
            minY = ghostY + 1;
            minDistance = distance;
            localeDir = DIRECTION::DOWN;
        }
    }
    if (ghostDir != DIRECTION::LEFT && ghostX + 1 < 28 && checkPermission(map[ghostY][ghostX + 1], permissionList)) {
        double distance = distanceFromDot(ghostX + 1, ghostY, targetX, targetY);
        if (distance < minDistance) {
            minX = ghostX + 1;
            minY = ghostY;
            minDistance = distance;
            localeDir = DIRECTION::RIGHT;
        }
    }
    if (ghostDir != DIRECTION::RIGHT && ghostX - 1 >= 0 && checkPermission(map[ghostY][ghostX - 1], permissionList)) {
        double distance = distanceFromDot(ghostX - 1, ghostY, targetX, targetY);
        if (distance < minDistance) {
            minX = ghostX - 1;
            minY = ghostY;
            minDistance = distance;
            localeDir = DIRECTION::LEFT;
        }
    }
    ghostDir = localeDir;
    //procesing teleports
    if (map[minY][minX] == LOCATION::LEFTPORT) minX = 26;
    else if (map[minY][minX] == LOCATION::RIGHTPORT) minX = 1;
    //check collision
    if (checkPermission(map[minY][minX], ghostsList)) {
        switch ((LOCATION)map[minY][minX]) {
        case LOCATION::REDGHOST:
            map[rGhostY][rGhostX] = varRGhost;
            rGhostX = ghostX;
            rGhostY = ghostY;
            map[rGhostY][rGhostX] = LOCATION::REDGHOST;
            redGhostTimeOut = true;
            break;
        case LOCATION::BLUEGHOST:
            map[bGhostY][bGhostX] = varBGhost;
            bGhostX = ghostX;
            bGhostY = ghostY;
            map[bGhostY][bGhostX] = LOCATION::BLUEGHOST;
            blueGhostTimeOut = true;
            break;
        case LOCATION::PINKGHOST:
            map[pGhostY][pGhostX] = varPGhost;
            pGhostX = ghostX;
            pGhostY = ghostY;
            map[pGhostY][pGhostX] = LOCATION::PINKGHOST;
            pinkGhostTimeOut = true;
            break;
        case LOCATION::ORANGEGHOST:
            map[oGhostY][oGhostX] = varOGhost;
            oGhostX = ghostX;
            oGhostY = ghostY;
            map[oGhostY][oGhostX] = LOCATION::ORANGEGHOST;
            orangeGhostTimeOut = true;
            break;
        }
        varGhost = (LOCATION)map[minY][minX];
        map[minY][minX] = ghostType;
        ghostX = minX;
        ghostY = minY;
    }
    else if ((LOCATION)map[minY][minX] == LOCATION::PACMAN) {
        isPacmanCaught = true;
    }
    else {
        map[ghostY][ghostX] = varGhost;
        varGhost = (LOCATION)map[minY][minX];
        map[minY][minX] = ghostType;
        ghostX = minX;
        ghostY = minY;
    }

}

void redGhostMove() {
    ghostMove(rGhostX, rGhostY, pacmanX, pacmanY, rGhostDir, varRGhost, LOCATION::REDGHOST, ghostRegular);
}

void pinkGhostMove(DIRECTION &dir) {
    if (dir == DIRECTION::RIGHT) {
        if (pacmanX + 3 < width && checkPermission(map[pacmanY][pacmanX + 3], ghostRegular)) {
            ghostMove(pGhostX, pGhostY, pacmanX + 3, pacmanY, pGhostDir, varPGhost, LOCATION::PINKGHOST, ghostRegular);
        }
        else ghostMove(pGhostX, pGhostY, pacmanX, pacmanY, pGhostDir, varPGhost, LOCATION::PINKGHOST, ghostRegular);
    }
    else if (dir == DIRECTION::LEFT) {
        if (pacmanX - 3 > 0 && checkPermission(map[pacmanY][pacmanX - 3], ghostRegular)) {
            ghostMove(pGhostX, pGhostY, pacmanX - 3, pacmanY, pGhostDir, varPGhost, LOCATION::PINKGHOST, ghostRegular);
        }
        else ghostMove(pGhostX, pGhostY, pacmanX, pacmanY, pGhostDir, varPGhost, LOCATION::PINKGHOST, ghostRegular);
    }
    else if (dir == DIRECTION::DOWN) {
        if (pacmanY + 3 < height && checkPermission(map[pacmanY + 3][pacmanX], ghostRegular)) {
            ghostMove(pGhostX, pGhostY, pacmanX, pacmanY + 3, pGhostDir, varPGhost, LOCATION::PINKGHOST, ghostRegular);
        }
        else ghostMove(pGhostX, pGhostY, pacmanX, pacmanY, pGhostDir, varPGhost, LOCATION::PINKGHOST, ghostRegular);
    }
    else {//Pink ghost UP
        if (pacmanX - 2 > 0 && pacmanY - 3 > 0 && checkPermission(map[pacmanY - 2][pacmanX - 3], ghostRegular)) {
            ghostMove(pGhostX, pGhostY, pacmanX - 2, pacmanY - 3, pGhostDir, varPGhost, LOCATION::PINKGHOST, ghostRegular);
        }
        else ghostMove(pGhostX, pGhostY, pacmanX, pacmanY, pGhostDir, varPGhost, LOCATION::PINKGHOST, ghostRegular);
    }
}

void blueGhostMove() {
    int targetX = 2 * pacmanX - rGhostX;
    int targetY = 2 * pacmanY - rGhostY;
    ghostMove(bGhostX, bGhostY, targetX, targetY, bGhostDir, varBGhost, LOCATION::BLUEGHOST, ghostRegular);
}

void orangeGhostMove() {
    if ((oGhostX - pacmanX) * (oGhostX - pacmanX) + (oGhostY - pacmanY) * (oGhostY - pacmanY) >= 49) {
        ghostMove(oGhostX, oGhostY, pacmanX, pacmanY, oGhostDir, varOGhost, LOCATION::ORANGEGHOST, ghostRegular);
    }
    else {
        ghostMove(oGhostX, oGhostY, 0, 31, oGhostDir, varOGhost, LOCATION::ORANGEGHOST, ghostRegular);
    }
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
    int stepCounter = 0;

    cout << "Press Enter\n";
    while (!(GetAsyncKeyState(VK_RETURN) & 0x8000)){}//wait enter 
    while (1) {
        show();
        //Sleep(100);
        //set direction and pacman move
        curDir = keyHandler();
        if (dir != curDir && curDir != DIRECTION::ELSE) {
            if (!pacmanMove(curDir, pacmanRegular)) pacmanMove(dir, pacmanRegular);
            else dir = curDir;
        } else pacmanMove(dir, pacmanRegular);
        //ghosts move logic
        if (!redGhostTimeOut) redGhostMove();
        if (!pinkGhostTimeOut && timer >= 1) { 
            if (!pStarted) {
                map[pGhostY][pGhostX] = varPGhost;
                varPGhost = (LOCATION)map[startPinkPath[stepCounter][0]][startPinkPath[stepCounter][1]];
                pGhostY = startPinkPath[stepCounter][0];
                pGhostX = startPinkPath[stepCounter][1];
                map[pGhostY][pGhostX] = LOCATION::PINKGHOST;
                if (stepCounter == startPinkPath.size()-1) {
                    pStarted = true;
                    stepCounter = 0;
                }
                ++stepCounter;
            } else pinkGhostMove(dir);
        }
        if (!blueGhostTimeOut && timer >= 20) {
            if (!bStarted) {
                map[bGhostY][bGhostX] = varBGhost;
                varBGhost = (LOCATION)map[startBluePath[stepCounter][0]][startBluePath[stepCounter][1]];
                bGhostY = startBluePath[stepCounter][0];
                bGhostX = startBluePath[stepCounter][1];
                map[bGhostY][bGhostX] = LOCATION::BLUEGHOST;
                if (stepCounter == startBluePath.size() - 1) {
                    bStarted = true;
                    stepCounter = 0;
                }
                ++stepCounter;
            }
            else blueGhostMove();
        }
        if (!orangeGhostTimeOut && timer >= 30) {
            if (!oStarted) {
                map[oGhostY][oGhostX] = varOGhost;
                varOGhost = (LOCATION)map[startOrangePath[stepCounter][0]][startOrangePath[stepCounter][1]];
                oGhostY = startOrangePath[stepCounter][0];
                oGhostX = startOrangePath[stepCounter][1];
                map[oGhostY][oGhostX] = LOCATION::ORANGEGHOST;
                if (stepCounter == startOrangePath.size() - 1) {
                    oStarted = true;
                    stepCounter = 0;
                }
                ++stepCounter;
            }
            else orangeGhostMove();
        }
        if (redGhostTimeOut) redGhostTimeOut = false;
        if (pinkGhostTimeOut) pinkGhostTimeOut = false;
        if (blueGhostTimeOut) blueGhostTimeOut = false;
        if (orangeGhostTimeOut) orangeGhostTimeOut = false;

        //ouput
        cout << "Score: " << score << "\n";
        ++timer;
    }
    return 0;
}
