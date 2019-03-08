#include <ncurses.h>
#include <unistd.h>

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
			// 無
			if (playField[h][w] == FREE) {
				mvprintw(h, w, " ");
			}
		}
	}
}

// テトリミノをフィールドへ設定する
void setTetrimino(int baseX, int baseY, int setBuf[TETRIMINO_HEIGHT][TETRIMINO_WIDTH]) {
	for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
		for (int w = 0; w < TETRIMINO_WIDTH; w++) {
			if (setBuf[h][w] == CONTROL) {
				playField[baseY + h][baseX + w] = setBuf[h][w];
			}
		}
	}
}

// テトリミノをフィールドから取り除く
void unsetTetrimino(int baseX, int baseY, int setBuf[TETRIMINO_HEIGHT][TETRIMINO_WIDTH]) {
	for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
		for (int w = 0; w < TETRIMINO_WIDTH; w++) {
			if (setBuf[h][w] == CONTROL) {
				playField[baseY + h][baseX + w] = FREE;
			}
		}
	}
}

int main() {
	// 画面を初期化
	initscr();
	// キー入力を1000ミリ秒で切り上げる(タイムアウトする)
	timeout(1000);

	// テトリミノのフィールド上の基準位置
	int x = 0;
	int y = 0;

	// 入力
	int ch = 0;
	while (ch != 'q') {
		// テトリミノを設定する
		setTetrimino(FALL_BASE_X + x, FALL_BASE_Y + y, tetriminos[0]);
	
		// フィールドを描画する
		drawField();

		// 入力待ち
		ch = getch();

		// テトリミノを取り除く
		unsetTetrimino(FALL_BASE_X + x, FALL_BASE_Y + y, tetriminos[0]);

		// テトリミノを落とす
		y += 1;
	}

	// 画面を終了
	endwin();
}
