#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VM_ADDRESS_COUNT 64
#define MM_ADDRESS_COUNT 32
#define VM_PAGE_COUNT 8
#define MM_PAGE_COUNT 4
#define PAGE_SIZE 8 // # of addresses in a page
#define MAXLINE 30
#define MAXARGC 30

struct Physical_Address {
	int value;  // each address in memory stores a single integer
};

struct Page {
	struct Physical_Address physical_address[PAGE_SIZE];
};

struct MainMemory {
	struct Page page[MM_PAGE_COUNT];
};

struct Disk {
	struct Page page[VM_PAGE_COUNT]; // The disk page number of each virtual page is the same as the virtual page number.
};

struct PageTableEntry {
	int valid_bit; // 1 if page is in main memory and 0 if page is on disk
	int dirty_bit //1 if corresponding page has been written to while in main memory. 0 if the page has not been written to since it has been in main memory.
	struct Page *mapped_page; // could be mapped to physical page or disk page
};

struct VirtualMemory {
	struct PageTableEntry PTE[VM_PAGE_COUNT];
};

// Used in map_linear_address function to return 2 values
struct PageAddress {
	int page_number;
	int index;
};


void parseline(char* cmdline, char *argv[]) {
	const char delimiter[2] = " ";
	char *token;
	int counter = 0;

	// Remove trailing newline character from input
	char *temp_pos;
	if ((temp_pos = strchr(cmdline, '\n')) != NULL) *temp_pos = '\0';

	// strtok returns one token at a time
	token = strtok(cmdline, delimiter);
	while (token != NULL) {
		argv[counter++] = token;
		token = strtok(NULL, delimiter);
	}
}


struct PageAddress map_linear_address(int lin_address) {
	// Maps linear address to page number and index in that page.
	struct PageAddress page_address = {lin_address / PAGE_SIZE,  lin_address % PAGE_SIZE};
	return page_address;
}


void read(int virtual_address, struct VirtualMemory *VM, struct MainMemory *MM) {
	// Prints the contents of a virtual memory address.
	
	int page_number, index;
	struct PageAddress page_address;
	struct PageTableEntry pte;
	
	if (virtual_address < 0 || virtual_address >= VM_ADDRESS_COUNT) {
		printf("Virtual Address must be between 0 and %d inclusive.\n", VM_ADDRESS_COUNT-1);
		return;
	}

	page_address = map_linear_address(virtual_address);
	page_number = page_address.page_number;
	index = page_address.index;
	pte = VM->PTE[page_number];

	if (pte.valid_bit == 0) {
		// If Page Fault occurs
		printf("A Page Fault Has Occurred\n");

		// should run Page Replacement Algorithm
	}

	else printf("%i\n", pte.mapped_page->physical_address[index].value);
}


void write(int virtual_address, int value, struct VirtualMemory *VM, struct MainMemory *MM) {
	// Writes data to a virtual memory location.

	int page_number, index;
	struct PageAddress page_address;
	struct PageTableEntry pte;
	
	if (virtual_address < 0 || virtual_address >= VM_ADDRESS_COUNT) {
		printf("Virtual Address must be between 0 and %d inclusive.\n", VM_ADDRESS_COUNT-1);
		return;
	}

	page_address = map_linear_address(virtual_address);
	page_number = page_address.page_number;
	index = page_address.index;
	pte = VM->PTE[page_number];

	if (pte.valid_bit == 0) {
		// If Page Fault occurs
		printf("A Page Fault Has Occurred\n");

		// should run Page Replacement Algorithm
	}

	else {
		pte.mapped_page->physical_address[index].value = value;
		pte.dirty_bit = 1;
	}
}


void show_main(int physical_page_number, struct MainMemory *MM) {
	// Prints the contents of a physical page in the main memory.

	struct Page page = MM->page[physical_page_number];
	int address_content, i = 0;
	int page_index = PAGE_SIZE * physical_page_number; // To get the beginning index in physical memory based on physical_page_number

	if (physical_page_number < 0 || physical_page_number >= MM_PAGE_COUNT) {
		printf("Physical page number must be between 0 and %d inclusive.\n", MM_PAGE_COUNT-1);
		return;
	}

	for (; i < PAGE_SIZE; i++) {
		address_content = page.physical_address[i].value;
		printf("%i:%i\n", page_index + i, address_content);
	}
}


void show_disk(int disk_page_number, struct Disk *D) {
	// Prints the contents of a page on disk.

	struct Page page = D->page[disk_page_number];
	int address_content, i = 0;
	int page_index = PAGE_SIZE * disk_page_number; // To get the beginning index on disk based on disk_page_number

	if (disk_page_number < 0 || disk_page_number >= VM_PAGE_COUNT) {
		printf("Disk page number must be between 0 and %d inclusive.\n", VM_PAGE_COUNT-1);
		return;
	}

	for (; i < PAGE_SIZE; i++) {
		address_content = page.physical_address[i].value;
		printf("%i:%i\n", page_index + i, address_content);
	}
}


void show_page_table(struct VirtualMemory *VM, struct MainMemory *MM, struct Disk *D) {
	// Prints the contents of each page table entry

	for (i = 0; i < VM_PAGE_COUNT; i++) {
		printf("%i:", i);
		printf("%i:", VM.PTE[i].valid_bit);
		printf("%i:", VM.PTE[i].dirty_bit);
		printf("%i\n", i);
	}
}


void VM_Simulator() {
	struct VirtualMemory VM;
	struct MainMemory MM;
	struct Disk D;
	int i = 0, j = 0;
	char cmdline[MAXLINE];
	char *argv[MAXARGC];

	// All virtual pages are initially on disk, so the valid bits and dirty bits of all page table entries are equal to 0.
	for (i = 0; i < VM_PAGE_COUNT; i++) {
		VM.PTE[i].valid_bit = 0;
		VM.PTE[i].dirty_bit = 0;
	}

	// All memory locations are initialized to the value of -1.
	for (i = 0; i < MM_PAGE_COUNT; i++) 
		for (j = 0; j < PAGE_SIZE; j++)
			MM.page[i].physical_address[j].value = -1;

	// All disk locations are initialized to the value of -1.
	for (i = 0; i < VM_PAGE_COUNT; i++) 
		for (j = 0; j < PAGE_SIZE; j++)
			D.page[i].physical_address[j].value = -1;

	
	while (1) {
		// Display prompt and get user input
		printf("> ");
		fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin)) exit(0);

		parseline(cmdline, argv);
		
		if (strcmp(argv[0], "read") == 0) read(atoi(argv[1]), &VM, &MM);
		else if (strcmp(argv[0], "write") == 0) write(atoi(argv[1]), atoi(argv[2]), &VM, &MM);
		else if (strcmp(argv[0], "showmain") == 0) show_main(atoi(argv[1]), &MM);
		else if (strcmp(argv[0], "showdisk") == 0) show_disk(atoi(argv[1]), &D);
		else if (strcmp(argv[0], "showptable") == 0) show_page_table(&VM, &MM, &D);
		else if (strcmp(argv[0], "quit") == 0) return;
		else printf("Invalid Input!\n");
	}
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

	VM_Simulator();

	return 0;
}