# UNIX-Shell
### Simple Unix Shell implementation in C with parallel commands, redirection and built-in commands.


## Sample output

![ezgif com-gif-maker](https://user-images.githubusercontent.com/79598598/167259115-67b4d2d3-be6b-4056-8bbc-a2d14c86d1fc.gif)

-------------------------------------------

# Program Specifications

## Basic Shell: UnixShell
Your basic shell, called UnixShell, is basically an interactive loop: it repeat-
edly prints a prompt UnixShell> (note the space after the greater-than sign),
parses the input, executes the command specified on that line of input, and
waits for the command to finish. This is repeated until the user types exit.
The name of your final executable should be UnixShell.
The shell can be invoked with either no arguments or a single argument;
anything else is an error. Here is the no-argument way:
prompt> ./UnixShell
UnixShell>
At this point, UnixShell is running, and ready to accept commands. Type
away!
The mode above is called interactive mode, and allows the user to type
commands directly. The shell also supports a batch mode, which instead reads input from a batch file and executes commands from therein. Here is
how you run the shell with a batch file named batch.txt:
prompt> ./UnixShell batch.txt
One difference between batch and interactive modes: in interactive mode,
a prompt is printed (UnixShell>). In batch mode, no prompt should be
printed.
You should structure your shell such that it creates a process for each new
command (the exception are built-in commands, discussed below). Your basic
shell should be able to parse a command and run the program corresponding
to the command. For example, if the user types ls -la /tmp, your shell
should run the program /bin/ls with the given arguments -la and /tmp
(how does the shell know to run /bin/ls? It’s something called the shell
path; more on this below).



### Structure

## Basic Shell
The shell is very simple (conceptually): it runs in a while loop, repeatedly
asking for input to tell it what command to execute. It then executes that
command. The loop continues indefinitely, until the user types the built-in
command exit, at which point it exits. That’s it!
For reading lines of input, you should use getline(). This allows you to
obtain arbitrarily long input lines with ease. Generally, the shell will be run
in interactive mode, where the user types a command (one at a time) and
the shell acts on it.
However, your shell will also support batch mode, in which the shell is
given an input file of commands; in this case, the shell should not read user
input (from stdin) but rather from this file to get the commands to execute.
In either mode, if you hit the end-of-file marker (EOF), you should call
exit(0) and exit gracefully.
To parse the input line into constituent pieces, you might want to use
strsep(). Read the man page (carefully) for more details.
To execute commands, look into fork(), exec(), and wait()/waitpid().
See the man pages for these functions.
You will note that there are a variety of commands in the exec family; for
this project, you must use execv. You should not use the system() library
function call to run a command. Remember that if execv() is successful,
it will not return; if it does return, there was an error (e.g., the command
does not exist). The most challenging part is getting the arguments correctly
specified.

## Paths
In our example above, the user typed ls but the shell knew to execute the
program /bin/ls. How does your shell know this?
It turns out that the user must specify a path variable to describe the set
of directories to search for executables; the set of directories that comprise
the path are sometimes called the search path of the shell. The path variable
contains the list of all directories to search, in order, when the user types a
command.
Important: Note that the shell itself does not implement ls or other
commands (except built-ins). All it does is find those executables in one of
the directories specified by path and create a new process to run them.
To check if a particular file exists in a directory and is executable, consider
the access() system call. For example, when the user types ls, and path is
set to include both /bin and /usr/bin, try access("/bin/ls", X OK). If
that fails, try /usr/bin/ls. If that fails too, it is an error.
Your initial shell path should contain one directory: /bin
Note: Most shells allow you to specify a binary specifically without using
a search path, using either absolute paths or relative paths. For example,
a user could type the absolute path /bin/ls and execute the ls binary
without a search path being needed. A user could also specify a relative
path which starts with the current working directory and specifies the ex-
ecutable directly, e.g., ./main. In this project, you do not have to worry
about these features.

## Built-in Commands
Whenever your shell accepts a command, it should check whether the com-
mand is a built-in command or not. If it is, it should not be executed like
other programs. Instead, your shell will invoke your implementation of the
built-in command. For example, to implement the exit built-in command,
you simply call exit(0); in your UnixShell source code, which then will exit
the shell.
In this project, you should implement exit, cd, and path as built-in
commands.
 exit: When the user types exit, your shell should simply call the exit
system call with 0 as a parameter. It is an error to pass any arguments
to exit.
 cd: cd always take one argument (0 or ¿1 args should be signaled as
an error). To change directories, use the chdir() system call with the argument supplied by the user; if chdir fails, that is also an error.
 path: The path command takes 0 or more arguments, with each argu-
ment separated by whitespace from the others. A typical usage would
be like this: UnixShell> path /bin /usr/bin, which would add /bin
and /usr/bin to the search path of the shell. If the user sets path to
be empty, then the shell should not be able to run any programs (ex-
cept built-in commands). The path command always overwrites the
old path with the newly specified path.

## Redirection
Many times, a shell user prefers to send the output of a program to a file
rather than to the screen. Usually, a shell provides this nice feature with the
'>' character. Formally this is named as redirection of standard output. To
make your shell users happy, your shell should also include this feature, but
with a slight twist (explained below).
For example, if a user types ls -la /tmp > output, nothing should be
printed on the screen. Instead, the standard output of the ls program should
be rerouted to the file output. In addition, the standard error output of the
program should be rerouted to the file output (the twist is that this is a little
different than standard redirection).
If the output file exists before you run your program, you should simple
overwrite it (after truncating it).
The exact format of redirection is a command (and possibly some argu-
ments) followed by the redirection symbol followed by a filename. Multiple
redirection operators or multiple files to the right of the redirection sign are
errors.
Note: don’t worry about redirection for built-in commands (e.g., we will
not test what happens when you type path /bin > file).


## Parallel Commands
Your shell will also allow the user to launch parallel commands. This is
accomplished with the ampersand operator as follows:
UnixShell> cmd1 & cmd2 args1 args2 & cmd3 args1
In this case, instead of running cmd1 and then waiting for it to finish, your
shell should run cmd1, cmd2, and cmd3 (each with whatever arguments the
user has passed to it) in parallel, before waiting for any of them to complete.
Then, after starting all such processes, you must make sure to use wait()
(or waitpid) to wait for them to complete. After all processes are done,return control to the user as usual (or, if in batch mode, move on to the next
line).

## Program Errors
The one and only error message. You should print this one and only
error message whenever you encounter an error of any type:
char error message[30] = "An error has occurred\n"; write(STDERR FILENO,
error message, strlen(error message));
The error message should be printed to stderr (standard error), as shown
above.
After most errors, your shell simply continue processing after printing the
one and only error message. However, if the shell is invoked with more than
one file, or if the shell is passed a bad batch file, it should exit by calling
exit(1).
There is a difference between errors that your shell catches and those that
the program catches. Your shell should catch all the syntax errors specified in
this project page. If the syntax of the command looks perfect, you simply run
the specified program. If there are any program-related errors (e.g., invalid
arguments to ls when you run it, for example), the shell does not have to
worry about that (rather, the program will print its own error messages and
exit).

## Testing
Once you are done with writing the shell, you can check its functionality
by performaing various tests. A script UnixShell-tester is provided, all you
need to do is to run it by providing the command ./UnixShell-tester, which
in turn apply various tests available in tests folder. You may look into the
folder various tests inputs and expected outputs. You can also run a specific
test by executing ./UnixShell-tester -t n, where n is a specific number of
test.

