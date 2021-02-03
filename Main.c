/*
Questions:
1. Data types of valid_bit and mapped_address
2. MainMemory contents (different struct for table)
3. Disk Structure
4. Where are the contents saved
*/



#include <stdio.h>

#define VM_ADDRESS_COUNT 64
#define MM_ADDRESS_COUNT 32
#define VM_PAGE_COUNT 8
#define MM_PAGE_COUNT 4
#define PAGE_SIZE 8 // # of addresses in a page

struct PageTableEntry {
	int valid_bit; // 1 if page is in main memory and 0 if page is on disk
	void *mapped_address; // could be physical page address or disk address
};

struct Page {
	int data;
};

struct PageTable {
	struct PageTableEntry PTE[PAGE_SIZE];
};


struct VirtualMemory {
	struct PageTable pages[VM_PAGE_COUNT];
};

struct MainMemory {
	Page[]
};

struct Disk {
	Page[];
};

void VM_Simulator() {
	VirtualMemory *VM = NULL;
	MainMemory *MM[MM_ADDRESS_COUNT];
}

int main(int argc, char *argv[]) {

	char *valid_command_line_arguments[] = {"FIFO", "LRU"};
	char *command_line_argument = NULL;
	
	if (argc == 2) {
		// if a command-line argument for the selection of page replacement algorithm if given
		command_line_argument = argv[2];
	}

	else if (argc > 2) {
		// if more than 1 argument is given
		printf("%s\n", "Simulator accepts only one optional command-line argument for the selection of the page replacement algorithm.");
		return -1;
	}

	return 0;
}