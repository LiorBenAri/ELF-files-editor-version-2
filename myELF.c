#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>
#include <string.h>

int debug = 0;
int Currentfd;
int Currentfd2;
int _open = 0;
int _open2 = 0;
void *map_start; /* will point to the start of the memory mapped file */
void* map_start2;
Elf32_Ehdr *header; /* this will point to the header structure */
Elf32_Ehdr *header2;
unsigned char* e_ident;
unsigned char* e_ident2;
int file_size;
int file_size2;
char* start = "_start\0";
char* symbol_array[1000];
char* undef_symbol_array[1000];
int arr_p = 0;
int arr_p2 = 0;
char* empty = "\0";


struct fun_desc
{
	char *name;
	void (*fun)();
};

void print_type(int type)
{
	switch(type)
	{
		case 0:
		printf("NULL\n");
		break;

		case 1:
		printf("PROGBITS\n");
		break;

		case 2:
		printf("SYMTAB\n");
		break;

		case 3:
		printf("STRTAB\n");
		break;

		case 4:
		printf("RELA\n");
		break;

		case 5:
		printf("HASH\n");
		break;

		case 6:
		printf("DYNAMIC\n");
		break;

		case 7:
		printf("NOTE\n");
		break;

		case 8:
		printf("NOBITS\n");
		break;

		case 9:
		printf("REL\n");
		break;

		case 10:
		printf("SHLIB\n");
		break;

		case 11:
		printf("DYNSYM\n");
		break;

		case 0x70000000:
		printf("LOPROC\n");
		break;

		case 0x7fffffff:
		printf("HIPROC\n");
		break;

		case 0x80000000:
		printf("LOUSER\n");
		break;

		case 0xffffffff:
		printf("HIUSER\n");
		break;

		case 0x6ffffffe:
		printf("VERNEED\n");
		break;

		case 0x6fffffff:
		printf("VERSYM\n");
		break;

		default:
		printf("\n");
		break;
	}
}

void close_files()
{
	if (_open)
	{
		munmap(map_start, file_size);
		close(Currentfd);
		Currentfd = -1;
		_open = 0;
	}
}

int open_file(char* filename)
{
	if ((Currentfd = open(filename, O_RDONLY, 0777)) < 0)
	{
		fprintf(stderr, "%s\n", "file opening failed");
		return 0;
	}
	_open = 1;
	return 1;
}

void close_files2()
{
	if (_open2)
	{
		munmap(map_start2, file_size2);
		close(Currentfd2);
		Currentfd2 = -1;
		_open2 = 0;
	}
}

int open_file2(char* filename)
{
	if ((Currentfd2 = open(filename, O_RDONLY, 0777)) < 0)
	{
		fprintf(stderr, "%s\n", "file opening failed");
		return 0;
	}
	_open2 = 1;
	return 1;
}

void start_check(int file_num, void* _map_start, Elf32_Ehdr* _header)
{
	if (file_num == 1)
	{
		if (!_open)
		{
			fprintf(stderr, "%s\n", "no vaild first file is open");
			return;
		}
	}

	else
	{
		if (!_open2)
		{
			fprintf(stderr, "%s\n", "no vaild second file is open");
			return;
		}
	}

	int sec_num = _header->e_shnum;
	int sec_idx;
	Elf32_Shdr *first_sec = (Elf32_Shdr*)(_map_start+_header->e_shoff);
	Elf32_Shdr *curr_sec = (Elf32_Shdr*)(_map_start+_header->e_shoff);
	int curr_sym_table_offset;
	int found = 0;
	Elf32_Sym* curr_symbol;
	int in_sym_idx;
	int sym_table_size;
	int sym_name_offset;
	int sym_str_table_offset;
	char* sym_str_table;
	int sym_str_table_idx;
	int found_start = 0;

	for (sec_idx = 0; sec_idx < sec_num; ++sec_idx)
	{	
		if ((int)curr_sec->sh_type == 2 || (int)curr_sec->sh_type == 11)
			found = 1;

		if (found)
		{
			curr_sym_table_offset = curr_sec->sh_offset;
			curr_symbol = (Elf32_Sym*)(_map_start+curr_sym_table_offset);

			sym_table_size = curr_sec->sh_size/16;

			sym_str_table_idx = curr_sec->sh_link;
			sym_str_table_offset = (first_sec+sym_str_table_idx)->sh_offset;
			sym_str_table = _map_start+sym_str_table_offset;

			for (in_sym_idx = 0; in_sym_idx < sym_table_size; ++in_sym_idx)
			{
				sym_name_offset = curr_symbol->st_name;
				if (strcmp((char*)(sym_str_table+sym_name_offset), start) == 0)
				{
					found_start = 1;
					break;
				}
				++curr_symbol;
			}
		}
		if (found_start)
		{
			printf("_start check: PASSED\n");
			break;
		}

		found = 0;
		++curr_sec;
	}
	if (!found_start)
		printf("_start check: FAILED\n");
}

void Duplicate_and_undef_Symbols_check1()
{
	if (!_open)
	{
		fprintf(stderr, "%s\n", "no vaild first file is open");
		return;
	}
	int sec_num = header->e_shnum;
	int sec_idx;
	Elf32_Shdr *first_sec = (Elf32_Shdr*)(map_start+header->e_shoff);
	Elf32_Shdr *curr_sec = (Elf32_Shdr*)(map_start+header->e_shoff);
	int curr_sym_table_offset;
	int found = 0;
	Elf32_Sym* curr_symbol;
	int in_sym_idx;
	int sym_table_size;
	int sym_name_offset;
	int sym_str_table_offset;
	char* sym_str_table;
	int sym_str_table_idx;

	for (sec_idx = 0; sec_idx < sec_num; ++sec_idx)
	{	
		if ((int)curr_sec->sh_type == 2 || (int)curr_sec->sh_type == 11)
			found = 1;

		if (found)
		{
			curr_sym_table_offset = curr_sec->sh_offset;
			curr_symbol = (Elf32_Sym*)(map_start+curr_sym_table_offset);

			sym_table_size = curr_sec->sh_size/16;

			sym_str_table_idx = curr_sec->sh_link;
			sym_str_table_offset = (first_sec+sym_str_table_idx)->sh_offset;
			sym_str_table = map_start+sym_str_table_offset;

			for (in_sym_idx = 0; in_sym_idx < sym_table_size; ++in_sym_idx)
			{
				sym_name_offset = curr_symbol->st_name;
				symbol_array[arr_p] = (char*)(sym_str_table+sym_name_offset);
				if (curr_symbol->st_shndx == 0 && ((strcmp((char*)(sym_str_table+sym_name_offset), empty)) != 0))
				{
					undef_symbol_array[arr_p2] = (char*)(sym_str_table+sym_name_offset);
					++arr_p2;
				}
				++curr_symbol;
				++arr_p;
			}
		}
		found = 0;
		++curr_sec;
	}
}

void Duplicate_Symbols_check2()
{
	if (!_open2)
	{
		fprintf(stderr, "%s\n", "no vaild first file is open");
		return;
	}
	int sec_num = header2->e_shnum;
	int sec_idx;
	Elf32_Shdr *first_sec = (Elf32_Shdr*)(map_start2+header2->e_shoff);
	Elf32_Shdr *curr_sec = (Elf32_Shdr*)(map_start2+header2->e_shoff);
	int curr_sym_table_offset;
	int found = 0;
	Elf32_Sym* curr_symbol;
	int in_sym_idx;
	int sym_table_size;
	int sym_name_offset;
	int sym_str_table_offset;
	char* sym_str_table;
	int sym_str_table_idx;
	int arr_idx = 0;
	int found_dup = 0;

	for (sec_idx = 0; sec_idx < sec_num; ++sec_idx)
	{	
		if ((int)curr_sec->sh_type == 2 || (int)curr_sec->sh_type == 11)
			found = 1;

		if (found)
		{
			curr_sym_table_offset = curr_sec->sh_offset;
			curr_symbol = (Elf32_Sym*)(map_start2+curr_sym_table_offset);

			sym_table_size = curr_sec->sh_size/16;

			sym_str_table_idx = curr_sec->sh_link;
			sym_str_table_offset = (first_sec+sym_str_table_idx)->sh_offset;
			sym_str_table = map_start2+sym_str_table_offset;

			for (in_sym_idx = 0; in_sym_idx < sym_table_size; ++in_sym_idx)
			{
				sym_name_offset = curr_symbol->st_name;

				for (arr_idx = 0; arr_idx < arr_p; ++arr_idx)
				{
					if ((strcmp((char*)(sym_str_table+sym_name_offset), symbol_array[arr_idx]) == 0) && (strcmp((char*)(sym_str_table+sym_name_offset), empty)) != 0)
					{
						found_dup = 1;
						printf("duplicate symbol: %s\n", (char*)(sym_str_table+sym_name_offset));
						break;
					}
				}
				arr_idx = 0;
				++curr_symbol;
			}
		}
		found = 0;
		++curr_sec;
	}
	if (!found_dup)
		printf("duplicate check: PASSED\n");
}

void undef_Symbols_check2()
{
	if (!_open2)
	{
		fprintf(stderr, "%s\n", "no vaild first file is open");
		return;
	}
	int sec_num = header2->e_shnum;
	int sec_idx;
	Elf32_Shdr *first_sec = (Elf32_Shdr*)(map_start2+header2->e_shoff);
	Elf32_Shdr *curr_sec = (Elf32_Shdr*)(map_start2+header2->e_shoff);
	int curr_sym_table_offset;
	int found = 0;
	Elf32_Sym* curr_symbol;
	Elf32_Sym* first_symbol;
	int in_sym_idx;
	int sym_table_size;
	int sym_name_offset;
	int sym_str_table_offset;
	char* sym_str_table;
	int sym_str_table_idx;
	int arr_idx = 0;
	int found_undef = 1;
	int found2 = 0;

	for (sec_idx = 0; sec_idx < sec_num; ++sec_idx)
	{	
		if ((int)curr_sec->sh_type == 2 || (int)curr_sec->sh_type == 11)
			found = 1;

		if (found)
		{
			curr_sym_table_offset = curr_sec->sh_offset;
			first_symbol = (Elf32_Sym*)(map_start2+curr_sym_table_offset);
			curr_symbol = (Elf32_Sym*)(map_start2+curr_sym_table_offset);

			sym_table_size = curr_sec->sh_size/16;

			sym_str_table_idx = curr_sec->sh_link;
			sym_str_table_offset = (first_sec+sym_str_table_idx)->sh_offset;
			sym_str_table = map_start2+sym_str_table_offset;

			for (arr_idx = 0; arr_idx < arr_p2; ++arr_idx)
			{
				for (in_sym_idx = 0; in_sym_idx < sym_table_size; ++in_sym_idx)
				{
					sym_name_offset = curr_symbol->st_name;

					if ((strcmp((char*)(sym_str_table+sym_name_offset), undef_symbol_array[arr_idx]) == 0) && ((strcmp((char*)(sym_str_table+sym_name_offset), empty)) != 0)
						&& (curr_symbol->st_shndx != 0))
					{
						found_undef = 0;
						break;
					}
					++curr_symbol;
				}
				if (found_undef)
				{
					found2 = 1;
					printf("undefined symbol: %s\n", undef_symbol_array[arr_idx]);
				}	
				found_undef = 1;
				curr_symbol = first_symbol;
			}
		}
		found = 0;
		++curr_sec;
	}
	if (!found2)
		printf("no undefined symbols\n");
}

void Toggle_Debug_Mode()
{
	if (debug)
	{
		debug = 0;
		printf("Debug flag now off\n");
	}
	else
	{
		debug = 1;
		printf("Debug flag now on\n");		
	}
}

void Examine_ELF_File()
{
	int elf = 1;
	int i;
	char magic_num[4] = {0x7f,0x45,0x4c,0x46};
	int len;
	char filename[20];
	printf("enter file name:\n");
	fgets(filename, 20, stdin);
	len = strlen(filename);
	filename[len-1]='\0';

	close_files();

	if(open_file(filename) == 0)
		return;

	file_size = lseek(Currentfd, 0, SEEK_END);
	lseek(Currentfd, 0, SEEK_SET);

	if ( (map_start = mmap(0, file_size, PROT_READ , MAP_SHARED, Currentfd, 0)) == MAP_FAILED )
	{
		perror("mmap failed");
		close(Currentfd);
		Currentfd = -1;
		_open = 0;
		return;
	}

	header = (Elf32_Ehdr *) map_start;
	e_ident = header->e_ident;
	printf("Magic number: ");
	for (i = 0; i < 3; ++i)
	{
		
		printf("%02hhX ", e_ident[i]);
		if (e_ident[i] != magic_num[i])
			elf = 0;
	}
	printf("\n");

	if (elf)
	{
		if (e_ident[i] != magic_num[i])
			elf = 0;
	}
	if (!elf)
	{
		fprintf(stderr, "%s\n", "not a elf file");
		munmap(map_start, file_size);
		close(Currentfd);
		Currentfd = -1;
		_open = 0;
		return;
	}
	arr_p = 0;
	arr_p2 = 0;

	printf("data encoding scheme: ");
	switch(e_ident[5]) 
	{
		case ELFDATA2LSB:
		printf("little endian\n");
		break;

		case ELFDATA2MSB:
		printf("big endian\n");
		break;

		default:
		printf("Invalid format\n");
		break;
	}
	printf("Entry point: ");
	printf("0x%X\n", header->e_entry);
	printf("offset to section header table: ");
	printf("%i\n", header->e_shoff);
	printf("The number of section header entries: ");
	printf("%i\n", header->e_shnum);
	printf("The size of section header entry: ");
	printf("%i\n", header->e_shentsize);
	printf("The file offset to the program header: ");
	printf("%i\n", header->e_phoff);
	printf("The number of program header entries: ");
	printf("%i\n", header->e_phnum);
	printf("The size of each program header entry: ");
	printf("%i\n", header->e_phentsize);
}

void Print_Section_Names()
{
	if (!_open)
	{
		fprintf(stderr, "%s\n", "no vaild file is open");
		return;
	}
	int sec_num = header->e_shnum;
	int str_table_idx = header->e_shstrndx;
	int curr_idx = 0;
	Elf32_Shdr *curr_sec = (Elf32_Shdr*)(map_start+header->e_shoff);
	int str_table_offset;
	char* str_table;
	int name_offset;

	while(curr_idx < sec_num)
	{
		if (curr_idx == str_table_idx)
		{
			str_table_offset = curr_sec->sh_offset;
			break; 		
		}
		++curr_idx;
		++curr_sec;
	}
	curr_sec = (Elf32_Shdr*)(map_start+header->e_shoff);
	str_table = map_start+str_table_offset;
	if (debug)
	{
		printf("e_shstrndx value: %i\n",str_table_idx);
		printf("names section offset:%i\n", str_table_offset);
	}
	printf("[Nr] %-19s %8s %6s %6s %4s\n", "Name", "Addr", "Off", "Size", "Type");
	for (curr_idx = 0; curr_idx < sec_num; ++curr_idx)
	{	
		name_offset = curr_sec->sh_name;
		printf("[%2d] %-19s %08x %06x %06x ",curr_idx,(char*)(str_table+(name_offset)),curr_sec->sh_addr,curr_sec->sh_offset,curr_sec->sh_size);
		print_type((int)curr_sec->sh_type);
		++curr_sec;
	}
}

void Print_Symbols()
{
	if (!_open)
	{
		fprintf(stderr, "%s\n", "no vaild file is open");
		return;
	}
	int sec_num = header->e_shnum;
	int sec_idx;
	Elf32_Shdr *first_sec = (Elf32_Shdr*)(map_start+header->e_shoff);
	Elf32_Shdr *curr_sec = (Elf32_Shdr*)(map_start+header->e_shoff);
	int curr_sym_table_offset;
	int found = 0;
	Elf32_Sym* curr_symbol;
	int in_sym_idx;
	int sym_table_size;
	int str_table_idx = header->e_shstrndx;
	int str_table_offset = (curr_sec+str_table_idx)->sh_offset;
	char* str_table = map_start+str_table_offset;
	int name_offset;
	int sym_name_offset;
	int sym_str_table_offset;
	char* sym_str_table;
	int sym_str_table_idx;
	int none = 2;

	for (sec_idx = 0; sec_idx < sec_num; ++sec_idx)
	{	
		if ((int)curr_sec->sh_type == 2 || (int)curr_sec->sh_type == 11)
			found = 1;

		if (found)
		{
			name_offset = curr_sec->sh_name;
			curr_sym_table_offset = curr_sec->sh_offset;
			curr_symbol = (Elf32_Sym*)(map_start+curr_sym_table_offset);

			sym_table_size = curr_sec->sh_size/16;
			if (sym_table_size == 0) --none;
			if (debug)
			{
				printf("symbol table size in bytes: %i\n", curr_sec->sh_size);
				printf("number of symbols on the symbol table: %i\n", sym_table_size);
			}

			sym_str_table_idx = curr_sec->sh_link;
			sym_str_table_offset = (first_sec+sym_str_table_idx)->sh_offset;
			sym_str_table = map_start+sym_str_table_offset;

			printf("[index] %-8s %-8s %-6s %-20s\n", "value", "section_index", "section_name", "symbol_name");
			for (in_sym_idx = 0; in_sym_idx < sym_table_size; ++in_sym_idx)
			{
				sym_name_offset = curr_symbol->st_name;
				printf("[%5d] %08x %-13d %-12s %-10s\n",in_sym_idx,curr_symbol->st_value,sec_idx,(char*)(str_table+(name_offset)),(char*)(sym_str_table+sym_name_offset));
				++curr_symbol;
			}
			printf("\n");
		}
		found = 0;
		++curr_sec;

	}
	if (!none) fprintf(stderr, "%s\n", "there no symbols the ELF file");
}

void Link_to()
{
	int elf = 1;
	int i;
	char magic_num[4] = {0x7f,0x45,0x4c,0x46};
	int len;
	char filename[20];
	printf("enter file name:\n");
	fgets(filename, 20, stdin);
	len = strlen(filename);
	filename[len-1]='\0';
	if (debug)
		printf("second file name: %s\n", filename);

	close_files2();

	if (debug)
		printf("opennig the second file\n");
	if(open_file2(filename) == 0)
		return;

	file_size2 = lseek(Currentfd2, 0, SEEK_END);
	lseek(Currentfd2, 0, SEEK_SET);

	if (debug)
		printf("mapping the second file\n");
	if ( (map_start2 = mmap(0, file_size2, PROT_READ , MAP_SHARED, Currentfd2, 0)) == MAP_FAILED )
	{
		perror("mmap failed");
		close(Currentfd2);
		Currentfd2 = -1;
		_open2 = 0;
		return;
	}

	header2 = (Elf32_Ehdr *) map_start2;
	e_ident2 = header2->e_ident;
	if (debug)
		printf("checking if the second file is an elf file\n");
	for (i = 0; i < 3; ++i)
	{	
		if (e_ident2[i] != magic_num[i])
		{
			elf = 0;
			break;			
		}
	}

	if (elf)
	{
		if (e_ident2[i] != magic_num[i])
			elf = 0;
	}
	if (!elf)
	{
		fprintf(stderr, "%s\n", "not a elf file");
		munmap(map_start2, file_size2);
		close(Currentfd2);
		Currentfd2 = -1;
		_open2 = 0;
		return;
	}

	printf("first file:\n");
	start_check(1, map_start, header);
	printf("second file:\n");
	start_check(2, map_start2, header2);
	Duplicate_and_undef_Symbols_check1();
	Duplicate_Symbols_check2();
	undef_Symbols_check2();

	for (i = 0; i < 1000; ++i)
	{
		undef_symbol_array[i] = NULL;
		symbol_array[i] = NULL;
	}
}


void Quit()
{
	if (debug){printf("quitting\n");}
	close_files();
	exit(0);
}

int main(int argc, char **argv)
{
	int file = open("ELFexecLong", O_RDONLY, 0777);
	int size1 = lseek(file,0,SEEK_END);
	printf("%i\n", size1);
	char str[10];
	int pick;
	static char* fun_names[] = {"Toggle Debug Mode","Examine ELF File", "Print Section Names","Print Symbols","Link to","Quit",NULL,NULL};
	typedef void (*funp)(void);
	static funp fun_p_array[] = {&Toggle_Debug_Mode,&Examine_ELF_File,&Print_Section_Names,&Print_Symbols,&Link_to,&Quit,NULL,NULL};
	int i;
	struct fun_desc fun_array[8];
	int bound = (sizeof(fun_array)/sizeof(fun_array[0]))-3;

	for (i = 0; fun_names[i] != NULL ; ++i)
	{
		fun_array[i].name = fun_names[i];
		fun_array[i].fun = fun_p_array[i];

	}
	fun_array[i].name = fun_names[i];
	fun_array[i].fun = fun_p_array[i];

	while(1)
	{
		printf("Please choose a function:\n");  
		for (i = 0; fun_names[i] != NULL ; ++i)
		{
			printf("%i) %s\n", i,fun_array[i].name);
		}
		fgets(str, 10, stdin);
		if (isdigit(*str) == 0)
		{
			printf("unvaild input\n");
			continue;
		}
		pick = atoi(str);
		printf("Function Number: %i\n", pick);

		if (0 <= pick && pick <= bound)
		{
			printf("within bounds\n");
			(fun_array[pick].fun)();
			printf("DONE.\n");
		}

		else
		{
			printf("not within bounds\n");
			continue;
		} 
	}
	return 0;  
}


	/*
	char* copy = "1111";
	memcpy(buffer, copy, 4);
	*/

	    	/*printf("%p\n", fun_p_array[i]);
    	printf("%p\n", fun_array[i].fun);*/

	/*and unmap*/
	/*
	for (i = 0; i < 10; ++i)
	{
		if (files_array[i].open)
		{
			close(files_array[i].file);
		}
	}
	*/

/*
	typedef struct
{
	int the_file;
	int open;
} file;
*/

/*
typedef struct
{
	int the_file;
	char* name;

} file;
*/

/*
file files_array[10];
int next_index_to_insert = 0;
*/

/*
void close_files()
{
	int i = 0;
	int curr_file;
	while(i < next_index_to_insert)
	{
		curr_file = files_array[i].the_file;
		close(curr_file);
		++i;
	}
}

void open_file(char* filename)
{
	if (next_index_to_insert == 10)
	{
		next_index_to_insert = 0;
	}
	if ((files_array[next_index_to_insert].the_file = open(filename, O_RDONLY)) < 0)
	{
		fprintf(stderr, "%s\n", "file opening failed");
		exit(-1);
	}
	files_array[next_index_to_insert].name = filename;
	++next_index_to_insert;
}

void Toggle_Debug_Mode()
{
	if (debug)
	{
		debug = 0;
		printf("Debug flag now off\n");
	}
	else
	{
		debug = 1;
		printf("Debug flag now on\n");		
	}
}

void Examine_ELF_File()
{
	int len;
	char filename[20];
	printf("enter file name:\n");
	fgets(filename, 20, stdin);
	len = strlen(filename);
	filename[len-1]='\0';
	close_files();
	open_file(filename);

}
*/

	/*[index] section_name section_address section_offset section_size  section_type*/
		/*
		%-12s
		printf("[%2d] ", curr_idx);
		printf("%20s", (char*)(str_table+name_offset));
		printf("%s\n", );
		*/
