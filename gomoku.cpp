#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include <thread>
#include <chrono>
#include <random>

using namespace std;

class GomokuGame {
private:
    vector<vector<char>> board;
    char currentPlayer;
    pair<int, int> lastMove;

    bool isBoardFull() const {
        for (const auto& row : board) {
            for (char cell : row) {
                if (cell == ' ') {
                    return false; // 盤面に空いている場所があればfalseを返す
                }
            }
        }
        return true; // 盤面が全て埋まっていたらtrueを返す
    }

    bool checkWin(int row, int col) const {
        char player = board[row][col];
        return (checkDirection(row, col, 1, 0, player) || // 横
            checkDirection(row, col, 0, 1, player) || // 縦
            checkDirection(row, col, 1, 1, player) || // 斜め（右下）
            checkDirection(row, col, 1, -1, player)); // 斜め（左下）
    } // 各方向にコマが5つ連続しているか確認する

    bool checkDirection(int row, int col, int dRow, int dCol, char player) const {
        int count = 0;
        for (int i = -4; i <= 4; ++i) {
            int newRow = row + i * dRow;
            int newCol = col + i * dCol;
            if (newRow >= 0 && newRow < 9 && newCol >= 0 && newCol < 9 && board[newRow][newCol] == player) {
                if (++count == 5) {
                    return true; // コマが5つ連続していたらtrueを返す
                }
            }
            else {
                count = 0; // カウントをリセット
            }
        }
        return false;
    }

    int countConsecutive(int row, int col, int dRow, int dCol, char player) const {
        int count = 0;
        for (int i = 1; i <= 4; ++i) {
            int newRow = row + i * dRow;
            int newCol = col + i * dCol;
            if (newRow >= 0 && newRow < 9 && newCol >= 0 && newCol < 9 && board[newRow][newCol] == player) {
                count++;
            }
            else {
                break;
            }
        } // 正方向にコマが連続している数を数える
        for (int i = -1; i >= -4; --i) {
            int newRow = row + i * dRow;
            int newCol = col + i * dCol;
            if (newRow >= 0 && newRow < 9 && newCol >= 0 && newCol < 9 && board[newRow][newCol] == player) {
                count++;
            }
            else {
                break;
            }
        } // 負方向にコマが連続している数を数える
        return count; // コマが連続している数を返す
    }

    pair<int, int> findBestMove(char player, char opponent) const {
        vector<pair<int, int>> bestMoves;
        pair<vector<int>, char> maxCount;
        pair<vector<int>, char> priorityCount;
        maxCount.first = vector<int>(4, -1);  // ベクトルの初期化

        for (auto row = board.begin(); row != board.end(); ++row) {
            for (auto col = row->begin(); col != row->end(); ++col) {
                int rowIdx = row - board.begin();
                int colIdx = col - row->begin();
                if (*col == ' ') {
                    vector<int> playerCounts = {
                        countConsecutive(rowIdx, colIdx, 1, 0, player), // 横
                        countConsecutive(rowIdx, colIdx, 0, 1, player), // 縦
                        countConsecutive(rowIdx, colIdx, 1, 1, player), // 斜め（右下）
                        countConsecutive(rowIdx, colIdx, 1, -1, player) // 斜め（左下）
                    };
                    vector<int> opponentCounts = {
                        countConsecutive(rowIdx, colIdx, 1, 0, opponent), // 横
                        countConsecutive(rowIdx, colIdx, 0, 1, opponent), // 縦
                        countConsecutive(rowIdx, colIdx, 1, 1, opponent), // 斜め（右下）
                        countConsecutive(rowIdx, colIdx, 1, -1, opponent) // 斜め（左下）
                    };

                    sort(playerCounts.rbegin(), playerCounts.rend()); // playerCountsベクトルを降順にソート
                    sort(opponentCounts.rbegin(), opponentCounts.rend()); // opponentCountsベクトルを降順にソート

                    int maxPlayerCount = playerCounts[0];
                    int maxOpponentCount = opponentCounts[0];

                    if (maxOpponentCount >= maxPlayerCount) {
                        priorityCount = { opponentCounts, opponent };
                    }
                    else {
                        priorityCount = { playerCounts, player };
                    } // プレイヤーとCPUのコマの最大連続数を比較

                    if (priorityCount == maxCount) {
                        bestMoves.push_back({ rowIdx, colIdx });
                    }
                    else {
                        for (int i = 0; i <= 3; ++i) {
                            if (priorityCount.first[i] > maxCount.first[i]) {
                                maxCount = priorityCount;
                                bestMoves.clear();
                                bestMoves.push_back({ rowIdx, colIdx });
                                break;
                            }
                            else if (priorityCount.first[i] == maxCount.first[i]) {
                                if (priorityCount.second != maxCount.second) {
                                    if (priorityCount.second == opponent) {
                                        bestMoves.clear();
                                        bestMoves.push_back({ rowIdx, colIdx });
                                    }
                                    else {
                                        break;
                                    }
                                }
                            }
                            else {
                                break;
                            }
                        }
                    } // 全てのマスの最大コマ数を比較し置くべき場所の候補を出す
                }
            }
        }

        if (!bestMoves.empty()) {
            srand(time(0)); // 乱数のシードの更新
            int randomIndex = rand() % bestMoves.size();
            return bestMoves[randomIndex]; // bestMovesからランダムに1つを選択して返す
        }

        return { -1, -1 };
    }

public:
    GomokuGame() : board(9, vector<char>(9, ' ')), currentPlayer('X'), lastMove({ -1, -1 }) {}

    void printBoard() const {
        // 列番号の表示
        cout << "\n" << "  ";
        for (int i = 0; i < 9; ++i) {
            cout << "  " << "\033[32m" << i << "\033[0m" << " ";
        }
        cout << endl;

        // 上部の枠線を表示
        cout << "  ";
        for (int i = 0; i < 9; ++i) {
            cout << "+---";
        }
        cout << "+" << endl;

        // 各行を表示
        for (int i = 0; i < 9; ++i) {
            // 行番号と左枠線を表示
            cout << "\033[33m" << i << "\033[0m" << " ";
            for (int j = 0; j < 9; ++j) {
                if (lastMove.first == i && lastMove.second == j) {
                    cout << "| " << "\033[5m" << board[i][j] << "\033[0m" << " ";
                }
                else {
                    cout << "| " << board[i][j] << " ";
                }
            }
            cout << "|" << endl;

            // 下部の枠線を表示
            cout << "  ";
            for (int j = 0; j < 9; ++j) {
                cout << "+---";
            }
            cout << "+" << endl;
        }
    }

    void WinCount(const string& winner) {
        ifstream datafile("win.txt"); // 今までの勝敗を読み込む
        regex rx(R"(\s)"); // 分割の文字の指定
        string str;
        int playerWins = 0; // 変数の初期化
        int cpuWins = 0; // 変数の初期化

        while (getline(datafile, str)) { // ファイルから1行ずつ読み込む
            sregex_token_iterator it(str.begin(), str.end(), rx, -1); // スペースで分割するイテレータ
            sregex_token_iterator end; // イテレータの終わり

            if (it != end) {
                string key = *it++;
                if (key == "Player" && it != end && *it++ == "Wins") {
                    playerWins = stoi(*it);
                }
                else if (key == "CPU" && it != end && *it++ == "Wins") {
                    cpuWins = stoi(*it);
                }
            }
        }

        datafile.close();

        if (winner == "Player") {
            playerWins++;
        }
        else if (winner == "CPU") {
            cpuWins++;
        }

        ofstream datafile_out("win.txt");
        datafile_out << "Player Wins " << playerWins << endl;
        datafile_out << "CPU Wins " << cpuWins << endl;
    }

    bool makeMove(int row, int col) {
        if (row < 0 || row >= 9 || col < 0 || col >= 9 || board[row][col] != ' ') {
            cout << "無効な動きです。再度入力してください。" << endl;
            return false;
        }
        board[row][col] = currentPlayer;
        lastMove = { row, col }; // 直前の動きを保存
        if (checkWin(row, col)) {
            printBoard();
            if (currentPlayer == 'X') {
                WinCount("Player");
                cout << "プレイヤーの勝ちです！" << endl;
            }
            else {
                WinCount("CPU");
                cout << "CPUの勝ちです！" << endl;
            }
            return true;
        } // 同じコマが5つ並んでいたら勝利の表示をする
        if (isBoardFull()) {
            printBoard();
            cout << "引き分けです！" << endl;
            return true;
        } // 全てのマスが埋まっていたら引き分けの表示をする

        if (currentPlayer == 'X') {
            currentPlayer = 'O';
        }
        else {
            currentPlayer = 'X';
        } // 次のターンに移行
        return false;
    }

    char getCurrentPlayer() const {
        return currentPlayer;
    }

    bool cpuMove() {
        this_thread::sleep_for(chrono::milliseconds(1000)); // CPUの動きを1秒遅延させる

        pair<int, int> bestMove = findBestMove('O', 'X');
        if (makeMove(bestMove.first, bestMove.second)) {
            return true;
        } // CPUの動作が正常に終了するとtrueを返す
        return false;
    }

};

bool getInput(int& row, int& col) {
    cout << "行と列を入力してください (例:\033[33m4 \033[32m4\033[0m): ";
    if (!(cin >> row >> col) || row < 0 || row >= 9 || col < 0 || col >= 9) {
        cout << "無効な入力です。0から8の数字を入力してください。" << endl;
        cin.clear(); // 無効な入力によるエラーを削除
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 無効な入力のデータを削除
        return false;
    }
    return true;
}

int main() {
    GomokuGame game;

    cout << "五目並べを始めます！\n";
    // ゲームを行う無限ループを開始
    while (true) {
        game.printBoard(); // 盤面を描画

        if (game.getCurrentPlayer() == 'X') {
            cout << "プレイヤーの番です。\n";
            
            int row, col;
            if (!getInput(row, col)) {
                continue;
            } // 入力を受け取る
            if (game.makeMove(row, col)) {
                char choice;
                cout << "もう一度プレイしますか？ (y/n): ";
                cin >> choice;
                if (choice == 'y' || choice == 'Y') {
                    game = GomokuGame(); // 新しいゲームを開始
                }
                else {
                    break;
                }
            } // ゲームを行う
        }

        else {
            cout << "CPUの番です。\n";
            if (game.cpuMove()) {
                char choice;
                cout << "もう一度プレイしますか？ (y/n): ";
                cin >> choice;
                if (choice == 'y' || choice == 'Y') {
                    game = GomokuGame(); // 新しいゲームを開始
                }
                else {
                    break;
                }
            }
        }
    }
}
