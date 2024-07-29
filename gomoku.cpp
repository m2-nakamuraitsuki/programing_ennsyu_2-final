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
                    return false; // �Ֆʂɋ󂢂Ă���ꏊ�������false��Ԃ�
                }
            }
        }
        return true; // �Ֆʂ��S�Ė��܂��Ă�����true��Ԃ�
    }

    bool checkWin(int row, int col) const {
        char player = board[row][col];
        return (checkDirection(row, col, 1, 0, player) || // ��
            checkDirection(row, col, 0, 1, player) || // �c
            checkDirection(row, col, 1, 1, player) || // �΂߁i�E���j
            checkDirection(row, col, 1, -1, player)); // �΂߁i�����j
    } // �e�����ɃR�}��5�A�����Ă��邩�m�F����

    bool checkDirection(int row, int col, int dRow, int dCol, char player) const {
        int count = 0;
        for (int i = -4; i <= 4; ++i) {
            int newRow = row + i * dRow;
            int newCol = col + i * dCol;
            if (newRow >= 0 && newRow < 9 && newCol >= 0 && newCol < 9 && board[newRow][newCol] == player) {
                if (++count == 5) {
                    return true; // �R�}��5�A�����Ă�����true��Ԃ�
                }
            }
            else {
                count = 0; // �J�E���g�����Z�b�g
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
        } // �������ɃR�}���A�����Ă��鐔�𐔂���
        for (int i = -1; i >= -4; --i) {
            int newRow = row + i * dRow;
            int newCol = col + i * dCol;
            if (newRow >= 0 && newRow < 9 && newCol >= 0 && newCol < 9 && board[newRow][newCol] == player) {
                count++;
            }
            else {
                break;
            }
        } // �������ɃR�}���A�����Ă��鐔�𐔂���
        return count; // �R�}���A�����Ă��鐔��Ԃ�
    }

    pair<int, int> findBestMove(char player, char opponent) const {
        vector<pair<int, int>> bestMoves;
        pair<vector<int>, char> maxCount;
        pair<vector<int>, char> priorityCount;
        maxCount.first = vector<int>(4, -1);  // �x�N�g���̏�����

        for (auto row = board.begin(); row != board.end(); ++row) {
            for (auto col = row->begin(); col != row->end(); ++col) {
                int rowIdx = row - board.begin();
                int colIdx = col - row->begin();
                if (*col == ' ') {
                    vector<int> playerCounts = {
                        countConsecutive(rowIdx, colIdx, 1, 0, player), // ��
                        countConsecutive(rowIdx, colIdx, 0, 1, player), // �c
                        countConsecutive(rowIdx, colIdx, 1, 1, player), // �΂߁i�E���j
                        countConsecutive(rowIdx, colIdx, 1, -1, player) // �΂߁i�����j
                    };
                    vector<int> opponentCounts = {
                        countConsecutive(rowIdx, colIdx, 1, 0, opponent), // ��
                        countConsecutive(rowIdx, colIdx, 0, 1, opponent), // �c
                        countConsecutive(rowIdx, colIdx, 1, 1, opponent), // �΂߁i�E���j
                        countConsecutive(rowIdx, colIdx, 1, -1, opponent) // �΂߁i�����j
                    };

                    sort(playerCounts.rbegin(), playerCounts.rend()); // playerCounts�x�N�g�����~���Ƀ\�[�g
                    sort(opponentCounts.rbegin(), opponentCounts.rend()); // opponentCounts�x�N�g�����~���Ƀ\�[�g

                    int maxPlayerCount = playerCounts[0];
                    int maxOpponentCount = opponentCounts[0];

                    if (maxOpponentCount >= maxPlayerCount) {
                        priorityCount = { opponentCounts, opponent };
                    }
                    else {
                        priorityCount = { playerCounts, player };
                    } // �v���C���[��CPU�̃R�}�̍ő�A�������r

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
                    } // �S�Ẵ}�X�̍ő�R�}�����r���u���ׂ��ꏊ�̌����o��
                }
            }
        }

        if (!bestMoves.empty()) {
            srand(time(0)); // �����̃V�[�h�̍X�V
            int randomIndex = rand() % bestMoves.size();
            return bestMoves[randomIndex]; // bestMoves���烉���_����1��I�����ĕԂ�
        }

        return { -1, -1 };
    }

public:
    GomokuGame() : board(9, vector<char>(9, ' ')), currentPlayer('X'), lastMove({ -1, -1 }) {}

    void printBoard() const {
        // ��ԍ��̕\��
        cout << "\n" << "  ";
        for (int i = 0; i < 9; ++i) {
            cout << "  " << "\033[32m" << i << "\033[0m" << " ";
        }
        cout << endl;

        // �㕔�̘g����\��
        cout << "  ";
        for (int i = 0; i < 9; ++i) {
            cout << "+---";
        }
        cout << "+" << endl;

        // �e�s��\��
        for (int i = 0; i < 9; ++i) {
            // �s�ԍ��ƍ��g����\��
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

            // �����̘g����\��
            cout << "  ";
            for (int j = 0; j < 9; ++j) {
                cout << "+---";
            }
            cout << "+" << endl;
        }
    }

    void WinCount(const string& winner) {
        ifstream datafile("win.txt"); // ���܂ł̏��s��ǂݍ���
        regex rx(R"(\s)"); // �����̕����̎w��
        string str;
        int playerWins = 0; // �ϐ��̏�����
        int cpuWins = 0; // �ϐ��̏�����

        while (getline(datafile, str)) { // �t�@�C������1�s���ǂݍ���
            sregex_token_iterator it(str.begin(), str.end(), rx, -1); // �X�y�[�X�ŕ�������C�e���[�^
            sregex_token_iterator end; // �C�e���[�^�̏I���

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
            cout << "�����ȓ����ł��B�ēx���͂��Ă��������B" << endl;
            return false;
        }
        board[row][col] = currentPlayer;
        lastMove = { row, col }; // ���O�̓�����ۑ�
        if (checkWin(row, col)) {
            printBoard();
            if (currentPlayer == 'X') {
                WinCount("Player");
                cout << "�v���C���[�̏����ł��I" << endl;
            }
            else {
                WinCount("CPU");
                cout << "CPU�̏����ł��I" << endl;
            }
            return true;
        } // �����R�}��5����ł����珟���̕\��������
        if (isBoardFull()) {
            printBoard();
            cout << "���������ł��I" << endl;
            return true;
        } // �S�Ẵ}�X�����܂��Ă�������������̕\��������

        if (currentPlayer == 'X') {
            currentPlayer = 'O';
        }
        else {
            currentPlayer = 'X';
        } // ���̃^�[���Ɉڍs
        return false;
    }

    char getCurrentPlayer() const {
        return currentPlayer;
    }

    bool cpuMove() {
        this_thread::sleep_for(chrono::milliseconds(1000)); // CPU�̓�����1�b�x��������

        pair<int, int> bestMove = findBestMove('O', 'X');
        if (makeMove(bestMove.first, bestMove.second)) {
            return true;
        } // CPU�̓��삪����ɏI�������true��Ԃ�
        return false;
    }

};

bool getInput(int& row, int& col) {
    cout << "�s�Ɨ����͂��Ă������� (��:\033[33m4 \033[32m4\033[0m): ";
    if (!(cin >> row >> col) || row < 0 || row >= 9 || col < 0 || col >= 9) {
        cout << "�����ȓ��͂ł��B0����8�̐�������͂��Ă��������B" << endl;
        cin.clear(); // �����ȓ��͂ɂ��G���[���폜
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // �����ȓ��͂̃f�[�^���폜
        return false;
    }
    return true;
}

int main() {
    GomokuGame game;

    cout << "�ܖڕ��ׂ��n�߂܂��I\n";
    // �Q�[�����s���������[�v���J�n
    while (true) {
        game.printBoard(); // �Ֆʂ�`��

        if (game.getCurrentPlayer() == 'X') {
            cout << "�v���C���[�̔Ԃł��B\n";
            
            int row, col;
            if (!getInput(row, col)) {
                continue;
            } // ���͂��󂯎��
            if (game.makeMove(row, col)) {
                char choice;
                cout << "������x�v���C���܂����H (y/n): ";
                cin >> choice;
                if (choice == 'y' || choice == 'Y') {
                    game = GomokuGame(); // �V�����Q�[�����J�n
                }
                else {
                    break;
                }
            } // �Q�[�����s��
        }

        else {
            cout << "CPU�̔Ԃł��B\n";
            if (game.cpuMove()) {
                char choice;
                cout << "������x�v���C���܂����H (y/n): ";
                cin >> choice;
                if (choice == 'y' || choice == 'Y') {
                    game = GomokuGame(); // �V�����Q�[�����J�n
                }
                else {
                    break;
                }
            }
        }
    }
}
