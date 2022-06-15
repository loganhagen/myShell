<h1 align="center">myShell</h1>

myShell is a custom UNIX shell program, modeled after a BASH shell. It was created as part of my upper-level operating systems coursework at university.

### Functionality
- Common shell commands along with appropriate flags (Ex. `ls -l`, `exit`, `cat -n`).
- Execution and handling of background commands.
- Input and output redirection.
- Command piping.
- Environment variables: Executable path, command history, and home directory.
- Builtin commands: `export`, `history`, and `cd`.

<br>Compilation: Enter `make` to build the myShell executable in the current directory.
<br>Execution: Enter `myShell` after compiling.

### Notes
- The shell uses a space as a delimiter. Not using a space between tokens will produce unpredictable behaviour.
- The default path for command executables is `/bin`. If you would like to change that, you can either use the `export` command to append a new directory, or include a profile file (`.myShell_profile`) in the home directory that includes a different value for `myPATH` (Ex. `myPATH=/usr/bin:/bin:/usr/sbin`).
- Like the path environment variable, the history and home environment variables are also set to defaults if no profile file is provided.
- `export myPATH=yourVal` never replaces the value of `myPATH`, only appends.
- `export myHOME=yourVal` and `export myHISTFILE=yourVal` always replace their respective values.
- `echo` implemented with limited capability. Can be used to see values of environment variables (ex. `echo $myPATH`).   
- Unlimited amount of input or output redirection operators is allowed (ex. `ls > out1 > out2 > out3`).
