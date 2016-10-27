#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>

#define MAX 128
#define MALFAIL(a) if (a == NULL) { \
			printf("Malloc Failed.\n"); \
			exit(1); \
		}

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
	MALFAIL(temp);

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
	int *visit1, *visit2;
}track;

void t_init (track t, int s1, int s2) {
	int l;

	for (l = 0; l < s1; l++)
		t.visit1[l] = 0;

	for (l = 0; l < s2; l++)
		t.visit2[l] = 0;

	return;
}

track diff (file *cs1, file *cs2, int size_cs1, int size_cs2) {
	int k, i;

	track dif;
	dif.visit1 = (int *)malloc(sizeof(int) * size_cs1);
	MALFAIL(dif.visit1);
	dif.visit2 = (int *)malloc(sizeof(int) * size_cs2);
	MALFAIL(dif.visit2);

	t_init(dif, size_cs1, size_cs2);

	i = 0; k = 0;
	while (k < size_cs1) {
		while (i < size_cs2) {
			if (cs2[i].cksum == cs1[k].cksum) { //found
				dif.visit2[i] = 1;
				dif.visit1[k] = 1;
				break;
			} else
				i++;
		} //not found
		k++;
		i = 0;
	}

	while (i < size_cs2) { //ignoring blank lines of file2
		if (cs2[i].cksum == 1)
			dif.visit2[i] = 1;
		i++;
	}

	return dif;
}
// ----------- start of ndiff functions

int SIZE = 8;

typedef struct ndata {
	char ch;
	int lnum;
	struct ndata *next;
}ndata;

ndata **ndiff;

void ninit () {
	ndiff = (ndata **)malloc(sizeof(struct ndata *) * SIZE);
	MALFAIL(ndiff);
	int i;
	for (i = 0; i < SIZE; i++)
		ndiff[i] = NULL;
}

void nstore (ndata d, int i) {
	ndata *tmp, *p;

	tmp = (ndata *)malloc(sizeof(ndata));
	MALFAIL(tmp);
	*tmp = d;

	p = ndiff[i];
	if (p == NULL)
		ndiff[i] = tmp;
	else {
		while (p->next != NULL)
			p = p->next;
		p->next = tmp;
	}

	return;
}

void ndiffsort (track dif, int l1, int l2) {
	int i, j, k;
	ndata d;
	ninit();

	k = 0; i = 0; j = 0;
	while (i < l1 || j < l2) {
		if (dif.visit1[i] == 0) {
			if (dif.visit2[j] == 1) {
				while (dif.visit1[i] == 0) {
					d.lnum = i + 1;
					d.ch = '-';
					d.next = NULL;
					nstore(d, k);
					i++;
				}
				k++;
			} else {
				while (dif.visit1[i] == 0) {
					d.lnum = i + 1;
					d.ch = '-';
					d.next = NULL;
					nstore(d, k);
					i++;
				}
				while (dif.visit2[j] == 0) {
					d.lnum = j + 1;
					d.ch = '+';
					d.next = NULL;
					nstore(d, k);
					j++;
				}
				k++;
			}
		} else {
			if (dif.visit2[j] == 0) {
				while (dif.visit2[j] == 0) {
					d.lnum = j + 1;
					d.ch = '+';
					d.next = NULL;
					nstore(d, k);
					j++;
				}
				k++;
			}
		}
		i++;
		j++;
		if (k == SIZE) {
			SIZE *= 2;
			ndiff = (ndata **)realloc(ndiff, sizeof(ndata *) * SIZE);
		}
	}

	if (j < l2) {
		if (dif.visit2[j] == 1)
			j++;
		else {
			while (dif.visit2[j] == 0) {
				d.lnum = j + 1;
				d.ch = '+';
				d.next = NULL;
				nstore(d, k);
				j++;
				if (j == l2)
					break;
			}
			k++;
			if (k == SIZE) {
				SIZE *= 2;
				ndiff = (ndata **)realloc(ndiff, sizeof(ndata *) * SIZE);
			}
		}
	}

	if (i < l1) {
		if (dif.visit1[i] == 1)
			i++;
		else {
			while (dif.visit1[i] == 0) {
				d.lnum = i + 1;
				d.ch = '-';
				d.next = NULL;
				nstore(d, k);
				i++;
				if (i == l1)
					break;
			}
			k++;
			if (k == SIZE) {
				SIZE *= 2;
				ndiff = (ndata **)realloc(ndiff, sizeof(ndata *) * SIZE);
			}
		}
	}

}

void printndiff(char *rm[], char *ad[]) {
	ndata *p, *q;
	p = ndiff[0];
	int i, j, k;

	i = 0; j = 0; k = 0;
	while (p) {
		if (p->ch == '-') {
			if (p->next == NULL)
				printf("%dd\n< %s", p->lnum, rm[i++]);
			else {
				q = p;
				while (q->ch == '-')
					q = q->next;
				printf("%dc%d\n", p->lnum, q->lnum);
				while (p->ch == '-') {
					printf("< %s", rm[i++]);
					p = p->next;
				}
				printf("-----\n");
				while (p) {
					printf("> %s", ad[k++]);
					p = p->next;
				}
			}
		} else {
			printf("%da\n", p->lnum);
			while (p) {
				printf("> %s", ad[k++]);
					p = p->next;
			}
		}
		j++;
		p = ndiff[j];
	}

}
// ----------- end of ndiff functions

// ----------- start of udiff functions

typedef struct udata {
	char ch;
	char *from, *to;
	struct udata *next;
}udata;

udata **udiff;

void uinit() {
	udiff = (udata **)malloc(sizeof(udata *) * SIZE);
	MALFAIL(udiff);
	int i;
	for (i = 0; i < SIZE; i++) {
		udiff[i] = NULL;
	}
}

void ustore (udata d, int i) {
	udata *tmp, *p;

	tmp = (udata *)malloc(sizeof(udata));
	MALFAIL(tmp);
	*tmp = d;

	p = udiff[i];
	if (p == NULL)
		udiff[i] = tmp;
	else {
		while (p->next != NULL)
			p = p->next;
		p->next = tmp;
	}

}

void udiffsort () {


	


}

void printudiff() {






}

void printcdiff() {

}

int main(int argc, char *argv[]) {

	if (strcmp(argv[1], "-h") == 0) {
		usage();
		return 0;
	} else {
// program 

	int i, j, size, size_f, len1, len2, n1, n2;
	char *line, *p, **rmlines, **adlines;
	char line_char;
	size_t p_size;
	file *file1, *file2;
	FILE *fd1, *fd2, *fcs1, *fcs2;
	track dif;

	size = 20;
	size_f = 10;
	line_char = '\0';
	p = NULL;
	p_size = 0;
	file1 = (file *)malloc(sizeof(file) * size_f);
	MALFAIL(file1);
	file2 = (file *)malloc(sizeof(file) * size_f);
	MALFAIL(file2);

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
	MALFAIL(line);
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
//	mergesort(file1, 0, len1 - 1);

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
//	mergesort(file2, 0, len2 - 1);

	fcs2 = fopen("checksum_file2.txt", "w");
	if (fcs2 == NULL) {
		perror("Couldn't write checksum of file2: ");
		return errno;
	}
	for (i = 0; i < len2; i++)
		fprintf(fcs2, "%d %" PRIu32 "\n", file2[i].line, file2[i].cksum);

	dif = diff (file1, file2, len1, len2);

	rmlines = (char **)malloc(sizeof(char *) * size_f);
	MALFAIL(rmlines);
	i = 0; j = 0;
	rewind(fd1);
	while (!feof(fd1)) {			
		getline(&p, &p_size, fd1);
		if (dif.visit1[i] == 0) {
			rmlines[j] = (char *)malloc(sizeof(char) * p_size);
			MALFAIL(rmlines[j]);
			strcpy(rmlines[j], p);
			j++;
			if (j == size_f) {
				size_f *= 2;
				rmlines = (char **)realloc(rmlines, sizeof(char *) * size_f);
			}
		}
		i++;
	}
	n1 = j;

	adlines = (char **)malloc(sizeof(char *) * size_f);
	MALFAIL(adlines);
	i = 0; j = 0;
	rewind(fd2);
	while (!feof(fd2)) {			
		getline(&p, &p_size, fd2);
		if (dif.visit2[i] == 0) {
			adlines[j] = (char *)malloc(sizeof(char) * p_size);
			MALFAIL(adlines[j]);
			strcpy(adlines[j], p);
			j++;
			if (j == size_f) {
				size_f *= 2;
				adlines = (char **)realloc(adlines, sizeof(char *) * size_f);
			}
		}
		i++;
	}
	n2 = j;

	ndiffsort(dif, len1, len2);
	printndiff(rmlines, adlines);

	for (i = 0; i < n1; i++)
		free(rmlines[i]);
	for (i = 0; i < n2; i++)
		free(adlines[i]);
	free(rmlines);
	free(adlines);
	free(file1);
	free(file2);
	free(line);
	free(dif.visit1);
	free(dif.visit2);
	fclose(fd1);
	fclose(fd2);
	fclose(fcs1);
	fclose(fcs2);

	return 0;

	} // end else
}
