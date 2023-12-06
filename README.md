# Nine Men's Morris C-Client 

## Overview

This project implements a Nine Men's Morris game client, allowing players to interact with a server, make moves, and play the game against each other or against a basic AI. It consists of several C source files responsible for establishing connections, handling shared memory, reading and processing configuration files, and managing communication between the client and the server.


## Files

- `sharedMemory.h`: Defines structures and functions related to shared memory.
- `sysprak-client.h`: Header file containing function prototypes and necessary includes.
- `performConnection.h`: Header file for handling server connection and message parsing.
- `config.h`: Header file defining configuration structures.
- `sharedMemory.c`: Implementation of shared memory functions for inter-process communication.
- `performConnection.c`: Functions for handling the game's server connection and interpreting messages.
- `config.c`: Reads and processes configuration data from a file.
- `think.c`: Executes the AI decision-making process.
  - The AI determines the best possible move based on the current game phase (placing tiles, moving tiles, capturing opponent tiles).
  - Various conditions and loops help in selecting the optimal move based on the game's rules.
- `main.c`: Main program file orchestrating game setup and execution.

### `Makefile`

- The Makefile contains instructions for building the project.

### Other header files

- `sysprak-client.h`, `performConnection.h`, and other header files contain function declarations and definitions used across multiple source files.

## How to Use

- Compile the project using the provided Makefile.
- Execute the resulting binary to run the client application.
- Ensure necessary dependencies and configurations are appropriately set before running the application.

## Dependencies

- This project might depend on certain system libraries and configurations, such as POSIX libraries, network-related headers (`sys/socket.h`, `netinet/in.h`, etc.), and standard C libraries (`stdio.h`, `stdlib.h`, etc.). It also requires the server implementation which is not included in this repository.

## Contributors

- This project resembles a university project completed by a team of four people.

