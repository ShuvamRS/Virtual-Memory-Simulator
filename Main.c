#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
	int page_number;
	struct Physical_Address physical_address[PAGE_SIZE];
};

struct MainMemory {
	struct Page page[MM_PAGE_COUNT];
	int currentPage; // used for the FIFO algorithm

	// need some value for LRU: How to keep track of the least recentlu used page?
	int justUsed[30];
	int counter;
};

struct Disk {
	struct Page page[VM_PAGE_COUNT]; // The disk page number of each virtual page is the same as the virtual page number.
};

struct PageTableEntry {
	int valid_bit; // 1 if page is in main memory and 0 if page is on disk
	int dirty_bit; //1 if corresponding page has been written to while in main memory. 0 if the page has not been written to since it has been in main memory.
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


void read(int virtual_address, struct VirtualMemory *VM, struct MainMemory *MM, struct Disk *D) {
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
		
		// add an if statement to see if mode is FIFO or LRU
		page_replacement_algorithm(&pte, &MM, &D, 0);
	}

	else {
		printf("%i\n", pte.mapped_page->physical_address[index].value);
	}
}


void write(int virtual_address, int value, struct VirtualMemory *VM, struct MainMemory *MM, struct Disk *D) {
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

		// add an if statement to see if mode is FIFO or LRU
		page_replacement_algorithm(&pte, &MM, &D, 0);
	}

	else {
		pte.mapped_page->physical_address[index].value = value;
		pte.dirty_bit = 1;
	}
}

void page_replacement_algorithm(struct PageTableEntry *pte, struct MainMemory *MM, struct Disk *D, int mode) {
	// VA is in DISK, must move to main memory

	bool room_in_mainmemory = false;

	if (MM->currentPage < MM_PAGE_COUNT) {
		room_in_mainmemory = true;
		int currentIndex = MM->currentPage++;	// assign index before incrementing MM->currentPage
		MM->page[currentIndex] = *pte->mapped_page; // Allocate given page data to mainmemory
		*pte->mapped_page = MM->page[currentIndex];  // have PTE point to the mainmemory
		MM->justUsed[MM->counter++] = pte->mapped_page->page_number; // keeping track of used page numbers for LRU
	}

	if (!room_in_mainmemory) { // mode = 0 is FIFO, mode = 1 is LRU
		// implement LRU and FIFO seperately for swapping between main memory and disk

		if (mode == 0) {
			// FIFO
			int page_num = MM->page[0].page_number; // oldest element
			D->page[page_num] = MM->page[0];	// allocate data back to disk
			
			// shift elements of the queue (MM page array) left one time
			int i;
			for (i = 0; i < MM_PAGE_COUNT - 1; i++) {
				MM->page[i] = MM->page[i+1];
			}
			MM->page[MM_PAGE_COUNT - 1] = *pte->mapped_page; // assign the given data to the end of MM

			*pte->mapped_page = MM->page[MM_PAGE_COUNT - 1]; // current PTE points to last element in MM
		}

		else {
			// LRU
			int leastRecentlyUsed = MM->justUsed[0]; // page number to get rid of
			// potential problem: what if a page number is used more than once? may have to account for this
			bool foundLRU = false; // pontential solution to problem described above

			// update justUsed array by shifting elements to the left 
			int i;
			for (i = 0; i < MM->counter - 1; i++) {	// counter keeps track of used index spaces in justUsed array
				MM->justUsed[i] = MM->justUsed[i + 1];
			}
			MM->justUsed[MM->counter - 1] = pte->mapped_page->page_number; // assign given page number to the end of justUsed array
			
			
			// finding the page number among the pages in MM
			int j;
			for (int j = 0; j < MM_PAGE_COUNT; j++) {
				if ( MM->page[j].page_number == leastRecentlyUsed) { // found victim
					foundLRU = true;
					D->page[leastRecentlyUsed] = MM->page[j]; // allocate data back to disk
					MM->page[j] = *pte->mapped_page; // assigm this MM page to new given data
					*pte->mapped_page = MM->page[j]; // have curent PTE point to MM
				}
			}

			while (!foundLRU) { // deals with page numbers already taken out but show up as duplicates in the justUsed array
				leastRecentlyUsed = MM->justUsed[0];

				// update justUsed array by shifting elements to the left 
				int i;
				for (i = 0; i < MM->counter - 1; i++) {	// counter keeps track of used index spaces in justUsed array
					MM->justUsed[i] = MM->justUsed[i + 1];
				}
				// exclude the following line 
				// MM->justUsed[MM->counter - 1] = pte->mapped_page->page_number; // assign given page number to the end of justUsed array
				// instead, update MM->counter
				MM->counter--;

				// finding the page number among the pages in MM
				int j;
				for (int j = 0; j < MM_PAGE_COUNT; j++) {
					if ( MM->page[j].page_number == leastRecentlyUsed) { // found victim
						foundLRU = true;
						D->page[leastRecentlyUsed] = MM->page[j]; // allocate data back to disk
						MM->page[j] = *pte->mapped_page; // assigm this MM page to new given data
						*pte->mapped_page = MM->page[j]; // have curent PTE point to MM
					}
				}
			}
		}
	}
	// Afterwards, update page table
	pte->valid_bit = 1;
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
	int i = 0;
	for (; i < VM_PAGE_COUNT; i++) {
		printf("%i:", i);
		printf("%i:", VM->PTE[i].valid_bit);
		printf("%i:", VM->PTE[i].dirty_bit);
		printf("%i\n", VM->PTE[i].mapped_page->page_number);
	}
}


void VM_Simulator() {
	struct VirtualMemory VM;
	struct MainMemory MM;
	struct Disk D;
	int i = 0, j = 0;
	char cmdline[MAXLINE];
	char *argv[MAXARGC];

	// All memory locations are initialized to the value of -1.
	for (i = 0; i < MM_PAGE_COUNT; i++) { 
		MM.page[i].page_number = i;
		for (j = 0; j < PAGE_SIZE; j++)
			MM.page[i].physical_address[j].value = -1;
	}
	MM.currentPage = 0;
	MM.counter = 0;

	// All disk locations are initialized to the value of -1.
	for (i = 0; i < VM_PAGE_COUNT; i++) {
		D.page[i].page_number = i;
		for (j = 0; j < PAGE_SIZE; j++)
			D.page[i].physical_address[j].value = -1;
	}

	// All virtual pages are initially on disk, so the valid bits and dirty bits of all page table entries are equal to 0.
	for (i = 0; i < VM_PAGE_COUNT; i++) {
		VM.PTE[i].valid_bit = 0;
		VM.PTE[i].dirty_bit = 0;
		VM.PTE[i].mapped_page = &D.page[i];
	}

	
	while (1) {
		// Display prompt and get user input
		printf("> ");
		fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin)) exit(0);

		parseline(cmdline, argv);
		
		if (strcmp(argv[0], "read") == 0) read(atoi(argv[1]), &VM, &MM, &D);
		else if (strcmp(argv[0], "write") == 0) write(atoi(argv[1]), atoi(argv[2]), &VM, &MM, &D);
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