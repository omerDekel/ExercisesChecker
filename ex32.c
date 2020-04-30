//Omer Dekel
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <memory.h>
#include <sys/stat.h>
#include <wait.h>

#define BAD_OUTPUT 1
#define SIMILAR_OUTPUT 2
#define GREAT_JOB 3
#define NO_C_FILE 4
#define TIME_OUT 5
#define COMPILATION_ERROR 6
#define ERROR -1
#define FD_STDERR 2
#define MAX_PATH_SIZE 160
#define EXECUTABLE_FILE "student.out"

/**
 * checks if the value we got is error value (-1) .
 * @param ret the value.
 */
void checkFail(int ret) {
    if (ret < 0) {
        write(FD_STDERR , "Error in system call\n" , sizeof("Error in system call"));
        exit(ERROR);
    }
}

/**
 *from stackoverflow site .
 * @param path string to check if the path of  directory.
 * @return 1 if the file in the path is directory, else 0
 */
int isDirectory(const char *path) {
    struct stat statbuf;
    if (stat(path , &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

/**
 * find c file in the file at the path.
 * @param path of the directory to search c file there.
 * @param input file path of the input to c file.
 * @param correctOutput file path with the correct output .
 * @return if c file found returns the return value from compiling function om this file,
 * else if not found c fille returns  NO_C_FILE_FOUND .
 */
int find(char *path , char *input , char *correctOutput) {
    int foundCfile = 0 , retVal = NO_C_FILE;
    struct dirent *de;
    DIR *dr = opendir(path);
    char newPath[MAX_PATH_SIZE] = {};
    // if couldn't open the directory
    if (dr == NULL) {
        write(FD_STDERR , "Error in system call\n" , sizeof("Error in system call"));
        exit(ERROR);
    } else {
        // go through the directory
        while ((de = readdir(dr)) != NULL) {
            char *deName = de->d_name;
            int length = strlen(de->d_name);
            if (strcmp("." , deName) == 0 || strcmp(".." , deName) == 0) {
                continue;
            }
            // making path for the file in the directory
            strcpy(newPath , path);
            strcat(newPath , "/");
            strcat(newPath , deName);
            if (isDirectory(newPath)) {
                // search recurcively c file in the subdirectory
                retVal = find(newPath , input , correctOutput);
                strcpy(newPath , path);
                // if we allredy find c file return
                if (foundCfile) {
                    return retVal;
                }
                // if it's c file
            } else if ((deName[length - 2] == '.') && deName[length - 1] == 'c') {
                retVal = compiling(correctOutput , input , newPath);
                strcpy(newPath , path);
                foundCfile = 1;
                return retVal;
            }
        }
    }
    closedir(dr);
    // making newPath to the parent directory
    strcpy(newPath , path);
    return retVal;
}

/**
 *check if executable file exists in cwd  .
 * @return 1 if found executable file, else 0.
 */
int compilationSuccess() {
    char cwdPath[MAX_PATH_SIZE];
    // get current working directory path to cwdPath
    getcwd(cwdPath , MAX_PATH_SIZE);
    struct dirent *de;
    DIR *dr = opendir(cwdPath);
    if (dr == NULL) {
        write(FD_STDERR , "Error in system call\n" , sizeof("Error in system call"));
        exit(ERROR);
    } else {
        // going through the directory
        while ((de = readdir(dr)) != NULL) {
            // if it's the executable file we search
            if (!strcmp(de->d_name , EXECUTABLE_FILE)) {
                return 1;
            }
        }
    }
    closedir(dr);
    return 0;
}

/**
 *reading the configuration file and saves in every buffer , line in the file .
 * @param file path of the configuration .
 * @param buf for the first line .
 * @param buf2 for the second line .
 * @param buf3 for the third line in the file .
 */
void readingConfigFile(char *file , char *buf , char *buf2 , char *buf3) {
    char read_buf[1];
    int i = 0;
    int fd = open(file , O_RDONLY);
    // reading first line
    read(fd , read_buf , 1);
    while (read_buf[0] != '\n') {
        buf[i++] = read_buf[0];
        read(fd , read_buf , 1);
    }
    // end of the buf
    buf[i] =  '\0';
    // skip the last char
    read(fd , read_buf , 1);
    i = 0;
    // reading the second line
    while (read_buf[0] != '\n') {
        buf2[i++] = read_buf[0];
        read(fd , read_buf , 1);
    }
    buf2[i] = '\0';
    // skip the '\n' char
    read(fd , read_buf , 1);
    // reading the third line
    i = 0;
    while (read_buf[0] != '\n') {
        buf3[i++] = read_buf[0];
        read(fd , read_buf , 1);
    }
    buf3[i] = '\0';

}

/**
 *compiling the c file with execvp , then send the executable file to run function .
 * @param path of the directory to search c file there.
 * @param input file path of the input to c file.
 * @param correctOutput file path with the correct output .
 * @return COMPILATION_ERROR if the compilation failed , else the return value of run function .
 */
int compiling(char correctOutput[] , char input[] , char path[]) {
    int pid = fork() , res;
    checkFail(pid);
    if (pid == 0) {
        //son process- execute compilation
        char *args[] = {"gcc" , "-o" , EXECUTABLE_FILE , path , NULL};
        unlink(EXECUTABLE_FILE);
        execvp("gcc" , &args[0]);
        exit(COMPILATION_ERROR);
    } else {
        // parent process - waiting to compilition to be executed
        waitpid(pid , NULL , 0);
        if (compilationSuccess()) {
            // if compilation succeed, run the executable
            res = run(correctOutput , input);
        } else {
            res = COMPILATION_ERROR;
        }
        return res;

    }
}

/**
 * running the executable file after compiled , then comparing the output file with the correctOutput file .
 * @param correctOutput file path with the correct output .
 * @param input file path of the input to c file.
 * @return TIME_OUT if it took more then 5 , else the return value of outputCheck
 * function with correctOutput the output file.
 *
 */
int run(char correctOutput[] , char input[]) {
    int stat;
    //making the input file to the stdin
    int fdin = open(input , O_RDONLY);
    checkFail(fdin);
    checkFail(dup2(fdin , STDIN_FILENO));
    // opening new output file
    unlink("out");
    int fdout = open("out" , O_TRUNC | O_CREAT | O_WRONLY , 0644);
    checkFail(fdout);
    // making "out" file to be stdout
    checkFail(dup2(fdout , STDOUT_FILENO));
    int pid = fork();
    checkFail(pid);
    if (pid == 0) {
        // son process - run the executable
        char *args[] = {"./student.out" , NULL};
        execvp(args[0] , args);
        exit(ERROR);
    } else {
        // parent process - wait 5 seconds to executable to be finished,
        // if didn't return timeout.
        sleep(5);
        if (waitpid(pid , &stat , WNOHANG) == 0)
            return TIME_OUT;
    }
    return outputCheck("out" , correctOutput);
}

/**
 *comparing the files with comp.out .
 * @param output the path of the file with the output of the current executable we run.
 * @param correctOutput file path with the correct output .
 * @return the return value of the executable comp.out
 */
int outputCheck(char *output , char *correctOutput) {
    int stat , res;
    char *args[] = {"./comp.out" , output , correctOutput , NULL};
    int pid = fork();
    checkFail(pid);
    if (pid == 0) {
        // son process - running comp.out with output path file and correctOutput path file
        execvp(args[0] , args);
        write(FD_STDERR , "Error in system call\n" , sizeof("Error in system call"));
        exit(ERROR);
    }
    if (pid > 0) {
        // parent process
        waitpid(pid , &stat , 0);
        res = WEXITSTATUS(stat);
        if (res == ERROR) {
            exit(ERROR);
        }
    }
    return res;
}

/***
 * @param val we translate to grade
 * @return string of the grade and the reason for it.
 */
char *retValToGrade(int val) {
    switch (val) {
        case NO_C_FILE:
            return "0,NO_C_FILE\n";
        case COMPILATION_ERROR:
            return "0,COMPILATION_ERROR\n";
        case TIME_OUT:
            return "0,TIME_OUT\n";
        case BAD_OUTPUT:
            return "60, BAD_OUTPUT\n";
        case SIMILAR_OUTPUT:
            return "80, SIMILAR_OUTPUT\n";
        case GREAT_JOB:
            return "100, GREAT_JOB\n";
    }
}

/**
 *write the student name ,the grade and the reason for it.
 * @param fdGradeFile file descriptor of result.csv
 * @param val the value we translate to grade.
 * @param studentName student name string.
 */
void writeToGradesFile(int fdGradeFile , int val , char studentName[]) {
    char studentDetails[MAX_PATH_SIZE] = {0};
    // concatenate student name with his grade and reason for this grade .
    strcat(studentName , ",");
    strcpy(studentDetails , studentName);
    strcat(studentDetails , retValToGrade(val));
    checkFail(write(fdGradeFile , studentDetails , strlen(studentDetails)));

}

/**
 *main .
 * @param argc .
 * @param argv configuration file path.
 * @return 0 if succeed, else -.
 */
int main(int argc , char *argv[]) {
    if (argc < 2) {
        return ERROR;
    }
    int retVal;
    // buf for the first line in the configuration file- students directory path, buf2 for the second
    // line - input file path ,buf3 for the third line - file path with the correct output .
    char buf[MAX_PATH_SIZE] , buf2[MAX_PATH_SIZE] , buf3[MAX_PATH_SIZE];
    readingConfigFile(argv[1] , buf , buf2 , buf3);
    struct dirent *de;
    DIR *dr = opendir(buf);
    char newPath[MAX_PATH_SIZE] = {};
    // creating results file
    int fdResult = open("results.csv" , O_CREAT | O_WRONLY | O_TRUNC , 0644);
    checkFail(fdResult);
    if (dr == NULL) {
        write(FD_STDERR , "Error in system call\n" , sizeof("Error in system call"));
        exit(ERROR);
    } else {
        // going through all the students directory
        while ((de = readdir(dr)) != NULL) {
            char *deName = de->d_name;
            if (strcmp("." , deName) == 0 || strcmp(".." , deName) == 0) {
                continue;
            }
            // creating the path for the subdircetory
            strcpy(newPath , buf);
            strcat(newPath , "/");
            strcat(newPath , deName);
            // looking for c file in the subdirectory, compiling it, running it and compare its output
            // to the correct output .
            retVal = find(newPath , buf2 , buf3);
            if (retVal == ERROR) {
                exit(ERROR);
            } else {
                // writing the line of the students and the details bout his grade to the results file
                writeToGradesFile(fdResult , retVal , deName);
            }
        }
        return 0;
    }
}
