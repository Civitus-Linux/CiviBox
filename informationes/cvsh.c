#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_MAX 1024
#define ARG_SIZE 64

int cvsh_cmd()
{
    char input[INPUT_MAX];

    while (1)
    {
        printf("$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = '\0';

        if (input[0] == '\0') continue;
        if (strcmp(input, "exit") == 0) break;

                char *args[ARG_SIZE];
        int argc = 0;

        char *token = strtok(input, " \t");

        while (token != NULL && argc < ARG_SIZE - 1) {
            args[argc++] = token;
            token = strtok(NULL, " \t");
        }

        args[argc] = NULL;

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            execvp(args[0], args);

            perror("execvp");
            _exit(127);
        }
        else waitpid(pid, NULL, 0);
    }
    return 0;
}
