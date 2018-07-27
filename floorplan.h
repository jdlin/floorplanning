#ifndef FLOORPLAN__H
#define FLOORPLAN__H

#include <malloc.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef NO_GRAPH
   #include "suit.h"
   #define   freeMem     SUIT_free
   #define   mallocMem   SUIT_malloc
#else
   #define   freeMem     free
   #define   mallocMem   malloc
#endif

#define RECTS_MAX 80

struct floors
{
    float x1, y1, x2, y2;
    char name[5];
};

struct tree {
    struct tree *left, *right;
    char *info;
    float area;
    struct boundary *curve;
};
struct boundary {
    float x, y;
    struct boundary *next, *p1, *p2;
};
struct triplet {
    float area, r, s;
    int dir;
};
/*-------------------------------------------------------*/
struct floors Rects[RECTS_MAX];
int nRects = 0;
struct triplet *cell;
int nFloor = 0;
float minWidth, minHeight, **matrixC;
int addWire = 0;
float wireCost = 0;
float wireLen = 0;
float beginArea;
float beginWidth,beginHeight, beginLen;

#endif
