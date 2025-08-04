#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../drivers/display.h"
#include "../drivers/keyboard.h"

#include "util.h"
#include "mem.h"

void* alloc(int n) {
    int *ptr = (int *) mem_alloc(n * sizeof(int));
    if (ptr == NULL_POINTER) {
        print_string("Memory not allocated.\n");
    } else {
        // Get the elements of the array
        for (int i = 0; i < n; ++i) {
//            ptr[i] = i + 1; // shorthand for *(ptr + i)
        }

        for (int i = 0; i < n; ++i) {
//            char str[256];
//            int_to_string(ptr[i], str);
//            print_string(str);
        }
//        print_nl();
    }
    return ptr;
}

void main() {
    clear_screen();
    print_string("Installing interrupt service routines (ISRs).\n");
    isr_install();

    print_string("Enabling external interrupts.\n");
    asm volatile("sti");

    print_string("Initializing keyboard (IRQ 1).\n");
    init_keyboard();

    clear_screen();

    print_nl();
    print_string("  888b    |                      ,88~-_   ,d88~~\\");
    print_nl();
    print_string("  |Y88b   |  e88~~8e  Y88b  /   d888   \\  8888   ");
    print_nl();
    print_string("  | Y88b  | d888  88b  Y88b/   88888    | `Y88b  ");
    print_nl();
    print_string("  |  Y88b | 8888__888   Y88b   88888    |  `Y88b,");
    print_nl();
    print_string("  |   Y88b| Y888    ,   /Y88b   Y888   /     8888");
    print_nl();
    print_string("  |    Y888  \"88___/   /  Y88b   `88_-~   \\__88P'");
    print_nl();
    print_nl();
    print_nl();

    print_string("init_dynamic_mem()\n");
    print_dynamic_node_size();
    print_dynamic_mem();
    print_nl();

    int *ptr1 = alloc(5);
    print_string("int *ptr1 = alloc(5)\n");
    print_dynamic_mem();
    print_nl();

    int *ptr2 = alloc(10);
    print_string("int *ptr2 = alloc(10)\n");
    print_dynamic_mem();
    print_nl();

    mem_free(ptr1);
    print_string("mem_free(ptr1)\n");
    print_dynamic_mem();
    print_nl();

    int *ptr3 = alloc(2);
    print_string("int *ptr3 = alloc(2)\n");
    print_dynamic_mem();
    print_nl();

    mem_free(ptr2);
    print_string("mem_free(ptr2)\n");
    print_dynamic_mem();
    print_nl();

    mem_free(ptr3);
    print_string("mem_free(ptr3)\n");
    print_dynamic_mem();
    print_nl();

    print_string("> ");
}