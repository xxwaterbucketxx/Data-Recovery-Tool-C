#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Invalid parameters. Run the program 'sudo ./programname device name'\n");
		exit(EXIT_FAILURE);
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("Open error");
		exit(EXIT_FAILURE);
	}

	print_info(fd);

	return 0;
}

