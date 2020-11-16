#include "call.h"
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <time.h>

#include "inode.h"
#include "superblock.h"
const char *HD = "HD";
int match_filename_inode(int, int, char*);
inode* read_inode(int,int);


int open_t(char *pathname)
{
	int inode_number;
	int inter_inode_number = 0;
	char* current;
	int fd = open("./HD",O_RDWR);
	if(fd<0){
		printf("Error: open()\n");
		return -1;
	}

	
	current = strtok(pathname,"/");
	//printf("First dir:%s\n",current);
	if(current==NULL) return 0;
	while(current!=NULL){
	    //printf("Pharase 1\n");
	    inter_inode_number=match_filename_inode(fd,inter_inode_number,current);
            //printf("Inter_inode_number=%d\n",inter_inode_number);
            if(inter_inode_number==-1){
		    printf("Can't find the file or directory %s\n",current);
		    return -1;
	    }

            		
	    current = strtok(NULL,"/");
	}
	inode_number = inter_inode_number;
	//printf("Inode_number for final return = %d\n",inode_number);
	return inode_number;
}

int read_t(int inode_number, int offset, void *buf, int count)
{
	int read_bytes,fd;
	int block_number,currpos;
	int a,a_prime,b,b_prime;
	a = offset/4096;
	b = offset%4096;
	a_prime = (offset+count-1)/4096;
	b_prime = (offset+count-1)%4096;
        fd = open("./HD",O_RDWR);
        
	read_bytes = 0;
        inode* ip;
        ip = read_inode(fd,inode_number);
        //printf("The block information:\nblock number:%d\n direct_block:%d %d\n indirect_blk:%d\n",ip->blk_num,ip->direct_blk[0],ip->direct_blk[1],ip->indirect_blk);
	DIR_NODE* p_block = (DIR_NODE*)malloc(BLOCK_SIZE);

         printf("a:%d\n",a);
	 while(count>0){
            if(a==0){
		strcpy(p_block[0].dir,"");
		printf("count before:%d a before:%d b before:%d\n",count,a,b);
	        block_number = ip->direct_blk[0];

                currpos=lseek(fd, DATA_OFFSET + block_number * BLOCK_SIZE+b, SEEK_SET);
		if(count<=4096-b){
		    read_bytes+=read(fd,p_block,count);
		    count=0;
		}
		else{

		    read_bytes+=read(fd,p_block,4096-b);
		    count-=(4096-b);
		}
		strcat((char*)buf,p_block[0].dir);
		if(count>0){
		a++;
		}
		b=0;

		printf("First direct block\n");
		printf("count later:%d a later:%d b later:%d\n",count,a,b);
		//printf("content read:%s\n",p_block[0].dir);
	    }
	    if(a==1){
		block_number = ip->direct_blk[1];
          //      printf("content before empty:%s\n",p_block[0].dir);
		strcpy(p_block[0].dir,"");

	//	printf("content after empty:%s\n",p_block[0].dir);
                currpos=lseek(fd, DATA_OFFSET + block_number * BLOCK_SIZE+b, SEEK_SET);
		//p_block[0].dir=" "
		//;
		printf("----------count before:%d a before:%d b before:%d------------\n",count,a,b);
	//	printf("content before read:%s\n**********************",p_block[0].dir);
                if(count<=4096-b){

		    read_bytes+=read(fd,p_block,count);
                    count=0;
                }
                else{
                        read_bytes+=read(fd,p_block,4096-b);

			count-=(4096-b);
                }
		strcat(buf,p_block[0].dir);

                if(count>0){
                a++;
                }
                b=0;
                 printf("second direct block\n");
                printf("count later:%d a later:%d b later:%d\n",count,a,b);
                //printf("content read:%s\n",p_block[0].dir);
		
	    }

	    if(a>=2){
                printf("INdirect blocks\n");
		printf("Block_number in total:%d\n",ip->blk_num);
		int current_blk_num = ip->blk_num-2;
                block_number = ip->indirect_blk;

		DIR_NODE* p_block = (DIR_NODE*)malloc(BLOCK_SIZE);
                DIR_NODE* indirect_blk_dir = (DIR_NODE*)malloc(BLOCK_SIZE);
 
                currpos=lseek(fd, DATA_OFFSET + block_number * BLOCK_SIZE, SEEK_SET);
                read(fd,indirect_blk_dir,4096);
		for(int i=a-2; i<current_blk_num;i++){
                    printf("blk num:%d\n",indirect_blk_dir[i].inode_number);
                    if(indirect_blk_dir[i].inode_number==0){
                        break;
		    }
		    
		    if(count<=4096-b){
                                             
		            read_bytes+=read(fd,p_block,count);
			    count=0;
                }
                    else{
                            read_bytes+=read(fd,p_block,4096-b);

			    count-=(4096-b);
                }
                printf("count later:%d \n",count);
		strcat(buf,p_block[0].dir);

                if(count==0) break;
                }

		count=0;
		free(p_block);
	    }

	}
	/*
        int block_number = ip->direct_blk[0];
        int currpos=lseek(fd, DATA_OFFSET + block_number * BLOCK_SIZE, SEEK_SET);
        read(fd, p_block, BLOCK_SIZE);
	printf("p_block info: %s\n",p_block[2].dir);
        */

	 //printf("Content in buf:%s\n, sizeof buf:%d\n",(char*)buf,strlen(buf));
	return read_bytes; 
}

int match_filename_inode(int fd, int inter_inode_number, char* filename){
    inode* ip;
    printf("INode number:%d\n",inter_inode_number);
    ip = read_inode(fd,inter_inode_number);
    if(ip -> file_type !=DIR){
        printf("Wrong path!\n");
	return -1;
    }

    DIR_NODE* p_block = (DIR_NODE*)malloc(BLOCK_SIZE);

    int block_number = ip->direct_blk[0];
    int currpos=lseek(fd, DATA_OFFSET + block_number * BLOCK_SIZE, SEEK_SET);
    read(fd, p_block, BLOCK_SIZE);
    
    int file_idx = 0;
    int target_inode_number = -1;
    //printf("target_filename:%s\n",filename);
    for(file_idx=0;file_idx<ip->file_num;file_idx++){
	//printf("Dir_name:%s,whether_equal=%d\n",p_block[file_idx].dir,strcmp(p_block[file_idx].dir,filename));
        if(strcmp(p_block[file_idx].dir,filename)==0){
	    target_inode_number = p_block[file_idx].inode_number;
	    free(p_block);
	    return target_inode_number;
	}
    }
         free(p_block);
         return -1;


}

inode* read_inode(int fd, int inode_number){
	inode* ip = malloc(sizeof(inode));
	int currpos=lseek(fd, INODE_OFFSET + inode_number * sizeof(inode), SEEK_SET);
	if(currpos<0){
		printf("Error: lseek()\n");
		return NULL;
	}

	//read inode from disk
	int ret = read(fd, ip, sizeof(inode));
	if(ret != sizeof (inode) ){
		printf("Error: read()\n");
		return NULL;
	}
	return ip;
}


// you are allowed to create any auxiliary functions that can help your implementation. But only “open_t()” and "read_t()" are allowed to call these auxiliary functions.
