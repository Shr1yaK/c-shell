# C Shell

A Unix shell built from scratch in C. Implements process management, I/O redirection, pipelines, job control, and signal handling — all without using any shell library functions.

## Features

### Built-in Commands
| Command | Description |
|---|---|
| `hop [path...]` | Change directory. Supports `~`, `..`, `-` (previous dir), and multiple paths |
| `reveal [-flags] [path]` | List directory contents (`ls` equivalent). Flags: `-a` (hidden files), `-l` (long format) |
| `log` | Show command history (last 15 commands, persists across sessions) |
| `log purge` | Clear command history |
| `log execute <n>` | Re-run the nth most recent command |
| `activities` | List all background/stopped jobs sorted by name |
| `fg [job_number]` | Bring a background or stopped job to the foreground |
| `bg [job_number]` | Resume a stopped job in the background |
| `ping <pid> <signal_number>` | Send a signal to a process |

### Shell Features
- **Pipelines** — chain commands with `|` (e.g. `ls | grep foo | wc -l`)
- **I/O Redirection** — `<`, `>`, `>>` for input/output/append
- **Background execution** — run commands with `&`
- **Sequential execution** — chain commands with `;`
- **Signal handling** — Ctrl-C (SIGINT), Ctrl-Z (SIGTSTP), Ctrl-D (logout)
- **Job control** — stop, resume, and track background processes
- **Persistent history** — command log survives shell restarts

## Build & Run

```bash
cd shell
make
./shell.out
```

Requires GCC with C99 and POSIX support.

## Architecture

```
shell/
├── main.c          # REPL loop, signal handler setup, startup
├── headers.h       # Shared includes and global externs
├── prompt.c/h      # Prompt rendering (user@host:path)
├── parser.c/h      # Tokenizer + recursive descent parser
├── execute.c/h     # Command dispatch and job tracking
├── commands.c/h    # Pipeline execution and I/O redirection
├── hop.c/h         # cd equivalent
├── reveal.c/h      # ls equivalent
├── log.c/h         # Command history
├── activities.c/h  # List background jobs
├── fgbg.c/h        # fg and bg commands
├── ping.c/h        # Signal sending utility
└── signals.c/h     # SIGINT, SIGTSTP, SIGCHLD handlers
```

The parser uses a context-free grammar to handle operator precedence:
```
shell_cmd → cmd_group ((&|;) cmd_group)* &?
cmd_group → atomic (| atomic)*
atomic    → word (word | < word | > word | >> word)*
```

Each pipeline stage runs in its own process group (`setpgid`), so signals from the terminal reach the correct process and not the shell itself.
