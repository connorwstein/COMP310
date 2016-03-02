#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include "disk_emu.h"
#include "sfs_api.h"
#include <stdio.h>
#include <math.h>


/*Finds a free block in the bitmap. Returns an int representing the block number. Returns -1 if no blocks available.*/
int find_free_block(void){
	int k;
	int bits_per_char=8;
	for(k=0;k<BLOCKSIZE;k++){
		if((free_data_blocks_bitmap[k]&128)==0){
			return k*bits_per_char+1;
		}
		else if((free_data_blocks_bitmap[k]&64)==0){
			return k*bits_per_char+2;
		}
		else if((free_data_blocks_bitmap[k]&32)==0){
			return k*bits_per_char+3;
		}
		else if((free_data_blocks_bitmap[k]&16)==0){
			return k*bits_per_char+4;
		}
		else if((free_data_blocks_bitmap[k]&8)==0){
			return k*bits_per_char+5;
		}
		else if((free_data_blocks_bitmap[k]&4)==0){
			return k*bits_per_char+6;
		}
		else if((free_data_blocks_bitmap[k]&2)==0){
			return k*bits_per_char+7;
		}
		else if((free_data_blocks_bitmap[k]&1)==0){
			return k*bits_per_char+8;
		}
	}
	printf("Error: disk is full, no available blocks.\n");
	return -1;
}
/*Accepts a block number and updates the bitmap to make that block marked as occupied. */
void update_free_block_list(int block_filled){
	int bits_per_char=8;
	int free_data_blocks_bitmap_index=(block_filled-1)/bits_per_char; /* integer division i.e. block 20 is filled, means 20/8=2 so second char in bitmap needs one of its bits changed*/
	char bit_to_be_updated=block_filled%bits_per_char; /*if bit_to_be_updated == 0 then that really means the MSB i.e. bit 7 (indexed at zero) */
	if(bit_to_be_updated==1){
		/*MSB should be set*/
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]|128; /*bit wise or with 1000000 will set the MSB */
	}
	else if(bit_to_be_updated==2){
		/*second MSB should be set*/
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]|64; /*bit wise or with 0100000 will set the second MSB */
	}
	else if(bit_to_be_updated==3){
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]|32; 
	}
	else if(bit_to_be_updated==4){
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=(free_data_blocks_bitmap[free_data_blocks_bitmap_index])|16; 
	}
	else if(bit_to_be_updated==5){
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]|8; 
	}
	else if(bit_to_be_updated==6){	
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]|4; 
	}
	else if(bit_to_be_updated==7){	
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]|2; 
	}
	else if(bit_to_be_updated==0){
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]|1; 
	}	
}
/*Accepts a block number and updates the bitmap to make that block marked as free.*/
void make_block_free(int new_free_block){
	int free_data_blocks_bitmap_index=(new_free_block-1)/8; /* integer division i.e. block 20 is filled, means 20/8=2 so second char in bitmap needs one of its bits changed*/
	char bit_to_be_updated=new_free_block%8; /*if bit_to_be_updated == 0 then that really means the MSB i.e. bit 7 (indexed at zero) */
	
	if(bit_to_be_updated==1){
		/*MSB should be set to 0*/
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]&127; /*bit wise and with 01111111 will set the MSB */
	}
	else if(bit_to_be_updated==2){
		/*second MSB should be set to 0*/
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]&191; /*bit wise and with 10111111 */
	}
	else if(bit_to_be_updated==3){
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]&223; /*bit wise or with 11011111 will set the MSB */
	}
	else if(bit_to_be_updated==4){
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=(free_data_blocks_bitmap[free_data_blocks_bitmap_index])&239; 
	}
	else if(bit_to_be_updated==5){
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]&247; 
	}
	else if(bit_to_be_updated==6){
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]&251; 
	}
	else if(bit_to_be_updated==7){
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]&253; 
	}
	else if(bit_to_be_updated==0){
		free_data_blocks_bitmap[free_data_blocks_bitmap_index]=free_data_blocks_bitmap[free_data_blocks_bitmap_index]&254; 
	}	
}
/*Write the bitmap block to the disk (last block on the disk).*/
void write_bitmap_to_disk(void){
	write_blocks(FILESYSTEMSIZE-1,1,free_data_blocks_bitmap);
}
/*Print the size of the inode and directory entries.*/
void struct_size_test(void){
	fprintf(stderr, "Size of INODE: %lu\n", sizeof(INODE));
	fprintf(stderr, "Size of dir_entry: %lu\n", sizeof(dir_entry));
}
/*Accepts a inode number, and writes the inode at that index in the inode table to disk. Will not affect other inodes in the same block.*/
void write_inode_to_disk(int inode_number){
	int bits_per_char=8;
	int block_of_inode=inode_number/bits_per_char+1; /* integer division to trucate i.e. 10th inode--> 10/8+1=2 i.e. go into the third block on disk (indexed at zero) (second block of inode table) */
	char inode_update_buf[BLOCKSIZE];
	memset(inode_update_buf,0, BLOCKSIZE);
	read_blocks(block_of_inode, 1, inode_update_buf);
	memcpy((void *)inode_update_buf+(inode_number%bits_per_char)*sizeof(INODE),(const void *) &inodes[inode_number], sizeof(INODE));
	write_blocks(block_of_inode, 1, inode_update_buf);	
}
/*Updates block number with blocksize-write_offset bytes from char *data. Returns the number of bytes written or -1 on error. */
int update_block_buffered(int block_number, char *data, int write_offset_bytes, int number_of_bytes_to_write){
	char buf[BLOCKSIZE];
	memset(buf, 0, BLOCKSIZE);
	/*read existing data into block*/
	read_blocks(block_number,1, buf);
	memcpy(buf+write_offset_bytes, data, number_of_bytes_to_write);
	int success=write_blocks(block_number, 1, buf);
	if(success){
		return number_of_bytes_to_write;
	}
	else{
		return -1;
	}
}
/*Copies all the inodes pointers excluding the indirect block value itself into array_of_block_pointers. Returns -1 if empty file 0 otherwise*/
int get_all_pointers_inode(int inode_index, int *array_of_block_pointers, int number_of_pointers){
	if(number_of_pointers==0){
		/*Empty file*/
		return -1;
	}
	int indirect_pointers=0;
	int direct_pointers=0;
	if(number_of_pointers<INODE_POINTERS){
		/*all direct pointers*/
		direct_pointers=number_of_pointers;
		int k;
		for(k=0;k<direct_pointers;k++){
			array_of_block_pointers[k]=inodes[inode_index].pointers[k];
		}
	}
	else{
		direct_pointers=INODE_POINTERS-1;
		indirect_pointers=number_of_pointers-(INODE_POINTERS-1);
		int k;
		for(k=0;k<direct_pointers;k++){
			array_of_block_pointers[k]=inodes[inode_index].pointers[k];
		}
		int indirect_buf[BLOCKSIZE/sizeof(int)];
		memset(indirect_buf,0,BLOCKSIZE);
		read_blocks(inodes[inode_index].pointers[INODE_POINTERS-1], 1, indirect_buf);
		int j=0;
		while(indirect_buf[j]!=0){
			array_of_block_pointers[j+direct_pointers]=indirect_buf[j];
			j++;
		}
		return 0;
	}
}
/*Returns total number of pointers at a given inode (including indirect block). Will be one more than actual assigned blocks with real data if indirect block is used, because indirect block is full of pointers.*/
int get_number_pointers_inode(int inode_index){
	int number_pointers=0;
	int k;
	for(k=0;k<INODE_POINTERS;k++){
		if(inodes[inode_index].pointers[k]!=0){
			number_pointers++;
		}
	}
	if(number_pointers==INODE_POINTERS){
		/*indirect block is used*/
		int indirect_buf[BLOCKSIZE/4];
		memset(indirect_buf,0,BLOCKSIZE);
		read_blocks(inodes[inode_index].pointers[INODE_POINTERS-1], 1, indirect_buf);
		int j=0;
		while(indirect_buf[j]!=0){
			j++;
		}
		if(j==0){
			fprintf(stderr,"Error: pointer to empty indirect block.\n");
		}
		else{
			number_pointers+=j; 
		}	
	}
	return number_pointers;
}
/*Handles writing the indirect block to disk, but not the data from the indirect blocks (assume that has already been written).*/
void update_inode_pointers(int inode_index, int *array_of_block_pointers, int number_of_pointers){
	if(number_of_pointers<=INODE_POINTERS-1){
		/*direct pointers only 1-11*/
		int i;
		for(i=0;i<number_of_pointers-1;i++){
			inodes[inode_index].pointers[i]=array_of_block_pointers[i];
		}
	}
	else{
		/*Indirect is used.*/
		/*Copy all the directs first.*/
		int i;
		for(i=0;i<INODE_POINTERS-1;i++){
			inodes[inode_index].pointers[i]=array_of_block_pointers[i];
		}
		int buf[BLOCKSIZE/sizeof(int)];
		memset(buf,0,BLOCKSIZE);
		if(inodes[inode_index].pointers[INODE_POINTERS-1]!=0){
			/*indirect block was already used*/
			/*need to append to it*/
			read_blocks(inodes[inode_index].pointers[INODE_POINTERS-1],1,buf);
			int initial_number_of_indirects=get_number_pointers_inode(inode_index)-INODE_POINTERS;
			int final_number_of_indirects=number_of_pointers-INODE_POINTERS+1;
			int k;
			for(k=0;k<(final_number_of_indirects-initial_number_of_indirects)-1;k++){
				buf[k+initial_number_of_indirects]=array_of_block_pointers[(INODE_POINTERS-1)+initial_number_of_indirects+k];
			}
			write_blocks(inodes[inode_index].pointers[INODE_POINTERS-1],1,buf);
		}
		else{
			/*Find a free block for the indirect, indirect block not already used.*/
			int free_block=find_free_block();
			inodes[inode_index].pointers[INODE_POINTERS-1]=free_block;
			update_free_block_list(free_block);
			/*Write all the pointers to it*/
			int k;
			for(k=0;k<(number_of_pointers-INODE_POINTERS+1)-1;k++){
				buf[k]=array_of_block_pointers[(INODE_POINTERS-1)+k];
			}	
			write_blocks(free_block,1,buf);
		}
	}
}
/*Handle case of writing to an empty file i.e. file has just been created and has no data blocks allocated yet.*/
int write_to_empty_file(int fileID, const char *buf, int length){
	/*If file is empty all inode pointers will be zero i.e. unallocated and rw_pointer will be at zero*/
	int number_of_bytes_written=0;
	int inode_index=directory_entry_table[fileID].inode_number;
	int number_of_blocks=length/(BLOCKSIZE)+1;
	
	if(number_of_blocks<=(INODE_POINTERS-1)){
		/*Indirect block not required.*/
		int j;
		char write_block_buffer[BLOCKSIZE];
		for(j=0;j<number_of_blocks-1;j++){
			memset(write_block_buffer,0,BLOCKSIZE); /*clear buffer*/
			memcpy(write_block_buffer,buf,BLOCKSIZE); /* copy one blocks worth of buffer*/
			inodes[inode_index].pointers[j]=find_free_block();
			int success=write_blocks(inodes[inode_index].pointers[j],1,write_block_buffer);
			if(success){
				number_of_bytes_written+=BLOCKSIZE;
			}
			update_free_block_list(inodes[inode_index].pointers[j]);
		}
		/*Handle last block, could involve a partial write. */
		char direct_pointer_buf_last[BLOCKSIZE];
		memset(direct_pointer_buf_last,0,BLOCKSIZE);
		memcpy(direct_pointer_buf_last,buf+(number_of_blocks-1)*BLOCKSIZE,length%BLOCKSIZE);
		int last_direct_block=find_free_block();
		update_free_block_list(last_direct_block);
		inodes[inode_index].pointers[number_of_blocks-1]=last_direct_block;
		int success=write_blocks(last_direct_block,1,direct_pointer_buf_last);
		if(success){
			number_of_bytes_written+=length%BLOCKSIZE;
		}
		file_descriptor_table[fileID].rw_pointer+=length;
		inodes[inode_index].size+=number_of_bytes_written;
		write_inode_to_disk(inode_index);
		write_bitmap_to_disk();
		return number_of_bytes_written;
	}
	else{
		/*Indirect block required.*/
		int j;
		char write_block_buffer[BLOCKSIZE];
		
		/*Fill all direct pointers first.*/
		for(j=0;j<INODE_POINTERS-1;j++){
			memset(write_block_buffer,0,BLOCKSIZE); /*clear buffer*/
			memcpy(write_block_buffer,buf,BLOCKSIZE); /* copy one blocks worth of buffer*/
			inodes[inode_index].pointers[j]=find_free_block();
			int success=write_blocks(inodes[inode_index].pointers[j],1,write_block_buffer);
			if(success){number_of_bytes_written+=BLOCKSIZE;}
			update_free_block_list(inodes[inode_index].pointers[j]);
		}
		/*Now fill the indirect.*/
		int number_indirect_pointers=number_of_blocks-(INODE_POINTERS-1);
		int free_block_indirect=find_free_block(); /*Get free block for the array of pointers to other blocks*/
		update_free_block_list(free_block_indirect);
		int array_of_indirect_pointers[number_indirect_pointers];
		int k;
		/*Loop through writing the rest of the buf data to the indirected block*/
		char indirect_pointer_buf[BLOCKSIZE];
		for(k=0;k<number_indirect_pointers-1;k++){
			array_of_indirect_pointers[k]=find_free_block();
			memset(indirect_pointer_buf,0,BLOCKSIZE);
			memcpy(indirect_pointer_buf,buf+(INODE_POINTERS-1)*BLOCKSIZE+k*BLOCKSIZE,BLOCKSIZE);
			int success=write_blocks(array_of_indirect_pointers[k],1,indirect_pointer_buf);
			if(success){number_of_bytes_written+=BLOCKSIZE;}
			update_free_block_list(array_of_indirect_pointers[k]);
		}
		/* Handle last indirected block could be not necessaryily full*/
		int last_indirect_pointers=find_free_block();
		array_of_indirect_pointers[number_indirect_pointers-1]=last_indirect_pointers;
		update_free_block_list(last_indirect_pointers);
		char indirect_pointer_buf_last[BLOCKSIZE];
		memset(indirect_pointer_buf_last,0,BLOCKSIZE);
		memcpy(indirect_pointer_buf_last,buf+(INODE_POINTERS-1)*BLOCKSIZE+(number_indirect_pointers-1)*BLOCKSIZE,length%BLOCKSIZE);
		int success=write_blocks(array_of_indirect_pointers[number_indirect_pointers-1],1,indirect_pointer_buf_last);
		if(success){number_of_bytes_written+=length%BLOCKSIZE;}
		/*Have all the required pointers, need to write the pointers to the indirect block and the indirect block to the inode*/
		/*Write the indirect pointers to the indirect block*/
		char indirect_buf[BLOCKSIZE];
		memset(indirect_buf,0,BLOCKSIZE);
		memcpy(indirect_buf,array_of_indirect_pointers,number_indirect_pointers*sizeof(int));
		write_blocks(free_block_indirect,1,indirect_buf); /* doesnt count as real data written*/
		inodes[inode_index].pointers[INODE_POINTERS-1]=free_block_indirect;
		file_descriptor_table[fileID].rw_pointer+=length;
		inodes[inode_index].size+=number_of_bytes_written;
		write_inode_to_disk(inode_index);
		write_bitmap_to_disk();
		return number_of_bytes_written;
	}	
}
/*Get the size of a file in bytes.*/
int get_file_size(int inode_index){
	
	int number_of_pointers=get_number_pointers_inode(inode_index);
	if(number_of_pointers>NUMBER_INODES-1){
		number_of_pointers--; /*If indirect block is used, remove that because it does not contain data.*/
	}
	int array_of_block_pointers[number_of_pointers]; /* -1 do not include indirect block*/
	get_all_pointers_inode(inode_index,array_of_block_pointers,number_of_pointers);
	int full_block_count=number_of_pointers-1; /*-1 for last block (not necessarily full)*/
	/*go to last block to check size*/
	int last_block_data=0;
	char buf[BLOCKSIZE];
	memset(buf,0,BLOCKSIZE);
	read_blocks(array_of_block_pointers[number_of_pointers-1],1, buf);
	int k=BLOCKSIZE-1;
	while(buf[k]==0){
		k--;
	}
	last_block_data=k+1;
	return full_block_count*BLOCKSIZE+last_block_data;
}

/*Creates the file system. Returns 0 on success, -1 on failure. */
int mksfs(int fresh){

	char buf[BLOCKSIZE];
	memset(buf,0, BLOCKSIZE); /* sets blocksize bytes starting at buf to zero */
	if(fresh){
		
		init_fresh_disk("mysfs", BLOCKSIZE, FILESYSTEMSIZE); /*init_fresh_disk(name, blocksize, number of blocks)*/
		
		
		/*Initialize the superblock*/
		super_block superblock={
			.magic=0xAABB0005,
			.block_size=BLOCKSIZE,
			.file_system_size=FILESYSTEMSIZE,
			.inode_table_length=NUMBER_INODES,
			.root_dir_inode_number=ROOTDIR_INODE_NUMBER
		};
		memcpy((void *)buf,(const void *) &superblock,  sizeof(super_block)); /* copies sizeof(super_block) bytes from super block to buf*/
		
		/*write the superblock to the first block in the disk*/
		int write_success=write_blocks(0,1, buf);
		if(write_success==-1){
			printf("Unsuccessful write\n");
			return -1;
		}
		
		/*Initialize Inode table*/
		memset(inodes, 0, sizeof(inodes));
		/*Set first inode to root directory inode
		which has a pointer to first data block */
		INODE root_dir_inode={
			.mode=0777, /* make all files rwxrwxrwx which is 0777 in octal notation*/
			.link_cnt=1, /* has one link pointing to it (from the super block)*/
			.uid=0, /*Uid of zero means super user */
			.gid=0, /* gid of zero means super user*/
			.size=0,
			.pointers={14,15,16,17,18,0,0,0,0,0,0,0,0} 
		};
		inodes[0]=root_dir_inode;
		write_inode_to_disk(0);

		/*Update the bitmap for all the meta data of the disk.*/
		memset(free_data_blocks_bitmap, 0, sizeof(free_data_blocks_bitmap));/* Initialize free_data_blocks_bitmap to zeros*/
		free_data_blocks_bitmap[0]=255; /*Set superblock and first part of inodes table to occupied.*/
		free_data_blocks_bitmap[1]=255; /* Set rest of inodes table to occupied.*/
		free_data_blocks_bitmap[2]=224; /* Set directory entry table blocks to occupied. */
		free_data_blocks_bitmap[BLOCKSIZE-1]=1; /*Set last bit of bitmap to occupied for the bitmap itself*/	
		write_bitmap_to_disk();

		/*Initialize Directory table*/
		memset(directory_entry_table, 0, sizeof(directory_entry_table));

		/*Initialize all file's RW pointers to 0 and open to 0 (i.e. unopened)*/
		memset(file_descriptor_table, 0, sizeof(file_descriptor_table));
		current_directory_entry_index=-1;
		return 0;
	}
	else{

		init_disk("mysfs", BLOCKSIZE, FILESYSTEMSIZE);
		/* Flush free_data_blocks_bitmap to zeros*/
		memset(free_data_blocks_bitmap,0, sizeof(free_data_blocks_bitmap));
		/*Read in existing bitmap */
		read_blocks(FILESYSTEMSIZE-1,1,free_data_blocks_bitmap);
		/*Read in existing superblock*/
		super_block superblock;
		read_blocks(0,1,&superblock);
		/*Read in existing inode table (13 blocks of inodes)*/
		char inodes_buffer[BLOCKS_INODES_TABLE*BLOCKSIZE];
		memset(inodes_buffer,0,sizeof(inodes_buffer));
		read_blocks(1,BLOCKS_INODES_TABLE,inodes_buffer);
		/* Inodes 1-8 are in inodes_buffer chars 0-511, Inodes 9-16 are in inodes_buffer chars 512-1023 ...*/
		int i;
		for(i=0;i<NUMBER_INODES;i++){
			memcpy((void *)&(inodes[i]),(const void *)(inodes_buffer+i*(BLOCKSIZE/INODES_PER_BLOCK)), BLOCKSIZE/INODES_PER_BLOCK); 
		}
		/*Read in existing directory table*/
		
		INODE root_dir=inodes[superblock.root_dir_inode_number];
		int k;
		char root_dir_table_buf[BLOCKSIZE];
		memset(root_dir_table_buf,0,BLOCKSIZE);
		for(k=0;k<INODE_POINTERS;k++){
			if(root_dir.pointers[k]!=0){
				read_blocks(root_dir.pointers[k],1, root_dir_table_buf); /*first 24 bytes are dir_entry 1, next 24 are dir_entry 2 etc. last few bytes will be unused*/
				int j;
				for(j=0;j<DIR_ENTRIES_PER_BLOCK;j++){
					if((j+DIR_ENTRIES_PER_BLOCK*k)>=MAXFILES){
						break;
					}
					memcpy((void *)&(directory_entry_table[j]), (const void *)(root_dir_table_buf+j*(sizeof(dir_entry))), sizeof(dir_entry));
					printf("Directory entry %d file name: %s\n", j+DIR_ENTRIES_PER_BLOCK*k, directory_entry_table[j].filename);
				}
				memset(root_dir_table_buf,0,BLOCKSIZE);
			}
		}
		/*Initialize all file's RW pointers to 0 and open to 0 (i.e. unopened)*/
		memset(file_descriptor_table, 0, sizeof(file_descriptor_table));
		current_directory_entry_index=-1;
		return 0;
	}
}
/*Opens file and return index of file in the file descriptor table.*/
/*If file does not exist, it creates the file and sets the size to 0*/
/*If the file does exist, open file in append mode i.e. set the file pointer to the end of the file*/
int sfs_fopen(char *name){
	/*Check if file exists*/
	int i;
	for(i=0;i<MAXFILES;i++){
		/*fprintf(stderr,"File %s compared to %s gives %d\n",directory_entry_table[i].filename,name, strncmp(directory_entry_table[i].filename, name, MAXFILENAME));*/
		if(strncmp(directory_entry_table[i].filename, name, MAXFILENAME)==0){ /*Only compares MAXFILENAME chars */
			/*FILE EXISTS*/
			/*check if already open */
			if(file_descriptor_table[i].open==1){
				fprintf(stderr,"File already open\n");
				return i; /* file descriptor is the index of the file in both file_descriptor and directory_entry tables.*/		
			}
			/*Open file*/
			file_descriptor_table[i].open=1;
			file_descriptor_table[i].rw_pointer=inodes[i+1].size;
			fprintf(stderr,"Opening file: %s and set rw_pointer to: %d\n", directory_entry_table[i].filename,file_descriptor_table[i].rw_pointer);
			return i;
		}
	}
	/*FILE DOES NOT EXIST NEED TO CREATE*/
	/*Allocate and initialize an inode
	Write the mapping between inode and file name in root directory
	write this information to disk
	No disk data block allocated file size is set to 0*/
	/*Find a free inode location, initialize inode at it and add it to inodes*/
	int free_inode_index;
	int k;
	/*Skip inode 0 that is the root directory*/
	for(k=1;k<NUMBER_INODES;k++){
		if(inodes[NUMBER_INODES-1].link_cnt==1){
			fprintf(stderr, "Error: disk full, already 100 inodes\n");
			return -1;
		}
		if(inodes[k].link_cnt==0){
			/*free inode*/
			free_inode_index=k;
			break;
		}
	}	
	INODE new_file_inode={
		.mode=0777, /* make all files rwxrwxrwx which is 0777 in octal notation*/
		.link_cnt=1, /* has one link pointing to it (from the root dir file)??*/
		.uid=0, /*Uid of zero means super user */
		.gid=0, /* gid of zero means super user*/
		.size=0,
		.pointers={0,0,0,0,0,0,0,0,0,0,0,0,0} 
	};
	inodes[free_inode_index]=new_file_inode;
	write_inode_to_disk(free_inode_index);

	/*Find a free directory entry location, initialize directory entry and add it to directory entry table*/
	int free_dir_entry_index;
	for (free_dir_entry_index=0;free_dir_entry_index<MAXFILES;free_dir_entry_index++){
		if(directory_entry_table[free_dir_entry_index].inode_number==0){
			break;	
		}
	}
	dir_entry new_file_dir_entry;
	strcpy(new_file_dir_entry.filename, name);
	new_file_dir_entry.inode_number=free_inode_index;
	directory_entry_table[free_dir_entry_index]=new_file_dir_entry;
	/*Write it to the disk.*/
	int block_of_free_dir_entry=free_dir_entry_index/DIR_ENTRIES_PER_BLOCK+BLOCKS_INODES_TABLE+1; /*Add one for the super block.*/
	char dir_entry_buf[BLOCKSIZE];
	memset(dir_entry_buf,0, BLOCKSIZE);
	read_blocks(block_of_free_dir_entry, 1, dir_entry_buf);
	memcpy((void *)dir_entry_buf+(free_dir_entry_index%DIR_ENTRIES_PER_BLOCK)*sizeof(dir_entry),(const void *) &directory_entry_table[free_dir_entry_index], sizeof(dir_entry));
	write_blocks(block_of_free_dir_entry,1,dir_entry_buf);
	file_descriptor_table[free_dir_entry_index].open=1;
	fprintf(stderr,"Creating file: %s\n", directory_entry_table[i].filename);
	return free_dir_entry_index;
} 

/*Closes the given file, note that it still exists in the file descriptor table, but its open property will be set to 0 i.e. closed.*/
int sfs_fclose(int fileID){
	if(file_descriptor_table[fileID].open==1){
		file_descriptor_table[fileID].open=0;
		fprintf(stderr,"Closing file %d open value: %d\n", fileID, file_descriptor_table[fileID].open);
		return 0;
	}
	else{
		fprintf(stderr,"Error: file %d already closed\n", fileID);
		return -1;
	}
} 
/*Writes length bytes of buffered data in buf onto the open file (fileID) starting from current file pointer.*/
int sfs_fwrite(int fileID, const char *buf, int length){

	int number_of_bytes_written=0;
	int inode_index=directory_entry_table[fileID].inode_number;
	/*Check if this write will overflow the file.*/
	if(file_descriptor_table[fileID].rw_pointer+length>(INODE_POINTERS-1+BLOCKSIZE/sizeof(int))*BLOCKSIZE){
		fprintf(stderr, "Error: file will overflow, cannot write that much\n");
		return -1;
	}
	
	int index_rw_pointer=file_descriptor_table[fileID].rw_pointer/BLOCKSIZE+1; /*index of rw pointer i.e. which block is it in currently in terms of the inode pointers.*/
	int rw_pointer_offset=file_descriptor_table[fileID].rw_pointer%BLOCKSIZE; /*rw pointer offset within its curretn block*/

	/*Determine number of blocks to update*/
	int number_of_blocks;
	if(length<BLOCKSIZE-rw_pointer_offset){
		number_of_blocks=1;
	}
	else{
		number_of_blocks=(length-(BLOCKSIZE-rw_pointer_offset))/BLOCKSIZE+1+1; /*first plus 1 is for the first partial block and second plus 1 is for the last partial block*/
	}
	/*If the inode pointers are uninitialized, writing to empty file.*/
	if(inodes[inode_index].pointers[0]==0){
		return write_to_empty_file(fileID,buf,length);
	}

	int initial_number_pointers;
	if(get_number_pointers_inode(inode_index)>INODE_POINTERS-1){
		initial_number_pointers=get_number_pointers_inode(inode_index)-1; /*Exclude the indirect block if its used*/
	}
	else{
		initial_number_pointers=get_number_pointers_inode(inode_index);
	}
	
	int array_of_block_pointers[initial_number_pointers];
	get_all_pointers_inode(inode_index,array_of_block_pointers,initial_number_pointers);
	int l;
	fprintf(stderr,"Initial data blocks of file %d %s [", fileID, directory_entry_table[fileID].filename);
	for(l=0;l<initial_number_pointers;l++){
		fprintf(stderr," %d,",array_of_block_pointers[l]);
	}
	fprintf(stderr,"]\n");	

	int number_unchanged_blocks=index_rw_pointer;
	/*Update all those blocks, while storing the block numbers*/
	int updated_array_of_block_pointers[number_of_blocks+number_unchanged_blocks];
	memset(updated_array_of_block_pointers,0,number_of_blocks*sizeof(int));
	/*Copy the unchanged block numbers*/
	int s;
	for(s=0;s<index_rw_pointer-1;s++){		
		updated_array_of_block_pointers[s]=array_of_block_pointers[s];
	}

	/*At index of rw_pointer block update the first block, then proceeed for the rest of the required blocks*/
	int j;
	for(j=index_rw_pointer-1;j<((number_of_blocks+number_unchanged_blocks)-1);j++){
		/*Allocate additional free blocks when j>total intial blocks*/
		if(j>initial_number_pointers-1){
			/*Need to add free block*/
			if(j==(number_of_blocks+number_unchanged_blocks-2)){
				/*last block need to do a partial write*/
				int free_block=find_free_block();
				updated_array_of_block_pointers[j]=free_block;
				update_free_block_list(free_block);
				if(number_of_blocks==1){
					number_of_bytes_written+=update_block_buffered(updated_array_of_block_pointers[j],(char*)(buf+(BLOCKSIZE-rw_pointer_offset)+BLOCKSIZE*(j-index_rw_pointer-1)),0,length);	
				}
				else{
					if((BLOCKSIZE-rw_pointer_offset)+BLOCKSIZE*(j-index_rw_pointer)<0){
						fprintf(stderr, "Error: writing in last block.\n");
						return -1;
					}
					number_of_bytes_written+=update_block_buffered(updated_array_of_block_pointers[j],(char*)(buf+(BLOCKSIZE-rw_pointer_offset)+BLOCKSIZE*(j-index_rw_pointer)),0,(length-(BLOCKSIZE-rw_pointer_offset))%BLOCKSIZE);			
				}			
				
			}
			else{				
				int free_block=find_free_block();
				updated_array_of_block_pointers[j]=free_block;
				update_free_block_list(free_block);
				number_of_bytes_written+=update_block_buffered(updated_array_of_block_pointers[j],(char*)(buf+(BLOCKSIZE-rw_pointer_offset)+BLOCKSIZE*(j-index_rw_pointer)),0,BLOCKSIZE);	
			}	
		}
		else{
			/*Overwriting existing block*/
			updated_array_of_block_pointers[j]=array_of_block_pointers[j];	/*block number is unchanged*/
			update_free_block_list(array_of_block_pointers[j]);
			if(j==index_rw_pointer-1){
				/*Overwriting first block of write. Will need to use the offset*/
				if(number_of_blocks==1){	
					/*Special case if only updating one block - will not be filling the first block of the write.*/
					number_of_bytes_written+=update_block_buffered(array_of_block_pointers[j],(char*)buf,rw_pointer_offset,length);
				}
				else{
					number_of_bytes_written+=update_block_buffered(array_of_block_pointers[j],(char*)buf,rw_pointer_offset,BLOCKSIZE-rw_pointer_offset);
				}
			}
			else if(j==(number_of_blocks+number_unchanged_blocks-2)){
				/*Last block of the write, may need to do a partial write*/
				if(number_of_blocks==1){
					/*Again special case for one block write.*/
					number_of_bytes_written+=update_block_buffered(updated_array_of_block_pointers[j],(char*)(buf+(BLOCKSIZE-rw_pointer_offset)+BLOCKSIZE*(j-index_rw_pointer-1)),0,length);
				}
				else{
					number_of_bytes_written+=update_block_buffered(updated_array_of_block_pointers[j],(char*)(buf+(BLOCKSIZE-rw_pointer_offset)+BLOCKSIZE*(j-index_rw_pointer-1)),0,(length-(BLOCKSIZE-rw_pointer_offset))%BLOCKSIZE);			
				}
				
			}
			else{
				/*No effect on file growth, overwriting a full existing block.*/
				number_of_bytes_written+=update_block_buffered(array_of_block_pointers[j],(char *)(buf+(BLOCKSIZE-rw_pointer_offset)+BLOCKSIZE*(j-index_rw_pointer-1)),0,BLOCKSIZE);
			}
		}
	}

	/*deal with storing the block pointers, put new update_array_of_pointers into inode*/
	update_inode_pointers(inode_index,updated_array_of_block_pointers,number_of_blocks+number_unchanged_blocks);
	int final_number_pointers=get_number_pointers_inode(inode_index);
	if(final_number_pointers>INODE_POINTERS-1){
		final_number_pointers--; /*Don't include superblock pointer if it is used.*/
	}
	int final_inode_pointers[final_number_pointers];
	get_all_pointers_inode(inode_index,final_inode_pointers,final_number_pointers);
	file_descriptor_table[fileID].rw_pointer+=length;
	inodes[inode_index].size=get_file_size(inode_index);
	int q;
	fprintf(stderr,"Final data blocks of file: %d %s [", fileID, directory_entry_table[fileID].filename);
	for(q=0;q<final_number_pointers;q++){
		fprintf(stderr," %d,",final_inode_pointers[q]);
	}
	fprintf(stderr,"], bytes written: %d,  file size: %d, read/write pointer: %d\n", number_of_bytes_written, inodes[inode_index].size, file_descriptor_table[fileID].rw_pointer);
	write_inode_to_disk(inode_index);
	write_bitmap_to_disk();
	return number_of_bytes_written;
}
	
/*read characters from disk into buf*/
/*starting from current rw pointer assuming*/
int sfs_fread(int fileID, char *buf, int length){
	int inode_index=directory_entry_table[fileID].inode_number;
	if(file_descriptor_table[fileID].open==0){
		fprintf(stderr,"Error: file %d closed cannot read\n", fileID);
		return -1;
	}
	if(length>inodes[inode_index].size){
		fprintf(stderr,"Error: attempting to read more than available in file, file size %d, trying to read %d, will read whole file\n",inodes[inode_index].size,length);
		length=inodes[inode_index].size;
	}
	int bytes_read=0;
	/*get all the pointers of the data blocks*/
	int init_num_pointers=0;
	init_num_pointers=get_number_pointers_inode(inode_index);
	if(init_num_pointers>INODE_POINTERS-1){
		init_num_pointers--;
	}
	int array_of_block_pointers[init_num_pointers];
	get_all_pointers_inode(inode_index,array_of_block_pointers,init_num_pointers);
	/*Determine number of blocks to read*/
	int index_rw_pointer=file_descriptor_table[fileID].rw_pointer/BLOCKSIZE+1; /*assume pointer is within the file*/
	int rw_pointer_offset=file_descriptor_table[fileID].rw_pointer%BLOCKSIZE;
	int number_of_blocks;
	if(length<(BLOCKSIZE-rw_pointer_offset)){
		/*Only reading one block*/
		char read_buf[BLOCKSIZE];
		memset(read_buf,0,BLOCKSIZE);
		read_blocks(array_of_block_pointers[index_rw_pointer-1],1,read_buf);
		memcpy(buf,read_buf+rw_pointer_offset,length);
		file_descriptor_table[fileID].rw_pointer+=length;
		return length;
	}
	else{
		number_of_blocks=(length-(BLOCKSIZE-rw_pointer_offset))/BLOCKSIZE+1+1; /*first plus 1 is for the first partial block and second plus 1 is for the last partial block*/
	}
	int k;
	char read_buf[BLOCKSIZE];
	for(k=index_rw_pointer-1;k<index_rw_pointer+number_of_blocks-1;k++){
		memset(read_buf,0,BLOCKSIZE);
		read_blocks(array_of_block_pointers[k],1,read_buf);
		if(k==index_rw_pointer-1){
			/*first block is special, may be a partial block read*/
			memcpy(buf,read_buf+rw_pointer_offset,BLOCKSIZE-rw_pointer_offset); /*only read the remaining data*/
			bytes_read+=(BLOCKSIZE-rw_pointer_offset);
		}
		else if(k==index_rw_pointer+number_of_blocks-2){
			/*last block is special, may be a partial block read*/
			memcpy(buf+bytes_read, read_buf, (length-(BLOCKSIZE-rw_pointer_offset))%BLOCKSIZE);
			bytes_read+=(length-(BLOCKSIZE-rw_pointer_offset))%BLOCKSIZE;
		}
		else{
			/*full block read*/
			memcpy(buf+bytes_read, read_buf, BLOCKSIZE);
			bytes_read+=BLOCKSIZE;
		}
		
	}
	file_descriptor_table[fileID].rw_pointer+=bytes_read;
	fprintf(stderr,"Successfuly read %d bytes from file %d %s. Read write pointer updated to %d\n", bytes_read,fileID,directory_entry_table[fileID].filename, file_descriptor_table[fileID].rw_pointer);
	return bytes_read;
}

/*Seek to offset*/
int sfs_fseek(int fileID, int offset){
	if(offset<0){
		fprintf(stderr,"Error: offset for seek must be greater than zero\n");
		return -1;
	}
	file_descriptor_table[fileID].rw_pointer=offset;
	fprintf(stderr,"Setting rw pointer of file %d to %d\n",fileID, offset);
	return 0;
}
/*Remove a file from the file system*/
int sfs_remove(char *file){
/*remove file from directory entry table and releases file allocation table entries and data blocks 
used by the file*/
	int i;
	for(i=0;i<MAXFILES;i++){
		if(strncmp(directory_entry_table[i].filename, file, MAXFILENAME)==0){
			/*Found the file to be deleted*/		
			/*Clear its data blocks*/
			int inode_index=directory_entry_table[i].inode_number;
			int init_num_pointers=get_number_pointers_inode(inode_index);
			int indirect_block=0;
			if(init_num_pointers>=INODE_POINTERS){
				/*has indirect pointers, remove pointer to block of pointers*/
				/*Clear the indirect block itself then remove from init_count*/
				indirect_block=inodes[inode_index].pointers[INODE_POINTERS-1];
				init_num_pointers--;
			}
			int array_of_block_pointers[init_num_pointers];
			get_all_pointers_inode(inode_index, array_of_block_pointers,init_num_pointers);
			/*Now we can remove indirect block*/
			if(indirect_block){
				char empty_buf[BLOCKSIZE];
				memset(empty_buf,0,BLOCKSIZE);
				write_blocks(indirect_block,1,empty_buf);
				make_block_free(indirect_block);
			}
			fprintf(stderr,"removed file pointers [");
			int f;
			for(f=0;f<init_num_pointers;f++){
				fprintf(stderr," %d,",array_of_block_pointers[f]);
			}
			fprintf(stderr,"]\n");
			int k;
			char cleared_block_buf[BLOCKSIZE];
			for(k=0;k<init_num_pointers;k++){
				memset(cleared_block_buf,0,BLOCKSIZE);
				write_blocks(array_of_block_pointers[k],1,cleared_block_buf);
				make_block_free(array_of_block_pointers[k]);
			}
			/*update the free bitmap*/
			write_bitmap_to_disk();
			/*Write an empty inode to its place in disk and memory*/
			INODE empty_inode={
				.mode=0, /* make all files rwxrwxrwx which is 0777 in octal notation*/
				.link_cnt=0, /* has one link pointing to it (from the root dir file)??*/
				.uid=0, /*Uid of zero means super user */
				.gid=0, /* gid of zero means super user*/
				.size=0,
				.pointers={0,0,0,0,0,0,0,0,0,0,0,0,0} 
			};
			inodes[inode_index]=empty_inode;
			write_inode_to_disk(inode_index);

			/*Set all directory table infrmation to 0*/
			memset(directory_entry_table[i].filename,0,MAXFILENAME);
			directory_entry_table[i].inode_number=0;
			int dir_entry_block=i/DIR_ENTRIES_PER_BLOCK+INODES_PER_BLOCK+1;
			/*Write this zeroed directory table information to the disk*/
			char dir_entry_buf[BLOCKSIZE];
			memset(dir_entry_buf,0, BLOCKSIZE);
			read_blocks(dir_entry_block, 1, dir_entry_buf);
			memcpy((void *)dir_entry_buf+(i%21)*sizeof(dir_entry),(const void *) &directory_entry_table[i], sizeof(dir_entry));
			write_blocks(dir_entry_block,1,dir_entry_buf);

			/*Clear file descripters*/
			file_descriptor_table[inode_index-1].rw_pointer=0;
			file_descriptor_table[inode_index-1].open=0;
			return 0;
		}
	}
	fprintf(stderr,"Error: file %s does not exist, cannot remove\n",file);
	return -1;
}

/*gets the name of next file in directory */
int sfs_get_next_filename(char* filename){
/*copies name of the next file in the directory into filename and returns non-zero if there is a new file*/
/*need to remember the current position in the directory at each call*/
/*Assume looping through whole directory so just start at the beginning of the directory*/
	while(1) {
		current_directory_entry_index++;	
		if (current_directory_entry_index == MAXFILES) {
			current_directory_entry_index = -1;		
			return 0;
		}
		if (directory_entry_table[current_directory_entry_index].inode_number != 0) {
			strcpy(filename, directory_entry_table[current_directory_entry_index].filename);
			return 1;
		}
	}
}

/*get the size of the given file */
int sfs_GetFileSize(const char* path){
/*returns the size of the given file*/
	/*extract the filename from the path string*/
	char *filename=basename((char *)path);
	int file_inode_number;
	int k;
	for(k=0;k<MAXFILES;k++){
		if(strcmp(directory_entry_table[k].filename,filename)==0){
			file_inode_number=directory_entry_table[k].inode_number;
			break;
		}
		if(k==MAXFILES-1){
			return -1;
		}
	}
	return get_file_size(file_inode_number); 
}

