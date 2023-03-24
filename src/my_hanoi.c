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

/*
* does not contain detailed info about piles and disks
* but rather, the high level game state
*/
enum game_state_type
{
    greeting   /* before any move is made */,
    waiting    /* for move */,
    assessing  /* legality of move */,
    processing /* a legal move */,
    won        /* the game */,
    exiting
} game_state;

/*
* handlers for each non-exiting game state
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
void screen_update_tick();
void stop_game();

/*
* an empty pile would have { null_disk, null_disk, null_disk }
* a 1-disk pile would have { null_disk, null_disk, red } (for example)
*/
typedef enum disk pile[3];

/*
* there are two such datasets, to help manage transitions
*/
struct game_state_data
{
    pile lhs;
    pile middle;
    pile rhs;
} game_data_current, game_data_candidate_new;

/*
* for example, {'l', 'r'} means move the topmost disk from
* the left pile to the topmost unused position on the right pile
*/
unsigned char command[2];

int main()
{
    start_game();
    
    while (game_state != exiting)
    {
        screen_update_tick();
    }
    
    stop_game();
    return 0;
}

void screen_update_tick()
{
    // array of pointers to state handler functions
    // we'll index into this using game state type enum
    void (*state_handlers[5])() = {greet, wait, assess, process, congratulate}; 

    // call the appropriate state handler function based on current game state type
    (*state_handlers[game_state])();
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

    // start with disks stacked in order on lhs pile
    game_data_current.lhs[0] = red;
    game_data_current.lhs[1] = orange;
    game_data_current.lhs[2] = yellow;
    game_data_current.middle[0] = null_disk;
    game_data_current.middle[1] = null_disk;
    game_data_current.middle[2] = null_disk;
    game_data_current.rhs[0] = null_disk;
    game_data_current.rhs[1] = null_disk;
    game_data_current.rhs[2] = null_disk;

    // will update this in assess()
    game_data_candidate_new = game_data_current;
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

    // wait until a user has pressed a key to start
    getch();
    
    // TODO : move this to somewhere in the process() function
    game_state = won;
}

void wait()
{
    // get rid of anything currently being displayed
    clear();

    // TODO : actually display towers here
    // ... maybe describe how to input commands too?

    // valid characters : 'l', 'm', or 'r'
    // we don't validate move here... we do that in assess()
    command[0] = getch(); // "from" pile
    command[1] = getch(); // "to" pile

    game_state = assessing;
}

void assess()
{
    // two ways a move can be illegal:
    // (1) invalid command (e.g. "lx" instead of "lm"... 'x' does not refer to a valid pile)
    // (2) move may place a big disk on top of a smaller disk
    
    // TODO
}

void process()
{
    // this function assumes if it's being called, that
    // it is only ever being asked to make a valid transition

    // TODO
}

void congratulate()
{
    // get rid of anything currently being displayed
    clear();

    printw(".==========================.\n");
    printw("|   THE TOWERS OF HANOI    |\n");
    printw(".==========================.\n");
    printw("\nCongratulations!\nYou have won the game.\nPress any key to exit.");
    
    // essentially a buffer swap
    refresh();

    // wait until a user has pressed a key to exit
    getch();
    game_state = exiting;
}
