#include "filesys.h"
#include "types.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "scheduler.h"

#define SIZE_BUF 32
/*
	init_filesys
	INPUT: unit32_t offset
	OUTPUT: 0
	SIDE_EFFECTS: Initializes the file system descriptors
	and populates the tables for the drivers
*/
int32_t init_filesys(uint32_t offset)
{
	int i;
	FILEMEM=offset;
	fs_entries=*((uint32_t*)FILEMEM);
	fs_inodes=*(((uint32_t*)FILEMEM)+1);
	fs_dblocks=*(((uint32_t*)FILEMEM)+2);				//Set entries according to the appropriate offsets
	
	fileop_table.open=&file_open;
	fileop_table.close=&file_close;
	fileop_table.read=&file_read;
	fileop_table.write=&file_write;						//populate file driver function pointers
	
	rtc_table.open=&rtc_open;
	rtc_table.close=&rtc_close;
	rtc_table.read=&rtc_read;
	rtc_table.write=&rtc_write; 						//populate rtc function pointers
	
	keyboard_table.open=&keyboard_open;
	keyboard_table.close=&keyboard_close;
	keyboard_table.read=&keyboard_read;
	keyboard_table.write=&keyboard_write;				//populate keyboard function pointers
	
	directory_table.open=&directory_open;
	directory_table.close=&directory_close;
	directory_table.read=&directory_read;
	directory_table.write=&directory_write;
	
	for(i=0;i<8;i++)
	{
		kernel_fdt[i].table_pointer=0;
		kernel_fdt[i].inode=0;
		kernel_fdt[i].file_position=0;
		kernel_fdt[i].flags=0;
	}
	return 0;
}

/*
	set_fileop_ptr
	INPUT: unit32_t fd, uint32_t type
	OUTPUT: 0
	SIDE_EFFECTS: Initializes the file system descriptors
	and populates the tables for the drivers
*/
uint32_t set_fileop_ptr(uint32_t fd, uint32_t type)
{
	if(type==0)
		process_array[schedule_index].cur_fd_ptr[fd].table_pointer=&rtc_table;
	if(type==1)
		process_array[schedule_index].cur_fd_ptr[fd].table_pointer=&directory_table;
	if(type==2)
		process_array[schedule_index].cur_fd_ptr[fd].table_pointer=&fileop_table;				//Depending on the type, set file descriptor accordingly
	
	return 0;
}

/*
	init_filesys
	INPUT: unit32_t offset, uint32_t idx
	OUTPUT: 0
	SIDE_EFFECTS: returns the allocated dblock memory
*/
uint8_t* get_dblock_mem(uint32_t idx, uint32_t offset)
{
	return ((uint8_t*)FILEMEM)+((1+fs_inodes+idx)*BLOCKSIZE)+offset;		//return block memory
}
/*
	read_dentry_by_name
	INPUT: const uint8_t* fname, dentry_t* dentry
	OUTPUT: 0
	SIDE_EFFECTS: Reads the entry according to name
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
	int i,j,found;
	dentry_t* ptr=(dentry_t*)FILEMEM;
	ptr+=1;	//Increment pointer
	
	if(dentry==0)
		return -1;
	
	for(i=0;i<fs_entries;i++)
	{
		found=1;
		for(j=0;j<NAMELEN;j++)
		{
			if(fname[j]==(ptr+i)->name[j] && fname[j]==0)					//Iterate until a corresponding element has been found
				break;
			if(fname[j]!=(ptr+i)->name[j])
			{
				found=0;
				break;
			}
		}
		if(j==NAMELEN && fname[j]!=0)
			found=0;
		
		if(found)
		{
			memcpy(dentry, (ptr+i), DENTRYSIZE);							//If element has been found, perform memcpy
			return 0;
		}
	}
	return -1;																//return -1 upon failure
}
/*
	read_dentry_by_index
	INPUT: unit32_t offset, dentry_t* dentry
	OUTPUT: 0
	SIDE_EFFECTS: reads entry according to index
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
	if(dentry==0)
		return -1;
	if(index > fs_entries-1)									//If index is out of bounds, return -1
		return -1;
	
	dentry_t* ptr=(dentry_t*)FILEMEM;
	ptr+=index+1;
	memcpy(dentry, ptr, DENTRYSIZE);										//Else perform memcpy and return 0 for success
	return 0;
}
/*
	read_data
	INPUT: uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length
	OUTPUT: 0
	SIDE_EFFECTS: Reads data from the given arguments
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	int i;
	
	uint8_t* start;
	uint32_t buf_position=0;
	uint32_t start_block=offset/BLOCKSIZE;
	uint32_t end_block=(offset+length-1)/BLOCKSIZE;
	uint32_t copy_len;
	uint32_t total_len=0;
	
	inode_t* ptr=(inode_t*)FILEMEM;
	if(inode > fs_inodes-1)
		return -1;																	//If out of bounds, return -1 as failure
	ptr+=1+inode;
	if(ptr->length < offset+length)
		end_block=(ptr->length-1)/BLOCKSIZE;										//Determine the end block image and adjust it accordingly
	
	if(ptr->dblock[start_block]>fs_dblocks -1)										//If this is out of bounds, return -1 as failure
		return total_len;
		
	if(start_block > end_block)
		return total_len;
	
	start=get_dblock_mem(ptr->dblock[start_block], offset%BLOCKSIZE);
	copy_len=BLOCKSIZE - (offset%BLOCKSIZE);										//Make a copy for iteration
	
	for(i=start_block; i<end_block; i++)
	{
		memcpy(buf+buf_position, start, copy_len);
		total_len+=copy_len;
		buf_position+=copy_len;
		length-=copy_len;
		copy_len=BLOCKSIZE;															//Iterate through the blocks and perform memcpy
		if( ptr->dblock[i+1]>fs_dblocks -1)
			return total_len;
		start=get_dblock_mem(ptr->dblock[i+1], 0);									//Adjust starting point		
	}
	memcpy(buf+buf_position, start, length);
	total_len+=length;
	return total_len;
}
/*
	get_avail_fd
	INPUT: none
	OUTPUT: 0
	SIDE_EFFECTS: returns file descriptor index
*/
int32_t get_avail_fd()
{
	int i;
	for(i=2;i<8;i++)
	{
		if(process_array[schedule_index].cur_fd_ptr[i].flags==0)												//from 2 to 8, as it is a size 8 array with the first 2 occupied
			return i;
	}
	return -1;																		//If descriptor is not present, return -1
}
/*
	file_open
	INPUT: uint32_t fd, uint32_t inode
	OUTPUT: 0
	SIDE_EFFECTS: Set the file descriptors
*/
int32_t file_open(uint32_t fd, uint32_t inode)
{
	if(fd<2 || fd > 7)
		return -1;
	process_array[schedule_index].cur_fd_ptr[fd].table_pointer=&fileop_table;
	process_array[schedule_index].cur_fd_ptr[fd].inode=inode;
	process_array[schedule_index].cur_fd_ptr[fd].file_position=0;
	process_array[schedule_index].cur_fd_ptr[fd].flags=1;													//Initialize file descriptor and mark as present
	return 0;
	
}
/*
	file_close
	INPUT: uint32_t fd
	OUTPUT: 0
	SIDE_EFFECTS: clear the file descriptors
*/
int32_t file_close(int32_t fd)
{
	if(process_array[schedule_index].cur_fd_ptr[fd].flags==0)												//If file descriptor doesn't exist, return -1
		return -1;
	
	process_array[schedule_index].cur_fd_ptr[fd].table_pointer=0;
	process_array[schedule_index].cur_fd_ptr[fd].inode=-1;
	process_array[schedule_index].cur_fd_ptr[fd].file_position=0;
	process_array[schedule_index].cur_fd_ptr[fd].flags=0;													//Mark as absent
	return 0;
	
}
/*
	file_read
	INPUT: int32_t fd, void* buf, int32_t nbytes
	OUTPUT: 0
	SIDE_EFFECTS: read the data from specific files
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
	int retval=0;
	if(process_array[schedule_index].cur_fd_ptr[fd].flags==0)
		return -1;
	retval= read_data(process_array[schedule_index].cur_fd_ptr[fd].inode, process_array[schedule_index].cur_fd_ptr[fd].file_position, buf, nbytes);		//Call the generic read_data function and return 0 upon success
	process_array[schedule_index].cur_fd_ptr[fd].file_position+=retval;
	return retval;
}
/*
	file_write
	INPUT: int32_t fd, const void* buf, int32_t nbytes
	OUTPUT: 0
	SIDE_EFFECTS: Literally just return -1
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;																					//At the moment, it is read only
}

/*
	directory_open
	INPUT: uint32_t fd, uint32_t inode
	OUTPUT: 0
	SIDE_EFFECTS: Set the directory descriptors
*/
int32_t directory_open(uint32_t fd, uint32_t inode)
{
	if(fd<2 || fd > 7)
		return -1;
	process_array[schedule_index].cur_fd_ptr[fd].table_pointer=&directory_table;
	process_array[schedule_index].cur_fd_ptr[fd].inode=0;
	process_array[schedule_index].cur_fd_ptr[fd].file_position=0;
	process_array[schedule_index].cur_fd_ptr[fd].flags=1;
	return 0;
}
/*
	directory_close
	INPUT: uint32_t fd
	OUTPUT: 0
	SIDE_EFFECTS: clear the file descriptors
*/
int32_t directory_close(int32_t fd)
{
	if(process_array[schedule_index].cur_fd_ptr[fd].flags==0)												//If file descriptor doesn't exist, return -1
		return -1;
	
	process_array[schedule_index].cur_fd_ptr[fd].table_pointer=0;
	process_array[schedule_index].cur_fd_ptr[fd].inode=-1;
	process_array[schedule_index].cur_fd_ptr[fd].file_position=0;
	process_array[schedule_index].cur_fd_ptr[fd].flags=0;													//Mark as absent
	return 0;
}

/*
	directory_read
	INPUT: int32_t fd, void* buf, int32_t nbytes
	OUTPUT: 0
	SIDE_EFFECTS: read directory filename by filename
*/

int32_t directory_read(int32_t fd, void* buf, int32_t nbytes)
{
	int i;
	if(fd<2 || fd > 7)
		return -1;
	
	dentry_t dent;
	if(read_dentry_by_index(process_array[schedule_index].cur_fd_ptr[fd].file_position, &dent)==-1)
		return 0;

	memcpy(buf, dent.name, SIZE_BUF);
	((unsigned char*)buf)[SIZE_BUF]=0;
	process_array[schedule_index].cur_fd_ptr[fd].file_position++;
	for(i=0;i<SIZE_BUF;i++)
	{
		if(((uint8_t*)buf)[i]==0)
			break;
	}
	return i;
}

/*
	directory_write
	INPUT: int32_t fd, const void* buf, int32_t nbytes
	OUTPUT: 0
	SIDE_EFFECTS: Literally just return -1
*/

int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

void init_fd()
{
	int i;
	process_array[schedule_index].cur_fd_ptr[0].table_pointer=&keyboard_table;
	process_array[schedule_index].cur_fd_ptr[0].inode=0;
	process_array[schedule_index].cur_fd_ptr[0].file_position=0;
	process_array[schedule_index].cur_fd_ptr[0].flags=1;
	process_array[schedule_index].cur_fd_ptr[1].table_pointer=&keyboard_table;
	process_array[schedule_index].cur_fd_ptr[1].inode=0;
	process_array[schedule_index].cur_fd_ptr[1].file_position=0;
	process_array[schedule_index].cur_fd_ptr[1].flags=1;														//STDIN, STDOUT handling, mark them as open
	for(i=2;i<8;i++)																							//Set the rest of the file descriptor as empty at first	
	{
		process_array[schedule_index].cur_fd_ptr[i].table_pointer=0;
		process_array[schedule_index].cur_fd_ptr[i].inode=0;
		process_array[schedule_index].cur_fd_ptr[i].file_position=0;
		process_array[schedule_index].cur_fd_ptr[i].flags=0;
	}
}


