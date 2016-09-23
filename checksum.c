#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>

#define MAX 128

const int MOD_CS = 65521;

uint32_t checksum_eval (char *line, size_t len) {
	uint32_t a = 1, b = 0;
	size_t index;

	for (index = 0; index < len; ++index) {
		a = (a + line[index]) % MOD_CS;
		b = (b + a) % MOD_CS;
	}

	return (b << 16) | a;
}

int main(int argc, char *argv[]) {
	int i, j, size, size_c;
	char *line;
	char line_char;
	uint32_t *cksum1, *cksum2;
	FILE *fd1, *fd2, *fcs1, *fcs2;

	size = 20;
	size_c = 10;
	line_char = '\0';
	cksum1 = (uint32_t *)malloc(sizeof(uint32_t) * size_c);
	cksum2 = (uint32_t *)malloc(sizeof(uint32_t) * size_c);

	if (argc != 3) {
		printf("Address of the type: ./program file1 file2\n");
		return EINVAL;
	}

	fd1 = fopen(argv[1], "r");
	if (fd1 == NULL) {
		perror("Can't open file1: ");
		return errno;
	}

	fd2 = fopen(argv[2], "r");
	if (fd2 == NULL) {
		perror("Can't open file2: ");
		return errno;
	}

	line = (char *)malloc(sizeof(char) * size);
	j = 0;
	while (!feof(fd1)) {
		i = 0;
		fread(&line_char, 1, 1, fd1);
		while (line_char != '\n') {
			line[i] = line_char;
			i++;
			if (i == size) {
				size *= 2;
				line = (char *)realloc(line, sizeof(char) * size);
			}
			fread(&line_char, 1, 1, fd1);
		}
		line[i] = '\0';
		cksum1[j] = checksum_eval(line, i);
		j++;
		if (j == size_c) {
			size_c *= 2;
			cksum1 = (uint32_t *)realloc(cksum1, sizeof(uint32_t) * size_c);
		}
	}

	fcs1 = fopen("checksum_file1.txt", "w");
	if (fcs1 == NULL) {
		perror("Couldn't write checksum of file1: ");
		return errno;
	}
	for (i = 0; i < j; i++)
		fprintf(fcs1, "%" PRIu32 "\n", cksum1[i]);

	j = 0;
	while (!feof(fd2)) {
		i = 0;
		fread(&line_char, 1, 1, fd2);
		while (line_char != '\n') {
			line[i] = line_char;
			i++;
			if (i == size) {
				size *= 2;
				line = (char *)realloc(line, sizeof(char) * size);
			}
			fread(&line_char, 1, 1, fd2);
		}
		line[i] = '\0';
		cksum2[j] = checksum_eval(line, i);
		j++;
		if (j == size_c) {
			size_c *= 2;
			cksum2 = (uint32_t *)realloc(cksum2, sizeof(uint32_t) * size_c);
		}
	}

	fcs2 = fopen("checksum_file2.txt", "w");
	if (fcs2 == NULL) {
		perror("Couldn't write checksum of file2: ");
		return errno;
	}
	for (i = 0; i < j; i++)
		fprintf(fcs2, "%" PRIu32 "\n", cksum2[i]);

	free(cksum1);
	free(cksum2);
	free(line);
	fclose(fd1);
	fclose(fd2);
	fclose(fcs1);
	fclose(fcs2);
	return 0;
}
