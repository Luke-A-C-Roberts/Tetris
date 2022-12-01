#include <algorithm>
#include <array>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include <ncurses.h>
#include <time.h>
#include <unistd.h>

#define WIDTH 10
#define HEIGHT 30

std::array<std::string, 7> tetrinoTemplates {
    "XX  "
    " X  "
    " X  "
    "    ",

    " XX "
    " X  "
    " X  "
    "    ",

    "    "
    "XXX "
    " X  "
    "    ",

    "XX  "
    "XX  "
    "    "
    "    ",

    " X  "
    " X  "
    " X  "
    " X  ",

    " X  "
    "XX  "
    "X   "
    "    ",

    "X   "
    "XX  "
    " X  "
    "    "
};

std::array<int, 7> rotateSizes {3, 3, 3, 2, 4, 3, 3};

std::array<int, 7> colors {
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_WHITE
};

std::array<char, 7> types {'L','J','T','O','I','Z','S'};

class Block {
public:
    explicit Block(int x, int y, int c);
    int x;
    int y;
    int c;
};

Block::Block(int tempX, int tempY, int tempC) {
    x = tempX;
    y = tempY;
    c = tempC;
}

class Renderer {
private:
    const int rendwidth = WIDTH;
    const int rendheight = HEIGHT;

    auto switchColor(int) -> int;
public:
    Renderer();
    void render(WINDOW*, std::vector<Block>, int, int, int, int);
    void choiceScreen(WINDOW*, int, int);
};

Renderer::Renderer() {
    init_pair(1, COLOR_BLACK, COLOR_CYAN);
    init_pair(2, COLOR_BLACK, COLOR_RED);
    init_pair(3, COLOR_BLACK, COLOR_GREEN);
    init_pair(4, COLOR_BLACK, COLOR_BLUE);
    init_pair(5, COLOR_BLACK, COLOR_YELLOW);
    init_pair(6, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(7, COLOR_BLACK, COLOR_WHITE);
    init_pair(8, COLOR_WHITE, COLOR_BLACK);
}

int Renderer::switchColor(int c) {
    int colorNum;
    switch (c) {
        case (COLOR_CYAN): colorNum = 1; break;
        case (COLOR_RED): colorNum = 2; break;
        case (COLOR_GREEN): colorNum = 3; break;
        case (COLOR_BLUE): colorNum = 4; break;
        case (COLOR_YELLOW): colorNum = 5; break;
        case (COLOR_MAGENTA): colorNum = 6; break;
        case (COLOR_WHITE): colorNum = 7; break;
        default: colorNum = 1; break;
    }
    return colorNum;
}

void Renderer::render (
        WINDOW *win,
        std::vector<Block> tempBlocks,
        int score = 0,
        int level = 0,
        int lines = 0,
        int nextT = 0)
    {
    clear();
    int colorNum = 0;
    attron(COLOR_PAIR(8));

    for (int i = 0; i < (rendwidth * 2) + 1; ++i)
        mvwprintw(win, 0, i, "%s", "**");

    for (int i = 0; i < (rendwidth * 2) + 1; ++i)
        mvwprintw(win, rendheight + 1, i, "%s", "**");

    for (int i = 0; i < rendheight + 1; ++i)
        mvwprintw(win, i, 0, "%s", "*");

    for (int i = 0; i < rendheight + 1; ++i)
        mvwprintw(win, i, rendwidth * 2 + 1, "%s", "*");

    attroff(COLOR_PAIR(8));
    for (auto &b : tempBlocks) {
        colorNum = switchColor(b.c);
        attron(COLOR_PAIR(colorNum));
        mvwprintw(win, b.y + 1, (b.x * 2) + 1, "%s", "  ");
        attroff(COLOR_PAIR(colorNum));
    }

    auto nextTShape = tetrinoTemplates[nextT];
    auto nextTColor = switchColor(colors[nextT]);

    
    char c;
    const int rx = rendwidth + 14;
    const int ry = 1;

    attron(COLOR_PAIR(nextTColor));

    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            c = nextTShape[x + y * 4];
            if (c == 'X')
                mvwprintw(win, y + ry, 2 * x + rx, "%s", "  ");
        }
    }
    
    attroff(COLOR_PAIR(nextTColor));
    attron(COLOR_PAIR(8));
    mvwprintw(win, rendheight + 2, 1, "Score: %d", score);
    mvwprintw(win, rendheight + 3, 1, "Lines: %d", lines);
    mvwprintw(win, rendheight + 4, 1, "Level: %d", level);
    attroff(COLOR_PAIR(8));
}

void Renderer::choiceScreen(WINDOW* win, int xpos, int ypos) {
    int colorNum, level;
    clear();
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 10; ++x) {
            level = y * 10 + x;
            if (x == xpos && y == ypos)
                colorNum = 4;
            else {
                switch (y) {
                case 0: colorNum = 3; break;
                case 1: colorNum = 7; break;
                case 2: colorNum = 5; break;
                case 3: colorNum = 2; break;
                }
            }
            attron(COLOR_PAIR(colorNum));
            mvwprintw(win, y * 2 + 1, x * 3 + 1, "%d%s", level, (level < 10 ? " " : ""));
            attroff(COLOR_PAIR(colorNum));
        }
    }
    attron(COLOR_PAIR(8));
    mvwprintw(win, 9, 1, "%s", "press y to select");
    attroff(COLOR_PAIR(8));
}

class Board {
private:
    bool isNewBlocks;
    const int width = WIDTH;
    const int height = HEIGHT;
    std::vector<Block> permBlocks;
    std::array<Block, 4> newBlocks = {
        Block(0,0,0),
        Block(0,0,0),
        Block(0,0,0),
        Block(0,0,0)
    };

    void delNewBlocks();

public:
    Board();
    ~Board() = default;

    auto checkFullLines(void) -> std::vector<int>;
    auto getBlocks(void) -> std::vector<Block>;
    auto getPermBlocks(void) -> std::vector<Block>;
    bool addNewBlocks(std::array<std::pair<int, int>, 4>, int);
    bool checkCollision(void);
    bool checkGameOver(void);
    void makePermblocks(void);
    void removeLines(int, std::vector<int>);
};

Board::Board() {
    for (auto& b : newBlocks)
        b = Block(0,0,0);
    isNewBlocks = false;
}

std::vector<Block> Board::getBlocks(void) {
    if (!isNewBlocks)
        return permBlocks;
    auto allBlocks = permBlocks;
    for (auto& b : newBlocks)
        allBlocks.push_back(b);
    return allBlocks;
}

bool Board::addNewBlocks(std::array<std::pair<int, int>, 4> tempCoords, int tempColor) {
    bool intersects = false;
    isNewBlocks = true;
    for(int i = 0; i < 4; ++i) {
        Block tempBlock(tempCoords[i].first, tempCoords[i].second, tempColor);
        for (auto b : permBlocks) {
            if (b.x == tempBlock.x && b.y == tempBlock.y)
                intersects = true;
        }
        newBlocks[i] = tempBlock;
    }
    return intersects;
}

void Board::delNewBlocks(void) {
    isNewBlocks = false;
    for(auto& b : newBlocks) {
        b = Block(0,0,0);
}}

void Board::makePermblocks(void) {
    for(int i = 0; i < 4; ++i)
        permBlocks.push_back(newBlocks[i]);
    delNewBlocks();
}

std::vector<int> Board::checkFullLines(void) {
    std::vector<int> fullLines {};
    for (int r = 0; r < height; ++r) {
        int blocksNum = 0;
        for (auto& b : permBlocks)
            blocksNum += (b.y == r ? 1 : 0);
        if (blocksNum == width)
            fullLines.push_back(r);
    }
    return fullLines;
}

bool Board::checkCollision(void) {
    std::vector<std::pair<int, int>> collisionCoords;
    for (auto b : newBlocks)
        collisionCoords.push_back({b.x, b.y + 1});
    for (auto c : collisionCoords) {
        if (c.second == height)
            return true;  
    }
    for (auto b : permBlocks) {
        for (auto c : collisionCoords) {
            if (b.x == c.first && b.y == c.second)
            return true;
    }}
    return false;
}

void Board::removeLines(int level, std::vector<int> fullLines) {

    for (auto& i : fullLines) {
        permBlocks.erase (
            std::remove_if (
                permBlocks.begin(),
                permBlocks.end(),
                [&i](Block b){return (b.y == i);}
            ),
            permBlocks.end()
        );
        std::for_each (
            permBlocks.begin(),
            permBlocks.end(),
            [&i](Block &b){b.y < i ? ++b.y : 0;}
        );
    }
}

std::vector<Block> Board::getPermBlocks(void) {
    return permBlocks;
}

bool Board::checkGameOver(void) {
    for (int n = 0; n < newBlocks.size(); ++n) {
        for (int p = 0; p < permBlocks.size(); ++p) {
            std::pair<int, int> nCoord = {newBlocks[n].x, newBlocks[n].y};
            std::pair<int, int> pCoord = {permBlocks[p].x, permBlocks[p].y};
            if (pCoord.first == nCoord.first && pCoord.second == nCoord.second) 
                return true;
        }
    }
    for (int p0 = 0; p0 < permBlocks.size(); ++p0) {
        for (int p1 = 0; p1 < permBlocks.size(); ++p1) {
            if (p0 != p1 && permBlocks[p0].x == permBlocks[p1].x && permBlocks[p0].y == permBlocks[p1].y ) {
                return true;
            }
        }
    }
    return false;
}

class Tetrino {
private:
    const int width = WIDTH;
    const int height = HEIGHT;
    int rotateSize, color, x, y;
    std::array<std::pair<int, int>, 4> relativePositions;

    bool isIntersection(std::vector<Block>, std::array<std::pair<int, int>, 4>);
    
public:
    Tetrino();
    Tetrino(int);
    ~Tetrino() = default;

    auto getPositions(void) -> std::array<std::pair<int, int>, 4>;
    auto getColor(void) -> int;

    void moveRight(std::vector<Block>);
    void moveLeft(std::vector<Block>);
    void moveDown(std::vector<Block>);
    void rotate(std::vector<Block>);
};

Tetrino::Tetrino() {

    x = 4;
    y = 0;

    int r = std::rand() % 7;
    auto tempTetrinoStr = tetrinoTemplates[r];
    rotateSize = rotateSizes[r];
    color = colors[r];

    std::vector<std::pair<int, int>> tempPositions;
    char c;
    for (int i = 0; i < tempTetrinoStr.length(); ++i) {
        c = tempTetrinoStr[i];
        if (c == 'X') {
            tempPositions.push_back({i % 4, i / 4});
    }}
    for (int i = 0; i < relativePositions.size(); ++i) {
        relativePositions[i] = tempPositions[i];
}}

Tetrino::Tetrino(int r) {

    x = 4;
    y = 0;

    auto tempTetrinoStr = tetrinoTemplates[r];
    rotateSize = rotateSizes[r];
    color = colors[r];

    std::vector<std::pair<int, int>> tempPositions;
    char c;
    for (int i = 0; i < tempTetrinoStr.length(); ++i) {
        c = tempTetrinoStr[i];
        if (c == 'X') {
            tempPositions.push_back({i % 4, i / 4});
    }}
    for (int i = 0; i < relativePositions.size(); ++i) {
        relativePositions[i] = tempPositions[i];
}}

std::array<std::pair<int, int>, 4> Tetrino::getPositions(void) {
    auto copyCoords = relativePositions;
    // std::for_each (
    //     copyCoords.begin(),
    //     copyCoords.end(),
    //     [&tempX, &tempY](auto &p){p.first += tempX; p.second += tempY;}
    // );
    for (auto& c : copyCoords)
        c = {c.first + x, c.second + y};
    
    return copyCoords;
}

int Tetrino::getColor(void) {
    return color;
}

bool Tetrino::isIntersection(std::vector<Block> blocks, std::array<std::pair<int,int>, 4> positions) {
    for (auto& b : blocks) {
        for (auto& p : positions) {
            if (b.x == p.first && b.y == p.second)
                return true;
    }}
    return false;
}

void Tetrino::moveLeft(std::vector<Block> blocks) {
    auto copyCoords = getPositions();
    for (auto& c : copyCoords) {
        if (--c.first < 0)
            return;
    }
    if (isIntersection(blocks, copyCoords)) {
        return;
    }
    x--;
}

void Tetrino::moveRight(std::vector<Block> blocks) {
    auto copyCoords = getPositions();
    for (auto& c : copyCoords) {
        if (++c.first >= width)
           return;
    }
    if (isIntersection(blocks, copyCoords)) {
        return;
    }
    x++;
}

void Tetrino::moveDown(std::vector<Block> blocks) {
    auto copyCoords = getPositions();
    for (auto& c : copyCoords) {
        if (++c.second >= height)
            return;
    }
    if (isIntersection(blocks, copyCoords)) {
        return;
    }
    y++;
}

void Tetrino::rotate(std::vector<Block> blocks) {
    auto tempPositions = relativePositions;
    if (rotateSize == 2)
        return;
    for (auto &p : tempPositions) {
        p = {p.second, (rotateSize - 1) - p.first};
        if (p.first + x < 0 || p.first + x >= width || p.second + y < 0 || p.second + y >= height)
            return;
    }

    auto newRealPositions = tempPositions;
    for (auto& p : newRealPositions)
        p = {p.first + x, p.second + y};
    
    if (isIntersection(blocks, newRealPositions)) {
        return;
    }

    relativePositions = tempPositions;
}

int calcWait(int score) {
    if (score > 40)
        return 100000;
    else if (score < 0)
        return 1000000;
    return (int)(30000000 / (9 * score + 30));
}

int calcScore(int level, int lines) {
    int score = 0;

    switch (lines) {
    case 0: break;
    case 1: score = (level + 1) * 40; break;
    case 2: score = (level + 1) * 100; break;
    case 3: score = (level + 1) * 300; break;
    case 4: score = (level + 1) * 1200; break;
    default: break;
    }
    return score;
}

int calcFirstLineNum(int startLevel) {
    return 10 * startLevel + 10;
}

int main() {
    std::srand(time(NULL));

    initscr();
    start_color();
    keypad(stdscr, true);
    nodelay(stdscr, true);
    
    Tetrino *t = new Tetrino;
    Board b;
    Renderer r;

    int c;
    int level = 0;

    // int start = 0;
    bool levelChoiceMade = true;
    int choiceX = 0;
    int choiceY = 0;

    while (levelChoiceMade) {
        switch (c = getch()) {
        case KEY_UP:
            if (choiceY > 0)
                choiceY--;
            break;
        case KEY_LEFT:
            if (choiceX > 0)
                choiceX--;
            break;
        case KEY_RIGHT:
            if (choiceX < 9)
                choiceX++;
            break;
        case KEY_DOWN:
            if (choiceY < 3)
                choiceY++;
            break;

        case 'y':
            level = choiceY * 10 + choiceX;
            levelChoiceMade = false;
            break;
        }

        r.choiceScreen(stdscr, choiceX, choiceY);
        usleep(10000);
    }

    int time;
    int score = 0;
    int lines = 0;
    int wait  = calcWait(level);
    int lineNum = calcFirstLineNum(level);
    bool downPressed;
    int nextRand = std::rand() % 7;

    std::vector<int> fullLines;

    b.addNewBlocks(t -> getPositions(), t -> getColor());
    r.render(stdscr, b.getBlocks());

    while (!b.checkGameOver()) {
        time = 0;
        while (time < wait) {

            downPressed = false;
            switch (c = getch()) {
            case KEY_UP:
                t -> rotate(b.getPermBlocks());
                break;
            case KEY_LEFT:
                t -> moveLeft(b.getPermBlocks());
                break;
            case KEY_RIGHT:
                t -> moveRight(b.getPermBlocks());
                break;
            case KEY_DOWN:
                t -> moveDown(b.getPermBlocks());
                downPressed = true;
                break;
            }

            // if (downPressed) {
            //     usleep(5000);
            //     break;
            // }

            b.addNewBlocks(t -> getPositions(), t -> getColor());
            r.render(stdscr, b.getBlocks(), score, level, lines, nextRand);

            usleep(10000);
            time += 10000;
            r.render(stdscr, b.getBlocks(), score, level, lines, nextRand);
        }

        if (!downPressed)
            t -> moveDown(b.getPermBlocks());

        if (b.checkCollision()) {
            usleep(10000);
            b.makePermblocks();
            delete t;
            Tetrino *t = new Tetrino(nextRand);
            nextRand = std::rand() % 7;
        }

        fullLines = b.checkFullLines();
        if (!fullLines.empty()) {
            usleep(10000);
            b.removeLines(level, fullLines);
            lines += fullLines.size();
            score += calcScore(level, fullLines.size());
        }

        if (lines >= lineNum) {
            lineNum = 10;
            lines = 0;
            level++;
            wait = calcWait(level);
        }
    }
    getch();

    endwin();
    return 0;
}