#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>


const float PI = 3.14159265f;

const int   WIN_W = 1000;
const int   WIN_H = 600;


const float SIDEWALK_BOT_TOP = 40;
const float ROAD_BOTTOM      = 40;
const float ROAD_TOP         = 240;
const float SIDEWALK_TOP_TOP = 290;
const float BUILD_BASE       = 290;

const float CAR_Y_MIN = 50;
const float CAR_Y_MAX = 190;

const float PLAYER_X = 170;
const float CAR_W    = 120;

const float JUMP_POWER  = 15.0f;
const float GRAVITY     = 0.72f;
const float MOVE_SPEED  = 4.0f;
const int   SLIDE_FRAMES = 35;
const float START_SPEED = 5.0f;
const float MAX_SPEED   = 15.0f;

const int MAX_OBSTACLES = 10;
const int MAX_COINS     = 30;
const int NUM_BUILDINGS = 16;


const float PB_X1 = 920, PB_Y1 = 545, PB_X2 = 975, PB_Y2 = 590;


const int STATE_PLAYING  = 0;
const int STATE_PAUSED   = 1;
const int STATE_GAMEOVER = 2;


struct Obstacle {
    float x, y;
    float speed;
    int   type;
    float r, g, b;
    bool  active;
};

struct Coin {
    float x, y;
    float h;
    bool  active;
};



int   gameState = STATE_PLAYING;
int   winW = WIN_W, winH = WIN_H;

float gameSpeed;
float roadScroll;
float wheelAngle;
int   gameTime;

int   score, coinCount, bestScore = 0;


float playerY;
float jumpH;
float jumpV;
bool  jumping;
bool  sliding;
int   slideTimer;
bool  upHeld, downHeld;


Obstacle obs[MAX_OBSTACLES];
Coin     coins[MAX_COINS];
int      obsTimer;
int      coinTimer;


float bX[NUM_BUILDINGS], bW[NUM_BUILDINGS], bH[NUM_BUILDINGS];
float bR[NUM_BUILDINGS], bG[NUM_BUILDINGS], bB[NUM_BUILDINGS];
float totalBuildW;


const float palette[8][3] = {
    {0.95f,0.45f,0.45f}, {0.98f,0.70f,0.30f}, {0.40f,0.75f,0.50f}, {0.40f,0.60f,0.90f},
    {0.75f,0.50f,0.85f}, {0.95f,0.85f,0.40f}, {0.30f,0.80f,0.80f}, {0.90f,0.55f,0.70f}
};
const float obsColors[6][3] = {
    {0.20f,0.40f,0.90f}, {0.20f,0.70f,0.30f}, {0.95f,0.75f,0.10f},
    {0.60f,0.30f,0.80f}, {0.95f,0.50f,0.10f}, {0.20f,0.75f,0.80f}
};


void drawRect(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();
}


void drawEllipse(float cx, float cy, float rx, float ry)
{
    const int SEG = 28;
    glBegin(GL_POLYGON);
    for (int i = 0; i < SEG; i++)
    {
        float a = 2.0f * PI * i / SEG;
        glVertex2f(cx + rx * cosf(a), cy + ry * sinf(a));
    }
    glEnd();
}

void drawCircle(float cx, float cy, float r)
{
    drawEllipse(cx, cy, r, r);
}


void drawLine(float x1, float y1, float x2, float y2, float width)
{
    glLineWidth(width);
    glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    glEnd();
}


void drawText(float x, float y, const char* s, void* font)
{
    glRasterPos2f(x, y);
    for (; *s; s++) glutBitmapCharacter(font, *s);
}


void drawTextCentered(float cx, float y, const char* s, void* font)
{
    int w = glutBitmapLength(font, (const unsigned char*)s);
    drawText(cx - w / 2.0f, y, s, font);
}


void drawStrokeCentered(float cx, float y, const char* s, float scale)
{
    float w = 0;
    for (const char* p = s; *p; p++) w += (float)glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
    w *= scale;
    glPushMatrix();
        glTranslatef(cx - w / 2.0f, y, 0);
        glScalef(scale, scale, 1);
        for (const char* p = s; *p; p++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    glPopMatrix();
}

float depthScale(float y)
{
    return 1.0f - (y - CAR_Y_MIN) / (CAR_Y_MAX - CAR_Y_MIN) * 0.25f;
}


void drawSky()
{

    glBegin(GL_QUADS);
        glColor3f(0.80f, 0.93f, 1.00f);  glVertex2f(0, 0);
        glColor3f(0.80f, 0.93f, 1.00f);  glVertex2f(WIN_W, 0);
        glColor3f(0.45f, 0.76f, 0.96f);  glVertex2f(WIN_W, WIN_H);
        glColor3f(0.45f, 0.76f, 0.96f);  glVertex2f(0, WIN_H);
    glEnd();


    glColor4f(1.0f, 0.95f, 0.4f, 0.25f);
    drawCircle(880, 510, 62);
    glColor3f(1.0f, 0.93f, 0.30f);
    drawCircle(880, 510, 40);


    glColor3f(1.0f, 0.90f, 0.30f);
    for (int i = 0; i < 12; i++)
    {
        float a = 2.0f * PI * i / 12 + gameTime * 0.005f;
        drawLine(880 + 48 * cosf(a), 510 + 48 * sinf(a),
                 880 + 66 * cosf(a), 510 + 66 * sinf(a), 3);
    }


    static const float cloudX[5] = {100, 350, 600, 850, 1100};
    static const float cloudY[5] = {520, 480, 545, 495, 530};
    glColor3f(1, 1, 1);
    for (int i = 0; i < 5; i++)
    {
        float x = cloudX[i] - fmodf(roadScroll * 0.08f, 1300.0f);
        if (x < -120) x += 1300;
        drawEllipse(x,       cloudY[i],      40, 16);
        drawEllipse(x - 25,  cloudY[i] - 5,  28, 12);
        drawEllipse(x + 28,  cloudY[i] - 4,  30, 13);
        drawEllipse(x + 5,   cloudY[i] + 10, 24, 14);
    }
}


void initBuildings()
{
    float x = 0;
    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        bW[i] = 80.0f + rand() % 50;
        bH[i] = 120.0f + rand() % 140;
        bX[i] = x;
        int c = rand() % 8;
        bR[i] = palette[c][0];
        bG[i] = palette[c][1];
        bB[i] = palette[c][2];
        x += bW[i] + 6;
    }
    totalBuildW = x;
}


void drawBuilding(int i, float x)
{
    float w = bW[i], h = bH[i], y = BUILD_BASE;


    glColor3f(bR[i], bG[i], bB[i]);
    drawRect(x, y, x + w, y + h);


    glColor3f(bR[i] * 0.6f, bG[i] * 0.6f, bB[i] * 0.6f);
    drawRect(x - 3, y + h, x + w + 3, y + h + 8);


    if (i % 4 == 0)
    {
        glColor3f(bR[i] * 0.5f, bG[i] * 0.5f, bB[i] * 0.5f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 3,     y + h + 8);
            glVertex2f(x + w + 3, y + h + 8);
            glVertex2f(x + w / 2, y + h + 38);
        glEnd();
    }
    else if (i % 4 == 1)
    {
        glColor3f(0.3f, 0.3f, 0.3f);
        drawLine(x + w / 2, y + h + 8, x + w / 2, y + h + 35, 2);
        glColor3f(1.0f, 0.2f, 0.2f);
        drawCircle(x + w / 2, y + h + 36, 3);
    }


    int row = 0;
    for (float wy = y + 15; wy + 18 < y + h - 5; wy += 32, row++)
    {
        int col = 0;
        for (float wx = x + 10; wx + 12 < x + w - 6; wx += 24, col++)
        {
            int k = (row * 7 + col * 3 + i) % 3;
            if (k == 0) glColor3f(1.00f, 0.95f, 0.60f);
            else if (k == 1) glColor3f(0.70f, 0.90f, 1.00f);
            else   glColor3f(bR[i] * 0.7f, bG[i] * 0.7f, bB[i] * 0.7f);
            drawRect(wx, wy, wx + 12, wy + 18);
        }
    }
}


void drawBuildings()
{
    float off = fmodf(roadScroll * 0.25f, totalBuildW);
    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        float x = bX[i] - off;
        if (x + bW[i] < 0) x += totalBuildW;
        if (x < WIN_W + 10) drawBuilding(i, x);
    }
}


void drawBridge()
{
    const float bw = 800;
    float bx = 1100.0f - fmodf(roadScroll * 0.5f, 2300.0f);
    if (bx > WIN_W || bx + bw < 0) return;

    float deckBottom = 340, deckTop = 360;


    glColor3f(0.50f, 0.50f, 0.55f);
    drawRect(bx + 70,       SIDEWALK_TOP_TOP, bx + 105,       deckBottom);
    drawRect(bx + bw / 2 - 17, SIDEWALK_TOP_TOP, bx + bw / 2 + 17, deckBottom);
    drawRect(bx + bw - 105, SIDEWALK_TOP_TOP, bx + bw - 70,   deckBottom);


    glColor3f(0.35f, 0.35f, 0.40f);
    drawRect(bx, deckBottom, bx + bw, deckBottom + 6);
    glColor3f(0.62f, 0.62f, 0.68f);
    drawRect(bx, deckBottom + 6, bx + bw, deckTop);


    float x0 = bx + 50, x1 = bx + bw - 50, archH = 95;
    glColor3f(0.85f, 0.15f, 0.15f);
    glLineWidth(6);
    glBegin(GL_LINES);
    for (int s = 0; s < 24; s++)
    {
        float t0 = s / 24.0f, t1 = (s + 1) / 24.0f;
        glVertex2f(x0 + (x1 - x0) * t0, deckTop + archH * sinf(PI * t0));
        glVertex2f(x0 + (x1 - x0) * t1, deckTop + archH * sinf(PI * t1));
    }
    glEnd();


    glColor3f(0.45f, 0.45f, 0.50f);
    glLineWidth(2);
    glBegin(GL_LINES);
    for (int k = 1; k < 12; k++)
    {
        float t = k / 12.0f;
        float x = x0 + (x1 - x0) * t;
        glVertex2f(x, deckTop);
        glVertex2f(x, deckTop + archH * sinf(PI * t));
    }
    glEnd();
}


void drawRoad()
{

    glColor3f(0.78f, 0.78f, 0.80f);
    drawRect(0, 0, WIN_W, SIDEWALK_BOT_TOP);
    glColor3f(0.58f, 0.58f, 0.60f);
    drawRect(0, SIDEWALK_BOT_TOP - 5, WIN_W, SIDEWALK_BOT_TOP);
    glColor3f(0.65f, 0.65f, 0.68f);
    glLineWidth(1);
    glBegin(GL_LINES);
    for (int i = -1; i < 16; i++)
    {
        float x = i * 70.0f - fmodf(roadScroll, 70.0f);
        glVertex2f(x, 0);
        glVertex2f(x, SIDEWALK_BOT_TOP - 5);
    }
    glEnd();


    glBegin(GL_QUADS);
        glColor3f(0.22f, 0.22f, 0.25f);  glVertex2f(0,     ROAD_BOTTOM);
        glColor3f(0.22f, 0.22f, 0.25f);  glVertex2f(WIN_W, ROAD_BOTTOM);
        glColor3f(0.34f, 0.34f, 0.38f);  glVertex2f(WIN_W, ROAD_TOP);
        glColor3f(0.34f, 0.34f, 0.38f);  glVertex2f(0,     ROAD_TOP);
    glEnd();


    glColor3f(0.95f, 0.95f, 0.95f);
    drawRect(0, ROAD_BOTTOM + 4, WIN_W, ROAD_BOTTOM + 8);
    drawRect(0, ROAD_TOP - 8,    WIN_W, ROAD_TOP - 4);


    float shift = fmodf(roadScroll, 120.0f);
    for (int i = -1; i < 10; i++)
    {
        float x = i * 120.0f - shift;
        glColor3f(0.95f, 0.95f, 0.95f);
        drawRect(x, 98, x + 60, 102);
        drawRect(x, 178, x + 60, 182);
        glColor3f(1.0f, 0.85f, 0.1f);
        drawRect(x + 30, 138, x + 90, 142);
    }


    glColor3f(0.82f, 0.82f, 0.84f);
    drawRect(0, ROAD_TOP, WIN_W, SIDEWALK_TOP_TOP);
    glColor3f(0.58f, 0.58f, 0.60f);
    drawRect(0, ROAD_TOP, WIN_W, ROAD_TOP + 6);
    glColor3f(0.68f, 0.68f, 0.70f);
    glLineWidth(1);
    glBegin(GL_LINES);
    for (int i = -1; i < 16; i++)
    {
        float x = i * 70.0f - fmodf(roadScroll, 70.0f);
        glVertex2f(x, ROAD_TOP + 6);
        glVertex2f(x, SIDEWALK_TOP_TOP);
    }
    glEnd();
}


void drawTree(float x, float y)
{
    glColor3f(0.45f, 0.28f, 0.12f);
    drawRect(x - 6, y, x + 6, y + 40);
    glColor3f(0.15f, 0.60f, 0.20f);
    drawCircle(x,      y + 58, 26);
    glColor3f(0.20f, 0.70f, 0.25f);
    drawCircle(x - 18, y + 42, 19);
    drawCircle(x + 18, y + 42, 19);
}


void drawStreetlight(float x, float y)
{
    glColor3f(0.25f, 0.25f, 0.30f);
    drawRect(x - 9, y, x + 9, y + 8);
    drawRect(x - 3, y, x + 3, y + 95);
    drawRect(x - 3, y + 90, x + 30, y + 96);
    drawRect(x + 22, y + 84, x + 38, y + 90);

    glColor4f(1.0f, 0.95f, 0.4f, 0.30f);
    drawCircle(x + 30, y + 80, 16);
    glColor3f(1.0f, 0.95f, 0.4f);
    drawCircle(x + 30, y + 81, 4);
}


void drawRoadside()
{
    int base = (int)(roadScroll / 200.0f);
    for (int k = -1; k < 7; k++)
    {
        int   idx = base + k;
        float sx  = idx * 200.0f - roadScroll + 100.0f;
        if (sx < -70 || sx > WIN_W + 70) continue;
        if (idx % 2 == 0) drawTree(sx, ROAD_TOP + 12);
        else              drawStreetlight(sx, ROAD_TOP + 12);
    }
}


void drawWheel(float cx, float cy)
{
    glColor3f(0.08f, 0.08f, 0.08f);
    drawCircle(cx, cy, 13);
    glColor3f(0.70f, 0.70f, 0.74f);
    drawCircle(cx, cy, 7);
    glColor3f(0.2f, 0.2f, 0.2f);
    for (int k = 0; k < 3; k++)
    {
        float a = (wheelAngle + k * 60.0f) * PI / 180.0f;
        drawLine(cx - 6 * cosf(a), cy - 6 * sinf(a),
                 cx + 6 * cosf(a), cy + 6 * sinf(a), 2);
    }
    glColor3f(0.9f, 0.9f, 0.9f);
    drawCircle(cx, cy, 2);
}


void drawCarShape(float r, float g, float b, int type)
{
    if (type == 0)
    {

        glColor3f(r, g, b);
        glBegin(GL_POLYGON);
            glVertex2f(4, 15);
            glVertex2f(116, 15);
            glVertex2f(116, 32);
            glVertex2f(104, 40);
            glVertex2f(18, 40);
            glVertex2f(4, 34);
        glEnd();


        glColor3f(r * 0.7f, g * 0.7f, b * 0.7f);
        drawRect(6, 24, 114, 27);


        glColor3f(r * 0.85f, g * 0.85f, b * 0.85f);
        glBegin(GL_POLYGON);
            glVertex2f(28, 40);
            glVertex2f(42, 60);
            glVertex2f(86, 60);
            glVertex2f(100, 40);
        glEnd();


        glColor3f(0.70f, 0.90f, 1.00f);
        glBegin(GL_POLYGON);
            glVertex2f(35, 41);
            glVertex2f(45, 56);
            glVertex2f(83, 56);
            glVertex2f(93, 41);
        glEnd();


        glColor3f(r * 0.85f, g * 0.85f, b * 0.85f);
        drawRect(63, 41, 66, 57);
    }
    else
    {

        glColor3f(r, g, b);
        glBegin(GL_POLYGON);
            glVertex2f(4, 15);
            glVertex2f(116, 15);
            glVertex2f(116, 38);
            glVertex2f(100, 56);
            glVertex2f(4, 56);
        glEnd();


        glColor3f(0.95f, 0.95f, 0.95f);
        drawRect(4, 24, 116, 29);


        glColor3f(0.70f, 0.90f, 1.00f);
        drawRect(12, 36, 40, 50);
        drawRect(46, 36, 76, 50);
        glBegin(GL_POLYGON);
            glVertex2f(84, 36);
            glVertex2f(110, 36);
            glVertex2f(98, 51);
            glVertex2f(84, 51);
        glEnd();
    }


    glColor3f(0.65f, 0.65f, 0.70f);
    drawRect(112, 15, 119, 25);
    drawRect(1, 15, 8, 25);


    glColor3f(1.0f, 0.95f, 0.3f);
    drawCircle(111, 29, 4);
    glColor3f(1.0f, 0.45f, 0.0f);
    drawRect(3, 28, 8, 34);


    drawWheel(28, 14);
    drawWheel(92, 14);
}


void drawMonkey()
{
    float bob = sinf(gameTime * 0.3f) * 1.5f;


    glColor3f(0.45f, 0.27f, 0.10f);
    glLineWidth(4);
    glBegin(GL_LINES);
    for (int i = 0; i < 8; i++)
    {
        float x1 = 48 - i * 5.0f,  y1 = 46 + i * 1.5f  + 5 * sinf(gameTime * 0.2f + i * 0.7f);
        float x2 = 48 - (i + 1) * 5.0f, y2 = 46 + (i + 1) * 1.5f + 5 * sinf(gameTime * 0.2f + (i + 1) * 0.7f);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
    glEnd();


    glColor3f(0.55f, 0.33f, 0.15f);
    drawEllipse(62, 48, 14, 12);
    glColor3f(0.95f, 0.80f, 0.55f);
    drawEllipse(64, 47, 8, 8);


    glColor3f(0.15f, 0.15f, 0.15f);
    drawEllipse(92, 45, 3, 9);
    glColor3f(0.55f, 0.33f, 0.15f);
    drawLine(68, 52, 90, 46, 5);
    drawCircle(90, 46, 4);


    glColor3f(0.55f, 0.33f, 0.15f);
    drawCircle(46, 68 + bob, 7);
    drawCircle(78, 68 + bob, 7);
    glColor3f(0.95f, 0.80f, 0.55f);
    drawCircle(46, 68 + bob, 4);
    drawCircle(78, 68 + bob, 4);


    glColor3f(0.55f, 0.33f, 0.15f);
    drawCircle(62, 66 + bob, 16);

    glColor3f(0.95f, 0.80f, 0.55f);
    drawCircle(56, 69 + bob, 7);
    drawCircle(68, 69 + bob, 7);
    drawEllipse(62, 63 + bob, 11, 9);


    glColor3f(1, 1, 1);
    drawCircle(57, 70 + bob, 3.5f);
    drawCircle(67, 70 + bob, 3.5f);
    glColor3f(0, 0, 0);
    drawCircle(58.2f, 70 + bob, 1.8f);
    drawCircle(68.2f, 70 + bob, 1.8f);


    glColor3f(0.35f, 0.2f, 0.1f);
    drawEllipse(62, 65 + bob, 2.5f, 1.8f);


    glColor3f(0.25f, 0.1f, 0.05f);
    glLineWidth(2);
    glBegin(GL_LINES);
    for (int i = 0; i < 8; i++)
    {
        float a1 = PI * (1.15f + 0.7f * i / 8.0f);
        float a2 = PI * (1.15f + 0.7f * (i + 1) / 8.0f);
        glVertex2f(62 + 6 * cosf(a1), 63 + bob + 4 * sinf(a1));
        glVertex2f(62 + 6 * cosf(a2), 63 + bob + 4 * sinf(a2));
    }
    glEnd();
}


void drawPlayer()
{
    float s = depthScale(playerY);


    float sh = 1.0f - jumpH / 200.0f;
    if (sh < 0.4f) sh = 0.4f;
    glColor4f(0.0f, 0.0f, 0.0f, 0.30f);
    drawEllipse(PLAYER_X + 60 * s, playerY, 55 * s * sh, 6 * s);

    glPushMatrix();
        glTranslatef(PLAYER_X, playerY + jumpH, 0);
        glScalef(s, sliding ? s * 0.7f : s, 1);


        glTranslatef(60, 20, 0);
        glRotatef(jumpV * 0.8f, 0, 0, 1);
        glTranslatef(-60, -20, 0);


        for (int i = 0; i < 3; i++)
        {
            glColor4f(0.8f, 0.8f, 0.8f, 0.45f - i * 0.12f);
            drawCircle(-6 - i * 14.0f, 14 + 3 * sinf(gameTime * 0.4f + i), 5.0f + i * 2);
        }


        if (sliding)
        {
            glColor3f(1.0f, 0.65f, 0.1f);
            glBegin(GL_TRIANGLES);
            for (int i = 0; i < 3; i++)
            {
                float fx = -4 - i * 9.0f - (gameTime % 3) * 2;
                glVertex2f(fx, 1);
                glVertex2f(fx - 8, 4 + i);
                glVertex2f(fx,7);
            }
            glEnd();
        }

        drawCarShape(1.0f, 0.10f, 0.10f, 0);
        drawMonkey();
}

void drawObstacle(const Obstacle& o)
{
    float s = depthScale(o.y);

    glColor4f(0.0f, 0.0f, 0.0f, 0.30f);
    drawEllipse(o.x + 60 * s, o.y, 55 * s, 6 * s);

    glPushMatrix();
        glTranslatef(o.x + CAR_W * s, o.y, 0);
        glScalef(-s, s, 1);
        drawCarShape(o.r, o.g, o.b, o.type);
    glPopMatrix();
}

void drawCoin(const Coin& c)
{
    float s = depthScale(c.y);


    glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
    drawEllipse(c.x, c.y, 10 * s, 3 * s);


    float spin = fabsf(cosf(gameTime * 0.1f + c.x * 0.02f));
    if (spin < 0.25f) spin = 0.25f;

    glPushMatrix();
        glTranslatef(c.x, c.y, 0);
        glScalef(s, s, 1);
        float cy = c.h + 14;
        glColor3f(1.0f, 0.80f, 0.0f);
        drawEllipse(0, cy, 13 * spin, 13);
        glColor3f(1.0f, 0.93f, 0.35f);
        drawEllipse(0, cy, 9 * spin, 9);
        glColor3f(0.85f, 0.62f, 0.0f);
        drawRect(-2 * spin, cy - 5, 2 * spin, cy + 5);
    glPopMatrix();
}


void drawHUD()
{
    char buf[64];


    glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
    drawRect(10, 520, 270, 590);

    sprintf(buf, "SCORE: %d", score);
    glColor3f(1.0f, 0.95f, 0.3f);
    drawText(22, 562, buf, GLUT_BITMAP_TIMES_ROMAN_24);


    glColor3f(1.0f, 0.80f, 0.0f);
    drawCircle(30, 538, 8);
    glColor3f(1.0f, 0.93f, 0.35f);
    drawCircle(30, 538, 5);
    sprintf(buf, "x %d  Speed: %d km/h", coinCount, (int)(gameSpeed * 10));
    glColor3f(1, 1, 1);
    drawText(45, 533, buf, GLUT_BITMAP_HELVETICA_18);


    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    drawRect(PB_X1, PB_Y1, PB_X2, PB_Y2);
    glColor3f(1, 1, 1);
    if (gameState == STATE_PAUSED)
    {
        glBegin(GL_TRIANGLES);
            glVertex2f(PB_X1 + 18, PB_Y1 + 9);
            glVertex2f(PB_X1 + 18, PB_Y2 - 9);
            glVertex2f(PB_X2 - 14, (PB_Y1 + PB_Y2) / 2);
        glEnd();
    }
    else
    {
        drawRect(PB_X1 + 15, PB_Y1 + 9, PB_X1 + 25, PB_Y2 - 9);
        drawRect(PB_X2 - 25, PB_Y1 + 9, PB_X2 - 15, PB_Y2 - 9);
    }


    glColor3f(0.25f, 0.25f, 0.30f);
    drawText(15, 14,
        "SPACE: Jump   DOWN: Slide   UP/W, S: Move   P: Pause   R: Restart   ESC: Exit",
        GLUT_BITMAP_HELVETICA_12);
}


void drawOverlay()
{
    if (gameState == STATE_PLAYING) return;


    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    drawRect(0, 0, WIN_W, WIN_H);


    glColor3f(0.95f, 0.95f, 0.95f);
    drawRect(290, 170, 710, 430);
    glColor3f(0.15f, 0.15f, 0.25f);
    drawRect(296, 176, 704, 424);

    char buf[64];
    if (gameState == STATE_GAMEOVER)
    {
        glColor3f(1.0f, 0.25f, 0.25f);
        glLineWidth(4);
        drawStrokeCentered(500, 340, "GAME OVER", 0.42f);

        sprintf(buf, "Score: %d", score);
        glColor3f(1.0f, 0.95f, 0.3f);
        drawTextCentered(500, 290, buf, GLUT_BITMAP_TIMES_ROMAN_24);

        sprintf(buf, "Coins: %d     Best: %d", coinCount, bestScore);
        glColor3f(1, 1, 1);
        drawTextCentered(500, 255, buf, GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.5f, 1.0f, 0.5f);
        drawTextCentered(500, 205, "Press R to Restart", GLUT_BITMAP_TIMES_ROMAN_24);
    }
    else
    {
        glColor3f(0.4f, 0.8f, 1.0f);
        glLineWidth(4);
        drawStrokeCentered(500, 320, "PAUSED", 0.5f);

        glColor3f(1, 1, 1);
        drawTextCentered(500, 250, "Press P to Resume", GLUT_BITMAP_TIMES_ROMAN_24);
        drawTextCentered(500, 215, "Press R to Restart", GLUT_BITMAP_HELVETICA_18);
    }
}
void resetGame()
{
    gameState  = STATE_PLAYING;
    gameSpeed  = START_SPEED;
    roadScroll = 0;
    wheelAngle = 0;
    gameTime   = 0;
    score      = 0;
    coinCount  = 0;

    playerY    = 120;
    jumpH = 0;  jumpV = 0;
    jumping = false;
    sliding = false;  slideTimer = 0;

    for (int i = 0; i < MAX_OBSTACLES; i++) obs[i].active = false;
    for (int i = 0; i < MAX_COINS; i++)     coins[i].active = false;

    obsTimer  = 80;
    coinTimer = 40;
}

void spawnObstacle()
{

    for (int i = 0; i < MAX_OBSTACLES; i++)
        if (obs[i].active && obs[i].x > WIN_W - 300) { obsTimer = 5; return; }

    for (int i = 0; i < MAX_OBSTACLES; i++)
    {
        if (!obs[i].active)
        {
            obs[i].active = true;
            obs[i].x = WIN_W + 20;


            if (rand() % 2 == 0)
                obs[i].y = playerY + (rand() % 60 - 30);
            else
                obs[i].y = CAR_Y_MIN + rand() % (int)(CAR_Y_MAX - CAR_Y_MIN + 1);
            if (obs[i].y < CAR_Y_MIN) obs[i].y = CAR_Y_MIN;
            if (obs[i].y > CAR_Y_MAX) obs[i].y = CAR_Y_MAX;

            obs[i].speed = 0.5f + (rand() % 20) / 10.0f;
            obs[i].type  = (rand() % 3 == 0) ? 1 : 0;
            int c = rand() % 6;
            obs[i].r = obsColors[c][0];
            obs[i].g = obsColors[c][1];
            obs[i].b = obsColors[c][2];
            return;
        }
    }
}

void spawnCoinRow()
{
    int   pattern = rand() % 3;
    float y = 0;

    for (int tries = 0; tries < 8; tries++)
    {
        y = CAR_Y_MIN + rand() % (int)(CAR_Y_MAX - CAR_Y_MIN + 1);
        bool ok = true;
        for (int i = 0; i < MAX_OBSTACLES; i++)
            if (obs[i].active && obs[i].x > WIN_W - 150 && fabsf(obs[i].y - y) < 45)
                ok = false;
        if (ok) break;
    }

    int placed = 0;
    for (int i = 0; i < MAX_COINS && placed < 5; i++)
    {
        if (!coins[i].active)
        {
            coins[i].active = true;
            coins[i].x = WIN_W + 30 + placed * 45.0f;
            coins[i].y = y;
            if (pattern == 0)      coins[i].h = 0;
            else if (pattern == 1) coins[i].h = 75;
            else                   coins[i].h = 80.0f * sinf(PI * placed / 4.0f);
            placed++;
        }
    }
}

bool checkCollision(const Obstacle& o)
{
    float ps = depthScale(playerY);
    float os = depthScale(o.y);

    float inset = sliding ? 24.0f : 12.0f;
    float pL = PLAYER_X + inset,  pR = PLAYER_X + CAR_W * ps - inset;
    float oL = o.x + 8,           oR = o.x + CAR_W * os - 8;
    if (pR < oL || pL > oR) return false;

    float depthLimit = sliding ? 18.0f : 26.0f;
    if (fabsf(playerY - o.y) > depthLimit) return false;


    float oTop = ((o.type == 0) ? 45.0f : 52.0f) * os;
    if (jumpH > oTop * 0.9f) return false;

    return true;
}

void updateGame()
{
    gameTime++;


    gameSpeed += 0.0025f;
    if (gameSpeed > MAX_SPEED) gameSpeed = MAX_SPEED;

    roadScroll += gameSpeed;
    wheelAngle += gameSpeed * 4.0f;
    if (wheelAngle > 360) wheelAngle -= 360;


    if (gameTime % 30 == 0) score += 1;


    if (upHeld)   playerY += MOVE_SPEED;
    if (downHeld) playerY -= MOVE_SPEED;
    if (playerY < CAR_Y_MIN) playerY = CAR_Y_MIN;
    if (playerY > CAR_Y_MAX) playerY = CAR_Y_MAX;


    if (jumping)
    {
        jumpH += jumpV;
        jumpV -= GRAVITY;
        if (jumpH <= 0) { jumpH = 0; jumpV = 0; jumping = false; }
    }


    if (sliding)
    {
        slideTimer--;
        if (slideTimer <= 0) sliding = false;
    }


    for (int i = 0; i < MAX_OBSTACLES; i++)
    {
        if (!obs[i].active) continue;
        obs[i].x -= (gameSpeed + obs[i].speed);
        if (obs[i].x < -200) obs[i].active = false;

        if (checkCollision(obs[i]))
        {
            gameState = STATE_GAMEOVER;
            if (score > bestScore) bestScore = score;
            return;
        }
    }

    float ps  = depthScale(playerY);
    float pcx = PLAYER_X + 60 * ps;
    for (int i = 0; i < MAX_COINS; i++)
    {
        if (!coins[i].active) continue;
        coins[i].x -= gameSpeed;
        if (coins[i].x < -50) { coins[i].active = false; continue; }

        if (fabsf(coins[i].x - pcx) < 45 && fabsf(coins[i].y - playerY) < 30)
        {
            float coinMid = coins[i].h + 14;
            float carTop  = jumpH + (sliding ? 45.0f : 80.0f);
            if (coinMid >= jumpH - 5 && coinMid <= carTop)
            {
                coins[i].active = false;
                coinCount++;
                score += 10;
            }
        }
    }


    obsTimer--;
    if (obsTimer <= 0)
    {
        spawnObstacle();
        if (obsTimer <= 0)
        {
            obsTimer = (int)((60 + rand() % 50) * (6.0f / gameSpeed));
            if (obsTimer < 30) obsTimer = 30;
        }
    }

    coinTimer--;
    if (coinTimer <= 0)
    {
        spawnCoinRow();
        coinTimer = (int)((90 + rand() % 90) * (6.0f / gameSpeed));
        if (coinTimer < 40) coinTimer = 40;
    }
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();


    drawSky();
    drawBuildings();
    drawBridge();
    drawRoad();
    drawRoadside();


    int order[MAX_OBSTACLES];
    int n = 0;
    for (int i = 0; i < MAX_OBSTACLES; i++)
        if (obs[i].active) order[n++] = i;
    for (int i = 1; i < n; i++)
    {
        int key = order[i];
        int j = i - 1;
        while (j >= 0 && obs[order[j]].y < obs[key].y)
        {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }


    for (int i = 0; i < MAX_COINS; i++)
        if (coins[i].active && coins[i].y > playerY) drawCoin(coins[i]);
    for (int k = 0; k < n; k++)
        if (obs[order[k]].y > playerY) drawObstacle(obs[order[k]]);


    drawPlayer();


    for (int k = 0; k < n; k++)
        if (obs[order[k]].y <= playerY) drawObstacle(obs[order[k]]);
    for (int i = 0; i < MAX_COINS; i++)
        if (coins[i].active && coins[i].y <= playerY) drawCoin(coins[i]);

    drawHUD();
    drawOverlay();

    glutSwapBuffers();
}


void togglePause()
{
    if (gameState == STATE_PLAYING)gameState = STATE_PAUSED;
    else if (gameState == STATE_PAUSED)gameState = STATE_PLAYING;
}


void keyboardDown(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:
            exit(0);
            break;

        case ' ':
            if (gameState == STATE_PLAYING && !jumping)
            {
                jumping = true;
                jumpV   = JUMP_POWER;
                sliding = false;
                slideTimer = 0;
            }
            break;

        case 'r': case 'R':
            resetGame();
            break;

        case 'p': case 'P':
            togglePause();
            break;

        case 'w': case 'W': upHeld   = true; break;
        case 's': case 'S': downHeld = true; break;
    }
}


void keyboardUp(unsigned char key, int x, int y)
{
    if (key == 'w' || key == 'W') upHeld   = false;
    if (key == 's' || key == 'S') downHeld = false;
}


void specialDown(int key, int x, int y)
{
    if (key == GLUT_KEY_UP) upHeld = true;

    if (key == GLUT_KEY_DOWN)
    {
        downHeld = true;
        if (gameState == STATE_PLAYING && !jumping && !sliding)
        {
            sliding    = true;
            slideTimer = SLIDE_FRAMES;
        }
    }
}


void specialUp(int key, int x, int y)
{
    if (key == GLUT_KEY_UP)   upHeld   = false;
    if (key == GLUT_KEY_DOWN) downHeld = false;
}


void mouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {

        float x = mx * (float)WIN_W / winW;
        float y = (winH - my) * (float)WIN_H / winH;
        if (x >= PB_X1 && x <= PB_X2 && y >= PB_Y1 && y <= PB_Y2)
            togglePause();
    }
}


void timer(int value)
{
    if (gameState == STATE_PLAYING) updateGame();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void reshape(int w, int h)
{
    if (h == 0) h = 1;
    winW = w;
    winH = h;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init()
{
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);

    srand((unsigned int)time(0));
    initBuildings();
    resetGame();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("Monkey Road Rush");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutMouseFunc(mouse);
    glutIgnoreKeyRepeat(1);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
