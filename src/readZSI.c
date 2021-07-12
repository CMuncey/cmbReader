#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* https://github.com/xdanieldzd/N3DSCmbViewer */

typedef struct
{
	char      magic[4];
	char  codename[12];
} header_t;

/* Slightly related to oot64 scene header flags? But not very */
/* https://wiki.cloudmodding.com/oot/Scenes_and_Rooms#Header_Commands */
typedef struct
{
	uint8_t cmd;
	uint8_t  d1;
	uint32_t d2;
} sceneFlag_t;

/* Just like the actor list from oot3d */
typedef struct
{
	uint16_t num;
	int16_t   pX;
	int16_t   pY;
	int16_t   pZ;
	int16_t   rX;
	int16_t   rY;
	int16_t   rZ;
	uint16_t var;
} actor_t;

typedef struct
{
	uint8_t   type;
	uint8_t  count;
	uint32_t start;
	uint32_t   end;
} mesh_t;

int main(int argc, char** argv)
{
	mesh_t mesh;
	header_t* head;
	sceneFlag_t* sflags;
	actor_t* actors;
	int i, numActors, cmbSize;
	uint8_t* data, *cmb;
	char fn[21];
	FILE* f;

	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s filename.zsi\n", argv[0]);
		exit(1);
	}

	f = fopen(argv[1], "rb");
	if(f == NULL)
	{
		perror(argv[1]);
		exit(1);
	}

	/* Read the entire file into memory because they're small */
	fseek(f, 0, SEEK_END);
	i = ftell(f);
	fseek(f, 0, SEEK_SET);
	data = malloc(i);
	fread(data, 1, i, f);
	fclose(f);

	/* Pointer magic to set header and scene flags */
	/* Add 0x10 to data because all offsets are after the header */
	head = (header_t*)(data);
	data += 0x10;
	sflags = (sceneFlag_t*)(data);

	/* Make sure the magic number is good */
	if(*(int*)head->magic != *(int*)("ZSI\x01"))
	{
		fprintf(stderr, "Error: %s not a valid ZSI file\n", argv[1]);
		fprintf(stderr, "Magic number does not match\n");
		exit(1);
	}

	/* Read the scene flags */
	for(i = 0; sflags[i].cmd != 0x14; ++i)
	{
		switch(sflags[i].cmd)
		{
			case 0x0A:
				//TODO
				/* Won't work with *_info.zsi, just the numbered ones */
				mesh = *(mesh_t*)(data + sflags[i].d2);
				break;

			case 0x01:
				actors = (actor_t*)(data + sflags[i].d2);
				numActors = sflags[i].d1;
				break;

			default:
				//printf("Scene flag 0x%02x not implemented\n", sflags[i].cmd);
		}
	}

	//printf("MESH type: %u\n", mesh.type);
	//printf("MESH count: %u\n", mesh.count);

	//TODO
	/* Get the cmb file */
	/* No numbered .zsi file should have both of these be non-zero */
	cmbSize = *(int*)(data + mesh.start + 8) + *(int*)(data + mesh.start + 12);
	cmb = data + cmbSize;
	cmbSize = mesh.end - cmbSize;

	printf("%s cmb: %.16s.cmb\n", argv[1], cmb + 0x10);

	strncpy(fn, cmb + 0x10, 16);
	strncat(fn, ".cmb", 5);

	f = fopen(fn, "wb");
	fwrite(cmb, 1, cmbSize, f);
	fclose(f);

	/* I have no regrets */
	free(data - 0x10);

	return(0);
}

