#ifndef ZSI_H
#define ZSI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ZSIHeaderCommands
{
	STARTING_POSITIONS = 0x00,
	ACTOR_LIST         = 0x01,
	MM_CAMERA_SETTINGS = 0x02,
	COLLISION_HEADER   = 0x03,
	ROOMS              = 0x04,
	WIND               = 0x05,
	ENTRANCE_LIST      = 0x06,
	SPECIAL_OBJECTS    = 0x07,
	ROOM_BEHAVIOR      = 0x08,
	UNUSED             = 0x09,
	MESH               = 0x0A,
	OBJECT_LIST        = 0x0B,
	UNUSED_LIGHTING    = 0x0C,
	PATHWAYS           = 0x0D,
	TRANSITION_ACTORS  = 0x0E,
	LIGHTING_SETTINGS  = 0x0F,
	TIME_SETTINGS      = 0x10,
	SKYBOX_SETTINGS    = 0x11,
	SKYBOX_MODIFIER    = 0x12,
	EXIT_LIST          = 0x13,
	END_MARKER         = 0x14,
	SOUND_SETTINGS     = 0x15,
	ECHO_SETTINGS      = 0x16,
	CUTSCENES          = 0x17,
	ALTERNATE_HEADERS  = 0x18,
	CAMERA_SETTINGS    = 0x19
};

typedef struct
{
	char     magic[4];
	char codename[12];
} zsiHeader_t;

/* Similar to oot64 scene header flags, but not exact */
/* https://wiki.cloudmodding.com/oot/Scenes_and_Rooms#Header_Commands */
typedef struct
{
	uint8_t cmd;
	uint8_t  d1;
	uint32_t d2;
} sceneFlag_t;

/* Just like the actor struct from oot64 */
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
} zsiActor_t;

/* Sound header */
typedef struct
{
	uint64_t  command : 8;
	uint64_t   config : 8;
	uint64_t nightSFX : 8;
	uint64_t padding1 : 8;
	uint64_t sequence : 8;
	uint64_t unknown1 : 8;
	uint64_t padding2 : 8;
	uint64_t unknown2 : 8;
} zsiSound_t;

/* Rooms are just .zsi files to open */
/* OoT size is 0x44, MM is 0x34 */
/* Might add on to this later */
typedef struct
{
	char fname[0x44];
} zsiRoom_t;

/* Transition actors */
typedef struct
{
	uint8_t     fRoom;
	uint8_t fCamSetup;
	uint8_t     bRoom;
	uint8_t bCamSetup;
	uint16_t      num;
	int16_t        pX;
	int16_t        pY;
	int16_t        pZ;
	int16_t        rY;
	uint16_t      var;
} zsiTransActor_t;

typedef struct
{
	uint64_t  command :  8;
	uint64_t  camMvmt :  8;
	uint64_t padding1 : 16;
	uint64_t   mapLoc :  8;
	uint64_t padding2 : 24;
} zsiCamSettings_t;

typedef struct
{
	// TODO
} zsiCollision_t;

/* Entrance table entry */
typedef struct
{
	uint8_t  pos;
	uint8_t room;
} zsiEntrance_t;

// TODO x location may be wrong
// TODO figure out what this does
typedef struct
{
	uint64_t     command :  8;
	uint64_t  elfMsgFile :  8;
	uint64_t     padding : 16;
	uint64_t glblObjFile : 32;
} zsiSpecialObj_t;

typedef struct
{
	uint64_t   command :  8;
	uint64_t  padding1 : 24;
	uint64_t skyboxNum : 8;
	uint64_t  padding2 : 4;
	uint64_t    clouds : 4;
	uint64_t  padding3 : 4;
	uint64_t    lights : 4;
	uint64_t  padding4 : 8;
} zsiSkybox_t;

typedef struct
{
	uint8_t X;
	uint8_t Y;
	uint8_t Z;
} zsiVec3_t;

typedef struct
{
	float        drawDist;
	float        fogStart;
	uint8_t      unknown1;
	uint8_t      unknown2;
	zsiVec3_t   ambiColor;
	zsiVec3_t   light1Dir;
	zsiVec3_t light1Color;
	zsiVec3_t   light2Dir;
	zsiVec3_t light2Color;
	zsiVec3_t    fogColor;
} zsiLight_t;

/* Just .cmb files I think? */
typedef struct
{
	uint8_t   type;
	uint8_t  count;
	uint32_t start;
	uint32_t   end;
} zsiMesh_t;

/* Scene setup */
// TODO trim this up, remove header things
typedef struct
{
	uint64_t soundHeader;
	zsiSound_t sound;

	uint64_t roomsHeader;
	uint8_t       nRooms;
	zsiRoom_t*     rooms;

	uint64_t   doorsHeader;
	uint8_t         nDoors;
	zsiTransActor_t* doors;

	uint64_t           camHeader;
	zsiCamSettings_t camSettings;

	uint64_t     collHeader;
	zsiCollision_t collData;

	uint64_t  entranceHeader;
	uint8_t       nEntrances;
	zsiEntrance_t* entrances;

	uint64_t  spcObjHeader;
	zsiSpecialObj_t spcObj;

	uint64_t startPosHeader;
	uint8_t       nStartPos;
	zsiActor_t*    startPos;

	uint64_t skyboxHeader;
	zsiSkybox_t    skybox;

	/* Exit list has no size */
	/* Game just assumes good reads */
	/* Get size from lighting header offset? */
	uint64_t exitsHeader;
	uint16_t*      exits;

	uint64_t lightsHeader;
	uint8_t       nLights;
	zsiLight_t*    lights;

	uint64_t cutsceneHeader;
	// TODO cutscenes are QDB files
	// Looks to be just one per setup?
} zsiScene_t;

void readScene(const uint8_t*, zsiScene_t*, int);
void printScene(const zsiScene_t*);
void printActor(const zsiActor_t*, int);
void printTransActor(const zsiTransActor_t*, int);
void printLight(const zsiLight_t*, int);

extern const char* headerStrs[32];

#ifdef __cplusplus
}
#endif

#endif
