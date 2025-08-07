#include "../drivers/display/display.h"
#include "../kernel/util.h"
#include "shell.h"

void execute_command(char *input) {
    if (compare_string(input, "EXIT") == 0) {
        print_string("Stopping The CPU. Farewell! :3\n");

        util_quit();

        asm volatile("hlt");
    }

    else if (compare_string(input, "CLS") == 0) {
        clear_screen();
    }

    else {
        print_string("Unknown command: ");
        print_string(input);
    }

    print_string("\n> ");
}