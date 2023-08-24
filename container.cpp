#include <iostream>
#include <sched.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/types.h>


#define STACK 8192
#define INITIAL_FLAGS (CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD | CLONE_NEWNS)

// Error messages
#define SYS_CALL_ERR "System error: %s\n"

#define PIDS_PATH "/sys/fs/cgroup/pids"
#define MEM_ALLOC_FAILURE_MSG "Failed in memory allocation"
#define MKDIR_FAILURE_MSG "Failed to create a directory"
#define CHDIR_FAILURE_MSG "Failed to change directory"
#define SET_HOSTNAME_FAILURE_MSG "Failed to set hostname."
#define CHROOT_FAILURE_MSG "Failed to change root of child process."
#define MOUNT_FAILURE_MSG "Failed to mount directory"
#define EXECVP_FAILURE_MSG "Failed to run new file name"
#define OPEN_FILE_FAILURE_MSG "Failed to open file"
#define CLOSE_FILE_FAILURE_MSG "Failed to close file"
#define UMOUNT_FAILURE_MSG "Failed to unmount fs."
#define DELETE_FILE_FAILURE_MSG "Failed in deleting a file."
#define RMDIR_FAILURE_MSG "Failed in directory deletion."
#define CLONE_FAILURE_MSG "Failed in clone"
#define WAIT_FAILURE_MSG "Failed in wait"

enum ArgIndex {
    NEW_HOSTNAME = 1,
    NEW_FS_DIR,
    NUM_PS,
    PATH_TO_PROG,
    ARGS_FOR_PROG
};

void setNumProcLmt(const char *numProc);

/**
 * Copies argv
 * @return the new copy
 */
char **cpy_argv(size_t argc, char **argv) {
    char **argv_cpy = (char **) malloc(sizeof(char *) * argc);
    if (argv_cpy == NULL) {
        printf(SYS_CALL_ERR, MEM_ALLOC_FAILURE_MSG);
        exit(1);
    }

    // fill argv_cpy with elements from argv starting at position PATH_TO_PROG
    for (size_t i = 0; i < argc - 1; i++) {
        argv_cpy[i] = (char *) malloc(sizeof(char) * (strlen(argv[i + NEW_HOSTNAME]) + 1));
        if (argv_cpy[i] == NULL) {
            fprintf(stderr, SYS_CALL_ERR, MEM_ALLOC_FAILURE_MSG);
            exit(1);
        }

        strcpy(argv_cpy[i], argv[i + NEW_HOSTNAME]);
    }

    argv_cpy[argc - 1] = (char *) 0;

    return argv_cpy;
}

void recursiveMkdir(const char *dirs[], size_t dirsSize, mode_t perm) {
    chdir("/");

    for (size_t i = 0; i < dirsSize; i++) {
        char *dir = (char *) calloc(strlen(".") + strlen(dirs[i]) + 1, sizeof (char));

        if (dir == NULL) {
            fprintf(stderr, SYS_CALL_ERR, MEM_ALLOC_FAILURE_MSG);
            exit(EXIT_FAILURE);
        }

        dir[0] = '.';
        strcat(dir, dirs[i]);

        int dirNotCreated = mkdir(dir, perm);
        if (dirNotCreated && errno != EEXIST) {
            fprintf(stderr, SYS_CALL_ERR, MKDIR_FAILURE_MSG);
            exit(EXIT_FAILURE);
        }
        if(chdir(dir) == -1) {
            fprintf(stderr, SYS_CALL_ERR, CHDIR_FAILURE_MSG);
            exit(EXIT_FAILURE);
        }

        free(dir);
    }
}

/**
 * Enrty point for the child process (the container).
 */
int child(void *arg) {
    char **temp = (char **) arg;

    char *hostName = temp[NEW_HOSTNAME - 1];
    char *fsDir = temp[NEW_FS_DIR - 1];
    char *numProc = temp[NUM_PS - 1];
    char *fileName = temp[PATH_TO_PROG - 1];
    char **argv_cpy = temp + PATH_TO_PROG - 1;

    if(sethostname(hostName, strlen(hostName)) == -1) {
        fprintf(stderr, SYS_CALL_ERR, SET_HOSTNAME_FAILURE_MSG);
        exit(EXIT_FAILURE);
    }

    if(chroot(fsDir) == -1) {
        fprintf(stderr, SYS_CALL_ERR, CHROOT_FAILURE_MSG);
        exit(EXIT_FAILURE);
    }

    setNumProcLmt(numProc);

    if(chdir("/") == -1) {
        fprintf(stderr, SYS_CALL_ERR, CHDIR_FAILURE_MSG);
        exit(1);
    }

    if(mount("proc", "/proc", "proc", 0, 0) == -1) {
        fprintf(stderr, SYS_CALL_ERR, MOUNT_FAILURE_MSG);
        exit(1);
    }

    if(execvp(fileName, argv_cpy) == -1) {
        fprintf(stderr, SYS_CALL_ERR, EXECVP_FAILURE_MSG);
        exit(1);
    }
    return -1;
}

/**
 * Sets a limit to the number of processes in the container.
 */
void setNumProcLmt(const char *numProc) {
    const char *dirs[] = {"/sys", "/fs", "/cgroup", "/pids"};
    size_t dirsSize = sizeof(dirs) / sizeof(dirs[0]);
    mode_t perm = 0755;

    recursiveMkdir(dirs, dirsSize, perm);

    if(chdir(PIDS_PATH) == -1) {
        fprintf(stderr, SYS_CALL_ERR, CHDIR_FAILURE_MSG);
        exit(1);
    }

    FILE *file = fopen("cgroup.procs", "w");
    if (file == NULL) {
        fprintf(stderr, SYS_CALL_ERR, OPEN_FILE_FAILURE_MSG);
        exit(1);
    }

    fprintf(file, "1");

    if(fclose(file) == EOF) {
        fprintf(stderr, SYS_CALL_ERR, CLOSE_FILE_FAILURE_MSG);
        exit(1);
    }

    file = fopen("pids.max", "w");
    if (file == NULL) {
        fprintf(stderr, SYS_CALL_ERR, OPEN_FILE_FAILURE_MSG);
        exit(1);
    }

    fprintf(file, numProc);
    if(fclose(file) == EOF) {
        fprintf(stderr, SYS_CALL_ERR, CLOSE_FILE_FAILURE_MSG);
        exit(1);
    }

    file = fopen("notify_on_release", "w");
    if (file == NULL) {
        fprintf(stderr, SYS_CALL_ERR, CLOSE_FILE_FAILURE_MSG);
        exit(1);
    }

    fprintf(file, "1");

    if(fclose(file) == EOF) {
        fprintf(stderr, SYS_CALL_ERR, CLOSE_FILE_FAILURE_MSG);
        exit(1);
    }
}

/**
 * Unmounts the Container's File System
 */
void umountContainerFs(char *fsDir) {

    size_t fsDirLen = strlen(fsDir);
    size_t procLen = strlen("/proc");

    char *dirToUmount = (char *) malloc(sizeof(char) * (fsDirLen + procLen + 1));
    if (dirToUmount == NULL) {
        fprintf(stderr, SYS_CALL_ERR, MEM_ALLOC_FAILURE_MSG);
        exit(EXIT_FAILURE);
    }

    strcpy(dirToUmount, fsDir);
    strcat(dirToUmount, "/proc");

    if(umount(dirToUmount) == -1) {
        fprintf(stderr, SYS_CALL_ERR, UMOUNT_FAILURE_MSG);
        exit(EXIT_FAILURE);
    }
    free(dirToUmount);
}

void removeContainerFiles(char *fsDir) {
    const char *files[] = {"cgroup.procs", "pids.max", "notify_on_release"};
    size_t fsDirLen = strlen(fsDir);

    for (auto file: files) {
        char *fileToRemove = (char *) malloc(sizeof(char) * (fsDirLen + strlen("/sys/fs/cgroup/pids/") + strlen(file) + 1));
        if(fileToRemove == NULL) {
            fprintf(stderr, SYS_CALL_ERR, MEM_ALLOC_FAILURE_MSG);
            exit(EXIT_FAILURE);
        }

        strcpy(fileToRemove, fsDir);
        strcat(fileToRemove, "/sys/fs/cgroup/pids/");
        strcat(fileToRemove, file);
        int ret_val = remove(fileToRemove);
        if (ret_val == -1) {
            fprintf(stderr, SYS_CALL_ERR, DELETE_FILE_FAILURE_MSG);
            exit(EXIT_FAILURE);
        }

        free(fileToRemove);
    }
}

void removeContainerDirs(char *fsDir) {
    const char *dirs[] = {PIDS_PATH, "/sys/fs/cgroup", "/sys/fs"};
    size_t fsDirLen = strlen(fsDir);
    for (auto dir: dirs) {
        char *dirToRemove = (char *) malloc(sizeof(char) * (fsDirLen + strlen(dir) + 1));
        if (dirToRemove == NULL) {
            fprintf(stderr, SYS_CALL_ERR, MEM_ALLOC_FAILURE_MSG);
            exit(EXIT_FAILURE);
        }

        strcpy(dirToRemove, fsDir);
        strcat(dirToRemove, dir);
        int ret_val = rmdir(dirToRemove);
        if (ret_val == -1) {
            fprintf(stderr, SYS_CALL_ERR, RMDIR_FAILURE_MSG);
            exit(EXIT_FAILURE);
        }
        free(dirToRemove);
    }
}

/**
 * Removes Container's files and dirs which been added by the current program.
 */
void removeContainerFilesystem(char *fsDir) {
    removeContainerFiles(fsDir);
    removeContainerDirs(fsDir);
}

void free_argv(char ***argv_cpy, size_t argc) {
    for (size_t i = 0; i < argc - 1; i++) {
        free((*argv_cpy)[i]);
    }
    free(*argv_cpy);
}

int main(int argc, char *argv[]) {

    char *stack = (char *)calloc(STACK, 1);
    if (stack == NULL) {
        fprintf(stderr, SYS_CALL_ERR, MEM_ALLOC_FAILURE_MSG);
        exit(1);
    }
    char **argv_cpy = cpy_argv(argc, argv);

    int child_pid = clone(child, (void *)(stack + STACK), INITIAL_FLAGS, argv_cpy);
    if (child_pid == -1) {
        fprintf(stderr, SYS_CALL_ERR, CLONE_FAILURE_MSG);
        exit(1);
    }

    if(wait(NULL) == -1) {
        fprintf(stderr, SYS_CALL_ERR, WAIT_FAILURE_MSG);
        exit(1);
    }

    umountContainerFs(argv[NEW_FS_DIR]);
    removeContainerFilesystem(argv[NEW_FS_DIR]);

    free(stack);
    free_argv(&argv_cpy, argc);

    return 0;
}
