#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "sigSearch.h"
#include "superBlock.h"

int main(int argc, char *argv[]) 
{
	int headerBlock;
	int blockSize;
	int offset;

	int test;
	
	unsigned char pdfSig[5] = {0x25, 0x25, 0x45, 0x4F, 0x46};
    unsigned char gifSig[2] = {0x3B, 0x00};

	bool eof = false;

	int eofOffset;
	int blockCounter;


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
	blockCounter = headerBlock;

	offset = headerBlock * blockSize;
	test = lseek(fd, offset, SEEK_SET);

	switch(fileType)
	{
		case 1:
			int gifFd = open("output.gif", O_APPEND | O_CREAT | O_WRONLY);
			writeOutput(fd, gifFd, block, blockSize, gifSig, blockCounter);
			close(gifFd);
			break;
		case 2:
			int pdfFd = open("output.pdf", O_APPEND | O_CREAT | O_WRONLY);
			writeOutput(fd, pdfFd, block, blockSize, pdfSig, blockCounter);
			close(pdfFd);
			break;
		default:
			printf("Wrong file type\n");
			break;
	}

	close(fd);

	return 0;
}

void writeOutput(int fd, int outfd, unsigned char block[], int blockSize, unsigned char sig[], int blockCounter){
	int j = 0;
	int sigSize = sizeof(sig) / sizeof(sig[0]);
	bool eof = false;
	int offset;
	int blockIndex = 0;
	unsigned char hexBlock[4];
	unsigned char nextHexBlock[4];
	bool indirectBlock = false;
	int nextBlock;
	unsigned char dataBlock[blockSize];
	int rBlockCounter = 0;

	while(read(fd, block, blockSize) > 0)
	{
		if(blockIndex <= 11)
		{
			for(int i = 0; i < blockSize - sigSize; i++)
			{
				eof = true;

				for(int j = 0; j < sigSize; j++) 
				{
					if(block[i] != sig[j])
					{
						eof = false;
					}
				}
				

				if(eof)
				{
					// EOF offset
					offset = i;
					break;
				}
			}

			write(outfd, block, blockSize);
		}

		if(eof)
		{
			break;
		}

		if(blockIndex == 12)
		{
			rBlockCounter = blockCounter - 256;
			int counter = -1;
			lseek(fd, 0, SEEK_SET);

			while(read(fd, block, blockSize) > 0)
			{

				for(int i = 0; i < 4; i++)
				{
					hexBlock[i] = block[i];
				}

				if(rBlockCounter == hexToint(hexBlock, 4))
				{
					for(int j = 4; j < 8; j++)
					{
						hexBlock[j-4] = block[j];
					}
					
					//If true then the block found is the indirect block needed
					if(++rBlockCounter == hexToint(hexBlock, 4))
					{
						for(int x = 0; x < blockSize - 4; x += 4)
						{
							for(int y = 0; y < 4; y++)
							{
								hexBlock[y] = block[x + y];
							}

							nextBlock = hexToint(hexBlock, 4) + 256;
							offset = nextBlock * blockSize;

							if(hexToint(hexBlock, 4) == 0)
							{
								eof = true;
								break;
							}

							lseek(fd, offset, SEEK_SET);
							read(fd, dataBlock, blockSize);
							write(outfd, dataBlock, blockSize);
						}
					}
				}				
			}
		}

		blockCounter++;
		blockIndex++;
	}
}

void decimal_to_hex(int decimal, unsigned char* hex) {
    int remainder, quotient, i = 1, j;
    char temp;
    quotient = decimal;
    while (quotient != 0) {
        remainder = quotient % 16;
        if (remainder < 10)
            hex[i++] = remainder + 48;
        else
            hex[i++] = remainder + 55;
        quotient = quotient / 16;
    }
    // Reverse the hexadecimal string
    for (j = 1; j < i; j++) {
        temp = hex[j];
        hex[j] = hex[i - j];
        hex[i - j] = temp;
    }
    hex[i] = '\0';
}

