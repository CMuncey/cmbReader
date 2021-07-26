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
/* File list is file's index in total list of subfiles */
typedef struct
{
	uint32_t nFiles;
	uint32_t flOffs;
	uint32_t tnOffs;
	uint32_t   ffff;
} filetype_t;

/* Data about each file in the archive */
/* There are header_t nFiles of these */
typedef struct
{
	uint32_t   size;
	uint32_t fnOffs;
} metadata_t;

const char* fixFN(char* s)
{
	int i;

	for(i = strlen(s) - 1; i >= 0; --i)
		if(s[i] == '\\')
			return(s + i + 1);
	return("");
}

int main(int argc, char** argv)
{
	header_t* head;
	filetype_t* fts;
	metadata_t* mds;
//	const char* fn;
//	int l;
	int i, j, k;
	uint8_t* data;
	FILE* f;

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

	if(*(uint32_t*)(head->magic) != *(uint32_t*)("ZAR\x01"))
	{
		fprintf(stderr, "Error: ZAR magic number does not match\n");
		fprintf(stderr, "\"ZAR\x91\" vs \"%.4s\"\n", head->magic);
		return(1);
	}

	fts  = (filetype_t*) (data + head->ftOffs);
	mds  = (metadata_t*) (data + head->fmOffs);

	/* Print out the data */
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

	/* Print file types info */
//	for(i = 0; i < head->nTypes; ++i)
//	{
//		printf("Filetype %d\n", i);
//		printf("\tNum files: %d\n", fts[i].nFiles);
//		printf("\tFile list offset: 0x%08x\n", fts[i].flOffs);
//		printf("\tType name offset: 0x%08x\n", fts[i].tnOffs);
//		printf("\tType name: %s\n", data + fts[i].tnOffs);
//		printf("\n");
//	}

	for(i = 0; i < head->nTypes; ++i)
	{
		if(fts[i].nFiles)
		{
			printf("%d %s files\n", fts[i].nFiles, data + fts[i].tnOffs);
			for(j = 0; j < fts[i].nFiles; ++j)
			{
				/* Get the index into the file list */
				k = *(int*)(data + fts[i].flOffs + (j << 2));
				printf("\t%3d: %s (%d bytes)", j, data + mds[k].fnOffs, mds[k].size);

				/* Dump the desired filetypes */
				/* This isn't very good, but that's fine */
//				if(strncmp((char*)(data + fts[i].tnOffs), "ctxb", 5) == 0)
//				{
//					printf("... Dumping");
//					fn = fixFN((char*)(data + mds[k].fnOffs));
//					f = fopen(fn, "wb");
//					l = *(int*)(data + head->datOffs + (k << 2));
//					fwrite(data + l, mds[k].size, 1, f);
//					fclose(f);
//				}

				printf("\n");
			}
			printf("\n");
		}
	}

	return(0);
}
