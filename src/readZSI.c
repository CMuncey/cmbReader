#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <byteswap.h>
#include "zsi.h"

const char* headerStrs[32] = {
	"STARTING_POSITIONS",
	"ACTOR_LIST",
	"MM_CAMERA_SETTINGS",
	"COLLISION_HEADER",
	"ROOMS",
	"WIND",
	"ENTRANCE_LIST",
	"SPECIAL_OBJECTS",
	"ROOM_BEHAVIOR",
	"UNUSED",
	"MESH",
	"OBJECT_LIST",
	"UNUSED_LIGHTING",
	"PATHWAYS",
	"TRANSITION_ACTORS",
	"LIGHTING_SETTINGS",
	"TIME_SETTINGS",
	"SKYBOX_SETTINGS",
	"SKYBOX_MODIFIER",
	"EXIT_LIST",
	"END_MARKER",
	"SOUND_SETTINGS",
	"ECHO_SETTINGS",
	"CUTSCENES",
	"ALTERNATE_HEADERS",
	"CAMERA_SETTINGS"
};

int main(int argc, char** argv)
{
	zsiScene_t* zs;
	zsiHeader_t* head;
	sceneFlag_t sflag;
	int i, numScenes;
	uint32_t* offsets;
	uint8_t* data;
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

	/* Make sure the magic number is good */
	/* MM ZSI header is "ZSI\x09" */
	head = (zsiHeader_t*)(data);
	if(*(int*)head->magic != *(int*)("ZSI\x01"))
	{
		fprintf(stderr, "Error: %s not a valid ZSI file\n", argv[1]);
		fprintf(stderr, "Magic number does not match\n");
		exit(1);
	}

	/* Check to see if the first scene flag is alternate headers */
	sflag = *(sceneFlag_t*)(data + 0x10);
	if(sflag.cmd == ALTERNATE_HEADERS)
	{
		numScenes = sflag.d1;
		offsets = (uint32_t*)(data + 0x10 + sflag.d2);
		zs = malloc(sizeof(zsiScene_t) * numScenes);
	}
	else
	{
		numScenes = 1;
		zs = malloc(sizeof(zsiScene_t));
	}

	printf("File has %d scene setups:\n", numScenes);
	for(i = 0; i < numScenes; ++i)
		printf("\t0x%08x\n", offsets[i] + 0x10);
	printf("\n");

	/* Start reading the scene after the header */
	for(i = 0; i < numScenes; ++i)
	{
		readScene(data, &zs[i], offsets[i] + 0x10);

		/* Print the scene */
		printf("Scene %d (0x%08x):\n", i, offsets[i] + 0x10);
		printScene(&(zs[i]));
	}

	/* Clean up */
	free(zs);
	free(data);

	return(0);
}

void readScene(const uint8_t* data, zsiScene_t* zs, int offs)
{
	int i;
	sceneFlag_t* sflags;

	/* Set the initial scene flags pointer */
	sflags = (sceneFlag_t*)(data + offs);

	/* Read the scene flags */
	for(i = 0; sflags[i].cmd != END_MARKER; ++i)
	{
		switch(sflags[i].cmd)
		{
			case ALTERNATE_HEADERS:
				/* Only the first setup should have this command */
				/* and it's already been handled */
				//fprintf(stderr, "Skipping alternate heaeder command ");
				//fprintf(stderr, "at offset 0x%08x\n", offs);
				break;

			case SOUND_SETTINGS:
				zs->soundHeader = *(uint64_t*)(&sflags[i]);
				zs->sound = *(zsiSound_t*)(&(sflags[i]));
				break;

			case ROOMS:
				// TODO figure out how to handle MM eventually
				// The string sizes are smaller there (0x34 vs 0x44)
				zs->roomsHeader = *(uint64_t*)(&sflags[i]);
				zs->nRooms = sflags[i].d1;
				zs->rooms = (zsiRoom_t*)(data + 0x10 + sflags[i].d2);
				break;

			case TRANSITION_ACTORS:
				zs->doorsHeader = *(uint64_t*)(&sflags[i]);
				zs->nDoors = sflags[i].d1;
				zs->doors = (zsiTransActor_t*)(data + 0x10 + sflags[i].d2);
				break;

			case CAMERA_SETTINGS:
				zs->camHeader = *(uint64_t*)(&sflags[i]);
				zs->camSettings = *(zsiCamSettings_t*)(&sflags[i]);
				break;

			case COLLISION_HEADER:
				// TODO
				zs->collHeader = *(uint64_t*)(&sflags[i]);
				break;

			case ENTRANCE_LIST:
				zs->entranceHeader = *(uint64_t*)(&sflags[i]);
				zs->nEntrances = sflags[i].d1;
				zs->entrances = (zsiEntrance_t*)(data + 0x10 + sflags[i].d2);
				break;

			case SPECIAL_OBJECTS:
				zs->spcObjHeader = *(uint64_t*)(&sflags[i]);
				zs->spcObj = *(zsiSpecialObj_t*)(&sflags[i]);
				break;

			case PATHWAYS:
				fprintf(stderr, "Scene command 0x%02X (PATHWAYS) unsupported\n", sflags[i].cmd);
				break;

			case STARTING_POSITIONS:
				zs->startPosHeader = *(uint64_t*)(&sflags[i]);
				zs->nStartPos = sflags[i].d1;
				zs->startPos = (zsiActor_t*)(data + 0x10 + sflags[i].d2);
				break;

			case SKYBOX_SETTINGS:
				zs->skyboxHeader = *(uint64_t*)(&sflags[i]);
				zs->skybox = *(zsiSkybox_t*)(&sflags[i]);
				break;

			case EXIT_LIST:
				zs->exitsHeader = *(uint64_t*)(&sflags[i]);
				zs->exits = (uint16_t*)(data + 0x10 + sflags[i].d2);
				break;

			case LIGHTING_SETTINGS:
				zs->lightsHeader = *(uint64_t*)(&sflags[i]);
				zs->nLights = sflags[i].d1;
				zs->lights = (zsiLight_t*)(data + 0x10 + sflags[i].d2);
				break;

			case CUTSCENES:
				zs->cutsceneHeader = *(uint64_t*)(&sflags[i]);
				break;

			/* These shouldn't happen in scene setup */
			/* These are exclusive to room setups */
			case ACTOR_LIST:
			case MM_CAMERA_SETTINGS:
			case WIND:
			case ROOM_BEHAVIOR:
			case UNUSED:
			case MESH:
			case OBJECT_LIST:
			case UNUSED_LIGHTING:
			case TIME_SETTINGS:
			case SKYBOX_MODIFIER:
			case ECHO_SETTINGS:
				fprintf(stderr, "Warning: Scenes should not have command 0x");
				fprintf(stderr, "%02x (%s)\n", sflags[i].cmd, headerStrs[sflags[i].cmd]);
				break;

			default:
				fprintf(stderr, "Scene command 0x%02x unrecognised\n", sflags[i].cmd);
		}
	}
}

void printScene(const zsiScene_t* zs)
{
	int i;

	/* Print out the sound settings */
	printf("\tSound settings (0x%016lx):\n", bswap_64(zs->soundHeader));
	printf("\t\tConfig:    0x%02x\n", zs->sound.config);
	printf("\t\tNight sfx: 0x%02x\n", zs->sound.nightSFX);
	printf("\t\tSequence:  0x%02x\n", zs->sound.sequence);
	printf("\n");

	/* Print out the room filenames */
	printf("\tRooms (0x%016lx):\n", bswap_64(zs->roomsHeader));
	for(i = 0; i < zs->nRooms; ++i)
		printf("\t\t%d | %.68s\n", i, zs->rooms[i].fname);
	printf("\n");

	/* Print transition actors */
	printf("\tDoors (0x%016lx):\n", bswap_64(zs->doorsHeader));
	for(i = 0; i < zs->nDoors; ++i)
		printTransActor(&(zs->doors[i]), i);

	/* Print the Camera settings */
	printf("\tCamera settings (0x%016lx):\n", bswap_64(zs->camHeader));
	printf("\t\tCamera movement setting: 0x%02x\n", zs->camSettings.camMvmt);
	printf("\t\tWorld map location:      0x%02x\n", zs->camSettings.mapLoc);
	printf("\n");

	/* Print the collision data */
	printf("\tCollision data (0x%016lx):\n", bswap_64(zs->collHeader));
	// TODO
	printf("\n");

	/* Print the entrance list data */
	printf("\tEntrances (0x%016lx):\n", bswap_64(zs->entranceHeader));
	for(i = 0; i < zs->nEntrances; ++i)
	{
		printf("\t\t%d | Start position: %d\n", i, zs->entrances[i].pos);
		printf("\t\t%d | Room:           %d\n\n", i, zs->entrances[i].room);
	}

	/* Print the special objects */
	printf("\tSpecial objects (0x%016lx):\n", bswap_64(zs->spcObjHeader));
	printf("\t\tElf_message file: %d\n", zs->spcObj.elfMsgFile);
	printf("\t\tGlobal object file: %d\n", zs->spcObj.glblObjFile);
	printf("\n");

	/* Print the starting positions */
	printf("\tStarting positions (0x%016lx):\n", bswap_64(zs->startPosHeader));
	for(i = 0; i < zs->nStartPos; ++i)
		printActor(&(zs->startPos[i]), i);

	/* Print exit info stuff */
	printf("\tExits (0x%016lx):\n", bswap_64(zs->exitsHeader));
	// TODO use lighting offset to find size of table
	for(i = 0; i < 2; ++i)
		printf("\t\t%d | %d\n", i, zs->exits[i]);
	printf("\n");

	/* Print the lighting info */
	printf("\tLights (0x%016lx):\n", bswap_64(zs->lightsHeader));
	for(i = 0; i < zs->nLights; ++i)
		printLight(&(zs->lights[i]), i);

	printf("\tCutscene (0x%016lx):\n", bswap_64(zs->cutsceneHeader));
}

void printActor(const zsiActor_t* a, int n)
{
	printf("\t\t%d | Actor number:   0x%04x\n", n, a->num);
	printf("\t\t%d | X position:     %d\n", n, a->pX);
	printf("\t\t%d | Y position:     %d\n", n, a->pY);
	printf("\t\t%d | Z position:     %d\n", n, a->pZ);
	printf("\t\t%d | X rotation:     %d\n", n, a->rX);
	printf("\t\t%d | Y rotation:     %d\n", n, a->rY);
	printf("\t\t%d | Z rotation:     %d\n", n, a->rZ);
	printf("\t\t%d | Actor variable: 0x%04x\n\n", n, a->var);
}

void printTransActor(const zsiTransActor_t* a, int n)
{
	printf("\t\t%d | Front room index:  0x%02x\n", n, a->fRoom);
	printf("\t\t%d | Front room camera: 0x%02x\n", n, a->fCamSetup);
	printf("\t\t%d | Back room index:   0x%02x\n", n, a->bRoom);
	printf("\t\t%d | Back room camera:  0x%02x\n", n, a->bCamSetup);
	printf("\t\t%d | Actor number:      0x%04x\n", n, a->num);
	printf("\t\t%d | X position:        %d\n", n, a->pX);
	printf("\t\t%d | Y position:        %d\n", n, a->pY);
	printf("\t\t%d | Z position:        %d\n", n, a->pZ);
	printf("\t\t%d | Y rotation:        %d\n", n, a->rY);
	printf("\t\t%d | Actor variable:    0x%04x\n\n", n, a->var);
}

void printLight(const zsiLight_t* l, int n)
{
	zsiVec3_t v;

	printf("\t\t%d | Draw distance: %8.2f\n", n, l->drawDist);
	printf("\t\t%d | Fog start: %8.2f\n", n, l->fogStart);
	v = l->ambiColor;
	printf("\t\t%d | Ambient color: 0x%02x%02x%02x\n", n, v.X, v.Y, v.Z);
	v = l->light1Dir;
	printf("\t\t%d | Light 1 direction: 0x%02x%02x%02x\n", n, v.X, v.Y, v.Z);
	v = l->light1Color;
	printf("\t\t%d | Light 1 color: 0x%02x%02x%02x\n", n, v.X, v.Y, v.Z);
	v = l->light2Dir;
	printf("\t\t%d | Light 2 direction: 0x%02x%02x%02x\n", n, v.X, v.Y, v.Z);
	v = l->light2Color;
	printf("\t\t%d | Light 2 color: 0x%02x%02x%02x\n", n, v.X, v.Y, v.Z);
	v = l->fogColor;
	printf("\t\t%d | Fog color: 0x%02x%02x%02x\n\n", n, v.X, v.Y, v.Z);
}
