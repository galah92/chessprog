#include <stdbool.h>
#include "GameManager.h"
#include "ArrayStack.h"


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
            res = ChessGame_SetDifficulty(manager->game, command.args[0]);
            if (res == CHESS_INVALID_ARGUMENT)
                manager->error = GAME_ERROR_INVALID_DIFF_LEVEL;
            break;
        case GAME_COMMAND_USER_COLOR:
            res = ChessGame_SetUserColor(manager->game, command.args[0]);
            if (res == CHESS_INVALID_ARGUMENT)
                manager->error = GAME_ERROR_INVALID_USER_COLOR;
            break;
        case GAME_COMMAND_LOAD_GAME:
            // TODO: implement
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

void processRunningCommand(GameManager *manager, GameCommand command) {
    if (!manager) return;
    ChessResult res;
    ChessMove move;
    ArrayStack *moves;
    ChessPos pos = { .x = command.args[0], .y = command.args[1] };
    switch (command.type) {
        case GAME_COMMAND_MOVE:
            move.from = (ChessPos){ .x = command.args[0], .y = command.args[1] };
            move.to = (ChessPos){ .x = command.args[2], .y = command.args[3] };
            res = ChessGame_IsValidMove(manager->game, move);
            switch (res) {
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
            break;
        case GAME_COMMAND_GET_MOVES:
            res = ChessGame_GetMoves(manager->game, pos, &moves);
            switch (res) {
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
            break;
        case GAME_COMMAND_SAVE:
            // TODO: implement
            break;
        case GAME_COMMAND_UNDO:
            res = ChessGame_UndoMove(manager->game);
            if (res == CHESS_EMPTY_HISTORY) {
                manager->error = GAME_ERROR_EMPTY_HISTORY;
            }
            break;
        case GAME_COMMAND_RESET:
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
    return manager;
}

void GameManager_Destroy(GameManager *manager) {
    ChessGame_Destroy(manager->game);
    free(manager);
}

GamePlayerType GameManager_GetCurrentPlayerType(GameManager *manager) {
    bool isOtherPlayer = manager->game->turn != manager->game->userColor;
    bool is1PlayerMode = manager->game->mode == CHESS_MODE_1_PLAYER;
    return is1PlayerMode & isOtherPlayer ? GAME_PLAYER_TYPE_AI : GAME_PLAYER_TYPE_HUMAN;
}

void GameManager_ProcessCommand(GameManager *manager, GameCommand command) {
    manager->error = GAME_ERROR_NONE;
    if (manager->phase == GAME_PHASE_SETTINGS) {
        processSettingsCommand(manager, command);
    } else if (manager->phase == GAME_PHASE_RUNNING) {
        processRunningCommand(manager, command);
    }
}
