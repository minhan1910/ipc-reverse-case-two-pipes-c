#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

/*
 *Design a program using ordinary pipes in which one process sends a
 string message to a second process, and the second process reverses
 the case of each character in the message and sends it back to the first
 process. For example, if the first process sends the message Hi There,
 the second process will return hI tHERE. This will require using two
 pipes, one for sending the original message from the first to the second
 process and the other for sending the modified message from the second
 to the first process. You can write this program using either UNIX or
 Windows pipes
 *
 * */

void reverse_case(char *str, size_t size)
{
    for (int i = 0; i < size; ++i)
    {
        char tmp = str[i];
        if (tmp >= 'A' && tmp <= 'Z')
        {
            str[i] = tmp + 'a' - 'A';
        }
        else if (tmp >= 'a' && tmp <= 'z')
        {
            str[i] = tmp - ('a' - 'A');
        }
    }
}

int create_pipe(int fd[2])
{
    if (pipe(fd) == -1)
    {
        perror("Pipe creation failed");
        return -1;
    }
    return 0;
}

int send_message(int fd[2], const char *buffer, size_t buffer_size)
{
    if (write(fd[1], buffer, buffer_size) == -1)
    {
        perror("Write to pipe failed");
        return -1;
    }

    return 0;
}

void receive_message(int fd[2], char *buffer, size_t buffer_size)
{
    if (read(fd[0], buffer, buffer_size) == -1)
    {
        perror("Read from pipe failed");
        return -1;
    }
    return 0;
}

void parent_process(int pipefd1[2], int pipefd2[2], const char *msg, size_t msg_size)
{
    printf("Before: %s\n", msg);

    // send msg
    close(pipefd1[0]);
    if (send_message(pipefd1[1], msg, msg_size) == -1)
    {
        return 1;
    }
    close(pipefd1[1]);

    wait(NULL);

    // receive reverse msg
    close(pipefd2[1]);
    char buffer[msg_size];
    if (receive_message(pipefd2[0], buffer, msg_size) == -1)
    {
        return 1;
    }
    close(pipefd2[0]);

    printf("After: %s\n", buffer);
}

void child_process(int pipefd1[2], int pipefd2[2], size_t msg_size)
{
    close(pipefd1[1]);
    char buffer[msg_size];
    // receive msg
    if (receive_message(pipefd1[0], buffer, msg_size) == -1)
    {
        exit(1);
    }
    close(pipefd1[0]);

    reverse_case(buffer, msg_size - 1);

    // send reverse msg
    close(pipefd2[0]);
    if (send_message(pipefd2[1], buffer, msg_size) == -1)
    {
        exit(1);
    }
    close(pipefd2[1]);
}

int main()
{
    int pipefd1[2], pipefd2[2];
    pid_t pid;
    char msg[] = "Hi There";
    size_t msg_size = strlen(msg) + 1; // strlen does not include null terminate so must be + 1

    if (create_pipe(pipefd1))
    {
        return 1;
    }

    if (create_pipe(pipefd2))
    {
        return 1;
    }

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    if (pid == 0)
    {
        child_process(pipefd1, pipefd2, msg_size);
    }
    else
    {
        parent_process(pipefd1, pipefd2, msg, msg_size);
    }

    return 0;
}