# Document Index IPC

A C client-server application that indexes documents (title, author, path, year, keyword) and answers queries over that index. The client and server communicate exclusively through named pipes (FIFOs), with the server optionally forking child processes to parallelize keyword searches.

## Features

- Add, consult, delete, and list indexed documents
- Keyword search, with optional multi-process parallelization
- Client-server communication over FIFOs, with a private response FIFO per client (identified by PID)
- Graceful server shutdown via client command

## System calls and IPC mechanisms

- `mkfifo`, `open`, `read`, `write`, `close`, `unlink` for FIFO-based client-server messaging
- `fork`, `wait`, `waitpid` for per-request child processes and concurrency control
- `pipe`, `dup2`, `execlp` to chain `grep` and `wc -l` for keyword counting
- `lseek`, `rename` for direct-access binary file storage and index compaction

## Tech stack

C, system calls.

## Run locally

Start the server:

```bash
make
./dserver
```

Run client commands (new terminal):

```bash
./dclient -a <title> <author> <year> <path>   # add a document
./dclient -c <key>                             # consult a document
./dclient -d <key>                             # delete a document
./dclient -l <key> <keyword>                   # list occurrences of a keyword
./dclient -s <keyword> [nr_processes]          # search documents by keyword
./dclient -f                                   # shut down the server
```

## Team

- David Sousa e Silva
- João Rafael Martins da Costa
- Tomás Barroso Ramalhete
