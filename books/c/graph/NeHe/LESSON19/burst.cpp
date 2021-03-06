#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>           // Standard C/C++ Input-Output
#include <stdlib.h>          // Standard C/C++ Library
#include <GL/glut.h>         // The GL Utility Toolkit (GLUT) Header

#include "bmprw.h"
#include "nal_fps.h"


#define WCX           1280/1.5    // Window Width
#define WCY           720/1.5    // Window Height
#define TEXTURES_NUM  1      // We Have 1 Texture
#define	MAX_DIV_X     30
#define	MAX_DIV_Y     30
#define	MAX_PARTICLES (MAX_DIV_X * MAX_DIV_Y)
#define NOT_BURST_FRAME_NUM     30


typedef struct N3D_Vertex
{
	GLfloat fX;
	GLfloat fY;
	GLfloat fZ;
	GLfloat fS;
	GLfloat fT;
	GLfloat fNX;
	GLfloat fNY;
	GLfloat fNZ;
} N3D_Vertex;

typedef struct tagPTSObj {
    int     iDivX, iDivY;
    float   flbX, flbY, flbZ; // left bottom XYZ
    float   flbS, flbT;
    float   fobjW, fobjH, fobjL;
    float   ftexW, ftexH;
    float   fdivObjW, fdivObjH, fdivObjL;
    float   fdivTexW, fdivTexH;
} PTSObj;

typedef struct tagPTSptiles {
    bool  active;   // Active (Yes/No)
    float life;     // Particle Life
    float fade;     // Fade Speed
    float r;        // Red Value
    float g;        // Green Value
    float b;        // Blue Value
    float x;        // X Position
    float y;        // Y Position
    float z;        // Z Position
    float s;
    float t;
    float xi;       // X Direction
    float yi;       // Y Direction
    float zi;       // Z Direction
    float xg;       // X Gravity
    float yg;       // Y Gravity
    float zg;       // Z Gravity
} PTSptiles;        // Particles Structure

typedef struct tagSCT_PTSlookat{
    float   ex, ey, ez;
    float   cx, cy, cz;
    float   ux, uy, uz;
} PTSlookat;

typedef struct tagPTSlookatMg{
    PTSlookat   latCur;
    PTSlookat   latOri;
    PTSlookat   latOff;
    PTSlookat   latCen;
    PTSlookat   latDir;
} PTSlookatMg;

typedef struct tagPTSeffMg{
    GLint   texId;
    int     isRainbow;
    float   slowdown;
    float   speedX;
    float   speedY;
    float   zoom;
    int     colIdx;
    int     delay;

    int     isTran;
    int     tranStep;
    int     tranSteps;
    int     notBurstNum;
    float   lastResetTime;
    float   recoverCen[MAX_PARTICLES][3];
} PTSeffMg;

// Global Variables
bool    g_gamemode;            // GLUT GameMode ON/OFF
bool    g_fullscreen;
bool    g_keys[256];           // Keys Array

GLuint	g_texid[TEXTURES_NUM]; // Our Textures' Id List


PTSObj          g_ptsObj;
PTSptiles       g_CurPtile[MAX_PARTICLES];
PTSptiles       g_OriPtile[MAX_PARTICLES];
PTSlookatMg     g_lookatMg;
PTSeffMg        g_effMg;
N3D_Vertex      g_vers[MAX_PARTICLES * 4];


#define N_PTS_COLORS    12
static GLfloat g_PTS_COLORS[][3] = {
    {1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
    {0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
    {0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f},
};
extern int dummy[ (sizeof(g_PTS_COLORS) / sizeof(g_PTS_COLORS[0]))
    == N_PTS_COLORS ? 0 : -1 ];


bool setup_textures()
{
    int         ret;
    sbitData    *bmpr;

    bmpr = bmpCreateObjForRead(EBMP_RGBA, 0);
    ret = bmpRead("ActorBall6.bmp", bmpr);
    assert(ret);

    glGenTextures(TEXTURES_NUM, g_texid);
    glBindTexture(GL_TEXTURE_2D, g_texid[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            bmpr->w, bmpr->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmpr->pdata);

    bmpDestroyObjForRead(&bmpr);

    return true;
}

bool PTS_initObj(PTSObj *ptsO)
{
    ptsO->iDivX     = MAX_DIV_X;
    ptsO->iDivY     = MAX_DIV_Y;
    ptsO->flbX      = 0.0;
    ptsO->flbY      = 0.0;
    ptsO->flbZ      = 0.0;
    ptsO->flbS      = 0.0;
    ptsO->flbT      = 0.0;
    ptsO->fobjW     = 1.0;
    ptsO->fobjH     = 1.0;
    ptsO->fobjL     = 0.0;
    ptsO->ftexW     = 1.0;
    ptsO->ftexH     = 1.0;
    ptsO->fdivObjW  = ptsO->fobjW / ptsO->iDivX;
    ptsO->fdivObjH  = ptsO->fobjH / ptsO->iDivY;
    ptsO->fdivObjL  = 0.0;
    ptsO->fdivTexW  = ptsO->ftexW / ptsO->iDivX;
    ptsO->fdivTexH  = ptsO->ftexH / ptsO->iDivY;

    return true;
}

bool PTS_initParticles(PTSptiles *pars, PTSptiles *parsOri, PTSObj *ptsO,
        int n)
{
    int i;
    float bsX = ptsO->flbX - ptsO->fobjW / 2.0;
    float bsY = ptsO->flbY - ptsO->fobjH / 2.0;
    float bsZ = ptsO->flbZ - ptsO->fobjL / 2.0;
    float bsS = ptsO->flbS;
    float bsT = ptsO->flbT;

    for ( i=0; i < n; i++ ) {
        int ix = i % ptsO->iDivX;
        int iy = i / ptsO->iDivX;
        parsOri[i].active = pars[i].active = true;
        parsOri[i].life = pars[i].life = 1.0f;
        parsOri[i].fade = pars[i].fade = float(rand()%100)/1000.0f+0.003f;
        parsOri[i].r  = pars[i].r  = g_PTS_COLORS[i/(n/N_PTS_COLORS)][0];
        parsOri[i].g  = pars[i].g  = g_PTS_COLORS[i/(n/N_PTS_COLORS)][1];
        parsOri[i].b  = pars[i].b  = g_PTS_COLORS[i/(n/N_PTS_COLORS)][2];
        parsOri[i].x  = pars[i].x  = bsX + (float)ix * ptsO->fdivObjW;
        parsOri[i].y  = pars[i].y  = bsY + (float)iy * ptsO->fdivObjH;
        parsOri[i].z  = pars[i].z  = bsZ;
        parsOri[i].s  = pars[i].s  = bsS + (float)ix * ptsO->fdivTexW;
        parsOri[i].t  = pars[i].t  = bsT + (float)iy * ptsO->fdivTexH;
        parsOri[i].xi = pars[i].xi = float((rand()%50)-26.0f)*10.0f;
        parsOri[i].yi = pars[i].yi = float((rand()%50)-25.0f)*10.0f;
        parsOri[i].zi = pars[i].zi = float((rand()%50)-25.0f)*10.0f;
        parsOri[i].xg = pars[i].xg = 0.0f;
        parsOri[i].yg = pars[i].yg = -0.8f;
        parsOri[i].zg = pars[i].zg = 0.0f;
    }

    return true;
}

bool PTS_initLookAtMg(PTSlookatMg *latMg)
{
    PTSlookat lookat[5] = {
        /*cur*/ { 0.0, 0.0, 0.0,   0.0, 0.0, -2.0,   0.0, 1.0, 0.0 },
        /*ori*/ { 0.0, 0.0, 0.0,   0.0, 0.0, -2.0,   0.0, 1.0, 0.0 },
        /*off*/ { 2.0, 0.2, 3.0,   2.0, 0.2,  1.0,   2.0, 3.0, 3.0 },
        /*cen*/ { 0.01, 0.01, 0.01,  0.002, 0.002, 0.002,  0.01, 0.00, 0.01 },
        /*dir*/ { 1.0, 1.0, 1.0,   1.0, 1.0, 1.0,    1.0, 1.0, 1.0 },
    };

    latMg->latCur = lookat[0];
    latMg->latOri = lookat[1];
    latMg->latOff = lookat[2];
    latMg->latCen = lookat[3];
    latMg->latDir = lookat[4];

    return 1;
}

bool PTS_initEffMg(PTSeffMg *ptsEM, GLuint texid)
{
    ptsEM->texId            = texid;
    ptsEM->isRainbow        = 1;
    ptsEM->slowdown         = 2.0;
    ptsEM->speedX           = 0.0;
    ptsEM->speedY           = 42.0;
    ptsEM->zoom             = -2.0;
    ptsEM->colIdx           = 0;
    ptsEM->delay            = 0;

    ptsEM->isTran           = 0;
    ptsEM->tranStep         = 0;
    ptsEM->tranSteps        = 100;
    ptsEM->notBurstNum      = NOT_BURST_FRAME_NUM;
    ptsEM->lastResetTime    = 0.0;
    memset(ptsEM->recoverCen, 0, sizeof(ptsEM->recoverCen));

    return 1;
}

bool PTS_init(PTSeffMg *ptsEM, PTSObj *ptsO, PTSptiles *pars,
        PTSptiles *parsOri, PTSlookatMg *latMg)
{
    PTS_initEffMg(ptsEM, g_texid[0]);
    PTS_initObj(ptsO);
    PTS_initParticles(pars, parsOri, ptsO, MAX_PARTICLES);
    PTS_initLookAtMg(latMg);

    return true;
}

void particlesReset(PTSeffMg *ptsEM, PTSptiles *pars, PTSptiles *parsOri,
        int n)
{
    int i;

    ptsEM->lastResetTime = Fps_getProgTime();
    ptsEM->notBurstNum = NOT_BURST_FRAME_NUM;

    for ( i = 0; i < n; i++ ) {
        pars[i].active = parsOri[i].active;
        pars[i].life   = parsOri[i].life;
        pars[i].fade   = parsOri[i].fade;
        pars[i].r      = parsOri[i].r;
        pars[i].g      = parsOri[i].g;
        pars[i].b      = parsOri[i].b;
        pars[i].x      = parsOri[i].x;
        pars[i].y      = parsOri[i].y;
        pars[i].z      = parsOri[i].z;
        pars[i].s      = parsOri[i].s;
        pars[i].t      = parsOri[i].t;
        pars[i].xi     = parsOri[i].xi;
        pars[i].yi     = parsOri[i].yi;
        pars[i].zi     = parsOri[i].zi;
        pars[i].xg     = parsOri[i].xg;
        pars[i].yg     = parsOri[i].yg;
        pars[i].zg     = parsOri[i].zg;
    }
}

bool init(void)
{
    Fps_initProgTime();

    if (!setup_textures())
        return false;

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClearDepth(1.0f);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,g_texid[0]);

    PTS_init(&g_effMg, &g_ptsObj, g_CurPtile, g_OriPtile, &g_lookatMg);
    memset(g_keys,0,sizeof(g_keys));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    return true;
}

void ParticlesDraw(PTSeffMg *ptsEM, PTSptiles *par, int n, PTSObj *ptsO,
        N3D_Vertex *vers)
{
    int i;

    glBindTexture(GL_TEXTURE_2D, ptsEM->texId);

    for ( i=0; i < n; i++ )
    {
        if ( !par[i].active )
            continue;

        if ( ptsEM->isTran )
        {
            par[i].x += ptsEM->recoverCen[i][0];
            par[i].y += ptsEM->recoverCen[i][1];
            par[i].z += ptsEM->recoverCen[i][2];
        }
        float x  = par[i].x;
        float y  = par[i].y;
        float z  = par[i].z + ptsEM->zoom;
        float cx = ptsO->fdivObjW;
        float cy = ptsO->fdivObjH;
        float s  = par[i].s;
        float t  = par[i].t;
        float cs = ptsO->fdivTexW;
        float ct = ptsO->fdivTexH;
#if 0 // NALD
        if ( i==0 || i==3 || i==23 )
            printf("NAL &&& i=%d, (x,y,z)=(%f,%f,%f)\n", i, x, y, z);
#endif

        // glColor4f(par[i].r, par[i].g, par[i].b, par[i].life);
        //glDisable(GL_TEXTURE_2D); // NALD
#if 0
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2d(s   , t+ct); glVertex3f(x,    y+cy, z);
        glTexCoord2d(s   , t   ); glVertex3f(x,    y,    z);
        glTexCoord2d(s+cs, t+ct); glVertex3f(x+cx, y+cy, z);
        glTexCoord2d(s+cs, t   ); glVertex3f(x+cx, y,    z);
        glEnd();
#else
        vers[i*4+0].fS = s;
        vers[i*4+0].fT = t+ct;
        vers[i*4+0].fX = x;
        vers[i*4+0].fY = y+cy;
        vers[i*4+0].fZ = z;

        vers[i*4+1].fS = s;
        vers[i*4+1].fT = t;
        vers[i*4+1].fX = x;
        vers[i*4+1].fY = y;
        vers[i*4+1].fZ = z;

        vers[i*4+2].fS = s+cs;
        vers[i*4+2].fT = t+ct;
        vers[i*4+2].fX = x+cx;
        vers[i*4+2].fY = y+cy;
        vers[i*4+2].fZ = z;

        vers[i*4+3].fS = s+cs;
        vers[i*4+3].fT = t;
        vers[i*4+3].fX = x+cx;
        vers[i*4+3].fY = y;
        vers[i*4+3].fZ = z;
#endif
    }

    for ( i=0; i < n; i++ )
    {
        glVertexPointer(3, GL_FLOAT, sizeof(N3D_Vertex), &(vers[4*i].fX));
        glTexCoordPointer(2, GL_FLOAT, sizeof(N3D_Vertex), &(vers[4*i].fS));
        // glNormalPointer(GL_FLOAT, sizeof(N3D_Vertex), &(vers[4*i].fNX));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void ParticlesUpdate(PTSeffMg *ptsEM, PTSptiles *par, int n)
{
    assert(ptsEM->colIdx >= 0 && ptsEM->colIdx < N_PTS_COLORS);

    int i;

    if ( --ptsEM->notBurstNum > 0 )
        return;
    if ( ptsEM->notBurstNum < 0 )
        ptsEM->notBurstNum = 0;

    for ( i=0; i < n; i++ ) {
        if ( par[i].active ) {
            par[i].x += par[i].xi/(ptsEM->slowdown*1000);
            par[i].y += par[i].yi/(ptsEM->slowdown*1000);
            par[i].z += par[i].zi/(ptsEM->slowdown*1000);

            par[i].xi += par[i].xg;
            par[i].yi += par[i].yg;
            par[i].zi += par[i].zg;
            par[i].life -= par[i].fade;

            if ( par[i].life < 0.0f )
            {
                par[i].life = 1.0f;
                par[i].fade = float(rand()%100)/1000.0f+0.003f;
                par[i].x = 0.0f;
                par[i].y = 0.0f;
                par[i].z = 0.0f;
                par[i].xi = ptsEM->speedX+float((rand()%60)-32.0f);
                par[i].yi = ptsEM->speedY+float((rand()%60)-30.0f);
                par[i].zi = float((rand()%60)-30.0f);
                par[i].r = g_PTS_COLORS[ptsEM->colIdx][0];
                par[i].g = g_PTS_COLORS[ptsEM->colIdx][1];
                par[i].b = g_PTS_COLORS[ptsEM->colIdx][2];
            }

            if (g_keys['\t']) {
#if 1
                particlesReset(&g_effMg, g_CurPtile, g_OriPtile, MAX_PARTICLES);
#else
                par[i].x = 0.0f;
                par[i].y = 0.0f;
                par[i].z = 0.0f;
                par[i].xi = float((rand()%50)-26.0f)*10.0f;
                par[i].yi = float((rand()%50)-25.0f)*10.0f;
                par[i].zi = float((rand()%50)-25.0f)*10.0f;
#endif
            }
        }
    }
}

void ParTranInit(PTSeffMg *ptsEM, PTSptiles *pars, PTSptiles *parsOri,
        int n, int steps)
{
    assert(n > 0 && steps != 0);

    int i;
    for ( i = 0; i < n; i++ )
    {
        ptsEM->recoverCen[i][0] = (parsOri[i].x - pars[i].x) / steps;
        ptsEM->recoverCen[i][1] = (parsOri[i].y - pars[i].y) / steps;
        ptsEM->recoverCen[i][2] = (parsOri[i].z - pars[i].z) / steps;
    }
}

void PTS_draw()
{
    ParticlesDraw(&g_effMg, g_CurPtile, MAX_PARTICLES, &g_ptsObj, g_vers);
}

void PTS_updateLookAt(PTSlookatMg *latMg)
{
    PTSlookat   *latCur = &(latMg->latCur);
    PTSlookat   *latOri = &(latMg->latOri);
    PTSlookat   *latOff = &(latMg->latOff);
    PTSlookat   *latCen = &(latMg->latCen);
    PTSlookat   *latDir = &(latMg->latDir);

    latCur->ex += latCen->ex * latDir->ex;
    latCur->ey += latCen->ey * latDir->ey;
    latCur->ez -= latCen->ez * latDir->ez;
    latCur->cx += latCen->cx * latDir->cx;
    latCur->cy += latCen->cy * latDir->cy;
    latCur->cz += latCen->cz * latDir->cz;
    latCur->ux += latCen->ux * latDir->ux;
    latCur->uy += latCen->uy * latDir->uy;
    latCur->uz += latCen->uz * latDir->uz;

    if ( fabsf(latCur->ex - latOri->ex) > latOff->ex ) latDir->ex *= -1;
    if ( fabsf(latCur->ey - latOri->ey) > latOff->ey ) latDir->ey *= -1;
    if ( fabsf(latCur->ez - latOri->ez) > latOff->ez ) latDir->ez *= -1;
    if ( fabsf(latCur->cx - latOri->cx) > latOff->cx ) latDir->cx *= -1;
    if ( fabsf(latCur->cy - latOri->cy) > latOff->cy ) latDir->cy *= -1;
    if ( fabsf(latCur->cz - latOri->cz) > latOff->cz ) latDir->cz *= -1;
    if ( fabsf(latCur->ux - latOri->ux) > latOff->ux ) latDir->ux *= -1;
    if ( fabsf(latCur->uy - latOri->uy) > latOff->uy ) latDir->uy *= -1;
    if ( fabsf(latCur->uz - latOri->uz) > latOff->uz ) latDir->uz *= -1;
}

void PTS_update(PTSeffMg *ptsEM)
{
    PTS_updateLookAt(&g_lookatMg);

    if ( Fps_getProgTime() - ptsEM->lastResetTime > 3.00 )
    {
        ptsEM->isTran = 1;
        if ( ptsEM->tranStep == 0 )
            ParTranInit(&g_effMg, g_CurPtile, g_OriPtile,
                    MAX_PARTICLES, ptsEM->tranSteps);
        // particlesReset();
    }
    else
    {
        ParticlesUpdate(&g_effMg, g_CurPtile, MAX_PARTICLES);
    }

    // ...
    if ( ptsEM->tranStep == ptsEM->tranSteps )
    {
        ptsEM->lastResetTime = Fps_getProgTime();
        ptsEM->isTran = 0;
        ptsEM->tranStep = 0;
        ptsEM->notBurstNum = NOT_BURST_FRAME_NUM;
    }
    if ( ptsEM->isTran && ptsEM->tranStep <= ptsEM->tranSteps)
    {
        ptsEM->tranStep++;
    }

    // ...
    if ( ptsEM->isRainbow && (ptsEM->delay > 25) )
    {
        ptsEM->delay = 0;
        ptsEM->colIdx++;
        if (ptsEM->colIdx >= N_PTS_COLORS) ptsEM->colIdx = 0;
    }
    ptsEM->delay++;
}


static inline void PTS_lookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
        GLfloat centerx, GLfloat centery, GLfloat centerz,
        GLfloat upx, GLfloat upy, GLfloat upz)
{
    GLfloat m[16];
    GLfloat x[3], y[3], z[3];
    GLfloat mag;
    /* Make rotation matrix */
    /* Z vector */
    z[0] = eyex - centerx;
    z[1] = eyey - centery;
    z[2] = eyez - centerz;
    mag = sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    if (mag) {
        z[0] /= mag;
        z[1] /= mag;
        z[2] /= mag;
    }
    /* Y vector */
    y[0] = upx;
    y[1] = upy;
    y[2] = upz;
    /* X vector = Y cross Z */
    x[0] = y[1] * z[2] - y[2] * z[1];
    x[1] = -y[0] * z[2] + y[2] * z[0];
    x[2] = y[0] * z[1] - y[1] * z[0];
    /* Recompute Y = Z cross X */
    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] = z[0] * x[1] - z[1] * x[0];

    mag = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }
    mag = sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag) {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }
#define M(row,col)  m[col*4+row]
    M(0, 0) = x[0];
    M(0, 1) = x[1];
    M(0, 2) = x[2];
    M(0, 3) = 0.0;
    M(1, 0) = y[0];
    M(1, 1) = y[1];
    M(1, 2) = y[2];
    M(1, 3) = 0.0;
    M(2, 0) = z[0];
    M(2, 1) = z[1];
    M(2, 2) = z[2];
    M(2, 3) = 0.0;
    M(3, 0) = 0.0;
    M(3, 1) = 0.0;
    M(3, 2) = 0.0;
    M(3, 3) = 1.0;
#undef M
    glMultMatrixf(m);
    /* Translate Eye to Origin */
    glTranslatef(-eyex, -eyey, -eyez);
}

void PTS_lookAtMg(PTSlookatMg *latMg)
{
    PTS_lookAt(
            latMg->latCur.ex,
            latMg->latCur.ey,
            latMg->latCur.ez,
            latMg->latCur.cx,
            latMg->latCur.cy,
            latMg->latCur.cz,
            latMg->latCur.ux,
            latMg->latCur.uy,
            latMg->latCur.uz
            );
}

// Our Rendering Is Done Here
void render(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    PTS_lookAtMg(&g_lookatMg);

    PTS_update(&g_effMg);
    PTS_draw();

    glFlush();
    glutSwapBuffers ( );

    Fps_updateFPS();
    //usleep(30 * 1000);
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (h == 0) h = 1;
    glFrustum(-(float)w/h/2.0, (float)w/h/2.0, -0.5, 0.5, 1.0, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void game_function(void)
{
    // if (g_keys[GLUT_KEY_PAGE_UP])	g_zoom += 0.1f;	// Zoom In
    // if (g_keys[GLUT_KEY_PAGE_DOWN])	g_zoom -= 0.1f;	// Zoom Out
    // if (g_keys[GLUT_KEY_UP] && (g_yspeed < 200))    g_yspeed += 1.0f;
    // if (g_keys[GLUT_KEY_DOWN] && (g_yspeed > -200)) g_yspeed -= 1.0f;
    // if (g_keys[GLUT_KEY_RIGHT] && (g_xspeed < 200)) g_xspeed += 1.0f;
    // if (g_keys[GLUT_KEY_LEFT] && (g_xspeed > -200)) g_xspeed -= 1.0f;

    render();
}

// Our Keyboard Handler (Normal Keys)
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        // case 'a': g_lookat.ex += 0.1; break;
        // case 'A': g_lookat.ex -= 0.1; break;
        // case 's': g_lookat.ey += 0.1; break;
        // case 'S': g_lookat.ey -= 0.1; break;
        // case 'w': g_lookat.ez += 0.1; break;
        // case 'W': g_lookat.ez -= 0.1; break;
        // case 'c': g_lookat.cx += 0.02; break;
        // case 'C': g_lookat.cx -= 0.02; break;
        // case 'r': g_lookat.cy += 0.02; break;
        // case 'R': g_lookat.cy -= 0.02; break;
        // case 'e': g_lookat.cz += 0.02; break;
        // case 'E': g_lookat.cz -= 0.02; break;
        // case 'd': g_lookat.ux += 0.1; break;
        // case 'D': g_lookat.ux -= 0.1; break;
        // case 'b': g_lookat.uz += 0.1; break;
        // case 'B': g_lookat.uz -= 0.1; break;
        case 'p':
                  // printf("NALL &&**&& zoom=%f, xs=%f, ys=%f,\n",
                  //         g_zoom, g_xspeed, g_yspeed);
                  break;
        case 27:
                  exit(0);
                  break;
        // case '+':
        //           if (g_slowdown > 1.0f) g_slowdown -= 0.01f;
        //           break;
        // case '-':
        //           if (g_slowdown < 4.0f) g_slowdown += 0.01f;
        //           break;
        // case '\n':
        //           g_rainbow = !g_rainbow;
        //           break;
        default:
                  g_keys[key] = true;
                  break;
    }
}

void special_keys(int a_keys, int x, int y)
{
    switch (a_keys) {
        case GLUT_KEY_F1:
            if (!g_gamemode) {
                g_fullscreen = !g_fullscreen;
                if (g_fullscreen) glutFullScreen();
                else glutReshapeWindow(WCX, WCY);
            }
            break;
        default:
            g_keys[a_keys] = true;
            break;
    }
}

// Our Keyboard Handler For Special Key Releases.
void special_keys_up(int key, int x, int y)
{
    g_keys[key] = false;
}

// Our Keyboard Handler For Normal Key Releases.
void normal_keys_up(unsigned char key, int x, int y)
{
    g_keys[key] = false;
}

// Ask The User If He Wish To Enter GameMode Or Not
void ask_gamemode()
{
    g_fullscreen = false;
}

// Main Function For Bringing It All Together.
int main(int argc, char** argv)
{
    ask_gamemode();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE);
    if (g_gamemode) {
        glutGameModeString("640x480:16");
        if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))
            glutEnterGameMode();
        else g_gamemode = false;
    }
    if (!g_gamemode) {
        glutInitWindowSize(WCX, WCY);
        glutCreateWindow("NeHe's OpenGL Framework");
    }
    if (!init()) {
        return -1;
    }
    glutDisplayFunc(render);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special_keys);
    glutKeyboardUpFunc(normal_keys_up);
    glutSpecialUpFunc(special_keys_up);
    glutIdleFunc(game_function);

    //ptsEM->lastResetTime = Fps_getProgTime();
    glutMainLoop();

    return 0;
}
