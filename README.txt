Victor Part:
- Planned psudocode and main algorithm
- Modified personal sigSearch files to be used as functions
- Modified personal superBlock files to be used as functions
- Developed main algorithm to write file bytes into output.pdf or output.gif
- Debugged algorithm to write data from the indirect block 
- Refactored and reformated code 

Eric Part:
- Developed algorithm to write data into output files
- Debugged code to write data from recovered file to output file
- Refactored and reformated code

How to run code:

In the command line you need to do these steps


1. Change your current working directory into CS4398FinalProject directory
    cd */CS4390FinalProject

2. Find the EXT3 formated drive
    fdisk -l

3. Compile the code
    gcc -o file_recover *.c -lm

4. Run the code with values "a" and "1" respective to your use case
    sudo file_recover /dev/sd[a] [1]

    1 indicates that you are looking for a GIF file
    2 indicates that you are looking for a PDF file

5. Wait
    the code will run and shortly the deleted file will be fully copied into the current working directory