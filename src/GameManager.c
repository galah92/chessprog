#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include "GameManager.h"
#include "ArrayStack.h"

#define LINE_MAX_LENGTH 64


ChessColor colorStrToChessColor(const char *color) {
    if (strcmp(color, "white") == 0) {
        return CHESS_PLAYER_COLOR_WHITE;
    } else if (strcmp(color, "black") == 0) {
        return CHESS_PLAYER_COLOR_BLACK;
    } else {
        return CHESS_PLAYER_COLOR_NONE;
    }
}

ChessMode modeStrToChessMode(const char *mode) {
    if (strcmp(mode, "1-player") == 0) {
        return CHESS_MODE_1_PLAYER;
    } else {
        return CHESS_MODE_2_PLAYER;
    }
}

ChessDifficulty difficultyStrToChessDifficulty(const char *difficulty) {
    if (strcmp(difficulty, "amateur") == 0) {
        return CHESS_DIFFICULTY_AMATEUR;
    } else if (strcmp(difficulty, "easy") == 0) {
        return CHESS_DIFFICULTY_EASY;
    } else if (strcmp(difficulty, "moderate") == 0) {
        return CHESS_DIFFICULTY_MODERATE;
    } else if (strcmp(difficulty, "hard") == 0) {
        return CHESS_DIFFICULTY_HARD;
    } else {
        return CHESS_DIFFICULTY_EXPERT;
    }    
}

char* chessPieceLocationToStr(ChessGame *game, int x, int y){
    switch (game->board[x][y]){
        case CHESS_PIECE_WHITE_PAWN:
        case CHESS_PIECE_BLACK_PAWN:
            return "pawn";
        case CHESS_PIECE_WHITE_BISHOP:
        case CHESS_PIECE_BLACK_BISHOP:
            return "bishop";  
        case CHESS_PIECE_BLACK_ROOK:
        case CHESS_PIECE_WHITE_ROOK:
            return "rook";  
        case CHESS_PIECE_BLACK_KNIGHT:
        case CHESS_PIECE_WHITE_KNIGHT:  
            return "knight";
        case CHESS_PIECE_BLACK_QUEEN:  
        case CHESS_PIECE_WHITE_QUEEN:   
            return "queen";
        case CHESS_PIECE_BLACK_KING:   
        case CHESS_PIECE_WHITE_KING:
            return "king";
        case CHESS_PIECE_NONE:
        default:
            return "";
    }
}

void handleLoadGame(GameManager *manager, const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        manager->error = GAME_ERROR_INVALID_FILE;
        return;
    }
    char line[LINE_MAX_LENGTH];
    fgets(line, LINE_MAX_LENGTH, fp); // turn
    manager->game->turn = colorStrToChessColor(strtok(line, " \n"));
    fgets(line, LINE_MAX_LENGTH, fp); // SETTINGS:
    fgets(line, LINE_MAX_LENGTH, fp);
    strtok(line, " \n"); // GAME_MODE
    manager->game->mode = modeStrToChessMode(strtok(NULL, " \n"));
    fgets(line, LINE_MAX_LENGTH, fp);
    strtok(line, " \n"); // DIFFICULTY
    manager->game->difficulty = difficultyStrToChessDifficulty(strtok(NULL, " \n"));
    fgets(line, LINE_MAX_LENGTH, fp);
    strtok(line, " \n"); // USER_COLOR
    manager->game->userColor = colorStrToChessColor(strtok(NULL, " \n"));
    for (int i = CHESS_GRID - 1; i >= 0; i--) {
        fgets(line, LINE_MAX_LENGTH, fp);
        strtok(line, " \n");
        for (int j = 0; j < CHESS_GRID; j++) {
            manager->game->board[j][i] = *strtok(NULL, " \n");
        }
    }
    fclose(fp);
}

void processSettingsCommand(GameManager *manager, GameCommand command) {
    if (!manager) return;
    ChessResult res;
    switch (command.type) {
        case GAME_COMMAND_GAME_MODE:
            res = ChessGame_SetGameMode(manager->game, command.args[0]);
            if (res == CHESS_INVALID_ARGUMENT)
                manager->error = GAME_ERROR_INVALID_GAME_MODE;
            break;
        case GAME_COMMAND_DIFFICULTY:
            if (manager->game->mode == CHESS_MODE_2_PLAYER) {
                manager->error = GAME_ERROR_INVALID_COMMAND;
                break;
            }
            res = ChessGame_SetDifficulty(manager->game, command.args[0]);
            if (res == CHESS_INVALID_ARGUMENT)
                manager->error = GAME_ERROR_INVALID_DIFF_LEVEL;
            break;
        case GAME_COMMAND_USER_COLOR:
            if (manager->game->mode == CHESS_MODE_2_PLAYER) {
                manager->error = GAME_ERROR_INVALID_COMMAND;
                break;
            }
            res = ChessGame_SetUserColor(manager->game, command.args[0]);
            if (res == CHESS_INVALID_ARGUMENT)
                manager->error = GAME_ERROR_INVALID_USER_COLOR;
            break;
        case GAME_COMMAND_LOAD_GAME:
            handleLoadGame(manager, command.path);
            break;
        case GAME_COMMAND_DEFAULT_SETTINGS:
            ChessGame_SetDefaultSettings(manager->game);
            break;
        case GAME_COMMAND_PRINT_SETTINGS:
            // done in CLIEngine
            break;
        case GAME_COMMAND_START:
            manager->phase = GAME_PHASE_RUNNING;
            break;
        case GAME_COMMAND_QUIT:
            manager->phase = GAME_PHASE_QUIT;
            break;
        case GAME_COMMAND_INVALID:
        default:
            manager->error = GAME_ERROR_INVALID_COMMAND;
            break;
    }
}

void handleMove(GameManager *manager, GameCommand command) {
    ChessMove move;
    move.from = (ChessPos){ .x = command.args[1] - 'A', .y = command.args[0] - 1 };
    move.to = (ChessPos){ .x = command.args[3] - 'A', .y = command.args[2] - 1 };
    switch (ChessGame_DoMove(manager->game, move)) {
        case CHESS_INVALID_ARGUMENT:
            manager->error = GAME_ERROR_INVALID_COMMAND;
            break;
        case CHESS_INVALID_POSITION:
            manager->error = GAME_ERROR_INVALID_POSITION;
            break;
        case CHESS_EMPTY_POSITION:
            manager->error = GAME_ERROR_EMPTY_POSITION;
            break;
        case CHESS_ILLEGAL_MOVE:
            manager->error = GAME_ERROR_INVALID_MOVE;
            break;
        case CHESS_KING_IS_STILL_THREATENED:
            manager->error = GAME_ERROR_INVALID_MOVE_KING_IS_T;
            break;
        case CHESS_KING_WILL_BE_THREATENED:
            manager->error = GAME_ERROR_INVALID_MOVE_KING_WILL_T;
            break;
        case CHESS_SUCCESS:
        default:
            break;
    }
}

void handleGetMoves(GameManager *manager, GameCommand command) {
    ChessPos pos = { .x = command.args[1] - 'A', .y = command.args[0] - 1 };
    ArrayStack_Destroy(manager->moves);
    switch (ChessGame_GetMoves(manager->game, pos, &manager->moves)) {
        case CHESS_INVALID_ARGUMENT:
            manager->error = GAME_ERROR_INVALID_COMMAND;
            break;
        case CHESS_INVALID_POSITION:
            manager->error = GAME_ERROR_INVALID_POSITION;
            break;
        case CHESS_SUCCESS:
        default:
            break;
    }
}

void handleSaveGame(GameManager *manager, const char *path) {
    FILE *fp = fopen(path, "w+");
    if (!fp) {
        manager->error = GAME_ERROR_FILE_ALLOC;
        return;
    }
    fputs(ChessColorToString[manager->game->turn].string, fp);
    fputs("\n", fp);
    ChessGame_SettingsToStream(manager->game, fp);
    ChessGame_BoardToStream(manager->game, fp);
    fclose(fp);
}

void handleUndoMove(GameManager *manager) {
    ArrayStack_Destroy(manager->moves);
    manager->moves = ArrayStack_Create(2, sizeof(ChessMove));
    ChessMove firstUndoneMove, secondUndoneMove;
    ChessResult res = ChessGame_UndoMove(manager->game, &firstUndoneMove);
    if (res == CHESS_EMPTY_HISTORY) {
        manager->error = GAME_ERROR_EMPTY_HISTORY;
        return;
    }
    if (manager->game->mode == CHESS_MODE_1_PLAYER) {
        res = ChessGame_UndoMove(manager->game, &secondUndoneMove);
        if (res != CHESS_EMPTY_HISTORY) {
            ArrayStack_Push(manager->moves, &secondUndoneMove);
        }
    }
    ArrayStack_Push(manager->moves, &firstUndoneMove);
}

void processRunningCommand(GameManager *manager, GameCommand command) {
    switch (command.type) {
        case GAME_COMMAND_MOVE:
            handleMove(manager, command);
            break;
        case GAME_COMMAND_GET_MOVES:
            handleGetMoves(manager, command);
            break;
        case GAME_COMMAND_SAVE:
            handleSaveGame(manager, command.path);
            break;
        case GAME_COMMAND_UNDO:
            handleUndoMove(manager);
            break;
        case GAME_COMMAND_RESET:
            ChessGame_ResetGame(manager->game);
            manager->phase = GAME_PHASE_SETTINGS;
            break;
        case GAME_COMMAND_QUIT:
            manager->phase = GAME_PHASE_QUIT;
            break;
        case GAME_COMMAND_INVALID:
            manager->error = GAME_ERROR_INVALID_COMMAND;
        default:
            break;
    }
}

GameManager* GameManager_Create() {
    GameManager *manager = malloc(sizeof(GameManager));
    if (!manager) return NULL;
    manager->game = ChessGame_Create();
    if (!manager->game) {
        GameManager_Destroy(manager);
        return NULL;
    }
    manager->phase = GAME_PHASE_SETTINGS;
    manager->error = GAME_ERROR_NONE;
    manager->moves = NULL;
    return manager;
}

void GameManager_Destroy(GameManager *manager) {
    ChessGame_Destroy(manager->game);
    ArrayStack_Destroy(manager->moves);
    free(manager);
}

GamePlayerType GameManager_GetCurrentPlayerType(GameManager *manager) {
    bool isOtherPlayer = manager->game->turn != manager->game->userColor;
    bool is1PlayerMode = manager->game->mode == CHESS_MODE_1_PLAYER;
    return is1PlayerMode & isOtherPlayer ? GAME_PLAYER_TYPE_AI : GAME_PLAYER_TYPE_HUMAN;
}

int getPieceScore(ChessPiece piece) {
    switch (piece) {
        case CHESS_PIECE_WHITE_PAWN:
        case CHESS_PIECE_BLACK_PAWN:
            return 1;
        case CHESS_PIECE_WHITE_KNIGHT:
        case CHESS_PIECE_BLACK_KNIGHT:
        case CHESS_PIECE_WHITE_BISHOP:
        case CHESS_PIECE_BLACK_BISHOP:
            return 3;
        case CHESS_PIECE_WHITE_ROOK:
        case CHESS_PIECE_BLACK_ROOK:
            return 5;
        case CHESS_PIECE_WHITE_QUEEN:
        case CHESS_PIECE_BLACK_QUEEN:
            return 9;
        case CHESS_PIECE_WHITE_KING:
        case CHESS_PIECE_BLACK_KING:
            return 100;
        case CHESS_PIECE_NONE:
        default:
            return 0;
    }
}

int getBoardScore(ChessGame *game) {
    ChessStatus status;
    ChessGame_GetGameStatus(game, &status);
    switch (status) {
        case CHESS_STATUS_DRAW:
            return 0;
        case CHESS_STATUS_CHECKMATE:
            return 1000;
        default:
            break;
    }
    int score = 0;
    ChessColor color;
    for (int i = 0; i < CHESS_GRID; i++) {
        for (int j = 0; j < CHESS_GRID; j++) {
            ChessGame_GetPieceColor(game->board[i][j], &color);
            if (color == game->turn)
            score += (color == game->turn ? 1 : -1) * getPieceScore(game->board[i][j]);
        }
    }
    return score;
}

// TODO: add pruning
int minimax(ChessGame *game, int depth, bool isMaximizing, ChessMove *bestMove) {
    if (depth == 0) return getBoardScore(game);
    int bestScore = isMaximizing ? INT_MIN : INT_MAX;
    int moveScore;
    ChessMove move;
    ChessMove tempMove; // only here as a garbage pointer - need to find a better way
    ChessColor color;
    ArrayStack *positions = NULL;
    for (int i = 0; i < CHESS_GRID; i++) {
        for (int j = 0; j < CHESS_GRID; j++) {
            move.from = (ChessPos){ .x = i, .y = j };
            ChessGame_GetPieceColor(game->board[i][j], &color);
            if (color != game->turn) continue;
            ChessGame *gameCopy = ChessGame_Copy(game);
            ChessGame_GetMoves(gameCopy, move.from, &positions);
            while (!ArrayStack_IsEmpty(positions)) {
                move.to = *(ChessPos *)ArrayStack_Pop(positions);
                ChessGame_DoMove(gameCopy, move);
                moveScore = minimax(gameCopy, depth - 1, !isMaximizing, &tempMove);
                if ((isMaximizing && moveScore > bestScore)
                    || (!isMaximizing && moveScore < bestScore)) {
                    memcpy(bestMove, &move, sizeof(ChessMove));
                    bestScore = moveScore;
                }
                ChessGame_UndoMove(gameCopy, &move);
            }
            ChessGame_Destroy(gameCopy);
        }
    }
    ArrayStack_Destroy(positions);
    return bestScore;
}

GameCommand GameManager_GetAIMove(GameManager *manager) {
    GameCommand command = { .type = GAME_COMMAND_MOVE };
    ChessMove move;
    minimax(manager->game, manager->game->difficulty, true, &move);
    command.args[1] = move.from.x + 'A';
    command.args[0] = move.from.y + 1;
    command.args[3] = move.to.x + 'A';
    command.args[2] = move.to.y + 1;
    return command;
}

void GameManager_ProcessCommand(GameManager *manager, GameCommand command) {
    manager->error = GAME_ERROR_NONE;
    if (manager->phase == GAME_PHASE_SETTINGS) {
        processSettingsCommand(manager, command);
    } else if (manager->phase == GAME_PHASE_RUNNING) {
        processRunningCommand(manager, command);
    }
}
