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

int main(int argc, char *argv[]) 
{
	int blockSize;
	int eofOffset;
	int headerBlock;
	int offset;
	
	unsigned char pdfSig[5] = {0x25, 0x25, 0x45, 0x4F, 0x46};
    unsigned char gifSig[2] = {0x3B, 0x00};

	bool eof = false;

	if (argc != 3) 
	{
		printf("Invalid parameters. Run the program 'sudo ./programname [device name] [file type] (1 = gif, 2 = pdf)'\n");
		exit(EXIT_FAILURE);
	}

	int fd = open(argv[1], O_RDONLY);
	// 1 = gif, 2 = pdf
	int fileType = atoi(argv[2]);

	if (fd < 0) 
	{
		perror("Open error");
		exit(EXIT_FAILURE);
	}

    lseek(fd, 0, SEEK_SET);

	blockSize = getBlockSize(fd);
	unsigned char block[blockSize];

	lseek(fd, 0, SEEK_SET);

	headerBlock = getHead(fd);

	offset = headerBlock * blockSize;
	lseek(fd, offset, SEEK_SET);

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