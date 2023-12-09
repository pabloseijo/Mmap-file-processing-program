# Mmap File Processing Program

## Description

This C program demonstrates the use of memory-mapped files (`mmap`) to process text files in a parent-child process model. It showcases inter-process communication through signals and shared memory. The program reads a text file, processes its contents (replacing numbers with asterisks and converting letters to uppercase), and writes the output to a new file.

🌟 **Key Features:**
- Uses `fork()` to create a child process.
- Implements memory mapping for efficient file reading and writing.
- Utilizes signals (`SIGUSR1`) for synchronization between parent and child processes.
- Processes the file in two halves - one by the parent and the other by the child.

## How It Works

- **File Reading**: The program opens a specified input file for reading.
- **Memory Mapping**: It maps the input file into memory for efficient access.
- **File Processing**:
  - The parent process converts alphabetic characters to uppercase in the first half of the file.
  - The child process replaces numbers with an equivalent number of asterisks in the second half.
- **Output Writing**: The processed content is written to a new file using memory mapping.
- **Synchronization**: Parent and child synchronize their work using signals.

## Requirements

To run this program, you need a Unix-like environment with GCC compiler.

## Compilation and Execution

1. **Clone the repository:**

   ```
   git clone https://github.com/your-username/mmap-file-processing.git
   cd mmap-file-processing
   ```
2. **Compile the program**
   ```
   gcc -o mmap_processor main.c
   ```
3. **Run the program**
   ```
   ./mmap_processor input.txt output.txt
   ```

Replace `input.txt` with your input file and `output.txt` with the desired output file name.

## Contributing

🤝 Contributions, issues, and feature requests are welcome! Feel free to check.

## Show your support

Give a ⭐️ if this project helped you!

---

README created with ❤️ by [Pablo Seijo](https://github.com/pabloseijo)



