#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>

std::vector<std::string> tokenize(const std::string &line)
{
    std::vector<std::string> tokv;
    std::istringstream stream(line);
    std::string tok;
    while (stream >> tok)
        tokv.push_back(tok);
    return tokv;
}

std::string find_cmd(const std::string &cmd)
{
    if (cmd.find('/') != std::string::npos)
        return cmd;
    char *path_env = getenv("PATH");
    if (!path_env)
        return std::string();
    std::string path(path_env);
    size_t start = 0;
    while (start <= path.size())
    {
        size_t colon = path.find(':', start);
        std::string dir = path.substr(start, (colon == std::string::npos ? path.size() : colon) - start);
        std::string full = dir + "/" + cmd;
        struct stat st;
        if (stat(full.c_str(), &st) == 0)
            return full;
        if (colon == std::string::npos)
            break;
        start = colon + 1;
    }
    return std::string();
}

pid_t spawn(const char *path, char *argv[], int in_fd, int out_fd)
{
    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        if (in_fd != STDIN_FILENO)
        {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO)
        {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        execv(path, argv);

        std::cerr << "execv() error: " << strerror(errno) << "\n";
        exit(1);
    }
    return pid;
}
