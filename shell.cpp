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

int main()
{
    signal(SIGINT, SIG_IGN);

    std::string line;
    char cwd[PATH_MAX];
    while (true)
    {
        if (getcwd(cwd, sizeof(cwd)))
            std::cout << cwd << "$ ";
        else
            std::cout << "? $ ";
        std::cout << std::flush;

        if (!std::getline(std::cin, line))
            break;

        std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty())
            continue;

        if (tokens[0] == "exit")
            break;
        if (tokens[0] == "cd")
        {
            const char *dir = nullptr;
            if (tokens.size() >= 2)
                dir = tokens[1].c_str();
            else
                dir = getenv("HOME");
            if (!dir)
                dir = "/";
            if (chdir(dir) != 0)
                std::cerr << "cd: " << strerror(errno) << "\n";
            continue;
        }

        std::vector<std::vector<std::string>> simple_cmds;
        std::vector<std::string> cur;
        for (std::string &t : tokens)
        {
            if (t == "|")
            {
                if (cur.empty())
                {
                    std::cerr << "Pipe operator located at start of line or immediately preceded by other pipe operator\n";
                    cur.clear();
                    break;
                }
                simple_cmds.push_back(cur);
                cur.clear();
            }
            else
                cur.push_back(t);
        }
        if (!cur.empty())
            simple_cmds.push_back(cur);
        if (simple_cmds.empty())
            continue; // in case "|" appears at the beginning of the entered text

        int prev_fd = STDIN_FILENO;
        std::vector<pid_t> children;
        for (size_t i = 0; i < simple_cmds.size(); ++i)
        {
            int pipefd[2] = {-1, -1};
            if (i + 1 < simple_cmds.size())
            {
                if (pipe(pipefd) != 0)
                {
                    std::cerr << "Pipe error\n";
                    break;
                }
            }

            std::string cmd = simple_cmds[i][0];
            std::string full = find_cmd(cmd);
            if (full.empty())
            {
                std::cerr << cmd << ": Command not found\n";
                if (pipefd[0] != -1)
                {
                    close(pipefd[0]);
                    close(pipefd[1]);
                }
                break;
            }

            std::vector<char *> args;
            for (const std::string &s : simple_cmds[i])
                args.push_back(const_cast<char *>(s.c_str()));
            args.push_back(nullptr);

            pid_t pid = spawn(full.c_str(), args.data(), prev_fd, (pipefd[1] == -1 ? STDOUT_FILENO : pipefd[1]));
            if (pid < 0)
            {
                std::cerr << "fork() returned negative PID\n";
            }
            else
                children.push_back(pid);

            if (prev_fd != STDIN_FILENO)
                close(prev_fd);
            if (pipefd[1] != -1)
                close(pipefd[1]);
            prev_fd = (pipefd[0] != -1 ? pipefd[0] : STDIN_FILENO);
        }

        if (prev_fd != STDIN_FILENO)
            close(prev_fd);

        for (pid_t c : children)
        {
            int status;
            waitpid(c, &status, 0);
        }
    }

    std::cout << "\n";
    return 0;
}