void execute_command(char *input) {
    if (compare_string(input, "EXIT") == 0) {
        print_string("Stopping The CPU. Farewell! :3\n");
        asm volatile("hlt");
    }

    print_string("Unknown command: ");
    print_string(input);
    print_string("\n> ");
}