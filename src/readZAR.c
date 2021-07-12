#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Header of the ZAR file */
/* Only one of these */
typedef struct
{
	char    magic[4];
	uint32_t    size;
	uint16_t  nTypes;
	uint16_t  nFiles;
	uint32_t  ftOffs;
	uint32_t  fmOffs;
	uint32_t datOffs;
	char    queen[8];
} header_t;

/* The types of files found in the ZAR */
/* There are header_t.nTypes of these */
typedef struct
{
	uint32_t nFiles;
	uint32_t flOffs;
	uint32_t tnOffs;
	uint32_t    xxx;
} filetype_t;

/* Data about each file in the archive */
/* There are header_t nFiles of these */
typedef struct
{
	uint32_t   size;
	uint32_t fnOffs;
} metadata_t;

/* This is bad */
char* fixFN(char* s)
{
	int i;

	for(i = strlen(s) - 1; i >= 0; --i)
		if(s[i] == '/')
			return(s + i + 1);
}

int main(int argc, char** argv)
{
	header_t* head;
	filetype_t* fts;
	metadata_t* mds;
	uint8_t* data;
	char* fn;
	FILE* f;
	int i, j, k, l;

	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s filename.zar\n", argv[0]);
		exit(1);
	}

	f = fopen(argv[1], "rb");
	if(!f)
	{
		perror(argv[1]);
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	i = ftell(f);
	fseek(f, 0, SEEK_SET);
	data = malloc(i);
	fread(data, 1, i, f);
	fclose(f);

	head = (header_t*) data;
	fts  = (filetype_t*) (data + head->ftOffs);
	mds  = (metadata_t*) (data + head->fmOffs);

	/* Print out the data
	printf("HEADER:\n");
	printf("\tMagic:           %.4s\n", head->magic);
	printf("\tSize:            %u\n", head->size);
	printf("\tNum Types:       %u\n", head->nTypes);
	printf("\tNum Files:       %u\n", head->nFiles);
	printf("\tFiletype offset: 0x%08x\n", head->ftOffs);
	printf("\tMetadata offset: 0x%08x\n", head->fmOffs);
	printf("\tData offset:     0x%08x\n", head->datOffs);
	printf("\tQueen:           %5s\n", head->queen);
	printf("\n");
	*/

	for(i = k = l = 0; i < head->nTypes; ++i)
	{
		if(fts[i].nFiles)
		{
			printf("%d %s files\n", fts[i].nFiles, data + fts[i].tnOffs);

			for(j = 1; j <= fts[i].nFiles; ++j, ++k)
			{
				printf("\t%3d: %s (%d bytes)\n", j, data + mds[k].fnOffs, mds[k].size);

				/* Dump the cmb files, I'll make a good one later maybe
				if(strncmp(data + fts[i].tnOffs, "cmb", 4) == 0)
				{
					fn = fixFN(data + mds[k].fnOffs);
					printf("\tDumping %s... ", fn);
					f = fopen(fn, "wb");
					l = *(int*)(data + head->datOffs + (k << 2));
					fwrite(data + l, mds[k].size, 1, f);
					fclose(f);
					printf("done\n");
				}
				*/
			}

			//printf("\n");
		}
	}


	free(data);
	return(0);
}
