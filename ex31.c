#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define BUF_SIZE 1024
#define SIZE 10
#define FD_STDERR 2
#define EQUAL 3
#define SIMILAR 2
#define DIFFERENT 1
#define ERROR -1
/**
 *my_read
 * reading file into a buffer .
 * @param fd file descriptor .
 * @param buf buffer .
 * @param count number of bytes to read .
 * @return how bytes we read or -1 if failed .
 */
int my_read(int fd , char *buf , size_t count) {
    static char read_buf[BUF_SIZE];
    static int read_offset = BUF_SIZE;
    static int read_max = BUF_SIZE;
    int i;
    for (i = 0; i < count; i++) {
        if (read_offset == read_max) {
            read_offset = 0;
            read_max = read(fd , read_buf , BUF_SIZE);
            if (read_max == -1) {
                write(FD_STDERR , "Error in system call\n" , sizeof("Error in system call"));
                printf("error in read\n");
                return ERROR;
            }
            if (read_max == 0) {
                return i;
            }
        }
        buf[i] = read_buf[read_offset++];
    }
    return i;
}

/**
 *my_read2.
 * reading file into a buffer .
 * @param fd file descriptor .
 * @param buf buffer .
 * @param count number of bytes to read .
 * @return how bytes we read or -1 if failed .
 */
int my_read2(int fd , char *buf , size_t count) {
    static char read_buf[BUF_SIZE];
    static int read_offset = BUF_SIZE;
    static int read_max = BUF_SIZE;
    int i;
    for (i = 0; i < count; i++) {
        if (read_offset == read_max) {
            read_offset = 0;
            read_max = read(fd , read_buf , BUF_SIZE);
            if (read_max == -1) {
                write(FD_STDERR , "Error in system call\n" , sizeof("Error in system call"));
                printf("error in read\n");
                return ERROR;
            }
            if (read_max == 0) {
                return i;
            }
        }
        buf[i] = read_buf[read_offset++];
    }
    return i;
}

/**
 *checkSimilar
 * check if the files can be similar. If buf1[i] and buf2[j] answering the conditions of similar files.
 * @param buf1 buffer of the first file
 * @param buf2 buffer of the second file .
 * @param i current index of buf1 .
 * @param j current index of buf2.
 * @return 1 if the files cant be similar or 2 if they do.
 */
int checkSimilar(char buf1[] , char buf2[] , int *i , int *j) {
    if (tolower(buf1[*i]) == tolower(buf2[*j])) {
        return SIMILAR;
    } else {
        if (buf1[*i] == ' ' || buf1[*i] == '\n') {
            //if there's space we make the next index in the other buf to be the current index.
            (*j)--;
            return SIMILAR;
        } else if (buf2[*j] == ' ' || buf2[*j] == '\n') {
            (*i)--;
            return SIMILAR;
        } else {
            return DIFFERENT;
        }
    }

}
/**
 * closeFile .
 * @param fd .
 */
void closeFile(int fd){
    if (close(fd) == -1) {
        printf("couldn't close the file");
        write(FD_STDERR , "Error in system call\n" , sizeof("Error in system call"));
    }
}

/**
 * main.
 * @param argc .
 * @param argv paths of two files .
 * @return -1 if the program failed, 1 if the files are different, 2 if similar,
 * 3 if equal.
 */
int main(int argc , char *argv[]) {
    int result = EQUAL;
    char buf1[SIZE];
    char buf2[SIZE];
    int i = 0 , j = 0 , in1 , in2 , count1 , count2;
    // not enough arguments
    if (argc < 3) {
        return ERROR;
    }
    in1 = open(argv[1] , O_RDONLY);
    in2 = open(argv[2] , O_RDONLY);
    //failed to open the files
    if (in1 == -1 || in2 == -1) {
        write(FD_STDERR , "Error in system call\n" , sizeof("Error in system call"));
        return ERROR;
    }
    // while we reading the first file and the second and we didnt get to eof.
    while ((count1 = my_read(in1 , buf1 , SIZE)) > 0 && (count2 = my_read2(in2 , buf2 , SIZE)) > 0) {
        i = 0;
        j = 0;
        // if they cant be equal we check if they might be similar
        if (buf1[i] != buf2[i]) {
            result = checkSimilar(buf1 , buf2 , &i , &j);
            if (result == DIFFERENT || result == ERROR) {
                closeFile(in1);
                closeFile(in2);
                return result;
            }
        }
        i++;
        j++;
        // check the rest bytes until we get to count .
        while (i < count1 && j < count2) {
            if ((buf1[i] != buf2[j])) {
                result = checkSimilar(buf1 , buf2 , &i , &j);
                if (result == DIFFERENT || result == ERROR) {
                    closeFile(in1);
                    closeFile(in2);
                    return result;
                }
            }
            i++;
            j++;
        }
    }
    closeFile(in1);
    closeFile(in2);
    // if one of the files is not empty it means the files aren't really equal
    // but one of the files is empty and the other one isn't
    if((result == 3) && (count1 != 0 || count2 != 0)) {
        return DIFFERENT;
    }
    return result;
}
