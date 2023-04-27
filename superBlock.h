#ifndef SUPER_H_
#define SUPER_H_

void print_info(int fd);
void getPartAddr(int fd);
void getSuperBlkAddr(int fd, int blockNum);
int hexToint(unsigned char hex[], int size);
int getBlockSize(int fd);

#endif //SUPER_H_