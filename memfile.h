#ifndef _WAL_MEMFILE_H_
#define _WAL_MEMFILE_H_

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>

const int MEM_FILE_LIMIT = 20971520; //20M 

class memfile{
    char *mem;
    int offset;
    int fd;
public:
    memfile():
        mem(nullptr),
        offset(0){
    }

    ~memfile(){
        const int size = offset;
        munmap(mem, size);
    }

    int create(const char *path){
        fd = ::open(path, O_RDWR | O_CREAT , 0664);
        if(fd<0) {
            fprintf(stderr, "open file error: %s\n", strerror(errno));
            close(fd);
            return -1;
        }
        ftruncate(fd, MEM_FILE_LIMIT);

        mem = (char*)::mmap(nullptr, MEM_FILE_LIMIT, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if(mem == MAP_FAILED) {
            fprintf(stderr, "mmap error: %s\n", strerror(errno));
            close(fd);
            return -1;
        }

        close(fd);
        return 0;
    }

    int load(const char *path){
        fd = ::open(path, O_RDWR, 0664);
        if(fd<0) {
            fprintf(stderr, "open file error: %s\n", strerror(errno));
            close(fd);
            return -1;
        }

        struct stat sb;
        if (stat(path, &sb) < 0) {
            fprintf(stderr, "stat %s fail\n", path);
            close(fd);
            return -1;
        }

        if(sb.st_size > MEM_FILE_LIMIT) {
            fprintf(stderr, "size:%d is too large\n", sb.st_size);
            close(fd);
            return -1;
        }

        char *mem = (char *)mmap(NULL, MEM_FILE_LIMIT, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem == (char *)-1) {
            fprintf(stderr, "mmap fail\n");
            close(fd);
            return -1;
        }

        offset = sb.st_size;
        close(fd);
        return 0;
    }

    int size(){
        return offset;
    }

    int write(int index, const char *data, int len){
        memcpy(mem+offset, &index, sizeof(int));
        memcpy(mem+offset+sizeof(int), &len, sizeof(int));
        memcpy(mem+offset+sizeof(int)*2, data, len);
        msync(mem+offset, len+2*sizeof(int)*2, MS_SYNC);
        offset += len+sizeof(int)*2;
        return 0;
    }

    int append(const char *data, int len){
        memcpy(mem, data, len);
        msync(mem, len, MS_SYNC);
        offset += len;
        return 0;
    }

    int read(int index, char **data, int *len){
        int pos =0;
        while(pos<offset){
            if(mem[pos]<index){
                pos += 2*sizeof(int)+mem[pos+sizeof(int)];
            }else if(mem[pos]==index){
                *data = mem+pos+2*sizeof(int);
                *len = ((int*)mem)[pos+sizeof(int)];
                return 0;
            }else{
                break;
            }
        }
        return -1;
    }

    int firstindex(){
        if(offset>0){
            return *(int*)mem;
        }else{
            return -1;
        }
    }

    char *location(int index){
        char *p = nullptr;
        int pos =0;
        while(pos<offset){
            if(mem[pos]<index){
                p = mem+pos;
                pos += 2*sizeof(int)+mem[pos+sizeof(int)];
            }else{
                break;
            }
        }
        return p;
    }

    int firsthalf(int index, char **data, int *len){
        char *dst = location(index);
        if(dst==nullptr){
            return -1;
        }

        *data = mem;
        *len = offset - (dst-mem);
        return 0;
    }

    int secondhalf(int index, char **data, int *len){
        char *dst = location(index);
        if(dst==nullptr){
            return -1;
        }

        *data = dst;
        *len = dst - mem;
        return 0;
    }

    int release(){
        ::close(fd);
        return 0;
    }

};

#endif
