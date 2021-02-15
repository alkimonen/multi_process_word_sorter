# Word Sorter for Multiple Files
This project sorts the words in ascending order in multiple files using multiple processes in **C programming language**. Program first reads the files then sorts all words together into an output file.

There are two different implementations in the project, *pwc* and *twc*. Both of them takes arguments in terminal. These arguments include number of files, input file names and output file names with the directories. Execution process and the order of arguments will be given below.

*pwc* creates child processes using *fork()* then uses these child processes to read input files. All words read from the files and thier frequencies are stored seperately in each process. Later POSIX message queues are used to send the words into the parent process. Parent compares the words coming from the children. After all words from children are cleared, the results are writen into the output file.

*twc* uses POSIX threads to read input files and stores words into a sorted linked list. After threads are terminated, lists are merged together and results are written into the output file.

The input files can be simple text files. All words and their number of occurences in all files will be sorted in ascending order and written into the output file. 

This project uses *Makefile* and *gcc*. These packages should be installed into the system before compilation. If not installed, install them first. Then navigate to the projects directory to compile the project and execute
```
make
```

After compilation, the execution command is given below.
```
./programName N in1 in2 ... inN outputFile
```
**programName:** Name of the program to be executed. (pwc or twc)

**N:** Number of imput files.

**in1, in2, ..., inN:** Directories of input files.

**outputFile:** Directory of output file.

An example to execute twc with 3 input files can be
```
./pwc 3 test/text1.txt test/text2.txt test/text3.txt results.txt
```

**P.S.** Reading more than 10 files are not recommended. The program supports more than that number of input files, however, it is not recommended as some problems may occur.
