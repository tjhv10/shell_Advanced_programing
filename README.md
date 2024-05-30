```markdown
# shell_Advanced_programing

Written by:
1.Achiya Ben Natan 213934599
2.Nisim Atiya 207302027
3.

This assignment includes a shell program that supports the following actions:
1. Redirect states - `>`, `>>`, `2>`
2.  piping - `|`
3. `if/else` command
4. Running the same command again - `!!`
5. Built-in shell commands
6. Command to change the cursor ״hello:״
7.echo command that prints the arguments
8."echo $?" command that will print the status of the last command executed
9."!!" command that repeats the last command
10."quit" command to exit the shell
11.Adding variables
12."read" command
13.command for flow control, i.e. IF/ELSE.
14.Memory of the last commands (at least 20). Option to browse by
Arrows

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
    ls > output.txt
    ```
    This command will write the output of `ls` to `output.txt`.

2. **Append Redirection**:
    ```sh
    echo "Hello, World!" >> output.txt
    ```
    This command will append "Hello, World!" to `output.txt`.

3. **Error Redirection**:
    ```sh
    ls non_existent_dir 2> error.txt
    ```
    This command will write the error message of `ls` to `error.txt`.

#### Piping
```sh
ls | grep myfile | sort
```
This command will list the files, filter those containing "myfile", and sort the results.

#### If/Else Command
```sh
if ls myfile.txt
then
    echo "File exists"
else
    echo "File does not exist"
fi
```
This command checks if `myfile.txt` exists and prints an appropriate message.

#### Running the Same Command Again
```sh
ls
!!  # Runs `ls` again
```
The `!!` command repeats the last executed command.

#### Entering Directories
```sh
cd /path/to/directory
```
This command changes the current directory to `/path/to/directory`.

#### Changing the Shell's Prompt
```sh
prompt new_prompt
```
This command changes the shell's prompt to `new_prompt`.

### Key Features and Shortcuts
1. **Arrow Keys**: Use the up and down arrow keys to navigate through the command history.
2. **Backspace**: Use the backspace key to delete characters while typing a command.
3. **Control-C**: Press `Ctrl+C` to interrupt the current command and display a custom message.

### Custom Variables
You can set and get environment variables within the shell:
```sh
$myvar = value
echo $myvar
```

### Reading Input
```sh
read myvar
```
This command reads a line from standard input and assigns it to the variable `myvar`.
```

