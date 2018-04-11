#include <stdbool.h>
#include "UIManager.h"
#include "GameManager.h"


int main(int argc, const char *argv[]) {
    bool toQuit = false;
    GameManager *gameManager = GameManager_Create();
    if (!gameManager) toQuit = true;
    UIManager *uiManager = UIManager_Create(argc, argv);
    if (!uiManager) toQuit = true;
    GameCommand command;
    while (!toQuit) {
        if (gameManager->phase == GAME_PHASE_SETTINGS) {
            command = UIManager_ProcessInput(uiManager);
        } else {
            switch (GameManager_GetCurrentPlayerType(gameManager)) {
                case GAME_PLAYER_TYPE_HUMAN:
                    command = UIManager_ProcessInput(uiManager);
                    break;
                case GAME_PLAYER_TYPE_AI:
                    command = GameManager_GetAIMove(gameManager);
                    break;
            }
        }
        GameManager_ProcessCommand(gameManager, command);
        UIManager_Render(uiManager, gameManager, command);
        toQuit = command.type == GAME_COMMAND_QUIT;
    }
    UIManager_Destroy(uiManager);
    GameManager_Destroy(gameManager);
    return 0;
}
