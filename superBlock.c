#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "superBlock.h"

//Driver function
void print_info(int fd)
{
    getPartAddr(fd);
    getSuperBlkAddr(fd, 0);
    getSuperBlkAddr(fd, 3);
}

//This function is used to get the block size of the file based on the values found in the zero'th superblock
int getBlockSize(int fd)
{
    int SB_Addr, Size_Field;
    unsigned char block_Size_Field[4];

    getPartAddr(fd);

    SB_Addr = lseek(fd, 0x400, SEEK_CUR);
    
    lseek(fd, SB_Addr, SEEK_SET);
    lseek(fd, 0x18, SEEK_CUR);
    read(fd, block_Size_Field, 4);

    Size_Field = hexToint(block_Size_Field, 4);

    return pow(2, (10 + Size_Field));
}

//Used to get the addresses of the partitions in the MBR
void getPartAddr(int fd)
{
    unsigned char MBR_sig[2];
    unsigned char LBA[4][4];
    int LBA_value_dec[4];
    int part_Addr[4];

    lseek(fd, 510, SEEK_SET);
    read(fd, MBR_sig, 2);

    //Security check for if the MBR is corrupt
    if (MBR_sig[0] != 0x55 && MBR_sig[1] != 0xAA)
    {
        printf("This device MBR is corrupt\n");
        return;
    }

    //This gets all of the addresses by aggregating all of the LBA values
    for (int x = 0; x < 4; x++)
    {
        lseek(fd, 0x1C6 + (x * 16), SEEK_SET);
        read(fd, LBA[x], 4);
        
        LBA_value_dec[x] = LBA[x][3];

        LBA_value_dec[x] = LBA_value_dec[x] << 8 | LBA[x][2];
        LBA_value_dec[x] = LBA_value_dec[x] << 8 | LBA[x][1];
        LBA_value_dec[x] = LBA_value_dec[x] << 8 | LBA[x][0];

        part_Addr[x] = LBA_value_dec[x] * 512;
    }

    //Below this can be updated for when other partitions tables are 
    //being looked for, but for the time being this assignment calls 
    //for only the super blocks of the first partition

    /* This code is not needed
    for (int x = 0; x < 4; x++)
    {
        if (part_Addr[x] != 0)
        {
            printf("\nPartition %d LBA:           0x%X(hex) or %d(dec)\n", x, LBA_value_dec[x], LBA_value_dec[x]);
            printf("Partition %d start address: 0x%X(hex) or %d(dec)\n\n", x, part_Addr[x], part_Addr[x]);
        }
    }
    */

    lseek(fd, part_Addr[0], SEEK_SET);
}

//This functions finds the corresponding superblock based 
//on the given block group number referened as blockGnum
void getSuperBlkAddr(int fd, int blockGnum)
{   
    int SB_Addr,
        M_Signature,
        Size_Field,
        B_Per_Group,
        B_Number,
        BG_Size;

    unsigned char magic_Sig[2],
                  block_Size_Field[4],
                  block_Per[4],
                  block_Num[2];

    //If this is the first block group, move past the buffer of zeros
    if (blockGnum == 0)
    {
        SB_Addr = lseek(fd, 0x400, SEEK_CUR);
    }
    //else, continue processing based on the given size 
    //of block groups and the given block group number
    else 
    {
        SB_Addr = lseek(fd, 0, SEEK_CUR);

        lseek(fd, 0x18, SEEK_CUR);
        read(fd, block_Size_Field, 4);

        lseek(fd, SB_Addr, SEEK_SET);
        lseek(fd, 0x20, SEEK_CUR);
        read(fd, block_Per, 4);

        Size_Field = hexToint(block_Size_Field, 4);
        B_Per_Group = hexToint(block_Per, 4);

        BG_Size = B_Per_Group * pow(2.0, (10 + Size_Field));

        SB_Addr = lseek(fd, SB_Addr - 1024 + (BG_Size * blockGnum), SEEK_SET);
    }

    //Each cluster will reset to the start of the block group, 
    //then move based on the offset to find the needed values
    lseek(fd, SB_Addr, SEEK_SET);
    lseek(fd, 0x38, SEEK_CUR);
    read(fd, magic_Sig, 2);

    lseek(fd, SB_Addr, SEEK_SET);
    lseek(fd, 0x18, SEEK_CUR);
    read(fd, block_Size_Field, 4);

    lseek(fd, SB_Addr, SEEK_SET);
    lseek(fd, 0x20, SEEK_CUR);
    read(fd, block_Per, 4);

    lseek(fd, SB_Addr, SEEK_SET);
    lseek(fd, 0x5A, SEEK_CUR);
    read(fd, block_Num, 2);

    lseek(fd, SB_Addr, SEEK_SET);

    //This clusters all of the conversions of hex 
    //numbers in unsigned char arrays into integers 
    //that are easily usable
    M_Signature = hexToint(magic_Sig, 2);
    Size_Field = hexToint(block_Size_Field, 4);
    B_Per_Group = hexToint(block_Per, 4);
    B_Number = hexToint(block_Num, 2);

    //This is a security check just in case something went wrong
    //and we check if the number of blocks in a block group matches 
    //the value given in the block group offset 0x5A
    if ((int)pow(2, 10 + Size_Field) * 8 != B_Per_Group)
    {
        printf("Something is wrong, the superblock is corrupt,");
        printf(" the number of blocks at the superblock offset ");
        printf("0x20 does not equal the calculated number of blocks\n");
        return;
    }

    //This cluster groups all of the print statments into one place
    printf("Superblock address: 0x%X(hex) or %d(dec)\n", SB_Addr, SB_Addr);
    printf("Block Size Field:   0x%X(hex) or %d(dec)\n", Size_Field, Size_Field);
    printf("Blocks per Group:   0x%X(hex) or %d(dec)\n", B_Per_Group, B_Per_Group);
    printf("Block Group #:      0x%X(hex) or %d(dec)\n\n", B_Number, B_Number);
}

//A funcition used to convert unsigned char arrays into easily usable integers
int hexToint(unsigned char hex[], int length)
{
    int return_Value;

    return_Value = hex[length - 1];

    for (int x = length - 2; x >= 0; x--)
    {
        return_Value = return_Value << 8 | hex[x];
    }
    
    return return_Value;
}

