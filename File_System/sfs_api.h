#define MAXFILENAME 20
#define MAXFILEEXTENTSION 3
#define MAXFILES 99 /*100 files, 1 of which is actually the directory file.*/
#define BLOCKSIZE 512
#define FILESYSTEMSIZE 4096 /*Total number of blocks, corresponds to one bitmap block.*/
#define ROOTDIR_INODE_NUMBER 0
#define INODE_POINTERS 13
#define NUMBER_INODES 100
#define INODES_PER_BLOCK 8 /*Number of inodes per block in the inode table on disk. Each inode is 64 bytes, can fit exactly 8 per block. Inode table occupies blocks 1-13.*/
#define DIR_ENTRIES_PER_BLOCK 21 /*Number of directory entries per block in the directory entry table on disk. Each directory entry is 24 bytes, can fit 21 per block. 5 blocks required for all 99 files.*/
#define BLOCKS_INODES_TABLE 13
/* Super block */
typedef struct{
	int magic; 
	int block_size;
	int file_system_size;
	int inode_table_length;
	int root_dir_inode_number;
}super_block;

/*Inode (64 bytes)*/
typedef struct{
	int mode; 
	char link_cnt; 
	char uid;
	char gid;
	int size;
	int pointers[INODE_POINTERS]; 
} INODE;

/* Directory entry (can fit 21 directory entries per block)*/
typedef struct{
	char filename[MAXFILENAME];
	int inode_number;
} dir_entry;

/*File descriptor tables is parallel table of structs to directory entry table, but exists only in memory*/
typedef struct{
	int rw_pointer;
	char open;
}FD;

INODE inodes[NUMBER_INODES];
dir_entry directory_entry_table[MAXFILES];
FD file_descriptor_table[MAXFILES];
unsigned char free_data_blocks_bitmap[BLOCKSIZE]; /*single byte array of one block size, each bit indicates whether a particular block is free or not*/
int current_directory_entry_index;

int mksfs(int fresh); /*creates the file system*/
int sfs_fopen(char *name); /*opens the given file*/
int sfs_fclose(int fileID);  /*closes the given file*/
int sfs_fwrite(int fileID, const char *buf, int length); /*write buf characters into disk*/ 
int sfs_fread(int fileID, char *buf, int length); /*read characters from disk into buf*/
int sfs_fseek(int fileID, int offset); /*seek to the location from beginning*/
int sfs_remove(char *file); /*remove a file from the file system*/
int sfs_get_next_filename(char* filename);/*gets the name of next file in directory */
int sfs_GetFileSize(const char* path);/*get the size of the given file */

