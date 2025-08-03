int compare_string(char s1[], char s2[]) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

void execute_command(char *input) {
    if (compare_string(input, "EXIT") == 0) {
        print_string("Stopping The CPU. Farewell! :3\n");
        asm volatile("hlt");
    }

    print_string("Unknown command: ");
    print_string(input);
    print_string("\n> ");
}