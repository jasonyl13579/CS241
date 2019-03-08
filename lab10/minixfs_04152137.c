/**
* Finding Filesystems Lab
* CS 241 - Spring 2018
*/

#include "minixfs.h"
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MIN(a,b) (((a)<(b))?(a):(b))
int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
	inode* node;
	if((node = get_inode(fs, path)) == NULL){
		errno = ENOENT;
		return -1;
	}
	//printf("%d\n",node->mode & 0x01FF);
	new_permissions = new_permissions & 0x01FF;
	//printf("%d\n",new_permissions);
	node->mode = node->mode & 0xFE00;
	node->mode = node->mode | new_permissions;
	//printf("%d\n",node->mode & 0x01FF);
	clock_gettime(CLOCK_REALTIME, &node->ctim);
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode* node;
	if((node = get_inode(fs, path)) == NULL){
		errno = ENOENT;
		return -1;
	}
	if (owner != (uid_t)-1) node->uid = owner;
	if (group != (gid_t)-1) node->gid = group;
	clock_gettime(CLOCK_REALTIME, &node->ctim);
    return 0;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
	puts("write");
	inode* node;
	if((node = get_inode(fs, path)) == NULL){
		node = minixfs_touch(fs, path);
	}
	//minixfs_min_blockcount
    return -1;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    // 'ere be treasure!
	//puts("read");
	inode* node;
	if((node = get_inode(fs, path)) == NULL){
		errno = ENOENT;
		return -1;
	}
	struct stat state;
	char* temp_path = strdup(path);
	minixfs_stat(fs, temp_path, &state);
	off_t size_of_file = state.st_size;
	if (*off >= size_of_file) return 0;
	off_t count_until_end = size_of_file - *off;
	off_t size = MIN((off_t)count, count_until_end);
	int blocks_to_read = state.st_blocks;
	int current_block = 0;
	off_t bytes_read = 0;
	off_t remain_to_read = size;
	off_t offset = *off;
	// += offset / sizeof(data_block);
	while (current_block < NUM_DIRECT_INODES){
		printf("idr_cur:%d\n",current_block);
		printf("remain:%ld\n",remain_to_read);
		data_block_number x = node->direct[current_block];
		printf("dir:%d\n",x);
		char* data = (char*)(fs->data_root + x);
		if (current_block == 0)
			data = data + *off;
		off_t num = MIN((off_t)sizeof(data_block), remain_to_read);
		//memcpy(buf + bytes_read, data, num);
		bytes_read += num;
		remain_to_read -= num;
		current_block ++;
		if (remain_to_read == 0) break;	
	}
	if (blocks_to_read != current_block){
		data_block_number x = node->indirect;
		printf("indir:%d\n",x);
		char* indir_data = (char*)(fs->data_root + x);
		printf("cur:%d\n",current_block);
		while (current_block < blocks_to_read){
			
			data_block_number y = *(indir_data + (current_block - NUM_DIRECT_INODES) * sizeof(data_block_number));
			char* data  = (char*)(fs->data_root + y);
			printf("%c\n",*data);
			off_t num = MIN((off_t)sizeof(data_block), remain_to_read);
			//memcpy(buf + bytes_read, data, num);
			bytes_read += num;
			remain_to_read -= num;
			current_block ++;
			if (remain_to_read == 0) break;	
		}
	}
	/*data_block_number x = node->direct[0];
	data_block_number y = node->direct[1];
	printf("%d,%d\n",x,y);
	char* data = (char*)(fs->data_root + x);
	//printf("size:%ld\n",*off);
	data = data + *off;
	memcpy(buf, data, size);*/
	*off += size;
	clock_gettime(CLOCK_REALTIME, &node->atim);
	free(temp_path);
    return size;
}
