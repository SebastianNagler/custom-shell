# Custom C++ Unix Shell

A command-line interpreter implemented in C++. This project was developed to demonstrate essential operating system concepts, including process management and inter-process communication.

## Features

* **Command Execution:** Parsing and executing standard Unix commands by resolving binaries against the `$PATH` environment variable.
* **Pipeline Support:** Executing multiple commands in succession and redirecting standard output of one simple command to standard input of another using the pipe operator (`|`).
* **Built-in Commands:** Essential shell built-ins are implemented explicitly:
  * `cd`: With support for absolute paths, relative paths, and default fallback to `$HOME`.
  * `exit`
* **Signal Handling:** Intercepting `SIGINT` (Ctrl+C) to prevent termination of the parent shell.
* **Continuous Integration:** Automated simple shell tests for pushes.

## Technical Implementation

* **Process Creation:** Uses `fork()` for duplicating the process and `execv()` for replacing the child process image with the target executable.
* **Process Synchronization:** Uses `waitpid()` to suspend parent execution until state changes in child processes occur; thereby prevents zombie processes.
* **Inter-Process Communication:** Uses `pipe()` to create unidirectional data channels and `dup2()` to duplicate file descriptors; maps `STDOUT` and `STDIN` between consecutive commands in a pipeline.
* **File System Operations:** Uses `stat()`, `getcwd()`, and `chdir()` to resolve files and traverse directories.

## Compilation

To compile the source via GCC, simply run `make`.

## Usage

Execute the compiled binary to enter the shell environment:

```bash
./shell
```

Example operations within the shell:

```bash
/home$ ls -la

/home$ cat shell.cpp | grep "pid" | wc -l

/home$ cd /var/log
/var/log$ pwd
```

## Limitations

Not implemented yet:

* Quotation parsing (single/double quotes) for commands with spaces
* Output redirection (`>`, `>>`) and input redirection (`<`) via files
* Background process execution (`&`)
