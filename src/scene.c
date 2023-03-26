
#include <stdio.h>
#include <string.h>
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
* returns -1 if the pile is full
* otherwise 0, 1, or 2, where the number represents the topmost null_disk's position
*/
int lowest_empty_slot(pile p);

/*
* "properly" means biggest to smallest, from the bottom up
*/
bool is_stacked_properly(pile p);

/*
* returns pointer to left, middle, or right pile
* caller specifies whether they want candidate pile (1), or current pile (0)
*/
enum disk* get_pile_by_label(char c, bool should_return_candidate_pile);

/*
* there are two of these game datasets, to help manage transitions
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

    // avoid having colour pairs defined all over the place
    start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK); // should be called at the end of any function that calls attron() with an argument of COLOR_PAIR
	init_pair(2, COLOR_RED, COLOR_RED);
    init_color(COLOR_BLUE, 1000, 500, 0); // there's no COLOR_ORANGE, so we repurpose COLOR_BLUE 
	init_pair(4, COLOR_BLUE, COLOR_BLUE);
	init_pair(3, COLOR_YELLOW, COLOR_YELLOW);
    init_color(COLOR_MAGENTA, 300, 300, 300); // there's no COLOR_GREY, so we repurpose COLOR_MAGENTA 
	init_pair(5, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(6, COLOR_BLACK, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    
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
    // ... actually also need to describe the win condition too.
    // ... simplest approach for now would be to make win condition
    // ... to have all disks, big-to-small from the bottom up, on the rhs pile
    // ... fancier would be to allow same pile condition but either middle or rhs

    // valid characters : 'l', 'm', or 'r'
    // we don't validate move here... we do that in assess()
    command[0] = getch(); // "from" pile
    command[1] = getch(); // "to" pile

    game_state = assessing;
}

void assess()
{
    // multiple ways a move can be illegal:
    // (1) invalid command (e.g. "lx" instead of "lm"... 'x' does not refer to a valid pile)
    // (2) move may move a null disk
    // (3) move may place a big disk on top of a smaller disk

    const char* allowed_letters = "lmr";
    char* from_pile = strchr(allowed_letters, command[0]);
    char* to_pile = strchr(allowed_letters, command[1]);
    
    // first we check (1)
     
    // handle case where invalid "from" or "to" pile is given
    // and case where command doesn't actually specify moving anything
    if (from_pile == NULL || to_pile == NULL || command[0] == command[1])
    {
        // just act like we didn't hear the command
        game_state = waiting;
        return;
    }

    // then we check (2)
    
    char pile_label_from = *from_pile;
    char pile_label_to = *to_pile;

    int lowest_empty_slot_from = lowest_empty_slot(get_pile_by_label(pile_label_from, 0 /* means current, not candidate*/));
    int lowest_empty_slot_to = lowest_empty_slot(get_pile_by_label(pile_label_to, 0 /* means current, not candidate*/));
    if (lowest_empty_slot_to == -1 || lowest_empty_slot_from == 2) // then player is trying to add to a full pile, or move a null_disk, probably both
    {
        // just act like we didn't hear the command
        game_state = waiting;
        return;
    }

    // then we check (3)
    
    enum disk* source_pile = get_pile_by_label(pile_label_from, 0 /* means current, not candidate*/);
    enum disk* destination_pile = get_pile_by_label(pile_label_to, 0 /* means current, not candidate*/);
    
    // "top" and "bottom" here refer to the disks that would be respectively on top/bottom if player's command is executed

    // already handled case where source pile is empty earlier
    int top_disk_position = lowest_empty_slot(source_pile) + 1;
    enum disk top_disk = source_pile[top_disk_position];
    int bottom_disk_position = lowest_empty_slot(destination_pile) + 1;
    enum disk bottom_disk = destination_pile[bottom_disk_position];

    if (top_disk < bottom_disk || bottom_disk == null_disk) // criterion for legality of the move
    {
        // looks like the player has given a command worth executing
        game_state = processing;
        return;
    }
    else
    {
        // just act like we didn't hear the command
        // would be nicer to give some feedback though
        game_state = waiting;
        return;
    }
}

void process()
{
    // this function assumes if it's being called, that
    // it is only ever being asked to make a valid transition

    // TODO

}

void print_spacer()
{
    attron(COLOR_PAIR(6)); printw("xxxx"); attron(COLOR_PAIR(1));
}

void print_null_disk()
{
    attron(COLOR_PAIR(6)); printw("xxxxxxxxxxxxxxx"); attron(COLOR_PAIR(1));
}

void print_red_disk()
{
    attron(COLOR_PAIR(6)); printw("xxxx"); attron(COLOR_PAIR(2)); printw("xxxxxxx"); attron(COLOR_PAIR(6)); printw("xxxx"); attron(COLOR_PAIR(1));
}

void print_orange_disk()
{
    attron(COLOR_PAIR(6)); printw("xx"); attron(COLOR_PAIR(4)); printw("xxxxxxxxxxx"); attron(COLOR_PAIR(6)); printw("xx"); attron(COLOR_PAIR(1));
}

void print_yellow_disk()
{
    attron(COLOR_PAIR(3)); printw("xxxxxxxxxxxxxxx"); attron(COLOR_PAIR(1));
}

void print_base_plate_disk()
{
    attron(COLOR_PAIR(5)); printw("xxxxxxxxxxxxxxx"); attron(COLOR_PAIR(1));
}

void print_disk_scene()
{
    // need to select appropriate disk to print for 12 of these postions, based on a data structure
    // always print base plates though 
    printw("\n");
    printw("\n");
    print_spacer(); print_spacer(); print_red_disk(); print_spacer(); print_null_disk(); print_spacer(); print_null_disk();
    printw("\n");
    print_spacer(); print_spacer(); print_orange_disk(); print_spacer(); print_null_disk(); print_spacer(); print_null_disk();
    printw("\n");
    print_spacer(); print_spacer(); print_yellow_disk(); print_spacer(); print_yellow_disk(); print_spacer(); print_null_disk();
    printw("\n");
    print_spacer(); print_spacer(); print_base_plate_disk(); print_spacer(); print_base_plate_disk(); print_spacer(); print_base_plate_disk();
    printw("\n");
}

void congratulate()
{
    // get rid of anything currently being displayed
    clear();

    print_disk_scene();

    attron(COLOR_PAIR(1));

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

int lowest_empty_slot(pile p)
{
    if (p[0] != null_disk)
    {
        return -1;
    }
    else if (p[0] == null_disk)
    {
        return 0;
    }
    else if (p[1] == null_disk)
    {
        return 1;
    }
    return 2; // means pile is completely empty
}

bool is_stacked_properly(pile p)
{
    enum disk top = p[0], middle = p[1], bottom = p[2];
    return bottom > middle && middle > top;
}

enum disk* get_pile_by_label(char c, bool should_return_candidate_pile)
{
    if (c == 'l')
    {
        return should_return_candidate_pile ? game_data_candidate_new.lhs : game_data_current.lhs;
    }
    else if (c == 'm')
    {
        return should_return_candidate_pile ? game_data_candidate_new.middle : game_data_current.middle;
    }
    else if (c == 'r')
    {
        return should_return_candidate_pile ? game_data_candidate_new.rhs : game_data_current.rhs;
    }
    return NULL;
}