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
        int fd = ::open(path, O_RDWR | O_CREAT , 0664);
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
        int fd = ::open(path, O_RDWR, 0664);
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

    int write(const char *data, const int len){
        memcpy(mem+offset, data, len);
        //msync(mem+offset, len, MS_SYNC);
        offset += len;
        return len;
    }

    int read(int pos, char* data, int len){
        if(pos>=offset){
            return -1;
        }
        int size = std::min(offset-pos, len);
        memcpy(data, mem+pos, size);
        return size;
    }
};

#endif
