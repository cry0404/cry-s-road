#define MAX_ROWS 100
#define MAX_COLS 100
#define VERSION_INFO "Labyrinth Game"

typedef struct {
    char map[MAX_ROWS][MAX_COLS];
    int rows; //整个迷宫的定义
    int cols; //其实就是map的编号对应
} Labyrinth;

typedef struct {
    int row;
    int col;
} Position;


bool isValidPlayer(char playerId);
bool loadMap(Labyrinth *labyrinth, const char *filename);
Position findPlayer(Labyrinth *labyrinth, char playerId);//找到玩家
Position findFirstEmptySpace(Labyrinth *labyrinth);//找到第一个空的位置
bool isEmptySpace(Labyrinth *labyrinth, int row, int col);
bool movePlayer(Labyrinth *labyrinth, char playerId, const char *direction);
bool saveMap(Labyrinth *labyrinth, const char *filename);
bool isConnected(Labyrinth *labyrinth);
