#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "recoveringFile.h"
#include "sigSearch.h"
#include "superBlock.h"


// Victor Vu, vhv180000
// Eric Ngo, ebn180000

// CS 4398.001 Final Project 

// This project demonstrates the topics and learning outcomes of all things to do 
// with CS 4398 digtial forensics, this project takes a zeroed EXT3 formated drive 
// that has a PDF or GIF file loaded onto it and then has subsequently been deleted 
// off the drive, this project will then looks for the header of the PDF or GIF file, 
// searches and copies all the associated files for the PDF or GIF file to then recover 
// the aformentioned deleated PDF or GIF file.

int main(int argc, char *argv[]) 
{
	int blockSize;
	int eofOffset;
	int headerBlock;
	int offset;
	
	//EOF file signatures for respective file types
	unsigned char pdfSig[5] = {0x25, 0x25, 0x45, 0x4F, 0x46};
    unsigned char gifSig[2] = {0x3B, 0x00};

	bool eof = false;

	if (argc != 3) 
	{
		printf("Invalid parameters. Run the program 'sudo ./programname [device name] [file type] (1 = gif, 2 = pdf)'\n");
		exit(EXIT_FAILURE);
	}

	//Opens file with given argument
	int fd = open(argv[1], O_RDONLY);
	
	// 1 = gif, 2 = pdf
	int fileType = atoi(argv[2]);

	//This is a catch for if the drive was unable to be opened
	if (fd < 0) 
	{
		perror("Open error");
		exit(EXIT_FAILURE);
	}

    lseek(fd, 0, SEEK_SET);

	//Gets block size of the drive based on the value given by the super block
	blockSize = getBlockSize(fd);
	unsigned char block[blockSize];

	lseek(fd, 0, SEEK_SET);

	//Gets absoulte block number based on respective file type 
	headerBlock = getHead(fd);

	//Gets absoulte byte offset with found absoulte block number and super block block size value
	offset = headerBlock * blockSize;
	lseek(fd, offset, SEEK_SET);

	//Creates respective output files with respective file type tags
	switch(fileType)
	{
		case 1:
			int gifFd = open("output.gif", O_APPEND | O_CREAT | O_WRONLY, 0644);
			writeOutput(fd, gifFd, block, blockSize, gifSig, headerBlock);
			close(gifFd);
			break;
		case 2:
			int pdfFd = open("output.pdf", O_APPEND | O_CREAT | O_WRONLY, 0644);
			writeOutput(fd, pdfFd, block, blockSize, pdfSig, headerBlock);
			close(pdfFd);
			break;
		default:
			printf("Wrong file type\n");
			break;
	}

	close(fd);

	return 0;
}
