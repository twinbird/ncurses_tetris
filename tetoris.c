#include <ncurses.h>

// フィールドの高さ(床含む)
#define FIELD_HEIGHT (21)
// フィールドの幅(壁含む)
#define FIELD_WIDTH (12)

// テトリミノの最大の高さ
#define TETRIMINO_HEIGHT (4)
// テトリミノの最大の幅
#define TETRIMINO_WIDTH (3)
// テトリミノの種類数
#define TETRIMINO_KINDS (7)

// 落下位置
#define FALL_BASE_X (1)
#define FALL_BASE_Y (0)

// ブロックの種類
enum blockType {
	// ブロックがない
	FREE,
	// 壁
	WALL,
	// 床
	FLOOR,
	// 固定されたブロック
	FIX,
	// 制御中のブロック
	CONTROL
};

// フィールド
int playField[FIELD_HEIGHT][FIELD_WIDTH] = {
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{2,2,2,2,2,2,2,2,2,2,2,2}
};

// テトリミノ
int tetriminos[TETRIMINO_KINDS][TETRIMINO_HEIGHT][TETRIMINO_WIDTH] = {
	{
		{4,0,0},
		{4,0,0},
		{4,0,0},
		{4,0,0}
	},
	{
		{0,4,0},
		{4,4,4},
		{0,0,0},
		{0,0,0}
	},
	{
		{4,4,0},
		{0,4,4},
		{0,0,0},
		{0,0,0}
	},
	{
		{0,4,4},
		{4,4,0},
		{0,0,0},
		{0,0,0}
	},
	{
		{4,4,0},
		{4,4,0},
		{0,0,0},
		{0,0,0}
	},
	{
		{4,4,0},
		{4,0,0},
		{4,0,0},
		{0,0,0}
	},
	{
		{4,4,0},
		{0,4,0},
		{0,4,0},
		{0,0,0}
	}
};

// フィールドを描画する
void drawField() {
	int h, w;
	for (h = 0; h < FIELD_HEIGHT; h++) {
		for (w = 0; w < FIELD_WIDTH; w++) {
			// 壁
			if (playField[h][w] == WALL) {
				mvprintw(h, w, "|");
			}
			// 床
			if (playField[h][w] == FLOOR) {
				mvprintw(h, w, "=");
			}
			// ブロック
			if (playField[h][w] == CONTROL) {
				mvprintw(h, w, "@");
			}
		}
	}
}

// テトリミノをフィールドへ設定する
void setTetrimino(int baseX, int baseY, int setBuf[TETRIMINO_HEIGHT][TETRIMINO_WIDTH]) {
	for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
		for (int w = 0; w < TETRIMINO_WIDTH; w++) {
			playField[baseY + h][baseX + w] = setBuf[h][w];
		}
	}
}

int main() {
	// 画面を初期化
	initscr();

	// テトリミノを設定する
	setTetrimino(FALL_BASE_X, FALL_BASE_Y, tetriminos[0]);

	// フィールドを描画する
	drawField();

	// qが入力されるまで無限に待つ
	int ch = 0;
	while (ch != 'q') {
		ch = getch();
	}

	// 画面を終了
	endwin();
}
