#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

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

// ブロックの色
enum colorPair {
	// フィールド境界の色
	BOUNDARY_COLOR,
	// コントロール中のブロックの色
	IN_CONTROL_COLOR,
	// 固定ブロックの色
	FIX_COLOR
};

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

// 現在のスコア
int currentGameScore = 0;

// アプリケーションの状態
enum appState {
	// 実行中
	RUNNING,
	// 終了待ち
	EXIT_WAIT,
	// ゲームオーバー
	GAME_OVER,
};

// アプリケーションの状態
int currentAppState = RUNNING;

// 色が使えるか
int enableColor = 0;

// =========================
// オプション用変数
// =========================
// 色を使うか
int useColorDrawing = 0;

// 引数のテトリミノバッファを枠外のボックスに描画(デバッグ用)
void drawTetriminoBox(int buf[TETRIMINO_WIDTH][TETRIMINO_HEIGHT]) {
	int drawBaseY = 11;
	int drawBaseX = FIELD_WIDTH + 2;

	for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
		for (int w = 0; w < TETRIMINO_WIDTH; w++) {
			mvprintw(drawBaseY + h, drawBaseX + w, "%d", buf[h][w]);
		}
	}
}

// フィールドを描画する
void drawField() {
	int h, w;
	for (h = 0; h < FIELD_HEIGHT; h++) {
		for (w = 0; w < FIELD_WIDTH; w++) {
			// 壁
			if (playField[h][w] == WALL) {
				if (enableColor) {
					color_set(BOUNDARY_COLOR, NULL);
				}
				mvprintw(h, w, "|");
			}
			// 床
			if (playField[h][w] == FLOOR) {
				if (enableColor) {
					color_set(BOUNDARY_COLOR, NULL);
				}
				mvprintw(h, w, "=");
			}
			// 操作中のブロック
			if (playField[h][w] == CONTROL) {
				if (enableColor) {
					color_set(IN_CONTROL_COLOR, NULL);
					mvprintw(h, w, " ");
				} else {
					mvprintw(h, w, "@");
				}
			}
			// 固定ブロック
			if (playField[h][w] == FIX) {
				if (enableColor) {
					color_set(FIX_COLOR, NULL);
					mvprintw(h, w, " ");
				} else {
					mvprintw(h, w, "#");
				}
			}
			// 無
			if (playField[h][w] == FREE) {
				if (enableColor) {
					color_set(BOUNDARY_COLOR, NULL);
				}
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
// 移動できなければ0を返す
int moveInControlTetrimino(int baseX, int baseY) {
	// 衝突しなければ座標を変更
	if (isCollision(baseX, baseY, inControlTetrimino) == 0) {
		currentTetriminoPositionX = baseX;
		currentTetriminoPositionY = baseY;
		return 1;
	}
	return 0;
}

// 制御中のテトリミノを回転する
// isClockwise: 1なら時計回り, 0なら反時計回り
void rotateInControlTetrimino(int isClockwise) {
	int buf[TETRIMINO_HEIGHT][TETRIMINO_WIDTH] = {};
	
	// 回転後のテトリミノの配置を一時バッファへ入れる
	if (isClockwise == 1) {
		// 時計回り
		for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
			for (int w = 0; w < TETRIMINO_WIDTH; w++) {
				int pw = (TETRIMINO_HEIGHT - 1) - w;
				int ph = h;
				assert(0 <= pw && pw < TETRIMINO_WIDTH);
				assert(0 <= ph && ph < TETRIMINO_HEIGHT);
				buf[h][w] = inControlTetrimino[pw][ph];
			}
		}
	} else {
		// 反時計回り
		for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
			for (int w = 0; w < TETRIMINO_WIDTH; w++) {
				int pw = w;
				int ph = (TETRIMINO_WIDTH - 1) - h;
				assert(0 <= pw && pw < TETRIMINO_WIDTH);
				assert(0 <= ph && ph < TETRIMINO_HEIGHT);
				buf[h][w] = inControlTetrimino[pw][ph];
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
		case 'j':
			// 下移動キー
			moveInControlTetrimino(currentTetriminoPositionX, currentTetriminoPositionY + 1);
			break;
		case 'k':
			// 高速落下移動キー
			while (moveInControlTetrimino(currentTetriminoPositionX, currentTetriminoPositionY + 1)) {
				;
			}
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
	assert(0 <= kind && kind < TETRIMINO_KINDS);
	for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
		for (int w = 0; w < TETRIMINO_WIDTH; w++) {
			inControlTetrimino[h][w] = tetriminos[kind][h][w];
		}
	}
}

// 新しいテトリミノをランダムに作成し, 制御中バッファに設定する
void generateTetrimino(int x, int y) {
	currentTetriminoPositionX = x;
	currentTetriminoPositionY = y;
	setNewControlTetrimino(rand() % TETRIMINO_KINDS);
}

// 現在操作中のテトリミノを固定して、次のテトリミノを用意する
void fixTetrimino() {
	// 操作中のテトリミノをフィールドバッファへ設定する
	setTetrimino(currentTetriminoPositionX, currentTetriminoPositionY, inControlTetrimino);

	// フィールドバッファの操作中テトリミノを固定
	for (int h = 0; h < TETRIMINO_HEIGHT; h++) {
		for (int w = 0; w < TETRIMINO_WIDTH; w++) {
			int ph = currentTetriminoPositionY + h;
			int pw = currentTetriminoPositionX + w;

			// 壁と床を考慮
			if ((0 < ph && ph < FIELD_HEIGHT - 1) &&
				(0 < pw && pw < FIELD_WIDTH - 1)) {
				assert(ph < FIELD_HEIGHT - 1);
				assert(0 < pw && pw < FIELD_WIDTH - 1);
				// 制御中を固定に変換
				if (playField[currentTetriminoPositionY+h][currentTetriminoPositionX+w] == CONTROL) {
					playField[currentTetriminoPositionY+h][currentTetriminoPositionX+w] = FIX;
				}
			}
		}
	}
}

// 指定行が完成していれば1
int isCompleteLine(int line) {
	for (int w = 0; w < FIELD_WIDTH; w++) {
		// 固定と壁以外が見つかった
		if (playField[line][w] != FIX && playField[line][w] != WALL) {
			return 0;
		}
	}
	return 1;
}

// 指定行を消去する
void eraseLine(int line) {
	for (int w = 0; w < FIELD_WIDTH; w++) {
		// 固定は削除(FREEへ変更)
		if (playField[line][w] == FIX) {
			playField[line][w] = FREE;
		}
	}
}

// 指定行より上の行を指定行まで下へ詰める
void compaction(int line) {
	for (int h = line; h > 0; h--) {
		for (int w = 0; w < FIELD_WIDTH; w++) {
			playField[h][w] = playField[h-1][w];
		}
	}
	// 一番上はFREEで詰める
	eraseLine(0);
}

// フィールド内の完成した行を消去する
void eraseCompleteLine() {
	for (int h = 0; h < FIELD_HEIGHT; h++) {
		if (isCompleteLine(h)) {
			eraseLine(h);
			compaction(h);
			// 安直に1行消すと10ptにする
			currentGameScore += 10;
		}
	}
}

// 操作ガイドを表示
void showOperationGuide() {
	mvprintw(3, FIELD_WIDTH + 1, "r: rotate clockwise");
	mvprintw(4, FIELD_WIDTH + 1, "e: rotate counter-clockwise");
	mvprintw(5, FIELD_WIDTH + 1, "h: move left");
	mvprintw(6, FIELD_WIDTH + 1, "l: move right");
	mvprintw(7, FIELD_WIDTH + 1, "j: move down");
	mvprintw(8, FIELD_WIDTH + 1, "k: fast move down");
	mvprintw(9, FIELD_WIDTH + 1, "q: quit game");
}

// スコアを表示
void showScore() {
	mvprintw(1, FIELD_WIDTH + 1, "SCORE: %d", currentGameScore);
}

// 色ペアを初期化する
void initializeColorPair() {
	init_pair(BOUNDARY_COLOR, COLOR_BLACK, COLOR_WHITE);
	init_pair(IN_CONTROL_COLOR, COLOR_BLACK, COLOR_BLUE);
	init_pair(FIX_COLOR, COLOR_BLACK, COLOR_GREEN);
}

// 色設定の初期化
void initializeColor() {
	// 色を有効化
	start_color();
	// 色ペアの初期化
	initializeColorPair();
	// 色使える?
	if (useColorDrawing == 1 && has_colors() == TRUE) {
		enableColor = 1;
	}
}

// アプリケーションの初期化
void initializeApp() {
	// 画面を初期化
	initscr();
	// 入力エコーを無効
	noecho();
	// 入力バッファリングを無効
	cbreak();
	// キー入力を切り上げる時間を設定
	timeout(KEYINPUT_TIMEOUT_TIME);
	// 色設定の初期化
	initializeColor();

	// テトリミノ生成用の乱数の種を用意
	srand((unsigned)time(NULL));

	// 最初のテトリミノを生成
	generateTetrimino(FALL_BASE_X, FALL_BASE_Y);

	// ゲームスコア欄を表示
	showScore();

	// 操作ガイドを表示
	showOperationGuide();
}

// ゲームオーバー画面を表示
void showGameOverScreen() {
	// ゲームオーバー画面を表示
	mvprintw(0, FIELD_WIDTH + 1, "GAME OVER");
	// ブロッキングモードにしてキー入力まで待つ
	timeout(-1);
	// キー入力待ちに入る
	getch();
}

// ゲームのメインループ
void gameLoop() {
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
		int is_fallen = moveInControlTetrimino(currentTetriminoPositionX, currentTetriminoPositionY + 1);
		// 落ちなければ固定して, 行を消し, 次のテトリミノを用意する
		if (!is_fallen) {
			fixTetrimino();
			eraseCompleteLine();
			generateTetrimino(FALL_BASE_X, FALL_BASE_Y);
			// 次のテトリミノが配置できなければゲームオーバー
			if (isCollision(FALL_BASE_X, FALL_BASE_Y, inControlTetrimino)) {
				currentAppState = GAME_OVER;
			}
		}
		// ゲームスコアを更新
		showScore();
	}

	// ゲームオーバー画面を表示
	if (currentAppState == GAME_OVER) {
		showGameOverScreen();
	}
}

// オプションの解析と設定
void parseOption(int argc, char *argv[]) {
	if (argc == 2) {
		if (strncmp(argv[1], "color", strlen("color")) == 0) {
			useColorDrawing = 1;
		}
	}
}

int main(int argc, char *argv[]) {
	// 表示オプションの設定
	parseOption(argc, argv);

	// アプリケーションの初期化
	initializeApp();

	// ゲームのメインループ
	gameLoop();

	// 画面を終了
	endwin();
}
