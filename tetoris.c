#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

// フィールドの高さ(床含む)
#define FIELD_HEIGHT (21)
// フィールドの幅(壁含む)
#define FIELD_WIDTH (12)

// テトリミノの最大の高さ
#define TETRIMINO_HEIGHT (4)
// テトリミノの最大の幅
#define TETRIMINO_WIDTH (4)
// テトリミノの種類数
#define TETRIMINO_KINDS (7)

// 落下位置
#define FALL_BASE_X (1)
#define FALL_BASE_Y (0)

// キー入力のタイムアウト時間(ミリ秒)
#define KEYINPUT_TIMEOUT_TIME (20)
// ブロック落下の間隔時間(秒)
#define FALL_TIME (1.0)

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
		{4,0,0,0},
		{4,0,0,0},
		{4,0,0,0},
		{4,0,0,0}
	},
	{
		{0,4,0,0},
		{4,4,4,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{
		{4,4,0,0},
		{0,4,4,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{
		{0,4,4,0},
		{4,4,0,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{
		{4,4,0,0},
		{4,4,0,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{
		{4,4,0,0},
		{4,0,0,0},
		{4,0,0,0},
		{0,0,0,0}
	},
	{
		{4,4,0,0},
		{0,4,0,0},
		{0,4,0,0},
		{0,0,0,0}
	}
};

// 制御中のテトリミノ
int inControlTetrimino[TETRIMINO_HEIGHT][TETRIMINO_WIDTH];

// 操作中のテトリミノのフィールド上の基準位置
int currentTetriminoPositionX = 0;
int currentTetriminoPositionY = 0;

// アプリケーションの状態
enum appState {
	// 実行中
	RUNNING,
	// 終了待ち
	EXIT_WAIT
};

// アプリケーションの状態
int currentAppState = RUNNING;

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

// テトリミノを配置すると衝突する場合には1を返す
int isCollision(int baseX, int baseY, int setBuf[TETRIMINO_HEIGHT][TETRIMINO_WIDTH]) {
	for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
		for (int w = 0; w < TETRIMINO_WIDTH; w++) {
			if (setBuf[h][w] == CONTROL) {
				if (playField[baseY + h][baseX + w] != FREE) {
					return 1;
				}
			}
		}
	}
	return 0;
}

// 制御中のテトリミノを移動する
void moveInControlTetrimino(int baseX, int baseY) {
	// 衝突しなければ座標を変更
	if (isCollision(baseX, baseY, inControlTetrimino) == 0) {
		currentTetriminoPositionX = baseX;
		currentTetriminoPositionY = baseY;
	}
}

// 制御中のテトリミノを回転する
// isClockwise: 1なら時計回り, 0なら反時計回り
void rotateInControlTetrimino(int isClockwise) {
	int buf[TETRIMINO_HEIGHT][TETRIMINO_WIDTH];
	// 回転後のテトリミノの配置を一時バッファへ入れる
	if (isClockwise == 1) {
		// 時計回り
		for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
			for (int w = 0; w < TETRIMINO_WIDTH; w++) {
				buf[h][w] = inControlTetrimino[(TETRIMINO_HEIGHT-1)-w][h];
			}
		}
	} else {
		// 反時計回り
		for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
			for (int w = 0; w < TETRIMINO_WIDTH; w++) {
				buf[h][w] = inControlTetrimino[w][(TETRIMINO_WIDTH-1)-h];
			}
		}
	}
	// 衝突しなければバッファを置き換え
	int baseX = currentTetriminoPositionX;
	int baseY = currentTetriminoPositionY;
	if (isCollision(baseX, baseY, buf) == 0) {
		for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
			for (int w = 0; w < TETRIMINO_WIDTH; w++) {
				inControlTetrimino[h][w] = buf[h][w];
			}
		}
	}
}

// キー入力に応じてプレイヤー操作を行う
void playerOperate(int ch) {
	// テトリミノを動かす
	switch (ch) {
		case 'l':
			// 右移動キー
			moveInControlTetrimino(currentTetriminoPositionX + 1, currentTetriminoPositionY);
			break;
		case 'h':
			// 左移動キー
			moveInControlTetrimino(currentTetriminoPositionX - 1, currentTetriminoPositionY);
			break;
		case 'r':
			// 回転キー
			rotateInControlTetrimino(1);
			break;
		case 'e':
			// 逆回転キー
			rotateInControlTetrimino(0);
			break;
		case 'q':
			// 終了キー
			currentAppState = EXIT_WAIT;
			break;
	}
}

// 新しいテトリミノを制御中バッファに設定する
void setNewControlTetrimino(int kind) {
	assert(0 <= kind && kind <= TETRIMINO_KINDS);
	for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
		for (int w = 0; w < TETRIMINO_WIDTH; w++) {
			inControlTetrimino[h][w] = tetriminos[kind][h][w];
		}
	}
}

int main() {
	// 画面を初期化
	initscr();
	// キー入力を切り上げる時間を設定
	timeout(KEYINPUT_TIMEOUT_TIME);

	// テトリミノの初期位置を設定
	currentTetriminoPositionX = FALL_BASE_X;
	currentTetriminoPositionY = FALL_BASE_Y;

	// 制御中のテトリミノを設定
	setNewControlTetrimino(1);

	// 入力
	int ch = 0;
	while (currentAppState == RUNNING) {
		// 現在時刻を取得して保存
		clock_t baseTime = time(NULL);

		while (1) {
			// テトリミノを設定する
			setTetrimino(currentTetriminoPositionX, currentTetriminoPositionY, inControlTetrimino);
		
			// フィールドを描画する
			drawField();
	
			// 入力待ち
			ch = getch();
	
			// テトリミノを取り除く
			unsetTetrimino(currentTetriminoPositionX, currentTetriminoPositionY, inControlTetrimino);
	
			// プレイヤー操作の反映
			playerOperate(ch);

			// 現在時刻が保存した時刻と比較してブロック落下の間隔時間を超えていない限り繰り返し
			clock_t currentTime = time(NULL);
			double diff = difftime(currentTime, baseTime);
			if (diff >= FALL_TIME) {
				break;
			}
		}

		// テトリミノを落とす
		moveInControlTetrimino(currentTetriminoPositionX, currentTetriminoPositionY + 1);
	}

	// 画面を終了
	endwin();
}
