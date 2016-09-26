#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>

#define MAX 128

const int MOD_CS = 65521;

uint32_t checksum (char *line, size_t len) {
	uint32_t a = 1, b = 0;
	size_t index;

	for (index = 0; index < len; ++index) {
		a = (a + line[index]) % MOD_CS;
		b = (b + a) % MOD_CS;
	}

	return (b << 16) | a;
}

typedef struct track {
	uint32_t *t1, *t2;
}track;

uint32_t INIT = 0;

void t_init(track *t, int len) {
	int l;
	for (l = 0; l < len; l++)
		t[l].t2 = &INIT;
	return;
}

track *diff (uint32_t *cs1, uint32_t *cs2, int size_cs1, int size_cs2) {
	int i, k;
	uint32_t *a, *b;

	i = k = 0;
	a = cs1;
	b = cs2;

	track *dif;
	dif = (track *)malloc(sizeof(track) * size_cs1);
	if (dif == NULL) {
		printf("Malloc Failed.\n");
		exit(1);
	}
	t_init(dif, size_cs1);

	while (k < size_cs1) {
		dif[k].t1 = a;
		i = 0;
		while (i < size_cs2) { 
			if (*b == *a) {
				dif[k].t2 = b;
				break;
			} else if (i == size_cs1 - 1)
					break;
				else
					b++;
			i++;
		}
		a++;
		k++;
		b = cs2;
	}

	return dif;

}

int *diffarray(track *dif, int len) {
	int j, k, *array;
	array = (int *)malloc(sizeof(int) * len);

	k = 0; j = 0;
	while (k < len) {
		if (*dif[k].t2 == 0) {
			array[j] = k;
			j++;
		}
		k++;
	}
	array[j] = INT_MAX;

	return array;
}

int main(int argc, char *argv[]) {
	int i, j, z, size, size_c, len1, len2, *arr_dif;
	char *line;
	char line_char;
	uint32_t *cksum1, *cksum2;
	FILE *fd1, *fd2, *fcs1, *fcs2;
	track *t_dif;

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
		cksum1[j] = checksum(line, i);
		j++;
		if (j == size_c) {
			size_c *= 2;
			cksum1 = (uint32_t *)realloc(cksum1, sizeof(uint32_t) * size_c);
		}
	}
	len1 = j;

	fcs1 = fopen("checksum_file1.txt", "w");
	if (fcs1 == NULL) {
		perror("Couldn't write checksum of file1: ");
		return errno;
	}
	for (i = 0; i < len1; i++)
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
		cksum2[j] = checksum(line, i);
		j++;
		if (j == size_c) {
			size_c *= 2;
			cksum2 = (uint32_t *)realloc(cksum2, sizeof(uint32_t) * size_c);
		}
	}
	len2 = j;

	fcs2 = fopen("checksum_file2.txt", "w");
	if (fcs2 == NULL) {
		perror("Couldn't write checksum of file2: ");
		return errno;
	}
	for (i = 0; i < len2; i++)
		fprintf(fcs2, "%" PRIu32 "\n", cksum2[i]);

	t_dif = diff(cksum1, cksum2, len1, len2);
	arr_dif = diffarray(t_dif, len1);

	i = 0;
	while (arr_dif[i] != INT_MAX) {
		z = arr_dif[i];
		printf("- line %d\n", z + 1);
		i++;
	}

	free(cksum1);
	free(cksum2);
	free(line);
	free(t_dif);
	free(arr_dif);
	fclose(fd1);
	fclose(fd2);
	fclose(fcs1);
	fclose(fcs2);

	return 0;
}
