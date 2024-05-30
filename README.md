
# shell_Advanced_programing

Written by:
1. Achiya Ben Natan 213934599
2. Nisim Atiya 207302027
3. Nitay levy 319096251

This assignment includes a shell program that supports the following actions:
1. Redirect states - `>`, `>>`, `2>`
2.  piping - `|`
3. `if/else` command
4. Running the same command again - `!!`
5. Built-in shell commands
6. Command to change the cursor ״hello:״
7. echo command that prints the arguments
8. "echo $?" command that will print the status of the last command executed
9. "!!" command that repeats the last command
10. "quit" command to exit the shell
11. Adding variables
12. "read" command
13. command for flow control, i.e. IF/ELSE.
14. Memory of the last commands (at least 20). Option to browse by Arrows

## Installation

To install shell_Advanced_programing, run on terminal:
```bash
git clone https://github.com/tjhv10/shell_Advanced_programing.git
```

## Usage

Run on terminal the following command:
```bash
make
```

To use the program, run on terminal the following command:
```bash
./myshell
```

### Examples

#### Redirect States
1. **Output Redirection**:
    ```sh
    ls > file.txt
    ```
    This command will write the output of `ls` to `file.txt`.

2. **Append Redirection**:
    ```sh
    echo "Hello, World!" >> file.txt
    ```
    This command will append "Hello, World!" to `file.txt`.

3. **Error Redirection**:
    ```sh
    ls  nofile 2> mylog.txt
    ```
    This command will write the error message of `ls` to `mylog.txt`.

#### Piping
```sh
cat colors.txt | cat | cat | cat
```
This command will print the colors.txt to the terminal.

#### If/Else Command
```sh
if date | grep Fri
then
  echo "Shabat Shalom"
else
  echo "Hard way to go"
fi
```
This command checks if it's Saturday today  and prints an appropriate message.

#### Running the Same Command Again
```sh
pwd
!!  
```
The `!!` command repeats the last executed command.

#### Entering Directories
```sh
cd /fils/directory1/directory2
```
This command changes the current directory to `/fils/directory1/directory2`.

#### Changing the Shell's Prompt
```sh
prompt = myprompt
```
This command changes the shell's prompt to `myprompt`.

### Custom Variables
You can set and get environment variables within the shell:
```sh
$myvar = value
echo $myvar
```
This command saves a variable and assigns it to the `myvar` variable and then we can use it (in this case we will print it).


### Reading Input
```sh
read name
hello
echo $name
hello
```
This command reads a line from standard input and assigns it to the variable `name` Then we can use it (in this case we will print it).


