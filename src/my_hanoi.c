#include <ncurses.h>

/*
* ascending size order
*/
enum disk
{
    null_disk,
    red,
    orange,
    yellow
};

enum game_state_type
{
    greeting   /* before any move is made */,
    waiting    /* for move */,
    assessing  /* legality of move */,
    processing /* a legal move */,
    won        /* the game */
} game_state;

/*
* handlers for each game state
*/
void greet();
void wait();
void assess();
void process();
void congratulate();

/*
* functions to control entering, continuing, and leaving gameplay 
*/
void start_game();
void screen_update_tick(bool* should_keep_ticking);
void stop_game();

/*
* an empty pile would have { null_disk, null_disk, null_disk }
* a 1-disk pile would have, for example { null_disk, null_disk, red }
*/
typedef enum disk pile[3];

struct game_state_data
{
    pile lhs_old;
    pile middle_old;
    pile rhs_old;

    pile lhs_new;
    pile middle_new;
    pile rhs_new;
};

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
    // array of pointers to state handler functions
    // we'll index into this using game state type enum
    void (*state_handlers[5])() = {greet, wait, assess, process, congratulate}; 

    // call the appropriate state handler function based on current game state type
    (*state_handlers[game_state])();

    // essentially a buffer swap
    refresh(); 
    
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
}

void wait()
{
    // TODO
}

void assess()
{
    // TODO
}

void process()
{
    // TODO
}

void congratulate()
{
    // TODO
}
