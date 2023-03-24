#include <ncurses.h>

enum game_state_type
{
    greeting,
    waiting_for_move,
    assessing_move,
    legal_move_done,
    illegal_move_done,
    won
} game_state;

void screen_update_tick(bool* should_keep_ticking);
void start_game();
void stop_game();
void greet();

int main()
{
    start_game();
    
    bool should_keep_ticking = true;
    while (should_keep_ticking)
    {
        screen_update_tick(&should_keep_ticking);
    }
    
    stop_game();
    return 0;
}


void screen_update_tick(bool* should_keep_ticking)
{
    if (game_state == greeting)
    {
        greet();
    }
    // TODO : else if ...

    refresh(); // essentially a buffer swap
    getch();
    *should_keep_ticking = false;
}

void start_game()
{
    // so that tick function starts in the right place
    game_state = greeting;

    // obtain memory for ncurses etc.
    // put the terminal into required mode (needs to be undone at end of game)
    initscr();

    // get characters directly from user, no line buffering
    raw();

    // user doesn't need to see the characters they type
    noecho();
}

void stop_game()
{
    // frees the memory taken by curses etc.
    // puts the terminal back into normal mode
    endwin(); 
}

void greet()
{
    printw(".==========================.\n");
    printw("|   THE TOWERS OF HANOI    |\n");
    printw(".==========================.\n");
    printw("\nWelcome!\n\nPress any key to start.");

    // essentially a buffer swap
    refresh(); 
}
