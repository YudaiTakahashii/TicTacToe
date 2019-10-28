#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <list>
typedef std::list<int> Int_List; 
 
// TicTacToeEx ゲーム管理クラス
class T3EBoard
{
	static const int BOARD_WIDTH = 3;	// 盤の幅
	static const int BOARD_HEIGHT = 3;	// 盤の高さ
	static const int BOARD_SIZE = BOARD_WIDTH * BOARD_HEIGHT;	// 盤のサイズ
	static const int MAX_PIECE_ON_BOARD = 3;	// 盤上に存在できるコマの数
	static const int MAX_TURNS = 99;	// 最大ターン数
	static const int NUM_PLAYERS = 2;	// プレイヤー数
	enum { PLAYER1 = 0, PLAYER2 = 1 };
	enum { PLAYER_HUMAN = 0, PLAYER_CPU = 1 };
	enum { NONE = 0, STONE1 = 1, STONE2 = -1 };
 
	int m_Board[BOARD_SIZE];	// 盤の状態 0:なし 正:先手コマ 負:後手コマ 値:置いたターン数
	int m_Turn;					// ターン
	int m_NextPlayer;			// 次の番 PLAYER1 or PLAYER2
	int m_Player[NUM_PLAYERS];	// プレイヤー 0:人間 1:CPU
 
	void Reset();				// ゲーム初期化
	void ShowBoard();			// 盤表示
	void SetStone(int place);	// コマを置く
	int CPU();					// CPUの思考
	int Human();				// 人間の手番
	int CheckWin();				// 勝利判定
 
	/*-----------------ここから自分で追加した部分------------------*/
	const int max_depth = 8;
	const int Line[8][3] = {
		{0, 1, 2}, {3, 4, 5}, {6, 7, 8},
		{0, 3, 6}, {1, 4, 7}, {2, 5, 8},
		{0, 4, 8}, {2, 4, 6}
	};
	
	//現在の盤の状況をみて，置けるかどうか判断する
	bool Can_Put_Value(int place, int pop_place, Int_List& stone_place);
	//ミニマックス法で探索する
	int Min_Max(int depth, Int_List& stone_place);
	//評価関数(列，行，対角にどの程度丸または×が埋まっているか調べる)
	int Evaluate(int depth, Int_List& stone_place);
	//stone_placeの状態を返す(デバック用)
	void Stone_Show(Int_List& stone_place);
	//石が置かれている場所を，置かれた順に格納するリストを生成する
	void Build_Stoneplace(Int_List& stone_place);
	bool Search_Win_or_Lose(Int_List& stone_place);
	/*-------------------ここまで自分が実装した部分------------------*/
public:
	T3EBoard();					// コンストラクタ
	~T3EBoard();				// デストラクタ
	void Play();				// ゲーム進行
};

 
// コンストラクタ
T3EBoard::T3EBoard()
{
	srand((unsigned int)clock());
	this->Reset();
}
 
// デストラクタ
T3EBoard::~T3EBoard()
{
}
 
// ゲーム初期化
void T3EBoard::Reset()
{
	m_Turn = 1;
	m_NextPlayer = PLAYER1;
	for (int i = 0; i < BOARD_SIZE; i++) {
		m_Board[i] = NONE;
	}
}
 
// ゲーム進行
void T3EBoard::Play()
{
	// ゲーム初期化
	std::cout << "拡張版○× ゲーム" << std::endl << std::endl;
	std::cout << "先手は 0:人間 1:CPU ?";
	std::cin >> m_Player[PLAYER1];
	std::cout << "後手は 0:人間 1:CPU ?";
	std::cin >> m_Player[PLAYER2];
	std::cout << "始め！" << std::endl;
 
	// ターン進行
	do {
		// ターン情報
		std::cout << std::endl << "ターン " << m_Turn << std::endl;
		this->ShowBoard();
		std::cout << (m_NextPlayer == PLAYER1 ? "先手" : "後手")
			<< "の番です" << std::endl;
 
		int place = (m_Player[m_NextPlayer] == PLAYER_HUMAN ? Human() : CPU());
		this->SetStone(place);
		int winner = this->CheckWin();
		if (winner >= 0) {
			this->ShowBoard();
			std::cout << std::endl << (m_NextPlayer == PLAYER1 ? "先手" : "後手") << " 勝利" << std::endl;
			break;
		}
 
		// 次の番へ
		m_NextPlayer = 1 - m_NextPlayer;
		if (m_NextPlayer == PLAYER1) m_Turn++;
		if (m_Turn >= MAX_TURNS) std::cout << std::endl << "引き分け！" << std::endl;
	} while (m_Turn < MAX_TURNS);
 
	std::cin.ignore(std::cin.rdbuf()->in_avail());	// 入力バッファクリア
	std::cin.get();	// 終了前のキー入力待ち
}
 
//------------------------------------ここから自分で実装--------------------------------
 
// CPUの思考アルゴリズム
// 戻り値：コマを打つ場所
int T3EBoard::CPU()
{
	//どこに石が置かれたかを，置かれた順に記憶するリスト(関数化したい?)
	Int_List stone_place;
	//石が置かれている場所を，置かれた順に格納するリストを生成する
	Build_Stoneplace(stone_place);
	//デバック用
	Stone_Show(stone_place);
	return this->Min_Max(0, stone_place);
}
 
 
 
void T3EBoard::Build_Stoneplace(Int_List& stone_place)
{
	//置かれているコマの絶対値の中でもっとも小さい値と大きい値
	int min_stone, max_stone;
	max_stone = m_Turn;
	//6ターン目以降かどうかでmin_stoneの値は異なる
	//---------------------m_Turn-3かどうかは再度考える必要あり----------------
	if (m_Turn > MAX_PIECE_ON_BOARD) min_stone = m_Turn - 3;
	else min_stone = 1;
 
	//絶対値が小さいコマから各自コマの場所を保存していく
	for (int stone_value = min_stone; stone_value <= max_stone; stone_value++) {
		for (int place = 0; place < BOARD_SIZE; place++) {
			if (m_Board[place] == stone_value) {
				stone_place.push_back(place);
			}
		}
		for (int place = 0; place < BOARD_SIZE; place++) {
			if (m_Board[place] == -stone_value) {
				stone_place.push_back(place);
			}
		}
	}
}
 
//stone_placeの状態を返す(デバック用)
void T3EBoard::Stone_Show(Int_List& stone_place)
{
	std::list<int>::iterator p = stone_place.begin();
	while (p != stone_place.end()) {
		std::cout << *p;
		p++;
	}
 
	std::cout << std::endl;
}
 
/*
Min-Maxによるゲーム木の探索
引数
depth : ノードの深さ
stone_place : コマが置いてある場所を置いた順に格納するリスト
is_first : 自分が先攻か後攻か調べる(評価関数の呼び出しで必要）
 
戻り値
CPU()からの呼び出し時 : 最適なコマの場所
再帰呼び出しの時 : ノードの評価値
*/
int T3EBoard::Min_Max(int depth, Int_List& stone_place)
{
	//探索が指定した深さまで達したら．その盤面を評価する
	//勝ちが確定した時と負けが確定した時にも評価するようにするべき
	//勝ち探索アルゴリズムと負け探索アルゴリズムを改良して使用
	if (depth == max_depth || Search_Win_or_Lose(stone_place))	
		return this->Evaluate(depth, stone_place);
 
	int child_value;	//子ノードの評価値
	int best_place = -1;		//評価値のもっとも高い場所
	int value;	//評価値
	if(depth % 2 == 0)	value = -10000;
	else value = 10000;
	int pop_place = -1;	//次のターンに消えるコマ
	if (depth == 0 && stone_place.size() > MAX_PIECE_ON_BOARD * 2) {
		pop_place = stone_place.front();
	}

	int pop_val = -1;
 
	for (int place = 0; place < BOARD_SIZE; place++) {
		//盤の場所placeにコマがおける時
		if (this->Can_Put_Value(place, pop_place, stone_place)) {
			stone_place.push_back(place);
			//次のターンに消えるコマがある時は，そのコマを一時的に削除
			//削除するコマはpop_valに保存
			if (stone_place.size() > MAX_PIECE_ON_BOARD * 2 + 1) {
				pop_val = stone_place.front();
				stone_place.pop_front();
			}
			//子ノードの評価値を計算する
			child_value = this->Min_Max(depth + 1, stone_place);

			//自分のノードの時は評価値valueを高い値に更新する
			//相手のノードの時は評価値valueを低い値に更新する
			if(depth % 2 == 0){
				if (child_value > value) {
					value = child_value;
					best_place = place;
				}
			}
			else{
				if (child_value < value) {
					value = child_value;
					best_place = place;
				}
			}
			//一時的に削除したコマを元に戻す
			if (pop_val != -1)	stone_place.push_front(pop_val);
			//一時的に追加したコマを元に戻す
			stone_place.pop_back();
		}
	}
 
	//ノードの深さが0まで戻ってきていたら，最適解を返す
	if (depth == 0) {
		std::cout << "best_place = " << best_place << std::endl;
		std::cout << "best_value = " << value << std::endl;
		return best_place;
	}
	//ノードの深さが0でないときは，その点での評価値を返す
	else	return value;
}
 
/*
盤面の指定した場所にコマが置けるかチェックする関数(Min-Max()内で用いる）
引数
place : 置きたい場所(この場所に置けるかチェックする）
pop_place : 次の手で消えるコマ（stone_placeには格納されてない可能性があるが，そこには置けない）
stone_place : コマが置いてある場所を置いた順に格納するリスト
 
戻り値
置ける場合 : true
置かない場合 : false
*/
bool T3EBoard::Can_Put_Value(int place, int pop_place, Int_List& stone_place)
{
	if (place == pop_place) return false;
	std::list<int>::iterator p = stone_place.begin();
	while (p != stone_place.end()) {
		if (*p == place)	return false;
		p++;
	}
	return true;
}

bool T3EBoard::Search_Win_or_Lose(Int_List& stone_place) {
	int count_mystone = 0;
	int count_opponentstone = 0;
	int count;
	std::list<int>::iterator p;

	int line_pattern = sizeof(Line) / sizeof(Line[0]);
	int line_size = sizeof(Line[0]) / sizeof(int);
 
	for (int i = 0; i < line_pattern; i++) {
		count_mystone = 0;
		count_opponentstone = 0;
		for(int j = 0; j < line_size; j++) {
			p = stone_place.begin();
			if(stone_place.size() > 6)	p++;
			count = 0;
			while (p != stone_place.end()) {
				if (Line[i][j] == *p) {
					if (count % 2 == 0)	count_mystone++;
					else count_opponentstone++;
				}
				p++;
				count++;
			}
		}
		if (count_mystone == 3 || count_opponentstone == 3) return true;
	}
	return false;
}
 
/*
各ノードの評価値を返す関数
引数
depth : ノードの深さ
stone_place : コマが置いてある場所を置いた順に格納するリスト
is_first : 自分が先攻か後攻か調べる(評価関数の呼び出しで必要）
 
戻り値
各ノードの評価値
*/
//depthが浅ければ浅いほど，　評価値が高くなるようにしたい
int T3EBoard::Evaluate(int depth, Int_List& stone_place)
{
	int const bingo_point = 10;
	int const rearch_point = 1;
 
	//---------最大深さまで到達せずに呼び出された(途中で勝敗は決まったとき)-----------
	//深さが奇数...自分が勝利したことによって終了
	//深さが偶数...相手が勝利したことによって終了
	if(depth < max_depth){
		if(depth % 2 == 0)	
			return -bingo_point * (max_depth - depth) * 10;
		else	
			return bingo_point * (max_depth - depth) * 10;
	}


	//----------最大深さまで到達した時---------------------------
	//その時の盤の優勢を定義して，それに応じた評価値を与える

	//stone_placeを，実際の盤の様子に変換する
	int copy_board[BOARD_SIZE];
	for (int i = 0; i < BOARD_SIZE; i++) 	copy_board[i] = 0;
	std::list<int>::iterator p = stone_place.begin();
	p++;	//最初の要素は次に消えるコマなので評価しない
	int count = 0;
	while (p != stone_place.end()) {
		if(max_depth % 2 == 0){
			if(count % 2 == 0) copy_board[*p] = STONE1;
			else copy_board[*p] = STONE2;
		}
		else{
			if(count % 2 == 0) copy_board[*p] = STONE2;
			else copy_board[*p] = STONE1;
		}
		count++;
		p++;
	}
 
	
	int value = 0;	//評価値
	//１列に自分のコマもしくは相手のコマがどれだけあるか調べる変数
	int count_mystone, count_opponentstone = 0;
 
	//３目並べの縦・横・斜めのラインの本数(8)
	int line_pattern = sizeof(Line) / sizeof(Line[0]);
	//1ラインにおける要素数
	int line_size = sizeof(Line[0]) / sizeof(int);
 
	//各ラインにおける自分のコマと相手のコマの数を調べる
	for (int i = 0; i < line_pattern; i++) {
		count_mystone = 0;
		count_opponentstone = 0;
		for (int j = 0; j < line_size; j++) {
			if (copy_board[Line[i][j]] == 1)		count_mystone++;
			else if (copy_board[Line[i][j]] == -1) count_opponentstone++;
		}
		if (count_mystone == 2)  value += rearch_point;
		else if (count_opponentstone == 2)  value -= rearch_point;
	}
	return value;
}
 
 

 
//------------------------------------------ここまで自分で実装----------------------------------------
 
// 人間の手番
// 戻り値：コマを打つ場所
int T3EBoard::Human()
{
	int place = 0;
	while (1) {
		std::cout << "0:左上 1:上 2:右上 3:左 4:中央 5:右 6:左下 7:下 8:右下" << std::endl
			<< "どこに打ちますか？ ";
		std::cin >> place;
		if (place >= 0 && place < BOARD_SIZE) {
			if (m_Board[place] == 0) break;
		}
	}
 
	return place;
}
 
// 盤にコマを置く
// 引数 int place : 置く位置
void T3EBoard::SetStone(int place)
{
	int sign = m_NextPlayer == PLAYER1 ? STONE1 : STONE2;
	m_Board[place] = m_Turn * sign;
 
	// 古いコマを消す
	if (m_Turn > MAX_PIECE_ON_BOARD) {
		int deleting = (m_Turn - MAX_PIECE_ON_BOARD) * sign;
		for (int i = 0; i < BOARD_SIZE; i++) {
			if (m_Board[i] == deleting) m_Board[i] = 0;
		}
	}
}
 
// 勝利判定
// 戻り値：-1 未決着 0(PLAYER1) 先手勝利 1(PLAYER2) 後手勝利
int T3EBoard::CheckWin()
{
	int row, col;
	bool check;
	int sign[2] = { STONE1, STONE2 };
 
	// プレイヤー毎に判定
	for (int player = PLAYER1; player <= PLAYER2; player++) {
		// 斜め方向1
		for (row = 0, check = true; row < BOARD_HEIGHT; row++) {
			if (m_Board[row * BOARD_WIDTH + row] * sign[player] <= 0) check = false;
		}
		if (check) return player;
 
		// 斜め方向2
		for (row = 0, check = true; row < BOARD_HEIGHT; row++) {
			if (m_Board[row * BOARD_WIDTH + (BOARD_HEIGHT - 1 - row)] * sign[player] <= 0) check = false;
		}
		if (check) return player;
 
		// 横方向
		for (row = 0; row < BOARD_HEIGHT; row++) {
			for (col = 0, check = true; col < BOARD_WIDTH; col++) {
				if (m_Board[row * BOARD_WIDTH + col] * sign[player] <= 0) check = false;
			}
			if (check) return player;
		}
 
		// 縦方向
		for (col = 0; col < BOARD_WIDTH; col++) {
			for (row = 0, check = true; row < BOARD_HEIGHT; row++) {
				if (m_Board[row * BOARD_WIDTH + col] * sign[player] <= 0) check = false;
			}
			if (check) return player;
		}
	}
 
	return -1;
}
 
// 盤表示
void T3EBoard::ShowBoard()
{
	for (int i = 0; i < BOARD_SIZE; i++) {
		std::cout << (m_Board[i] == 0 ? "　" :
			(m_Board[i] > 0 ? "○ " : "× "));
		if (i % BOARD_WIDTH < BOARD_WIDTH - 1) {
			std::cout << " | ";
		}
		else {
			std::cout << std::endl;
			if (i < BOARD_SIZE - 1) std::cout << " ─ ┼  ─  ┼ ─" << std::endl;
		}
	}
}

 
int main()
{
	T3EBoard board;
	board.Play();
 
	return 0;
}