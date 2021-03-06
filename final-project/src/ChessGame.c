#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ChessGame.h"

#define CHESS_HISTORY_SIZE          6
#define CHESS_MAX_POSSIBLE_MOVES    27 // 7 * 3 + 6 for a queen piece


/**
 * Check whether a given ChessPos's location is on board.
 * @param   pos         the pos to check
 * @return  true        if pos is on board
 *          false       otherwise
 */
bool isValidPositionOnBoard(ChessPos pos) {
    return pos.x >= 0 && pos.x < CHESS_GRID && pos.y >= 0 && pos.y < CHESS_GRID;
}

/**
 * Check whether a given ChessMove's locations is on board.
 * @param   move        the move to check
 * @return  true        if both move.from && move.to are on board
 *          false       otherwise
 */
bool isValidPositionsOnBoard(ChessMove move) {
    return isValidPositionOnBoard(move.from) && isValidPositionOnBoard(move.to);
}

/**
 * Retrieve a ChessColor for a given ChessPiece
 * @param   piece       the ChessPiece to retrieve the color for
 * @return  color       the piece color
 */
ChessColor getPieceColor(ChessPiece piece) {
    switch (piece) {
        case CHESS_PIECE_WHITE_PAWN:
        case CHESS_PIECE_WHITE_ROOK:
        case CHESS_PIECE_WHITE_KNIGHT:
        case CHESS_PIECE_WHITE_BISHOP:
        case CHESS_PIECE_WHITE_QUEEN:
        case CHESS_PIECE_WHITE_KING:
            return CHESS_PLAYER_COLOR_WHITE;
        case CHESS_PIECE_BLACK_PAWN:
        case CHESS_PIECE_BLACK_ROOK:
        case CHESS_PIECE_BLACK_KNIGHT:
        case CHESS_PIECE_BLACK_BISHOP:
        case CHESS_PIECE_BLACK_QUEEN:
        case CHESS_PIECE_BLACK_KING:
            return CHESS_PLAYER_COLOR_BLACK;
        case CHESS_PIECE_NONE:
        default:
            return CHESS_PLAYER_COLOR_NONE;
    }
}

/**
 * Check whether a given ChessMove's "from" location is of the current player,
 * according to a given game.
 * @param   game        the game the move is based on
 * @param   move        the move to check
 */
bool isPosOfPlayerPiece(ChessGame *game, ChessPos pos) {
    if (!game) return false; // sanity check
    return getPieceColor(game->board[pos.x][pos.y]) == game->turn;
}

/**
 * Check whether a given moving piece isn't stepping on a friendly piece.
 * A friendly piece is a piece that shares the same color as the moving piece.
 * @param   game        the game instance which provides the board
 * @param   move        the move to check it's legitimacy
 * @return              0 if the 'end' move location contains a friendly piece
 *                      1 otherwise
 */
bool isValidToPosition(ChessGame *game, ChessMove move) {
    if (!game) return false; // sanity check
    ChessColor fromColor = getPieceColor(game->board[move.from.x][move.from.y]);
    ChessColor toColor = getPieceColor(game->board[move.to.x][move.to.y]);
    return fromColor != toColor;
}

bool isValidPawnMove(ChessGame *game, ChessMove move) {
    ChessColor color = getPieceColor(game->board[move.from.x][move.from.y]);
    int isInStartPos = move.from.y == (color == CHESS_PLAYER_COLOR_WHITE ? 1 : 6);
    int horDiff = abs(move.from.x - move.to.x);
    int verDiff = (move.from.y - move.to.y) * (color == CHESS_PLAYER_COLOR_WHITE ? -1 : 1);
    int isCapture = game->board[move.to.x][move.to.y] != CHESS_PIECE_NONE &&
        color != getPieceColor(game->board[move.to.x][move.to.y]);
    int regularMove = !isCapture && verDiff == 1 && horDiff == 0;
    int startingMove = !isCapture && isInStartPos && verDiff == 2 && horDiff == 0;
    int capturingMove = isCapture && verDiff == 1 && horDiff == 1;
    return regularMove || startingMove || capturingMove;
}

bool isValidRookMove(ChessGame *game, ChessMove move) {
    int horDiff = move.from.x - move.to.x;
    int verDiff = move.from.y - move.to.y;
    if (!((horDiff != 0) ^ (verDiff != 0))) return false; // exclusive ver / hor move
    if (horDiff != 0) { // check there isn't overleap
        int start = move.from.x < move.to.x ? move.from.x + 1 : move.to.x + 1;
        int end = move.from.x < move.to.x ? move.to.x : move.from.x;
        for (int i = start; i < end; i++) {
            if (game->board[i][move.from.y] != CHESS_PIECE_NONE) return false;
        }
    } else { // verDiff != 0
        int start = move.from.y < move.to.y ? move.from.y + 1 : move.to.y + 1;
        int end = move.from.y < move.to.y ? move.to.y : move.from.y;
        for (int i = start; i < end; i++) {
            if (game->board[move.from.x][i] != CHESS_PIECE_NONE) return false;
        }
    }
    return true;
}

bool isValidKnightMove(ChessGame *game, ChessMove move) {
    (void)game; // here for completness of isValidPieceMove()
    int horDiff = abs(move.from.x - move.to.x);
    int verDiff = abs(move.from.y - move.to.y);
    return (verDiff == 2 && horDiff == 1) ^ (horDiff == 2 && verDiff == 1);
}

bool isValidBishopMove(ChessGame *game, ChessMove move) {
    int horAbs = abs(move.from.x - move.to.x);
    int verAbs = abs(move.from.y - move.to.y);
    if (horAbs != verAbs || !horAbs) return false;
    if (horAbs == 1 && getPieceColor(game->board[move.to.x][move.to.y]) != game->turn) {
        return true;
    }
    bool rightup   = (move.from.x < move.to.x) && (move.from.y < move.to.y);
    bool leftup    = (move.from.x > move.to.x) && (move.from.y < move.to.y);
    bool rightdown = (move.from.x < move.to.x) && (move.from.y > move.to.y);
    bool leftdown  = (move.from.x > move.to.x) && (move.from.y > move.to.y);
    int startx, starty, endx;
    if (rightup || rightdown){
        startx = move.from.x + 1;
        endx = move.to.x;
        if (rightup) starty = move.from.y + 1;
        else starty = move.from.y - 1;
    } else {
        startx = move.to.x + 1;
        endx = move.from.x;
        if (leftup) starty = move.to.y - 1;
        else starty = move.to.y + 1;
    }

    for (int i = startx, j = starty; i < endx;){
        if (game->board[i][j] != CHESS_PIECE_NONE) return false;
        if (rightup){i++; j++;}
        if (leftup){i++; j--;}
        if (rightdown){i++; j--;}
        if (leftdown){i++; j++;}
    }
    return true;
}

bool isValidQueenMove(ChessGame *game, ChessMove move) {
    return isValidRookMove(game, move) || isValidBishopMove(game, move);
}

bool isValidKingMove(ChessGame *game, ChessMove move) {
    (void)game; // here for completness of isValidPieceMove()
    int horDiff = abs(move.from.x - move.to.x);
    int verDiff = abs(move.from.y - move.to.y);
    return horDiff <= 1 && verDiff <= 1 && ((horDiff > 0) || (verDiff > 0));
}

bool isValidPieceMove(ChessGame *game, ChessMove move) {
    if (!game) return false;
    switch (game->board[move.from.x][move.from.y])
    {
        case CHESS_PIECE_WHITE_PAWN:
        case CHESS_PIECE_BLACK_PAWN:
            return isValidPawnMove(game, move);
        case CHESS_PIECE_WHITE_ROOK:
        case CHESS_PIECE_BLACK_ROOK:
            return isValidRookMove(game, move);    
        case CHESS_PIECE_WHITE_KNIGHT:
        case CHESS_PIECE_BLACK_KNIGHT:
            return isValidKnightMove(game, move);
        case CHESS_PIECE_WHITE_BISHOP:
        case CHESS_PIECE_BLACK_BISHOP:
            return isValidBishopMove(game, move);
        case CHESS_PIECE_WHITE_QUEEN:
        case CHESS_PIECE_BLACK_QUEEN:
            return isValidQueenMove(game, move);
        case CHESS_PIECE_WHITE_KING:
        case CHESS_PIECE_BLACK_KING:
            return isValidKingMove(game, move);
        case CHESS_PIECE_NONE:
        default:
            return false; // shouldn't happen
    }
}

ChessColor switchColor(ChessColor color) {
    switch (color) {
        case CHESS_PLAYER_COLOR_BLACK:
            return CHESS_PLAYER_COLOR_WHITE;
        case CHESS_PLAYER_COLOR_WHITE:
            return CHESS_PLAYER_COLOR_BLACK;
        default:
            return CHESS_PLAYER_COLOR_NONE;
    }
}

bool isPosThreatenedBy(ChessGame *game, ChessPos pos, ChessColor playerColor) {
    ChessMove move = { .to = pos };
    for (int i = 0; i < CHESS_GRID; i++) {
        for (int j = 0; j < CHESS_GRID; j++) {
            if (getPieceColor(game->board[i][j]) == playerColor) {
                move.from = (ChessPos){ .x = i, .y = j };
                if (isValidPieceMove(game, move)) return true;
            }
        }
    }
    return false;
}

bool isKingThreatenedBy(ChessGame *game, ChessColor playerColor) {
    ChessPiece king = switchColor(playerColor) == CHESS_PLAYER_COLOR_WHITE
        ? CHESS_PIECE_WHITE_KING
        : CHESS_PIECE_BLACK_KING;
    ChessPos opponentKingPos;
    for (int i = 0 ; i < CHESS_GRID; i++) {
        for (int j = 0; j < CHESS_GRID; j++) {
            if (game->board[i][j] == king) {
                opponentKingPos.x = i;
                opponentKingPos.y = j;
            }
        }
    }
    return isPosThreatenedBy(game, opponentKingPos, playerColor);
}

void pseudoDoMove(ChessGame *game, ChessMove *move) {
    move->player = game->turn;
    move->capturedPiece = game->board[move->to.x][move->to.y];
    game->board[move->to.x][move->to.y] = game->board[move->from.x][move->from.y];
    game->board[move->from.x][move->from.y] = CHESS_PIECE_NONE;
}

void pseudoUndoMove(ChessGame *game, ChessMove *move) {
    game->board[move->from.x][move->from.y] = game->board[move->to.x][move->to.y];
    game->board[move->to.x][move->to.y] = move->capturedPiece; 
}

bool hasMoves(ChessGame *game) {
    ArrayStack *possibleMoves = NULL;
    ChessPos pos;
    ChessColor color;
    for (int i = 0; i < CHESS_GRID; i ++) {
        for (int j = 0; j < CHESS_GRID; j++) {
            ChessGame_GetPieceColor(game->board[i][j], &color);
            if (color != game->turn) continue;
            pos = (ChessPos){ .x = i, .y = j };
            ChessResult res = ChessGame_GetMoves(game, pos, &possibleMoves);
            bool areThereMoves = !ArrayStack_IsEmpty(possibleMoves);
            ArrayStack_Destroy(possibleMoves);
            if (res != CHESS_SUCCESS || areThereMoves) return true;
        }
    }
    return false;
}

ChessPosType getMoveType(ChessGame *game, ChessMove move) {
    pseudoDoMove(game, &move);
    bool isThreatened = isPosThreatenedBy(game, move.to, !game->turn);
    pseudoUndoMove(game, &move);
    bool isCapture = game->board[move.to.x][move.to.y] != CHESS_PIECE_NONE;
    if (isThreatened && isCapture) return CHESS_POS_BOTH;
    if (isThreatened) return CHESS_POS_THREATENED;
    if (isCapture) return CHESS_POS_CAPTURE;
    return CHESS_POS_STANDARD;
}

/**
 * Check if a given ChessMove is a valid next move for a given ChessGame.
 * @param   game        the instance to set mode to
 * @param   move        the move to check it's validity
 * return   CHESS_INVALID_ARGUMENT if game == NULL
 *          CHESS_INVALID_POSITION if move locations are not on board
 *          CHESS_EMPTY_POSITION if the source position doesn't contain
 *              the player's piece
 *          CHESS_ILLEGAL_MOVE if the piece can't move in that way,
 *              or if the destination position contains a player's piece
 *          CHESS_KING_IS_STILL_THREATENED if the status is CHECK and
 *              the move won't change that status
 *          CHESS_KING_WILL_BE_THREATENED if the move will expose the
 *              player to CHECK
 *          CHESS_SUCCESS otherwise (there's no other choise left!)
 */
ChessResult isValidMove(ChessGame *game, ChessMove move) {
    if (!game) return CHESS_INVALID_ARGUMENT;
    if (!isValidPositionsOnBoard(move)) return CHESS_INVALID_POSITION;
    if (!isPosOfPlayerPiece(game, move.from)) return CHESS_EMPTY_POSITION;
    if (!isValidToPosition(game, move)) return CHESS_ILLEGAL_MOVE;
    if (!isValidPieceMove(game, move)) return CHESS_ILLEGAL_MOVE;
    bool isThreatened = isKingThreatenedBy(game, game->turn);
    pseudoDoMove(game, &move);
    bool willBeThreatened = isKingThreatenedBy(game, switchColor(game->turn));
    pseudoUndoMove(game, &move);
    if (isThreatened && willBeThreatened) return CHESS_KING_IS_STILL_THREATENED;
    if (willBeThreatened) return CHESS_KING_WILL_BE_THREATENED;
    return CHESS_SUCCESS;
}

ChessGame* ChessGame_Create() {
    ChessGame *game = malloc(sizeof(ChessGame));
    if (!game) return ChessGame_Destroy(game);
    game->turn = CHESS_PLAYER_COLOR_WHITE;
    ChessGame_SetDefaultSettings(game);
    game->history = ArrayStack_Create(CHESS_HISTORY_SIZE, sizeof(ChessMove));
    if (!game->history) ChessGame_Destroy(game);
    return game;
}

ChessGame* ChessGame_Copy(const ChessGame *game) {
    ChessGame *copy = malloc(sizeof(ChessGame));
    if (!copy) return NULL;
    memcpy(copy, game, sizeof(ChessGame));
    copy->history = ArrayStack_Copy(game->history);
    if (!copy->history) ChessGame_Destroy(copy);
    return copy;
}

ChessGame* ChessGame_Destroy(ChessGame *game) {
    if (!game) return NULL;
    ArrayStack_Destroy(game->history);
    free(game);
    return NULL;
}

ChessResult ChessGame_ResetGame(ChessGame *game) {
    if (!game) return CHESS_INVALID_ARGUMENT;
    game->turn = CHESS_PLAYER_COLOR_WHITE;
    ChessGame_InitBoard(game);
    ArrayStack_Destroy(game->history);
    game->history = ArrayStack_Create(CHESS_HISTORY_SIZE, sizeof(ChessMove));
    return CHESS_SUCCESS;
}

ChessResult ChessGame_SetDefaultSettings(ChessGame *game) {
    if (!game) return CHESS_INVALID_ARGUMENT;
    game->mode = CHESS_MODE_1_PLAYER;
    game->difficulty = CHESS_DIFFICULTY_EASY;
    game->userColor = CHESS_PLAYER_COLOR_WHITE;
    return CHESS_SUCCESS;
}

ChessResult ChessGame_SetGameMode(ChessGame *game, ChessMode mode) {
    if (!game) return CHESS_INVALID_ARGUMENT;
    if (mode < CHESS_MODE_1_PLAYER) return CHESS_INVALID_ARGUMENT;
    if (mode > CHESS_MODE_2_PLAYER) return CHESS_INVALID_ARGUMENT;
    game->mode = mode;
    return CHESS_SUCCESS;
}

ChessResult ChessGame_SetDifficulty(ChessGame *game, ChessDifficulty difficulty) {
    if (!game) return CHESS_INVALID_ARGUMENT;
    if (difficulty < CHESS_DIFFICULTY_AMATEUR) return CHESS_INVALID_ARGUMENT;
    if (difficulty > CHESS_DIFFICULTY_EXPERT) return CHESS_INVALID_ARGUMENT;
    game->difficulty = difficulty;
    return CHESS_SUCCESS;
}

ChessResult ChessGame_SetUserColor(ChessGame *game, ChessColor userColor) {
    if (!game) return CHESS_INVALID_ARGUMENT;
    if (userColor < CHESS_PLAYER_COLOR_BLACK) return CHESS_INVALID_ARGUMENT;
    if (userColor > CHESS_PLAYER_COLOR_WHITE) return CHESS_INVALID_ARGUMENT;
    game->userColor = userColor;
    return CHESS_SUCCESS;
}

ChessResult ChessGame_InitBoard(ChessGame *game) {
    if (!game) return CHESS_INVALID_ARGUMENT;
    game->board[0][0] = game->board[7][0] = CHESS_PIECE_WHITE_ROOK;
    game->board[1][0] = game->board[6][0] = CHESS_PIECE_WHITE_KNIGHT;
    game->board[2][0] = game->board[5][0] = CHESS_PIECE_WHITE_BISHOP;
    game->board[3][0] = CHESS_PIECE_WHITE_QUEEN;
    game->board[4][0] = CHESS_PIECE_WHITE_KING;
    game->board[0][7] = game->board[7][7] = CHESS_PIECE_BLACK_ROOK;
    game->board[1][7] = game->board[6][7] = CHESS_PIECE_BLACK_KNIGHT;
    game->board[2][7] = game->board[5][7] = CHESS_PIECE_BLACK_BISHOP;
    game->board[3][7] = CHESS_PIECE_BLACK_QUEEN;
    game->board[4][7] = CHESS_PIECE_BLACK_KING;
    for (int j = 0; j < CHESS_GRID; j++) {
        game->board[j][1] = CHESS_PIECE_WHITE_PAWN;
        game->board[j][6] = CHESS_PIECE_BLACK_PAWN;
    }
    for (int i = 2; i < CHESS_GRID - 2; i++) {
        for (int j = 0; j < CHESS_GRID; j++) {
            game->board[j][i] = CHESS_PIECE_NONE;
        }
    }
    return CHESS_SUCCESS;
}

ChessResult ChessGame_GetGameStatus(ChessGame *game, ChessStatus *status) {
    if (!game) return CHESS_INVALID_ARGUMENT;
    if (isKingThreatenedBy(game, !game->turn)) {
        *status = hasMoves(game) ? CHESS_STATUS_CHECK : CHESS_STATUS_CHECKMATE;
    } else {
        *status = hasMoves(game) ? CHESS_STATUS_RUNNING : CHESS_STATUS_DRAW;
    }
    return CHESS_SUCCESS;
}

ChessResult ChessGame_DoMove(ChessGame *game, ChessMove move) {
    ChessResult isValidResult = isValidMove(game, move);
    if (isValidResult != CHESS_SUCCESS) return isValidResult;
    pseudoDoMove(game, &move);
    ArrayStack_Push(game->history, &move);
    game->turn = switchColor(game->turn);
    return CHESS_SUCCESS;
}

ChessResult ChessGame_UndoMove(ChessGame *game, ChessMove *move) {
    if (ArrayStack_IsEmpty(game->history)) return CHESS_EMPTY_HISTORY;
    *move = *(ChessMove *)ArrayStack_Pop(game->history);
    pseudoUndoMove(game, move);
    game->turn = switchColor(game->turn);
    return CHESS_SUCCESS;
}

ChessResult ChessGame_GetMoves(ChessGame *game, ChessPos pos, ArrayStack **stack) {
    *stack = ArrayStack_Create(CHESS_MAX_POSSIBLE_MOVES, sizeof(ChessPos));
    if (!game) return CHESS_INVALID_ARGUMENT;
    if (!isValidPositionOnBoard(pos)) return CHESS_INVALID_POSITION;
    if (game->board[pos.x][pos.y] == CHESS_PIECE_NONE) return CHESS_EMPTY_POSITION;
    ChessColor originalTurn = game->turn;
    if (!isPosOfPlayerPiece(game, pos)) {
        game->turn = switchColor(game->turn);
    }
    ChessMove move = { .from = pos };
    for (int i = 0; i < CHESS_GRID; i++) {
        for (int j = 0; j < CHESS_GRID; j++) {
            move.to = (ChessPos){ .x = i, .y = j };
            if (isValidMove(game, move) == CHESS_SUCCESS) {
                move.to.type = getMoveType(game, move);
                ArrayStack_Push(*stack, &move.to);
            }
        }
    }
    game->turn = originalTurn;
    return CHESS_SUCCESS;
}

ChessResult ChessGame_GetPieceColor(ChessPiece piece, ChessColor *color) {
    *color = getPieceColor(piece);
    return CHESS_SUCCESS;
}
