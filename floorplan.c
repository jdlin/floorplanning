/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                     *
 *  floorplan.c --- Simulated Annealing Algorithm for Floorplan        *
 *                                                                     *
 *  maintenance page                                                   *
 *                                                                     *
 *  Build Instructions:                                                *
 *                                                                     *
 *    1.  Text mode:                                                   *
 *        a. Borland C++ V3.1                                          *
 *           bcc -ml -G -DNO_GRAPH floorplan.c                         *
 *        b. Unix GCC                                                  *
 *           gcc -DNO_GRAPH floorplan.c -lm                            *
 *                                                                     *
 *    2.  Graphic mode:                                                *
 *        a. Borland C++ V3.1                                          *
 *           bcc -ml -G floorplan.c                                    *
 *        b. Unix GCC                                                  *
 *           gcc floorplan.c -lm                                       *
 *                                                                     *
 *  Required compiler options settings:                                *
 *                                                                     *
 *  Language:        C Language + SUIT Toolkit                         *
 *  Platform:        (1) Sun Sparc, Unix                               *
 *                   (2) i386, DOS 5.0                                 *
 *  Application:     CAD for Digital System: Physical Design           *
 *  Author:          Jordon Lin                                        *
 *                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
#include "floorplan.h"
float INIT_TEMP = 20.0;

/*-------------------------------------------------------------------*/
struct stack {
    struct tree *key;
    struct stack *next;
};
  
static struct stack *head, *z;
/*-------------------------------------------------------------------*/
  
void stackInit(void)
{
    head = (struct stack *) mallocMem(sizeof (*head));
    z = (struct stack *) mallocMem(sizeof (*z));
  
    head->key = NULL;
    head->next = z;
    z->next = z;
}
  
void freeStack(void)
{
    freeMem(head);
    freeMem(z);
}
  
void pushStack(struct tree *v)
{
    struct stack *t;
  
    t = (struct stack *) mallocMem(sizeof (*t));
    t->key = v;
    t->next = head->next;
    head->next = t;
}
  
struct tree *popStack(void)
{
    struct tree *v;
    struct stack *t;
  
    t = head->next;
    head->next = t->next;
    v = t->key;
    freeMem(t);
  
    return v;
}
  
int stackEmpty(void)
{
    return head->next == z;
}
/*-------------------------------------------------------------------*/
void deleteNext(struct boundary *c)
{
    struct boundary *t;
  
    t = c->next->next;
    freeMem(c->next);
    c->next = t;
}
void freeCurve(struct boundary *t)
{
    while (t->next != t)
        deleteNext(t);
    freeMem(t);
}
#ifdef DEBUG
void printCurve(struct boundary *b)
{
    struct boundary *t;
  
    t = b;
    while (t->next != b)
    {
        fprintf(stderr,"(%g,%g)",t->x, t->y);
        t = t->next;
    }
    fprintf(stderr,"(%g,%g)\n",t->x, t->y);
}
void printCurves(struct tree **b)
{
    int i;
  
    for (i = 1; i < 2 * nFloor; i++)
    {
        fprintf(stderr,"%s=>",b[i]->info);
        printCurve(b[i]->curve);
    }
}
#endif
struct boundary *insertBoundary(float w, float h, struct boundary *b, struct boundary *p1, struct boundary *p2)
{
    struct boundary *t;
  
    t = (struct boundary *) mallocMem(sizeof *t);
    t->x = w;
    t->y = h;
    t->p1 = p1;
    t->p2 = p2;
    t->next = b->next;
    b->next = t;
#ifdef DEBUG3
    fprintf(stderr,"(%g, %g) = (%g, %g) + (%g, %g)\n",w,h,p1->x,p1->y,p2->x,p2->y);
#endif
    return t;
}
struct boundary *unionBoundary(float w, float h, struct boundary *b, struct boundary *p1, struct boundary *p2)
{
    struct boundary *t;
    int only;
  
    only = 1;
    t = b->next;
    while (t != b && t != NULL)
    {
        if (t->x == w && t->y == h)
        {
            only = 0;
            break;
        }
        t = t->next;
    }
    if (only)
        insertBoundary(w, h, b, p1, p2);
  
    return b;
}
struct boundary *initBoundary(struct boundary *b)
{
    b = (struct boundary *) mallocMem(sizeof *b);
    b->x = (float) INT_MAX;
    b->y = (float) INT_MAX;
    b->p1 = NULL;
    b->p2 = NULL;
    b->next = b;
  
    return b;
}
  
struct boundary *newBoundary(int k)
{
    struct boundary *b;
    float w[5], h[5], d;
    int i, n;
  
    n = 2;
    b = initBoundary(b);
    for (i = 0; i < n; i++)
        w[i] = h[i] = 0;
    w[0] = sqrt((double) cell[k].area / cell[k].r);
    h[0] = sqrt((double) cell[k].area * cell[k].r);
    if (cell[k].s != cell[k].r)               /* flexible */
    {
        w[n-1] = sqrt((double) cell[k].area / cell[k].s);
        h[n-1] = sqrt((double) cell[k].area * cell[k].s);
        d = w[n-1] - w[0] / (n-1);
        for (i = 1; i < n-1; i++)
        {
            w[i] = w[i-1] + d;
            h[i] = cell[k].area / w[i];
        }
    }
    for (i = 0; i < n; i++)
        if (w[i]*h[i])
	    /*insertBoundary(w[i], h[i], b, NULL, NULL);*/
	    unionBoundary(w[i], h[i], b, NULL, NULL);
  
    if (cell[k].dir == 1)                      /* flexible orientation */
        for (i = 0; i < n; i++)
            if (w[i]*h[i])
                unionBoundary(h[i], w[i], b, NULL, NULL);
  
    return b;
}
  
struct boundary *addBoundary(char op, struct boundary *b1, struct boundary *b2)
{
    struct boundary *b, *t1, *t2;
    float w, h, Area;
  
    b = initBoundary(b);
    t1 = b1->next;
    Area = (float) INT_MAX;
    while(t1 != b1)
    {
        t2 = b2->next;
        while(t2 != b2)
        {
            switch (op) {
                case '+':
                    w = (t1->x > t2->x) ? t1->x : t2->x;
                    h = t1->y + t2->y;
                    break;
                case '*':
                    w = t1->x + t2->x;
                    h = (t1->y > t2->y) ? t1->y : t2->y;
                    break;
                default :
                    break;
            }
            /*unionBoundary(w, h, b, t1, t2);*/
            insertBoundary(w, h, b, t1, t2);
            if (w * h < Area)
            {
                b->x = w;
                b->y = h;
                b->p1 = t1;
                b->p2 = t2;
                Area = w * h;
/*
                b1->x = t1->x;
                b1->y = t1->y;
                b1->p1 = t1->p1;
                b1->p2 = t1->p2;
                b2->x = t2->x;
                b2->y = t2->y;
                b2->p1 = t2->p1;
                b2->p2 = t2->p2;
*/
            }
            t2 = t2->next;
        }
        t1 = t1->next;
    }
  
    return b;
}
  
float minArea(struct boundary *b)
{
    struct boundary *t;
    float area, minA;
  
    t = b->next;
    minA = (float) INT_MAX;
    while(t != b)
    {
        area = t->x * t-> y;
        if (area < minA)
        {
            minA = area;
            b->x = t->x;
            b->y = t->y;
            b->p1 = t->p1;
            b->p2 = t->p2;
        }
        t = t->next;
    }
    minWidth = b->x;
    minHeight = b->y;
    area = minWidth * minHeight;
  
    return area;
}
/*-------------------------------------------------------------------*/
struct tree *zTree;
  
struct tree *slicingTree(char postString[])
{
    struct tree *x;
    char *str;
  
    zTree = (struct tree *) mallocMem(sizeof (*zTree));
    zTree->left = zTree;
    zTree->right = zTree;
    zTree->info = NULL;
    zTree->area = 0;
    zTree->curve = NULL;
    stackInit();
    str = strtok(postString," ");
    while(str != NULL)
    {
        x = (struct tree *) mallocMem(sizeof (*x));
        x->info = strdup(str);
        if (str[0] == '+' || str[0] == '*')
        {
            x->right = popStack();
            x->left = popStack();
            x->curve = addBoundary(str[0], x->left->curve, x->right->curve);
            x->area = minArea(x->curve);
        }
        else
        {
            x->left = zTree;
            x->right = zTree;
            x->curve = newBoundary(atoi(str));
            x->area = cell[atoi(str)].area;
        }
        pushStack(x);
        str = strtok(NULL, " ");
    }
    x = popStack();
    freeStack();
  
    return x;
}
#ifdef NO_GRAPH
void visit(struct tree *t, float x, float y, float w, float h)
{
#ifndef DEBUG2
    if (t->info[0] != '*' && t->info[0] != '+')
#endif
        fprintf(stdout,"rlabel metal1 %8.0f %8.0f %8.0f %8.0f %i %s\n",
        x/minWidth*100, y/minHeight*100,
        (x+w)/minWidth*100, (y+h)/minHeight*100,
        0,t->info);
}
#endif
void defFloor(struct tree *t, float x, float y, float w, float h)
{
    Rects[nRects].x1 = x;
    Rects[nRects].y1 = y;
    Rects[nRects].x2 = x+w;
    Rects[nRects].y2 = y+h;
    strcpy(Rects[nRects++].name, t->info);
}
void layout1(struct tree *t, struct boundary *c, float x, float y)
{
    float px, py;

    if (c != NULL)
    {
	defFloor(t, x, y, c->x, c->y);
#ifdef NO_GRAPH
        visit(t, x, y, c->x, c->y);
#endif
	if (t->left != zTree && t->right != zTree)
        {
	    layout1(t->left, c->p1, x, y);
	    px = (strcmp(t->info, "*") == 0) ? x + c->p1->x : x;
	    py = (strcmp(t->info, "+") == 0) ? y + c->p1->y : y;
	    layout1(t->right, c->p2, px, py);
	}
    }
}
void layout(struct tree *t, float x, float y, float w, float h)
{
    layout1(t, t->curve, 0, 0);
}

void freeNode(struct tree *t)
{
    freeCurve(t->curve);
    freeMem(t->info);
    freeMem(t);
}
  
void freeSubtree(struct tree *t)
{
    struct tree *r;
  
    if (t != zTree)
    {
        freeSubtree(t->left);
        r = t->right;
        freeNode(t);
        freeSubtree(r);
    }
}
  
void freeTree(struct tree *t)
{
    freeSubtree(t);
}
/*-------------------------------------------------------------------*/
void itob(int n, char s[], int b)
{
    int i, len;
    char c;
  
    i = 0;
    do
    {
        s[i++] = n % b + '0';
    } while ((n /= b) > 0);
    s[i] = '\0';
    len = i;
    for (i = 0; i < len / 2; i++)
    {
        c = s[i];
        s[i] = s[len-1-i];
        s[len-1-i] = c;
    }
  
   /*strrev(s);*/
}
char *initFloor(void)
{
    char *postString, *s;
    char label[6];
    int i;
  
    s = (char *) mallocMem(7 * nFloor * sizeof(char));
    s[0] = '1';
    s[1] = '\0';
    for (i = 2; i <= nFloor; i++)
    {
        strcat(s, " ");
        itob(i, label, 10);
        strcat(s, label);
        strcat(s, " ");
        strcat(s, "*");
    }
    postString = strdup(s);
    freeMem(s);
  
    return postString;
}
  
char *initFloor2(void)
{
    char *postString, *s;
    char label[6];
    int i;
  
    s = (char *) mallocMem(7 * nFloor * sizeof(char));
    s[0] = '1';
    s[1] = '\0';
    for (i = 2; i <= nFloor; i++)
    {
        strcat(s, " ");
        itob(i, label, 10);
        strcat(s, label);
    }
    for (i = 1; i < nFloor; i++)
    {
        strcat(s, " ");
        if (i % 2 == 0)
            strcat(s, "*");
        else
            strcat(s, "+");
    }
    postString = strdup(s);
    freeMem(s);
  
    return postString;
}
float finalArea(struct boundary *c)
{
    struct boundary *t;
    float aspect, p, q;
    float area, minA;
  
    p = cell[0].r;
    q = cell[0].s;
    minA = (float) INT_MAX;
    t = c->next;
    while (t != c)
    {
        aspect = t->y / t->x;
        if (aspect >= p && aspect <= q)
        {
            area = t->x * t-> y;
            if (area < minA)
            {
                minA = area;
                c->x = t->x;
                c->y = t->y;
                c->p1 = t->p1;
                c->p2 = t->p2;
            }
        }
        t = t->next;
    }
    minWidth = c->x;
    minHeight = c->y;
    minA = minWidth * minHeight;
  
    return minA;
}
float getArea(struct tree **b)
{
    b[0]->area = finalArea(b[0]->curve);
  
    return b[0]->area;
}
/*-------------------------------------------------------------------*/
void initMatrix(void)
{
    int i ,j;
  
    matrixC = (float **) mallocMem(sizeof(float *) * nFloor);
    for (i = 0; i < nFloor; i++)
    {
        matrixC[i] = (float *) mallocMem(sizeof(float) * nFloor);
        for (j = 0; j < nFloor; j++)
            matrixC[i][j] = (float) rand() / (float) INT_MAX;
    }
}
void freeMatrix(void)
{
    int i;
    for (i = 0; i < nFloor; i++)
        freeMem(matrixC[i]);
    freeMem(matrixC);
}
/*-------------------------------------------------------------------*/

void defineFloors(struct tree *t, struct boundary *c, float x, float y)
{
    float px, py;
    int n;

    if (t != zTree)
    {
	if (c->p1 == NULL && c->p2 == NULL)
	{
            n = atoi(t->info) - 1;
            Rects[n].x1 = x;
            Rects[n].y1 = y;
            Rects[n].x2 = x + c->x;
	    Rects[n].y2 = y + c->y;
	}
	else
	{
	    defineFloors(t->left, c->p1, x, y);
	    px = (strcmp(t->info, "*") == 0) ? x + c->p1->x : x;
	    py = (strcmp(t->info, "+") == 0) ? y + c->p1->y : y;
	    defineFloors(t->right, c->p2, px, py);
	}
	
    }
}
float getWire(struct tree **b)
{
    float cost, d, cx1, cy1, cx2, cy2;
    int i, j;
  
    cost = 0;
    defineFloors(b[0], b[0]->curve,  0, 0);
    for (i = 0; i < nFloor; i++)
        for (j = 0; j < nFloor; j++)
        {
            cx1 = (Rects[i].x1 + Rects[i].x2) / 2;
            cy1 = (Rects[i].y1 + Rects[i].y2) / 2;
            cx2 = (Rects[j].x1 + Rects[j].x2) / 2;
            cy2 = (Rects[j].y1 + Rects[j].y2) / 2;
            d = fabs(cx1 - cx2) + fabs(cy1 - cy2);
            cost += matrixC[i][j] * d;
        }
  
    return cost;
}
float getCost(struct tree **b)
{
    float cost;
  
    cost = getArea(b);
    if (addWire)
        cost += wireCost * getWire(b);
  
    return cost;
}
float reCost(int me, struct tree **b)
{
    int i;
    char *s;
    struct tree *t;
  
    stackInit();
    for (i = 1; i < me; i++)
    {
        s = b[i]->info;
        if (s[0] == '+' || s[0] == '*')
        {
	    popStack();
	    popStack();
        }
        pushStack(b[i]);
    }
    for (i = me; i < 2 * nFloor; i++)
    {
        s = b[i]->info;
        if (s[0] == '+' || s[0] == '*')
        {
            freeCurve(b[i]->curve);
	    b[i]->right = popStack();
            b[i]->left  = popStack();
            b[i]->curve = addBoundary(s[0], b[i]->left->curve, b[i]->right->curve);
            b[i]->area = minArea(b[i]->curve);
        }
	pushStack(b[i]);
    }
    t = popStack();
    /* Cost of this configuration */
    b[0]->info = t->info;
    b[0]->left = t->left;
    b[0]->right = t->right;
    b[0]->curve = t->curve;
    b[0]->area = finalArea(b[0]->curve);
    freeStack();
  
    return b[0]->area;
}
void swap(int i,int j, struct tree **b)
{
    struct tree *t;
  
    t = b[i];
    b[i] = b[j];
    b[j] = t;
}
void complement(int me, struct tree **b)
{
    int i;
    char c;
  
    for (i = me; i < 2 *nFloor; i++)
    {
        c = b[i]->info[0];
        if (c != '+' && c != '*')
            break;
        else
            b[i]->info[0] = (c == '+') ? '*' : '+';
    }
}
int move3(struct tree **b, int *n1)
{
    int me, node;
    int *d, *pair;
    int i, m;
    int zeros;
    int fail;
  
    fail = 0;
    d = (int *) mallocMem(2 * nFloor * sizeof(int));
    pair = (int *) mallocMem(2 * nFloor * sizeof(int));
    d[0] = 1;
    for (i = 1, m = 0; i < 2 * nFloor; i++)
    {
        d[i] = (b[i]->info[0] == '+' || b[i]->info[0] == '*')
        ? 0 : 1;
        if ((!d[i-1] & d[i]) | (d[i-1] & !d[i]))
            pair[m++] = i;
    }
    do
    {
        me = (int)((float) rand() / (float) INT_MAX * m);
        zeros = 0;
        node = pair[me] - 1;
        if (d[node] == 1 && d[node+1] == 0) /* operand operator */
        {
            for (i = 1; i <= node+1; i++)
                zeros = (d[i] == 0) ? zeros + 1 : zeros;
            if (2 * zeros < node)
                break;
        }
        else
            break;
    } while (1);
    if (strcmp(b[node+1]->info, b[node-1]->info) != 0 &&
        strcmp(b[node]->info, b[node+2]->info) != 0)
    {
        swap(node, node+1, b);
        reCost(node, b);
        *n1 = node;
    }
    else
        fail = 1;
    freeMem(d);
    freeMem(pair);
  
    return fail;
}
void move(struct tree **b, int *rtype, int *n1, int *n2)
{
    int me, adj, node;
    int type;
    char *result;
    int *operands, *operators;
    int *d, *pair;
    int i, m, n;
    int zeros;
    int again;
    float r;
  
  
    operands = (int *) mallocMem(nFloor * sizeof(int));
    operators = (int *) mallocMem((nFloor-1) * sizeof(int));
    do {
        r  = (float) rand() / (float) INT_MAX;
        if (r < 1.0 / 3.0)
            type = 1;
        else if (r >= 2.0 / 3.0)
            type = 2;
        else
            type = 3;
        again = 0;
        switch(type)
        {
            case 1:
                for (i = 1, n = 0; i < 2 * nFloor; i++)
                    if (b[i]->info[0] != '+' && b[i]->info[0] != '*')
                        operands[n++] = i;
                me  = (int)((float) rand() / (float) INT_MAX * nFloor);
                if (me == 0)
                    adj = 1;
                else if (me == nFloor-1)
                    adj = -1;
                else
                    adj = ((float) rand() / (float) INT_MAX > 0.5) ? 1 : -1;
                swap(operands[me], operands[me+adj], b);
                node = (adj == -1) ? operands[me+adj] : operands[me];
                reCost(node, b);
                *rtype = 1;
                *n1 = node;
                *n2 = (adj == -1) ? operands[me] : operands[me+adj];
                break;
            case 2:
                for (i = 1, m = 0; i < 2 * nFloor; i++)
                    if (b[i]->info[0] == '+' || b[i]->info[0] == '*')
                        operators[m++] = i;
                me = (int)((float) rand() / (float) INT_MAX * (nFloor-1));
                for (i = node = operators[me]; i > 0; i--)
                {
                    if (b[i]->info[0] != '+' && b[i]->info[0] != '*')
                        break;
                    node = i;
                }
                complement(node, b);
                reCost(node, b);
                *rtype = 2;
                *n1 = node;
                *n2 = 0;
                break;
            case 3:
                again = move3(b, &node);
                *rtype = 3;
                *n1 = node;
                *n2 = *n1 + 1;
                break;
            default:
                again = 1;
                break;
        }
    } while(again);
    freeMem(operators);
    freeMem(operands);
#ifdef DEBUG
    fprintf(stdout,"Move %i:",type);
#endif
}
/*-------------------------------------------------------------------*/
struct tree **initSequence(char *postString)
{
    struct tree **b;
    char *str;
    int i, which;
  
    zTree = (struct tree *) mallocMem(sizeof (*zTree));
    zTree->left = zTree;
    zTree->right = zTree;
    zTree->info = NULL;
    zTree->area = 0;
    zTree->curve = NULL;
    b = (struct tree **) mallocMem(2 * nFloor * sizeof(*b));
    b[0] = (struct tree *) mallocMem(sizeof (**b));
    stackInit();
    str = strtok(postString, " ");
    for (i = 1; str != NULL; i++)
    {
        b[i] = (struct tree *) mallocMem(sizeof (**b));
        b[i]->info = strdup(str);
        if (str[0] == '+' || str[0] == '*')
        {
            b[i]->right = popStack();
            b[i]->left = popStack();
            b[i]->curve = addBoundary(str[0], b[i]->left->curve, b[i]->right->curve);
            b[i]->area = minArea(b[i]->curve);
        }
        else
        {
            b[i]->left = zTree;
            b[i]->right = zTree;
            b[i]->curve = newBoundary(atoi(str));
            b[i]->area = cell[atoi(str)].area;
        }
        pushStack(b[i]);
        str = strtok(NULL, " ");
#ifdef DEBUG3
        printCurve(b[i]->curve);
#endif
    }
    popStack(); /* Cost of this configuration */
    freeStack();
    b[0]->info = b[i-1]->info;
    b[0]->left = b[i-1]->left;
    b[0]->right = b[i-1]->right;
    b[0]->curve = b[i-1]->curve;
    b[0]->area = finalArea(b[0]->curve);
  
    if (addWire)
        wireLen = getWire(b);
    else
        wireLen = 0;
  
    freeMem(postString);
  
    return b;
}
void freeSequence(struct tree **b)
{
    int i;
  
  
    for (i = 1; i < 2 * nFloor; i++)
        freeNode(b[i]);
    freeMem(b[0]);
    freeMem(b);
}
char *toString(struct tree **b)
{
    char *s, *postString;
    int i;
  
    s = (char *) mallocMem(7 * nFloor * sizeof(char));
    s[0] = '\0';
    strcat(s, b[1]->info);
    for (i = 2; i < 2 * nFloor; i++)
    {
        strcat(s, " ");
        strcat(s, b[i]->info);
    }
    postString = strdup(s);
    freeMem(s);
  
    return postString;
}
/*-------------------------------------------------------------------*/
char *anneal(struct tree **b, float *cost, float T)
{
    char *result;
    float diff, newCost;
    double p;
    int type, n1, n2;
  
    result = NULL;
    move(b, &type, &n1, &n2);
    newCost = getCost(b);
    diff = newCost - *cost;
    if (diff < 0)
    {
        result = toString(b);
        *cost = newCost;
    }
    else
    {
        p = exp((double) - diff / T);
        if ((float) rand() / (float) INT_MAX >= p) /* reject */
        {
#ifdef DEBUG
            fprintf(stderr,"reject...");
#endif
            switch (type)
            {
                case 1:
                case 3:
                    swap(n1, n2, b);
                    break;
                case 2:
                    complement(n1, b);
                    break;
                default:
                    break;
            }
            reCost(n1, b);
        }
        else /* accept */
        {
            result = toString(b);
            *cost = newCost;
        }
    }
  
    return result;
}
void initConfig(struct tree **b)
{
    beginArea = b[0]->area;
    beginWidth = b[0]->curve->x;
    beginHeight = b[0]->curve->y;
    if (addWire)
        beginLen = wireLen;
    else
        beginLen = 0;
}
char *simulate(float T)
{
    char *newString, *bestString;
    struct tree **b;
    float temp;
    int i, times;
    float cost, bestCost, oldCost;
    int accept, reject;
    char ch;
    time_t t;
  
    time(&t);
    srand(t);
    times = nFloor;
    bestString = initFloor(); /* initialize */
    newString = strdup(bestString);
    b = initSequence(newString); /* s will be free */
    cost = getCost(b);
    initConfig(b);
    bestCost = cost;
    oldCost = cost;
    for (temp = T; temp > 0.1; temp = 0.85 * temp)
    {
        accept = 0;
        reject = 0;
#ifdef NO_GRAPH
        fprintf(stderr,"temperature = %g\n",temp);
#endif
        for(i = 0; i < times;)
        {
            newString = anneal(b, &cost, temp);
            if (newString == NULL)
            {
                reject++;
            }
            else
            {
                accept++;
                oldCost = cost;
                if (cost < oldCost) /* downhill move */
                    i++;
                if (cost < bestCost)
                {
                    freeMem(bestString);
                    bestString = newString;
                    bestCost = cost;
                }
                else
                    freeMem(newString);
#ifdef DEBUG
                fprintf(stderr,"%g (%s)\n",cost, newString);
#endif
            }
            if (accept+reject == 2 * times)
                break;
        }
#ifdef NO_GRAPH
        fprintf(stderr,"accept = %i, reject = %i\n",accept, reject);
#endif
#ifdef CURVE
        printCurves(b);
#endif
        if ((float) accept / (float) (accept+reject) < 0.05)
            break;
    }
#ifdef NO_GRAPH
    fprintf(stderr,"Final temperature = %g\n",temp);
#endif
    freeSequence(b);
  
    return bestString;
}
void defineResult(char *postString)
{
    struct tree *t;
    struct tree **b;
    float cost;
    char *newString;

    nRects = 0;
    newString = strdup(postString);
    b = initSequence(newString);
    getCost(b);
    layout(b[0], 0, 0, minWidth, minHeight);
#ifdef NO_GRAPH
    if (addWire)
        fprintf(stderr,"Cost = %g + %g * %g\n",
                        getArea(b), wireCost, getWire(b));
    else
	fprintf(stderr,"Cost = %g\n", getArea(b));
#endif

    freeMem(newString);
    freeSequence(b);
  
}
#ifndef NO_GRAPH
SUIT_object floorPlan;
#endif
int readFile(char *filename)
{
    int num;
    int i;
    float p, q;
    float area, r, s;
    int dir;
    FILE *fp;
    /*char *newString;
    struct tree **b;*/
  
    if ((fp = fopen(filename, "r")) == NULL)
        return 0;
    else
    {
        fscanf(fp,"#numbers %i\n", &num);
        if (cell != NULL)
            freeMem(cell);
        cell = (struct triplet *) mallocMem((num+1) * sizeof(*cell));
        fscanf(fp,"#p %f\n", &p);
        fscanf(fp,"#q %f\n", &q);
        nFloor = num;
        cell[0].area = num;
        cell[0].r = p;
        cell[0].s = q;
        for (i = 1; i <= num && !feof(fp); i++)
        {
            fscanf(fp,"%f %f %f %i\n", &area, &r, &s, &dir);
            cell[i].area = area;
            cell[i].r = r;
            cell[i].s = s;
            cell[i].dir = dir;
#ifdef NO_GRAPH
            fprintf(stderr,"%f %f %f %i\n", area, r, s, dir);
#endif
        }
	fclose(fp);

        return 1;
    }
}
#ifdef NO_GRAPH
void main(int argc, char *argv[])
{
    char *postString;
    int i;
    float a;
  
    if (argc > 1)
    {
        if (readFile(argv[1]) != 0)
        {
            fprintf(stderr,"relative important of wire length = ");
            fscanf(stdin,"%f", &wireCost);
            addWire = (wireCost > 0) ? 1 : 0;
            if (addWire)
                initMatrix();
            postString = simulate(INIT_TEMP);
            for (i = 1, a = 0; i <= nFloor; i++)
                a += cell[i].area;
            fprintf(stderr,"original area = %g\n",a);
            fprintf(stderr,"initial area = %g(%g,%g)\n",
            beginArea, beginWidth, beginHeight);
            if (addWire)
                fprintf(stdout,"initial wire length = %g\n",wireLen);
            nRects = 0;
            fprintf(stdout,"magic\ntech scmos\ntimestamp\n<< labels >>\n");
            defineResult(postString);
            fprintf(stderr,"result = %s\n", postString);
            fprintf(stdout,"<< end >>\n");
            freeMem(postString);
            freeMem(cell);
            if (addWire)
                freeMatrix();
        }
        else
            fprintf(stderr,"File not found!\n");
    }
    else
        fprintf(stderr,"Input file name!\n");
}
#endif
/*------------------------------------------------------------------*/
#ifndef NO_GRAPH
char *textString = "Configuration:\nMove set:\nCost function\nCooling schedule";
SUIT_object textEditor, typeInBox, areaBox;
  
int fileOpened = 0;
/*------------------------------------------------------------------*/
void displayText(SUIT_object o)
    /* Callback function for both the text editor and the type-in box. */
{
    char *filename;
  
    filename = SUIT_getText(o, CURRENT_VALUE);
}
  
void loadFruitFile(SUIT_object o)
    /* Callback function for the "Load fruit file" button. */
{
    char *fileText, buffer[100];
    int i;
    float area;
    char *newString;
    struct tree **b;
 
    fileText = SUIT_textOfFile(SUIT_getText(typeInBox, CURRENT_VALUE));
    SUIT_setText(textEditor, CURRENT_VALUE, fileText);
    fileOpened = readFile(SUIT_getText(typeInBox, CURRENT_VALUE));

    if (fileOpened)
    {
	newString = initFloor();
	SUIT_setText (SUIT_name("Postfix String"), CURRENT_VALUE, newString);
/*        
	nRects = 0;
	b = initSequence(newString);
	finalArea(b);
        layout(b[0], 0, 0, minWidth, minHeight);
        SUIT_redisplayRequired(floorPlan);
	freeSequence(b);
*/
        freeMem(newString);

	for (i = 1, area = 0; i <= nFloor; i++)
            area += cell[i].area;
	sprintf (buffer, "%.2f   ", area);
	SUIT_setText (SUIT_name("sum"), LABEL, buffer);
    }
    else
    {

	buffer[0] = '0';
	buffer[1] = '\0';
	SUIT_setText (SUIT_name("sum"), LABEL, buffer);
	SUIT_setText (SUIT_name("begin area"), LABEL, buffer);
	SUIT_setText (SUIT_name("begin width"), LABEL, buffer);
	SUIT_setText (SUIT_name("begin height"), LABEL, buffer);
	SUIT_setText (SUIT_name("begin length"), LABEL, buffer);
	SUIT_setText (SUIT_name("begin cost"), LABEL, buffer);
	SUIT_setText (SUIT_name("Postfix String"), CURRENT_VALUE, buffer);
    }
    buffer[0] = '0';
    buffer[1] = '\0';
    SUIT_setText (SUIT_name("area"), LABEL, buffer);
    SUIT_setText (SUIT_name("width"), LABEL, buffer);
    SUIT_setText (SUIT_name("height"), LABEL, buffer);
    SUIT_setText (SUIT_name("length"), LABEL, buffer);
    SUIT_setText (SUIT_name("cost"), LABEL, buffer);
}
char *stringPtr;
void initTempValue (SUIT_object o)
{
    float t;

    stringPtr = SUIT_getText(o, CURRENT_VALUE);
    t = atof(stringPtr);
    if (t > 20)
    {
	INIT_TEMP = t;
    }
    else
    {
	INIT_TEMP = 20.0;
	stringPtr = "20";
    }
    SUIT_setText(o, CURRENT_VALUE, stringPtr);
      
}
char *stringPtr2;
void costValue (SUIT_object o)
{
    float t;

    stringPtr2 = SUIT_getText(o, CURRENT_VALUE);
    t = atof(stringPtr2);
    if (t > 0)
    {
	wireCost = t;
    }
    else
    {
	wireCost = 0.0;
	stringPtr2 = "0";
    }
    SUIT_setText(o, CURRENT_VALUE, stringPtr2);
  
}
/*------------------------------------------------------------------*/
static void displayFloor(SUIT_object me)
{
    int i;
    float maxX, maxY;
    GP_rectangle shape;
  
    maxX = (minWidth > minHeight) ? minWidth : minHeight;
    maxY = (minWidth > minHeight) ? minWidth : minHeight;
    for (i = 0; i < nRects; i++)
    {
        if (strcmp(Rects[i].name, "*") != 0 && strcmp(Rects[i].name, "+") != 0)
            GP_setColor(SUIT_getColor(me, "demo color"));
        else
            GP_setColor(GP_defColor("salmon", FALSE));
        shape = GP_defRectangle(Rects[i].x1/maxX, Rects[i].y1/maxY,
        Rects[i].x2/maxX, Rects[i].y2/maxY);
        GP_drawRectangle(shape);
        if (strcmp(Rects[i].name, "*") != 0 && strcmp(Rects[i].name, "+") != 0)
            GP_justifyTextInRectangle(Rects[i].name, JUSTIFY_CENTER, shape);
    }
}
  
void run(SUIT_object o)
{
  
    char *postString;
    char buffer[100];
    int i;
    float a;
  
    nRects = 0;
    loadFruitFile(o);
    if (fileOpened)
    {
	costValue(SUIT_name("wire ratio"));
	initTempValue(SUIT_name("initial temp"));
/*
        for (i = 1, a = 0; i <= nFloor; i++)
            a += cell[i].area;
        sprintf (buffer, "%.2f   ", a);
	SUIT_setText (SUIT_name("sum"), LABEL, buffer);
*/
	addWire = (wireCost > 0) ? 1 : 0;
	if (addWire)
	    initMatrix();
	
	postString = simulate(INIT_TEMP);
	SUIT_setText (SUIT_name("Postfix String"), CURRENT_VALUE, postString);
	nRects = 0;
	defineResult(postString);
          
	SUIT_redisplayRequired(floorPlan);

	sprintf (buffer, "%.2f        ", beginWidth*beginHeight);
	SUIT_setText (SUIT_name("begin area"), LABEL, buffer);
    	sprintf (buffer, "%.2f        ", beginWidth);
    	SUIT_setText (SUIT_name("begin width"), LABEL, buffer);
    	sprintf (buffer, "%.2f        ", beginHeight);
	SUIT_setText (SUIT_name("begin height"), LABEL, buffer);
	sprintf (buffer, "%.2f        ", beginLen);
    	SUIT_setText (SUIT_name("begin length"), LABEL, buffer);
    	sprintf (buffer, "%.2f        ", beginWidth*beginHeight+wireCost*beginLen);
	SUIT_setText (SUIT_name("begin cost"), LABEL, buffer);

        sprintf (buffer, "%.2f        ", minWidth*minHeight);
        SUIT_setText (SUIT_name("area"), LABEL, buffer);
        sprintf (buffer, "%.2f        ", minWidth);
        SUIT_setText (SUIT_name("width"), LABEL, buffer);
        sprintf (buffer, "%.2f        ", minHeight);
	SUIT_setText (SUIT_name("height"), LABEL, buffer);
        sprintf (buffer, "%.2f        ", wireLen);
        SUIT_setText (SUIT_name("length"), LABEL, buffer);
        sprintf (buffer, "%.2f        ", minWidth * minHeight + wireCost * wireLen);
        SUIT_setText (SUIT_name("cost"), LABEL, buffer);

        SUIT_free(postString);
	SUIT_free(cell);
	if (addWire)
	    freeMatrix();
    }
    
}
/*------------------------------------------------------------------*/
void main(int argc, char *argv[])
{
    SUIT_object menu, l1, l2, l3, l4, l5, l6, l7, l8, l9 ,l10, l11;
    SUIT_object m2, i1, i2, i3, i4, i5, i6, i7, i8, i9 ,i10, i11;
    SUIT_object doneButtom, loadButtom, runButtom, abortButtom;

    SUIT_deluxeInit(&argc, argv);
    /*SUIT_initFromCode(argv[0]);*/
  
    loadButtom = SUIT_createButton("Load", loadFruitFile);
    runButtom = SUIT_createButton("Run", run);
    doneButtom = SUIT_createDoneButton(NULL);
    abortButtom = SUIT_createAbortButton(NULL);
  
    textEditor = SUIT_createTextEditorWithScrollBar("Type text here", displayText);
    SUIT_setText(textEditor, CURRENT_VALUE, textString);
  
    typeInBox = SUIT_createTypeInBox("Enter one line here", loadFruitFile);
    SUIT_createLabel ("File Name:");
    SUIT_createTypeInBox("wire ratio", costValue);
    SUIT_createLabel ("relative important of wire length:");
    SUIT_createTypeInBox("initial temp", initTempValue);
    SUIT_createLabel ("initial temperature:");
    /*SUIT_setText (SUIT_name("initial temp"), CURRENT_VALUE, "20");*/

    menu = SUIT_createBulletinBoard("main menu");
    l1 = SUIT_createLabel("area:");
    l2 = SUIT_createLabel("area");
    l3 = SUIT_createLabel("width:");
    l4 = SUIT_createLabel("width");
    l5 = SUIT_createLabel("height:");
    l6 = SUIT_createLabel("height");
    l7 = SUIT_createLabel("length:");
    l8 = SUIT_createLabel("length");
    l9 = SUIT_createLabel("cost:");
    l10 = SUIT_createLabel("cost");
    l11 = SUIT_createLabel("result");
    SUIT_addChildToObject (menu, l1);
    SUIT_addChildToObject (menu, l2);
    SUIT_addChildToObject (menu, l3);
    SUIT_addChildToObject (menu, l4);
    SUIT_addChildToObject (menu, l5);
    SUIT_addChildToObject (menu, l6);
    SUIT_addChildToObject (menu, l7);
    SUIT_addChildToObject (menu, l8);
    SUIT_addChildToObject (menu, l9);
    SUIT_addChildToObject (menu, l10);
    SUIT_addChildToObject (menu, l11);
  
    m2 = SUIT_createBulletinBoard("initial values");
    i1 = SUIT_createLabel("initial area:");
    i2 = SUIT_createLabel("begin area");
    i3 = SUIT_createLabel("initial width:");
    i4 = SUIT_createLabel("begin width");
    i5 = SUIT_createLabel("initial height:");
    i6 = SUIT_createLabel("begin height");
    i7 = SUIT_createLabel("initial length:");
    i8 = SUIT_createLabel("begin length");
    i9 = SUIT_createLabel("initial cost:");
    i10 = SUIT_createLabel("begin cost");
    SUIT_createLabel("total area:");
    SUIT_createLabel("sum");
    i11 = SUIT_createLabel("initialization");
  
    SUIT_addChildToObject (m2, i1);
    SUIT_addChildToObject (m2, i2);
    SUIT_addChildToObject (m2, i3);
    SUIT_addChildToObject (m2, i4);
    SUIT_addChildToObject (m2, i5);
    SUIT_addChildToObject (m2, i6);
    SUIT_addChildToObject (m2, i7);
    SUIT_addChildToObject (m2, i8);
    SUIT_addChildToObject (m2, i9);
    SUIT_addChildToObject (m2, i10);
    SUIT_addChildToObject (m2, i11);
  
    SUIT_createLabel("Polish Expression:");
    SUIT_createTypeInBox("Postfix String", displayText);
  
    SUIT_createClock("Timer");
    SUIT_createTextBox("Title",
		       "@c(CSE788 Final Project: @b(Floorplan)   designed by @u(Jiun-dar Lin))");

    floorPlan = SUIT_createObject("Floor", "Layout");
    SUIT_addDisplayToObject(floorPlan, "standard", NULL, displayFloor);
  
    SUIT_beginStandardApplication();
}
#endif
