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
* returns pointer to left, middle, or right pile
* caller specifies whether they want candidate pile (1), or current pile (0)
*/
enum disk* get_pile_by_label(char c);

/*
* there are two of these game datasets, to help manage transitions
*/
struct game_state_data
{
    pile lhs;
    pile middle;
    pile rhs;
} game_data_current;

/*
* for example, {'l', 'r'} means move the topmost disk from
* the left pile to the topmost unused position on the right pile
*/
unsigned char command[2];

/*
* front-end functions
*/
void print_spacer();
void print_null_disk();
void print_red_disk();
void print_orange_disk();
void print_yellow_disk();
void print_base_plate_disk();
void print_disk_scene();
void print_player_prompt();

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
    
    refresh();

    // wait until a user has pressed a key to start
    getch();
    game_state = waiting;
}

void wait()
{
    clear();

    // display towers now
    print_disk_scene();

    // display player instructions and get player input
    print_player_prompt();
    print_spacer();
    echo(); // helpful for the player to see the command they're typing
    command[0] = getch(); // "from" pile
    command[1] = getch(); // "to" pile
    noecho();

    game_state = assessing; // valid characters : 'l', 'm', or 'r', but we need to check more than just that
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

    int lowest_empty_slot_from = lowest_empty_slot(get_pile_by_label(pile_label_from));
    int lowest_empty_slot_to = lowest_empty_slot(get_pile_by_label(pile_label_to));
    if (lowest_empty_slot_to == -1 || lowest_empty_slot_from == 2) // then player is trying to add to a full pile, or move a null_disk, probably both
    {
        // just act like we didn't hear the command
        game_state = waiting;
        return;
    }

    // then we check (3)
    
    enum disk* source_pile = get_pile_by_label(pile_label_from);
    enum disk* destination_pile = get_pile_by_label(pile_label_to);
    
    // "top" and "bottom" here refer to the disks that would be respectively on top/bottom if player's command is executed

    // already handled case where source pile is empty earlier
    int top_disk_position = lowest_empty_slot(source_pile) + 1;
    int bottom_disk_position = lowest_empty_slot(destination_pile) + 1;
    enum disk top_disk = source_pile[top_disk_position];
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

    // apply move to game data, don't need a buffer
    enum disk* source_pile = get_pile_by_label(command[0]);
    enum disk* destination_pile = get_pile_by_label(command[1]);
    int top_disk_position = lowest_empty_slot(source_pile) + 1;
    destination_pile[lowest_empty_slot(destination_pile)] = source_pile[top_disk_position];
    source_pile[top_disk_position] = null_disk;
    
    if (destination_pile[0] == red) // win condition
    {
        game_state = won;
    }
    else
    {
        game_state = waiting;
    }
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
    // get top row of left, middle and right piles
    enum disk top_row[3] = {game_data_current.lhs[0], game_data_current.middle[0], game_data_current.rhs[0]};

    // get middle row of left, middle and right piles
    enum disk middle_row[3] = {game_data_current.lhs[1], game_data_current.middle[1], game_data_current.rhs[1]};

    // get bottom row of left, middle and right piles
    enum disk bottom_row[3] = {game_data_current.lhs[2], game_data_current.middle[2], game_data_current.rhs[2]};

    void (*disk_printers[4])() = {print_null_disk, print_red_disk, print_orange_disk, print_yellow_disk};
    
    // print top row
    printw("\n");
    printw("\n");
    print_spacer(); print_spacer();
    for (int i = 0; i < 3; i++)
    {
        int disk_printer_index = top_row[i];
        (*disk_printers[disk_printer_index])();
        print_spacer();
    }
    printw("\n");

    // print middle row    
    print_spacer(); print_spacer();
    for (int i = 0; i < 3; i++)
    {
        int disk_printer_index = middle_row[i];
        (*disk_printers[disk_printer_index])();
        print_spacer();
    }
    printw("\n");
    
    // print bottom row    
    print_spacer(); print_spacer();
    for (int i = 0; i < 3; i++)
    {
        int disk_printer_index = bottom_row[i];
        (*disk_printers[disk_printer_index])();
        print_spacer();
    }
    printw("\n");
    
    // print base plate row    
    print_spacer(); print_spacer(); print_base_plate_disk(); print_spacer(); print_base_plate_disk(); print_spacer(); print_base_plate_disk();
    printw("\n");
}

void print_player_prompt()
{
    printw("\n\n");
    print_spacer();
    printw("To win: stack the disks up from biggest to smallest on a new pile.\n\n");
    print_spacer();
    printw("Rules:\n");
    print_spacer();
    printw("(1) move only one disk at a time\n");
    print_spacer();
    printw("(2) a bigger disk cannot go on top of a smaller disk\n\n");
    print_spacer();
    printw("To move:\n");
    print_spacer();
    printw("\"rm\" moves top disk from right pile to middle pile,\n");
    print_spacer();
    printw("\"ml\" moves from middle to left, etc.\n\n");
    print_spacer();
    printw("Enter your move: ");
}

void congratulate()
{
    clear();

    attron(COLOR_PAIR(1));
    printw(".==========================.\n");
    printw("|   THE TOWERS OF HANOI    |\n");
    printw(".==========================.\n");
    printw("\nCongratulations!\nYou have won the game.\nPress any key to exit.");
    
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
    else if (p[2] == null_disk)
    {
        return 2;
    }
    else if (p[1] == null_disk)
    {
        return 1;
    }
    return 0;
}

enum disk* get_pile_by_label(char c)
{
    if (c == 'l')
    {
        return game_data_current.lhs;
    }
    else if (c == 'm')
    {
        return game_data_current.middle;
    }
    else if (c == 'r')
    {
        return game_data_current.rhs;
    }
    return NULL;
}