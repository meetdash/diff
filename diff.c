#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>

#define MAX 128

void usage(void) {
	printf("DIFF Implementation\nThis programs compares two files.\n");
	printf("The command line address should be: './diff filename1 filename2' \n");
}

int icomp (const void *a, const void *b) {
	return ( *(int *)a - *(int *)b );
}

const int MOD_CS = 65521;

typedef struct file {
	uint32_t cksum;
	int line;
}file;

uint32_t checksum (char *line, size_t len) {
	uint32_t a = 1, b = 0;
	size_t index;

	for (index = 0; index < len; index++) {
		a = (a + line[index]) % MOD_CS;
		b = (b + a) % MOD_CS;
	}

	return (b << 16) | a;
}

void merge(file *a, int i1, int j1, int i2, int j2);
void mergesort(file *a, int i, int j) {
	int mid;

	if(i < j) {
		mid = (i + j) / 2;
		mergesort(a, i, mid);
		mergesort(a, mid + 1, j);
		merge(a, i, mid, mid + 1, j);
	}
}

void merge(file *a, int i1, int j1, int i2, int j2) {
	file *temp;
	int i, j, k;
	i = i1;
	j = i2;
	k = 0;

	temp = (file *)malloc(sizeof(file) * j2 + 1);
	if (temp == NULL) {
		printf("Malloc Failed.\n");
		exit(1);
	}

	while(i <= j1 && j <= j2)
		if(a[i].cksum < a[j].cksum) {
			temp[k].cksum = a[i].cksum;
			temp[k++].line = a[i++].line;
		} else {
			temp[k].cksum = a[j].cksum;
			temp[k++].line = a[j++].line;
		}

	while(i <= j1) {
		temp[k].cksum = a[i].cksum;
		temp[k++].line = a[i++].line;
	}

	while(j <= j2) {
		temp[k].cksum = a[j].cksum;
		temp[k++].line = a[j++].line;
	}

	for(i = i1, j = 0; i <= j2; i++, j++) {
		a[i].cksum = temp[j].cksum;		
		a[i].line = temp[j].line;		
	}

	free(temp);
}

typedef struct track {
	file *t1, *t2;
}track;

void t_init(track *t, int len) {
	int l;
	for (l = 0; l < len; l++)
		t[l].t2 = NULL;
	return;
}

track *rmdif (file *cs1, file *cs2, int size_cs1, int size_cs2) {
	int k, first, last, mid;
	file *a;

	k = 0;
	a = cs1;
	first = 0;
	last = size_cs2 - 1;
	mid = (first + last) / 2;

	track *dif;
	dif = (track *)malloc(sizeof(track) * size_cs1);
	if (dif == NULL) {
		printf("Malloc Failed.\n");
		exit(1);
	}
	t_init(dif, size_cs1);

	while (k < size_cs1) { 
		dif[k].t1 = a;
		while (first <= last) {
			if (cs2[mid].cksum < dif[k].t1->cksum)
				first = mid + 1;
			else if (cs2[mid].cksum == dif[k].t1->cksum) {
					dif[k].t2 = &cs2[mid];
					break;
				} else
					last = mid - 1;
			mid = (first + last) / 2;
		}
		a++;
		k++;
		first = 0;
		last = size_cs2 - 1;
		mid = (first + last) / 2;
	}

	return dif;
}

int *rmdifarray(track *dif, int *len) {
	int i, k, *arr;

	arr = (int *)malloc(sizeof(int) * (*len));
	if (arr == NULL) {
		printf("Malloc Failed.\n");
		exit(1);
	}

	k = 0; i = 0;
	while (k < *len) {
		if (dif[k].t2 == NULL) {
			if (dif[k].t1->cksum == 1) {
				k++;
				continue;
			}
			arr[i] = dif[k].t1->line;
			i++;
		}
		k++;
	}
	*len = i;

	return arr;
}

int main(int argc, char *argv[]) {

	if (strcmp(argv[1], "-h") == 0) {
		usage();
		return 0;
	} else {
// program 

	int i, j, size, size_f, count, len1, len2, *rmarr1, *rmarr2, n1, n2;
	char *line, *p;
	char line_char;
	size_t p_size;
	file *file1, *file2;
	FILE *fd1, *fd2, *fcs1, *fcs2;
	track *t_dif1, *t_dif2;

	size = 20;
	size_f = 10;
	line_char = '\0';
	p = NULL;
	p_size = 0;
	file1 = (file *)malloc(sizeof(file) * size_f);
	file2 = (file *)malloc(sizeof(file) * size_f);

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
		file1[j].cksum = checksum(line, i);
		file1[j].line = j + 1;
/*		if (file1[j].cksum == 1)
			file1[j].cksum += j;
*/		j++;
		if (j == size_f) {
			size_f *= 2;
			file1 = (file *)realloc(file1, sizeof(file) * size_f);
		}
	}
	len1 = j - 1;
	mergesort(file1, 0, len1 - 1);

	fcs1 = fopen("checksum_file1.txt", "w");
	if (fcs1 == NULL) {
		perror("Couldn't write checksum of file1: ");
		return errno;
	}

	for (i = 0; i < len1; i++)
		fprintf(fcs1, "%d %" PRIu32 "\n", file1[i].line, file1[i].cksum);

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
		file2[j].cksum = checksum(line, i);
		file2[j].line = j + 1;
/*		if (file2[j].cksum == 1)
			file2[j].cksum += j;
*/		j++;
		if (j == size_f) {
			size_f *= 2;
			file2 = (file *)realloc(file2, sizeof(file) * size_f);
		}
	}
	len2 = j - 1;
	mergesort(file2, 0, len2 - 1);

	fcs2 = fopen("checksum_file2.txt", "w");
	if (fcs2 == NULL) {
		perror("Couldn't write checksum of file2: ");
		return errno;
	}
	for (i = 0; i < len2; i++)
		fprintf(fcs2, "%d %" PRIu32 "\n", file2[i].line, file2[i].cksum);

	t_dif1 = rmdif(file1, file2, len1, len2);
	n1 = len1;
	rmarr1 = rmdifarray(t_dif1, &n1);
	qsort(rmarr1, n1, sizeof(int), icomp);

	t_dif2 = rmdif(file2, file1, len2, len1);
	n2 = len2;
	rmarr2 = rmdifarray(t_dif2, &n2);
	qsort(rmarr2, n2, sizeof(int), icomp);

	i = 0; count = 1;
	rewind(fd1);
	while (i < n1) {
		while (!feof(fd1)) {			
			getline(&p, &p_size, fd1);
			if (count == rmarr1[i]) 
				printf("- %s", p);
			count++;
			if (count > rmarr1[i])
				break;
		}
		i++;
	}

	i = 0; count = 1;
	rewind(fd2);
	while (i != n2) {
		while (!feof(fd2)) {
			getline(&p, &p_size, fd2);
			if (count == rmarr2[i])
				printf("+ %s", p);
			count++;
			if (count > rmarr2[i])
				break;
		}
		i++;
	}

	free(file1);
	free(file2);
	free(line);
	free(t_dif1);
	free(t_dif2);
	free(rmarr1);
	free(rmarr2);
	fclose(fd1);
	fclose(fd2);
	fclose(fcs1);
	fclose(fcs2);

	return 0;

	} // end else
}
