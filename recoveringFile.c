#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "recoveringFile.h"

//This function is used to write found files associated to a recently deleted PDF or GIF file 
// on a EXT3 formated file system, this function also assumes that the given fd has its 
// pointer pointed to the first block where the file is located 
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

	// This while loop will read the given EXT3 file system block by block 
	// and loads each respective block into a block unsigned char array
	while(read(fd, block, blockSize) > 0 && !eof)
	{
		// This checks if the block is a direct block
		if(blockIndex <= 11)
		{
			//This for loop is used to check if the EOF marker 
			// is located within the first eleven direct blocks
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

			printf("Copying deleted file inode index %d block\n", blockIndex);
			write(outfd, block, blockSize);
		}

		// This gets activated when an first indirect block is needed 
		if(blockIndex == 12)
		{
			// Since the values in the indirect block have no reference to the 
			// boot block there is a relative block number and absolute block 
			// number depeneding on how you access the drive, in the case of 
			// the project, we access the entire drive and so the block numbers
			// that are in the indriect block are off by 256 blocks since the blocks
			// referenced in the indirect block have no reference to things outside the partition
			rBlockCounter = blockCounter - 256;
			
			// We set the fd pointer back to the beginning so we can search all the blocks to find the 
			// block that has its first four bytes equal the next sequential block after the direct blocks
			lseek(fd, 0, SEEK_SET);

			printf("Searching for respective indirect block...\n");
			//This is similar to reading blocks in the prior step of direct blocks
			while(read(fd, block, blockSize) > 0 && !eof)
			{
				for(int i = 0; i < 4; i++)
				{
					hexBlock[i] = block[i];
				}
				
				//This checks if the block we are at is the correct indirect block 
				if(rBlockCounter == hexToint(hexBlock, 4))
				{
					for(int j = 4; j < 8; j++)
					{
						hexBlock[j-4] = block[j];
					}
					
					//This checks the next block to gurantee that it is infact the correct indirect block
					if(++rBlockCounter == hexToint(hexBlock, 4))
					{
						printf("Respective indirect block found...\n");

						for(int x = 0; x < blockSize - 4; x += 4)
						{
							for(int y = 0; y < 4; y++)
							{
								hexBlock[y] = block[x + y];
							}

							//This calculates the absolute location of the block based on the relative location inside the indirect block
							rBlock = hexToint(hexBlock, 4) + 256;
							offset = rBlock * blockSize;
							
							//This checks if the current four bytes is zero to exit the indirect block
							if(hexToint(hexBlock, 4) == 0)
							{
								eof = true;
								break;
							}
							
							//This checks if the next block is zero dictate whether or not we need to trim the end of the block 
							for(int z = 4; z < 8; z++)
							{
								hexBlock[z - 4] = block[x + z];
							}
							nextBlock = hexToint(hexBlock, 4);
						
							//This either trims the block to the EOF markers or writes the entire block to the new file
							if(nextBlock == 0)
							{
								printf("Writing last indirect block...\n");
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

//This function is used write the last block in the indirect block with the trimmed ending and not having trailing zeros 
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