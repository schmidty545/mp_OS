#ifndef _FILESYS_H_
#define _FILESYS_H_


#include "types.h"

#define DENTRYSIZE 64
#define BLOCKSIZE 4096
#define NAMELEN 32


/* File system driver struct*/
typedef struct {
	unsigned char name[32];
	unsigned int type;
	unsigned int inode;
	unsigned char reserved[24];
}dentry_t;

/* File system driver struct*/
typedef struct {
	uint32_t length;
	uint32_t dblock[1023];
}inode_t;

/* function pointer structs as a generic linkage*/
typedef struct{
	int (*open)(uint32_t, uint32_t);
	int (*close)(int32_t);
	int (*read)(int32_t, void*, int32_t);
	int (*write)(int32_t, const void*, int32_t);
}fileop_table_t;
/* Descriptor table struct*/
typedef struct{
	fileop_table_t* table_pointer;
	uint32_t inode;
	uint32_t file_position;
	uint32_t flags;
}fd_t;

/* global variables to keep track of our descriptor tables*/
uint32_t FILEMEM;
uint32_t fs_entries;
uint32_t fs_inodes;
uint32_t fs_dblocks;
fd_t kernel_fdt[8];

fileop_table_t fileop_table;
fileop_table_t directory_table;


/* File system helper function to receive index*/
int32_t get_avail_fd();
/* File system open function to initialize descriptor tables*/
int32_t file_open(uint32_t fd, uint32_t inode);
/* File system close function to close descriptor tables*/
int32_t file_close(int32_t fd);
/* File system read function to read descriptor tables*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
/* File system write function to literally do nothing*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
/* Directory open function to initialize descriptor tables*/
int32_t directory_open(uint32_t fd, uint32_t inode);
/* Directory close function to close descriptor tables*/
int32_t directory_close(int32_t fd);
/* Directory read function to read entries in the directory */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
/* Directory write function that does nothing */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);

/* File system open function to set descriptor table pointer to functions*/
uint32_t set_fileop_ptr(uint32_t fd, uint32_t type);
/* global variables to keep track of our descriptor tables*/
int32_t init_filesys();
/* global variables to keep track of our descriptor tables*/
uint8_t* get_dblock_mem(uint32_t idx, uint32_t offset);
/* global variables to keep track of our descriptor tables*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
/* global variables to keep track of our descriptor tables*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
/* global variables to keep track of our descriptor tables*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

void init_fd();

#endif
