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
	const int max_depth = 3;
	int Estimated_value[BOARD_SIZE];
	const int Line[8][3] = {
		{0, 1, 2}, {3, 4, 5}, {6, 7, 8},
		{0, 3, 6}, {1, 4, 7}, {2, 5, 8},
		{0, 4, 8}, {2, 4, 6}
	};
	void Search_Checkmate();
	void Search_Place_toProtect();
	//現在の盤の状況をみて，置けるかどうか判断する
	bool Can_Put_Value(int place, Int_List stone_place);
	//ミニマックス法で探索する
	int Min_Max(int depth, Int_List& stone_place, bool is_first);
	//評価関数(列，行，対角にどの程度丸または×が埋まっているか調べる)
	int Evaluate(int depth, Int_List& stone_place, bool is_first);
	//stone_placeの状態を返す(デバック用)
	void Stone_Show(Int_List& stone_place);
	//石が置かれている場所を，置かれた順に格納するリストを生成する
	//ついでに，自分が先攻か後攻かを戻り値として返す
	bool Build_Stoneplace(Int_List& stone_place);
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
 
// CPUの思考アルゴリズム
// 戻り値：コマを打つ場所
int T3EBoard::CPU()
{
	//どこに石が置かれたかを，置かれた順に記憶するリスト(関数化したい?)
	Int_List stone_place;
	//石が置かれている場所を，置かれた順に格納するリストを生成する
	//ついでに，自分が先攻か後攻かを戻り値として返す
	bool is_first = Build_Stoneplace(stone_place);
	//デバック用
	Stone_Show(stone_place);
	return this->Min_Max(0, stone_place, is_first);
}

bool T3EBoard::Build_Stoneplace(Int_List& stone_place)
{
	//今までにおかれているコマの数(自分が先攻か後攻か判断するために用いる)
	int num_stone = 0;
	//stone_placeに一番最初に格納したコマの場所に入っていた値
	int first_val = 0;
	//置かれているコマの絶対値の中でもっとも小さい値と大きい値
	int min_stone, max_stone; 
	max_stone = m_Turn;
	//6ターン目以降かどうかでmin_stoneの値は異なる
	//---------------------m_Turn-3かどうかは再度考える必要あり----------------
	if (m_Turn > MAX_PIECE_ON_BOARD) min_stone = m_Turn - 3;
	else min_stone = 1;
 
	//絶対値が小さいコマから各自コマの場所を保存していく
	for(int stone_value = min_stone; stone_value<= max_stone; stone_value++){
		for(int place=0; place<BOARD_SIZE; place++){
			if (m_Board[place] == stone_value) {
				if(num_stone == 0) first_val = m_Board[place];
				stone_place.push_back(place);
				num_stone++;
			}
		}
		for(int place = 0; place < BOARD_SIZE; place++){
			if (m_Board[place] == -stone_value) {
				if(num_stone == 0) first_val = m_Board[place];
				stone_place.push_back(place);
				num_stone++;
			}
		}
	}

	//m_Turnが3以下の時...今までのコマの数が偶数であったら先攻
	//m_Turnが4より大きい時...first_valが正の値だったら先攻
	//今までに置かれたコマの数が偶数であったら先攻
	if(m_Turn <= MAX_PIECE_ON_BOARD){
		if(num_stone % 2 == 0) 	return true;
		else return false;
	}
	else{
		if(first_val > 0)	return true;
		else return false;
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
 
 
int T3EBoard::Min_Max(int depth, Int_List &stone_place, bool is_first)
{
	//探索が指定した深さまで達したら．その盤面を評価する
	//勝ちが確定した時と負けが確定した時にも評価するようにするべき
	//勝ち探索アルゴリズムと負け探索アルゴリズムを改良して使用
	if(depth == max_depth)	return this->Evaluate(depth, stone_place, is_first);
 
	int child_value;	//子ノードの評価値
	static int best_place = -1;		//評価値のもっとも高い場所
	static int value = -10000;		//現在のノードの評価値
	//１回目の呼び出しの時(再帰呼び出しではなく，CPU()からの呼び出しの時)に初期化
	if (depth == 0) {
		best_place = -1;
		value = -10000;
	}
	
	for(int place = 0; place < BOARD_SIZE; place++){
		if (this->Can_Put_Value(place, stone_place)){
			stone_place.push_back(place);
			if(stone_place.size() > MAX_PIECE_ON_BOARD * 2 + 1)	
				stone_place.pop_front();
			
			child_value = this->Min_Max(depth + 1, stone_place, is_first);
			if(child_value > value){
				value = child_value;
				best_place = place;
			}
			//デバック用
			//Stone_Show(stone_place, value);
			stone_place.pop_back();
		}
	}
 
	//ノードの深さが0まで戻ってきていたら，最適解を返す
	if (depth == 0) {
		std::cout << "best_place = " << best_place << std::endl;
		return best_place;
	}
	//ノードの深さが0でないときは，その点での評価値を返す
	else			return value;
}

bool T3EBoard::Can_Put_Value(int place, Int_List stone_place)
{
    std::list<int>::iterator p = stone_place.begin();
 
	while(p!=stone_place.end()){
		if(*p == place)	return false;
		p++;
	}
	return true;
}
 

//depthが浅ければ浅いほど，　評価値が高くなるようにしたい
int T3EBoard::Evaluate(int depth, Int_List &stone_place, bool is_first)
{	
	int copy_board[BOARD_SIZE];
	for(int i=0; i<BOARD_SIZE; i++){
		copy_board[i] = 0;
	}
	std::list<int>::iterator p = stone_place.begin();
	int count = 0;
	while(p != stone_place.end()){
		//自分のコマの時
		if (is_first) {
			if (count % 2 == 0) copy_board[*p] = 1;
			else copy_board[*p] = -1;
		}
		else {
			if (count % 2 == 0) {
				if (count % 2 == 0) copy_board[*p] = -1;
				else copy_board[*p] = 1;
			}
		}
		p++;
		count++;
	}
 
	int const bingo_point = 1000;
	int const rearch_point = 100;
	int const re_rearch_point = 10;
 
	int value = 0;
	int count_mystone = 0;
	int count_yourstone = 0;
	
	int line_pattern = sizeof(Line) / sizeof(Line[0]);
	int line_size = sizeof(Line[0])/ sizeof(int);
 
	for(int i=0; i < line_pattern; i++){
		count_mystone = 0;
		count_yourstone = 0;
		for(int j=0; j<line_size; j++){
			if(copy_board[Line[i][j]] == 1)		count_mystone++;
			else if(copy_board[Line[i][j]] == -1) count_yourstone++;
		}
		if(count_mystone == 3)	value += bingo_point;
		else if(count_yourstone == 3) value -= bingo_point/2;
		else if(count_mystone == 2 && count_yourstone == 0) 
			value += rearch_point;
		else if(count_mystone == 0 && count_yourstone == 2){
			value -= rearch_point;
		}
		else if(count_mystone == 1 && count_yourstone != 2){
			value += re_rearch_point;
		}
		else if(count_mystone == 0 && count_yourstone == 1){
			value -= re_rearch_point;
		}
	}
	//std::cout << "val = " << value << std::endl;
	return value;
}
 
/*
リーチが発生しているところがあるか調べる関数(つまり、次に置くと勝つ手があるかしらべる)
リーチが発生している場合: 次に置くべき場所(0-8)を返す
リーチが発生していない場合: -1を返す
*/
void T3EBoard::Search_Checkmate()
{
	const int point = 100;	//条件を満たしたときに加算される点数
	int copy_Board[BOARD_SIZE];	//盤面をコピーして保存する用の変数
	for (int i = 0; i < BOARD_SIZE; i++) copy_Board[i] = m_Board[i];
 
	//CPUが〇と×のどちらを打つのか調べる（〇...STONE1=1, ×...STONE2=-1)
	int sign = (m_NextPlayer == PLAYER1 ? STONE1 : STONE2);
 
	// 次に消えるコマを見つけて、一時的に消去する
	int deleting_place = -1;
	if (m_Turn > MAX_PIECE_ON_BOARD) {
		int deleting = (m_Turn - MAX_PIECE_ON_BOARD) * sign;
		for (int i = 0; i < BOARD_SIZE; i++) {
			if (copy_Board[i] == deleting) copy_Board[i] = NONE;
			deleting_place = i;
		}
	}
 
	//コンピューターが後攻の時、〇と×を入れ替える(次のifでの条件判定のため）
	if (sign == STONE2) {
		for (int i = 0; i < BOARD_SIZE; i++)		copy_Board[i] = -1 * copy_Board[i];
	}
 
	//どの目に置くと勝てるのかそれぞれ調べる
	//斜め方向1
	int empty_place;		//まだ何も置いていないような場所を保存する
	int count = 0;	//それぞれの列、行、対角線にどのくらい自分のコマがおいてあるか数える
	for (int row = 0; row < BOARD_HEIGHT; row++) {
		if (copy_Board[row * BOARD_WIDTH + row] > 0) count++;
		else if (copy_Board[row * BOARD_WIDTH + row] == 0) empty_place = row * BOARD_WIDTH + row;
	}
	if (count == 2 && empty_place > 0 && empty_place != deleting_place)	Estimated_value[empty_place] += point;
	else {
		count = 0;
		empty_place = -1;
	}
 
	// 斜め方向2
	for (int row = 0; row < BOARD_HEIGHT; row++) {
		if (copy_Board[row * BOARD_WIDTH + (BOARD_HEIGHT - 1 - row)] > 0) count++;
		else if (copy_Board[row * BOARD_WIDTH + (BOARD_HEIGHT - 1 - row)] == 0) empty_place = row * BOARD_WIDTH + (BOARD_HEIGHT - 1 - row);
	}
	if (count == 2 && empty_place > 0 && empty_place != deleting_place)	Estimated_value[empty_place] += point;
	else {
		count = 0;
		empty_place = -1;
	}
 
	// 横方向
	for (int row = 0; row < BOARD_HEIGHT; row++) {
		for (int col = 0; col < BOARD_WIDTH; col++) {
			if (copy_Board[row * BOARD_WIDTH + col] > 0) count++;
			else if (copy_Board[row * BOARD_WIDTH + col] == 0) empty_place = row * BOARD_WIDTH + col;
		}
		if (count == 2 && empty_place > 0 && empty_place != deleting_place)	Estimated_value[empty_place] += point;
		else {
			count = 0;
			empty_place = -1;
		}
	}
	
 
	// 縦方向
	for (int col = 0; col < BOARD_WIDTH; col++) {
		for (int row = 0; row < BOARD_HEIGHT; row++) {
			if (copy_Board[row * BOARD_WIDTH + col] > 0) count++;
			else if (copy_Board[row * BOARD_WIDTH + col] == 0) empty_place = row * BOARD_WIDTH + col;
		}
		if (count == 2 && empty_place > 0 && empty_place != deleting_place)	Estimated_value[empty_place] += point;
		else {
			count = 0;
			empty_place = -1;
		}
	}
}
 
/*
相手にリーチが発生しているか調べる関数(そこに置かないと負けてしまう場所があるか調べる)
リーチが発生している場合: 次に置くべき場所(0-8)を返す
リーチが発生していない場合: -1を返す
*/
void T3EBoard::Search_Place_toProtect()
{
	const int point = 50;	//条件を満たしたときに50点加算する
	int copy_Board[BOARD_SIZE];	//盤面をコピーして保存する用の変数
	for (int i = 0; i < BOARD_SIZE; i++) copy_Board[i] = m_Board[i];
 
	//CPUが〇と×のどちらを打つのか調べる（〇...STONE1=1, ×...STONE2=-1)
	int sign = (m_NextPlayer == PLAYER1 ? STONE1 : STONE2);
 
	//次の相手のターンが何番目なのか調べる
	int Next_Turn_of_Opponent;
	if (sign == STONE1)	Next_Turn_of_Opponent = m_Turn;
	else Next_Turn_of_Opponent = m_Turn + 1;
 
	// 相手の次に消えるコマを見つけて、一時的に消去する
	int deleting_place = -1;
	if (Next_Turn_of_Opponent > MAX_PIECE_ON_BOARD) {
		int deleting = (Next_Turn_of_Opponent - MAX_PIECE_ON_BOARD) * sign * (-1);
		for (int i = 0; i < BOARD_SIZE; i++) {
			if (copy_Board[i] == deleting) copy_Board[i] = NONE;
			deleting_place = i;
		}
	}
 
	//コンピューターが先攻の時、〇と×を入れ替える(次のifでの条件判定のため）
	//相手の手が常に〇を出しているように変更させる
	if (sign == STONE1) {
		for (int i = 0; i < BOARD_SIZE; i++)		copy_Board[i] = -1 * copy_Board[i];
	}
 
	//どの手を打たないと負けるか調べる
	//斜め方向1
	int empty_place = -1;		//まだ何も置いていないような場所を保存する
	int count = 0;	//それぞれの列、行、対角線にどのくらい相手のコマがおいてあるか数える
	for (int row = 0; row < BOARD_HEIGHT; row++) {
		if (copy_Board[row * BOARD_WIDTH + row] > 0) count++;
		else if (copy_Board[row * BOARD_WIDTH + row] == 0) empty_place = row * BOARD_WIDTH + row;
	}
	if (count == 2 && empty_place > 0 && empty_place != deleting_place)	Estimated_value[empty_place] += point;
	else {
		count = 0;
		empty_place = -1;
	}
 
	// 斜め方向2
	for (int row = 0; row < BOARD_HEIGHT; row++) {
		if (copy_Board[row * BOARD_WIDTH + (BOARD_HEIGHT - 1 - row)] > 0) count++;
		else if (copy_Board[row * BOARD_WIDTH + (BOARD_HEIGHT - 1 - row)] == 0) empty_place = row * BOARD_WIDTH + (BOARD_HEIGHT - 1 - row);
	}
	if (count == 2 && empty_place > 0 && empty_place != deleting_place)	Estimated_value[empty_place] += point;
	else {
		count = 0;
		empty_place = -1;
	}
 
	// 横方向
	for (int row = 0; row < BOARD_HEIGHT; row++) {
		for (int col = 0; col < BOARD_WIDTH; col++) {
			if (copy_Board[row * BOARD_WIDTH + col] > 0) count++;
			else if (copy_Board[row * BOARD_WIDTH + col] == 0) empty_place = row * BOARD_WIDTH + col;
		}
		if (count == 2 && empty_place > 0 && empty_place != deleting_place)	Estimated_value[empty_place] += point;
		else {
			count = 0;
			empty_place = -1;
		}
	}
 
 
	// 縦方向
	for (int col = 0; col < BOARD_WIDTH; col++) {
		for (int row = 0; row < BOARD_HEIGHT; row++) {
			if (copy_Board[row * BOARD_WIDTH + col] > 0) count++;
			else if (copy_Board[row * BOARD_WIDTH + col] == 0) empty_place = row * BOARD_WIDTH + col;
		}
		if (count == 2 && empty_place > 0 && empty_place != deleting_place)	Estimated_value[empty_place] += point;
		else {
			count = 0;
			empty_place = -1;
		}
	}
}
 
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