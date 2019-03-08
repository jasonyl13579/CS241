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
	//puts("write");
	inode* node;
	if((node = get_inode(fs, path)) == NULL){
		node = minixfs_touch(fs, path);
		//printf("touch\n");
	}
	off_t offset = *off;
	
	//int blocks_to_write = count / sizeof(data_block) + 1;
	int blocks_to_write = (count + offset) / sizeof(data_block) + 1;
	//printf("block:%d\n",blocks_to_write);
	if ( (size_t) blocks_to_write > NUM_DIRECT_INODES + NUM_INDIRECT_INODES){
		errno = ENOSPC;
		return -1;
	}
	if (minixfs_min_blockcount(fs, path, blocks_to_write) == -1 ){
		errno = ENOSPC;
		return -1;
	}
	int current_block = 0;
	off_t bytes_write = 0;
	off_t remain_to_write = (off_t)count;
	// add offset
	current_block += offset / sizeof(data_block);
	//printf("cur:%d\n",current_block);
	offset = offset % sizeof(data_block);
	int start = 1;

	//direct block
	while (current_block < NUM_DIRECT_INODES){
		data_block_number x = node->direct[current_block];
		//printf("x:%d\n",x);
		//printf("current_block:%d\n",current_block);
		//printf("remain_to_write:%ld\n",remain_to_write);
		char* data = (char*)(fs->data_root + x);
		off_t num = MIN((off_t)sizeof(data_block) - offset, remain_to_write);
		if (start == 1){
			data = data + offset;
			start = 0;
			offset = 0;
		}		
		memcpy(data, buf + bytes_write, num);
		// printf("%c\n",*data);
		bytes_write += num;
		remain_to_write -= num;
		current_block ++;
		if (remain_to_write == 0) break;	
	}
	//indirect block
	if (remain_to_write != 0){
		data_block_number x = node->indirect;
		//printf("indir:%d\n",x);
		char* indir_data = (char*)(fs->data_root + x);
		//printf("cur:%d\n",current_block);
		while ((size_t)current_block < NUM_DIRECT_INODES + NUM_INDIRECT_INODES){			
			data_block_number y = *(indir_data + (current_block - NUM_DIRECT_INODES) * sizeof(data_block_number));
			char* data  = (char*)(fs->data_root + y);
			//printf("indir:%d\n",y);
			off_t num = MIN((off_t)sizeof(data_block) - offset, remain_to_write);
			if (start == 1){
				data = data + offset;
				start = 0;
				offset = 0;
			}	
			
			memcpy(data, buf + bytes_write, num);
			bytes_write += num;
			remain_to_write -= num;
			current_block ++;
			if (remain_to_write == 0) break;	
		}
	}

	*off += count;
	node->size = *off;
	clock_gettime(CLOCK_REALTIME, &node->atim);
	clock_gettime(CLOCK_REALTIME, &node->mtim);
    return count;
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
	//printf("size_of_file:%ld\n",size_of_file);
	if (*off >= size_of_file) return 0;
	off_t count_until_end = size_of_file - *off;
	off_t size = MIN((off_t)count, count_until_end);
	int blocks_to_read = state.st_blocks;
	int current_block = 0;
	off_t bytes_read = 0;
	off_t remain_to_read = size;
	// add offset
	off_t offset = *off;
	current_block += offset / sizeof(data_block);
	offset = offset % sizeof(data_block);
	
	int start = 1;
	//direct block
	while (current_block < NUM_DIRECT_INODES){
		//printf("idr_cur:%d\n",current_block);
		//printf("remain:%ld\n",remain_to_read);
		data_block_number x = node->direct[current_block];
		//printf("dir:%d\n",x);
		char* data = (char*)(fs->data_root + x);
		off_t num = MIN((off_t)sizeof(data_block) - offset, remain_to_read);
		if (start == 1){
			data = data + offset;
			start = 0;
			offset = 0;
		}		
		memcpy(buf + bytes_read, data, num);
		bytes_read += num;
		remain_to_read -= num;
		current_block ++;
		if (remain_to_read == 0) break;	
	}
	//indirect block
	if (blocks_to_read != current_block && remain_to_read != 0){
		data_block_number x = node->indirect;
		//printf("indir:%d\n",x);
		char* indir_data = (char*)(fs->data_root + x);
		//printf("cur:%d\n",current_block);
		while (current_block < blocks_to_read){			
			data_block_number y = *(indir_data + (current_block - NUM_DIRECT_INODES) * sizeof(data_block_number));
			char* data  = (char*)(fs->data_root + y);
			off_t num = MIN((off_t)sizeof(data_block) - offset, remain_to_read);
			if (start == 1){
				data = data + offset;
				start = 0;
				offset = 0;
			}
			
			memcpy(buf + bytes_read, data, num);
			bytes_read += num;
			remain_to_read -= num;
			current_block ++;
			if (remain_to_read == 0) break;	
		}
	}

	*off += size;
	clock_gettime(CLOCK_REALTIME, &node->atim);
	free(temp_path);
    return size;
}
