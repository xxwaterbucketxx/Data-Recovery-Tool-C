#ifndef RECOVERINGFILE_H_
#define RECOVERINGFILE_H_

void writeOutput(int fd, int outfd, unsigned char block[], int blockSize, unsigned char sig[], int blockCounter);
void writeLastBlock(int fd, int outfd, int offset, unsigned char dataBlock[], int blockSize);

#endif //RECOVERINGFILE_H_