#ifndef _ANTIGRAV_H_
#define _ANTIGRAV_H_

#define ABS(x) (((x) > 0)?(x):(-(x)))
#define DEG(x) ((x) * 180.0 / M_PI)
#define RAD(x) ((x) * M_PI / 180.0)
#define MIN(x, y) ( ((x) < (y)) ? (x) : (y) )
#define MAX(x, y) ( ((x) > (y)) ? (x) : (y) )

#ifdef HAVE_CONFIG_H
#include "../config.h"
#else
#define DATADIR "./data"
#endif

#include "font.h"

#include "tinyxml.h"
#include "m3dmaterial.h"
#include "m3dtexture.h"
#include "m3dmesh.h"
#include "terrain.h"

#include "vector2.h"
#include "craft.h"
#include "level.h"
#include "player.h"
#include "background.h"
#include "game.h"
#include "menu.h"
#include "ring.h"

ALuint loadWavBuffer(const char *filename);

#endif

