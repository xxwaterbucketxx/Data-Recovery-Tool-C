#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "recoveringFile.h"

void writeOutput(int fd, int outfd, unsigned char block[], int blockSize, unsigned char sig[], int blockCounter)
{
	int sigSize = sizeof(sig) / sizeof(sig[0]);
	bool eof = false;
	int offset = 0;
	int blockIndex = 0;
	unsigned char hexBlock[4];
	unsigned char nextHexBlock[4];
	bool indirectBlock = false;
	int nextBlock = 0;
	int rBlock = 0;
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

							rBlock = hexToint(hexBlock, 4) + 256;
							offset = rBlock * blockSize;
							
							if(hexToint(hexBlock, 4) == 0)
							{
								eof = true;
								break;
							}
							
							for(int z = 4; z < 8; z++)
							{
								hexBlock[z - 4] = block[x + z];
							}
							nextBlock = hexToint(hexBlock, 4);
						

							if(nextBlock == 0)
							{
								writeLastBlock(fd, outfd, offset, dataBlock, blockSize);
							}
							else
							{
								lseek(fd, offset, SEEK_SET);
								read(fd, dataBlock, blockSize);
								write(outfd, dataBlock, blockSize);
							}
						}
					}
				}				
			}
		}

		blockCounter++;
		blockIndex++;
	}
}

void writeLastBlock(int fd, int outfd, int offset, unsigned char dataBlock[], int blockSize)
{
	lseek(fd, offset, SEEK_SET);
	read(fd, dataBlock, blockSize);

	for(int a = blockSize - 1; a > -1; a--)
	{
		if(dataBlock[a] != 0)
		{
			blockSize = a;
			break;
		}
	}

	write(outfd, dataBlock, blockSize + 1);
}