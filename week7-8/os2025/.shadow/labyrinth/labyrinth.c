#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <testkit.h>
#include "labyrinth.h"
#include <getopt.h>
//用于查找迷宫中有没有这个东西。
void printMap(Labyrinth *labyrinth) {
    for (int r = 0; r < labyrinth->rows; r++) {
        for (int c = 0; c < labyrinth->cols; c++) {
            putchar(labyrinth->map[r][c]);
        }
        putchar('\n');
    }
}


struct option long_options[] = {
    {"map", required_argument, 0, 'm'},
    {"player", required_argument, 0, 'p'},
    {"move", required_argument, 0, 'v'},  // 修改为 'v'，避免和 map 冲突
    {"version", no_argument, 0, 'V'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[]) {
    const char *mapFile = NULL;
    char playerId = '\0';
    const char *direction = NULL;
    int version_flag = 0;
    int help_flag = 0;

    int opt;
    while ((opt = getopt_long(argc, argv, "m:p:v:Vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'm':
                mapFile = optarg;
                break;
            case 'p':
                playerId = optarg[0];
                break;
            case 'v':  // 处理 move 选项
                direction = optarg;
                break;
            case 'V':
                version_flag = 1;
                break;
            case 'h':
                help_flag = 1;
                break;
            case '?':
                printUsage();
                return 1;
            default:
                printUsage();
                return 1;
        }
    }

    // 检查 --version 和 --help 的独占性
    if (version_flag || help_flag) {
        if (argc != 2) {  // 确保没有其他参数
            printUsage();
            return 1;
        }
        if (version_flag) {
            printf("Labyrinth Game v1.0\n");
            return 0;
        }
        if (help_flag) {
            printUsage();
            return 0;
        }
    }

    // 验证必需参数
    if (!mapFile || !playerId || !isValidPlayer(playerId)) {
        printUsage();
        return 1;
    }

    Labyrinth labyrinth;
    if (!loadMap(&labyrinth, mapFile)) {
        return 1;
    }

    if (direction) {  // 如果指定了移动方向
        if (!movePlayer(&labyrinth, playerId, direction)) {
            fprintf(stderr, "Move failed\n");
            return 1;
        }
        // 移动成功后，必须保存到原始地图文件
        if (!saveMap(&labyrinth, mapFile)) {
            fprintf(stderr, "Failed to save map\n");
            return 1;
        }
    }

    printMap(&labyrinth);
    return 0;
}
//注意Labyrinth的定义中就包含了，所以我们直接存入Labyrinth即可
void printUsage() {
    printf("Usage:\n");
    printf("  labyrinth --map map.txt --player id\n");
    printf("  labyrinth -m map.txt -p id\n");
    printf("  labyrinth --map map.txt --player id --move direction\n");
    printf("  labyrinth --version\n");
}

bool isValidPlayer(char playerId) {
    // TODO: Implement this function
    return (playerId >= '0' && playerId <= '9');
}

bool loadMap(Labyrinth *labyrinth, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return false;
    }

    char tempMap[MAX_ROWS][MAX_COLS];
    int row = 0, maxCols = 0;
    char line[MAX_COLS + 2];

    while (fgets(line, sizeof(line), file)) {
        int col = strlen(line);
        if (line[col - 1] == '\n') {
            line[col - 1] = '\0';
            col--;
        }
        if (col > MAX_COLS) {
            fclose(file);
            fprintf(stderr, "Error: Map is too wide!\n");
            return false;
        }
        if (row >= MAX_ROWS) {
            fclose(file);
            fprintf(stderr, "Error: Map is too tall!\n");
            return false;
        }
        strcpy(tempMap[row], line);
        if (col > maxCols) maxCols = col;
        row++;
    }
    fclose(file);

    labyrinth->rows = row;
    labyrinth->cols = maxCols;
    memcpy(labyrinth->map, tempMap, sizeof(tempMap));

    // 检查地图格式
    for (int r = 0; r < labyrinth->rows; r++) {
        for (int c = 0; c < labyrinth->cols; c++) {
            char ch = labyrinth->map[r][c];
            if (ch != '.' && ch != '#' && !isValidPlayer(ch)) {
                fprintf(stderr, "Error: Invalid character in map!\n");
                return false;
            }
        }
    }

    if (!isConnected(labyrinth)) {
        fprintf(stderr, "Error: Maze is not connected!\n");
        return false;
    }
    saveMap(labyrinth, "map.txt");
    return true;
}
bool saveMap(Labyrinth *labyrinth, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Cannot open file for writing: %s\n", filename);
        return false;
    }

    // 确保写入完整的地图
    for (int r = 0; r < labyrinth->rows; r++) {
        for (int c = 0; c < labyrinth->cols; c++) {
            if (fputc(labyrinth->map[r][c], file) == EOF) {
                fprintf(stderr, "Error writing to file\n");
                fclose(file);
                return false;
            }
        }
        if (fputc('\n', file) == EOF) {
            fprintf(stderr, "Error writing newline to file\n");
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;
}
Position findPlayer(Labyrinth *labyrinth, char playerId) {
    Position pos = {-1, -1};
    for (int r = 0; r < labyrinth->rows; r++) {
        for (int c = 0; c < labyrinth->cols; c++) {
            if (labyrinth->map[r][c] == playerId) {
                pos.row = r;
                pos.col = c;
                return pos; // 直接返回找到的位置
            }
        }
    }
    return pos; // 未找到，返回默认的 {-1, -1}
}


Position findFirstEmptySpace(Labyrinth *labyrinth) {//这里的空地应该不算玩家
    Position pos = {-1, -1};
    for (int r = 0; r < labyrinth->rows; r++) {
        for (int c = 0; c < labyrinth->cols; c++) {
            if (labyrinth->map[r][c] == '.') {
                pos.row = r;
                pos.col = c;
                return pos; // 直接返回
            }
        }
    }
    return pos;
}


bool isEmptySpace(Labyrinth *labyrinth, int row, int col) {
    if (row < 0 || row >= labyrinth->rows || col < 0 || col >= labyrinth->cols) {
        return false;
    }
    if(labyrinth->map[row][col] >= '0' && labyrinth->map[row][col] <= '9'){
        return true;
    }
    return true;
}

bool movePlayer(Labyrinth *labyrinth, char playerId, const char *direction) {
    Position pos = findPlayer(labyrinth, playerId);
    Position newPos = pos;

    // 玩家不存在时找第一个空位
    if (pos.row == -1 && pos.col == -1) {
        pos = findFirstEmptySpace(labyrinth);
        if (pos.row == -1) {
            fprintf(stderr, "No empty space available\n");
            return false;
        }
        labyrinth->map[pos.row][pos.col] = playerId;
        return saveMap(labyrinth, "map.txt");
    }

    // 计算新位置
    if (strcmp(direction, "up") == 0) {
        newPos.row = pos.row - 1;
        newPos.col = pos.col;
    } else if (strcmp(direction, "down") == 0) {
        newPos.row = pos.row + 1;
        newPos.col = pos.col;
    } else if (strcmp(direction, "left") == 0) {
        newPos.row = pos.row;
        newPos.col = pos.col - 1;
    } else if (strcmp(direction, "right") == 0) {
        newPos.row = pos.row;
        newPos.col = pos.col + 1;
    } else {
        fprintf(stderr, "Invalid direction\n");
        return false;
    }

    // 边界检查
    if (newPos.row < 0 || newPos.row >= labyrinth->rows || 
        newPos.col < 0 || newPos.col >= labyrinth->cols) {
        fprintf(stderr, "Move out of bounds\n");
        return false;
    }

    // 检查新位置是否为墙
    if (labyrinth->map[newPos.row][newPos.col] == '#') {
        fprintf(stderr, "Cannot move into wall\n");
        return false;
    }
    if(labyrinth->map[newPos.row][newPos.col] >= '0' && labyrinth->map[newPos.row][newPos.col] <= '9'){
        fprintf(stderr, "Cannot move into another player\n");
        return false;
    }
     // 移动成功后不需要在这里调用 saveMap，
    // 而是在 main 函数中保存到原始地图文件
    char temp = labyrinth->map[pos.row][pos.col];
    labyrinth->map[pos.row][pos.col] = '.';
    labyrinth->map[newPos.row][newPos.col] = temp;

    return true;
}

// Check if all empty spaces are connected using DFS
//找到第一个，如果能完全遍历就是可以的
void dfs(Labyrinth *labyrinth, int row, int col, bool visited[MAX_ROWS][MAX_COLS]) {
    if (row < 0 || row >= labyrinth->rows || col < 0 || col >= labyrinth->cols || visited[row][col]) {
        return;
    }
    if (!isEmptySpace(labyrinth, row, col)) return;
    visited[row][col] = true;
    dfs(labyrinth, row + 1, col, visited);
    dfs(labyrinth, row - 1, col, visited);
    dfs(labyrinth, row, col + 1, visited);
    dfs(labyrinth, row, col - 1, visited);
}

bool isConnected(Labyrinth *labyrinth) {
    bool visited[MAX_ROWS][MAX_COLS] = {false};
    Position start = findFirstEmptySpace(labyrinth);
    if (start.row == -1) return true; // 没有空地，视为连通
    dfs(labyrinth, start.row, start.col, visited);

    for (int r = 0; r < labyrinth->rows; r++) {
        for (int c = 0; c < labyrinth->cols; c++) {
            if (isEmptySpace(labyrinth, r, c) && !visited[r][c]) {
                return false;
            }
        }
    }
    return true;
}

