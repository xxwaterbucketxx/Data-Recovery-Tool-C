#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "sigSearch.h"
#include "superBlock.h"

int getHead(int fd)
{    
    int blockSize = getBlockSize(fd);
    // Block size
    unsigned char block[blockSize];
    
    // GIF header signature
    unsigned char gif87aSig[6] = {0x47, 0x49, 0x46, 0x38, 0x37, 0x61};
    unsigned char gif89aSig[6] = {0x47, 0x49, 0x46, 0x38, 0x39, 0x61};
    
    //PDF header signature
    unsigned char pdfSig[5] = {0x25, 0x50, 0x44, 0x46, 0x2D};

    // Initialize block number
    int block_number = 0;

    lseek(fd, 0, SEEK_SET);

    // Reads in file partition for each block
    while(read(fd, block, blockSize) > 0)
    {
        // For every 6 bytes it checks to see if the bytes matches with the GIF header signature
        for(int x = 0; x < 6; x++)
        {
            if(!(block[x] == gif87aSig[x] || block[x] == gif89aSig[x]))
                break;
            
            if (x == 5)
            {
                printf("Match found for GIF87a or GIF89a at the relative block #%d\n", block_number - 256);
                return block_number;
            }
        }

        // For every 5 bytes it checks to see if the bytes matches with the PDF header signature
        for(int y = 0; y < 5; y++)
        {
            if(block[y] != pdfSig[y])
                break;

            if (y == 4)
            {
                printf("Match found for PDF at the relative block #%d\n", block_number - 256);
                return block_number;
            }
        }
        // Iterates the block number
        block_number++;
    }
    
    return block_number;
}