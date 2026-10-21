#include "game.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    Game game;
    if (!game_boot(&game)) {
        game_shutdown(&game);
#ifdef __vita__
        sceKernelExitProcess(1);
#endif
        return 1;
    }

    game_run(&game);
    game_shutdown(&game);

#ifdef __vita__
    sceKernelExitProcess(0);
#endif
    return 0;
}
