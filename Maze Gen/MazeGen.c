#include <stdlib.h>
#include <gl\glut.h>

#define WIDTH 900
#define HEIGHT 900
#define GRID 100
#define SPACING WIDTH/GRID
#define BUFFER SPACING/32.

typedef struct cell {
	int visited;
	int n,s,e,w;
	float dist;
};
typedef struct rgb {
	float r, g, b;
};
typedef struct hsl {
	float h, s, l;
};

struct cell defCell = { 0,0,0,0,0,0 };
static struct cell map[GRID][GRID];
int path[GRID*GRID];
int step;
int filled;
int maxstep;

int wx, wy;

struct rgb HSLtoRGB(struct hsl in) {
	if (in.h < 0) in.h++;
	if (in.s < 0) in.s = 0;
	if (in.l < 0) in.l = 0;
	struct rgb out;
	float temp1;
	float temp2;
	float tempr, tempg, tempb;
	if (in.s == 0) {
		out.r = in.l;
		out.g = in.l;
		out.b = in.l;
		return out;
	}
	else if (in.l < 0.5) {
		temp1 = in.l * (1.0 + in.s);
	}
	else if (in.l >= 0.5) {
		temp1 = in.l + in.s - in.l*in.s;
	}
	temp2 = 2 * in.l - temp1;
	tempr = in.h + 0.333; if (tempr > 1) tempr--;
	tempg = in.h;
	tempb = in.h - 0.333; if (tempb < 0) tempb++;

	if (6 * tempr < 1) {
		out.r = temp2 + (temp1 - temp2) * 6 * tempr;
	}
	else if (2 * tempr < 1) {
		out.r = temp1;
	}
	else if (3 * tempr < 2) {
		out.r = temp2 + (temp1 - temp2) * (0.666 - tempr) * 6;
	}
	else {
		out.r = temp2;
	}

	if (6 * tempg < 1) {
		out.g = temp2 + (temp1 - temp2) * 6 * tempg;
	}
	else if (2 * tempg < 1) {
		out.g = temp1;
	}
	else if (3 * tempg < 2) {
		out.g = temp2 + (temp1 - temp2) * (0.666 - tempg) * 6;
	}
	else {
		out.g = temp2;
	}

	if (6 * tempb < 1) {
		out.b = temp2 + (temp1 - temp2) * 6 * tempb;
	}
	else if (2 * tempb < 1) {
		out.b = temp1;
	}
	else if (3 * tempb < 2) {
		out.b = temp2 + (temp1 - temp2) * (0.666 - tempb) * 6;
	}
	else {
		out.b = temp2;
	}

	return out;
}

void drawCell(int x, int y, struct rgb color) {
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_QUADS);
	glVertex2i((x + 0.5) * SPACING - (SPACING / 2 - BUFFER), (y + 0.5) * SPACING - (SPACING / 2 - BUFFER));
	glVertex2i((x + 0.5) * SPACING + (SPACING / 2 - BUFFER), (y + 0.5) * SPACING - (SPACING / 2 - BUFFER));
	glVertex2i((x + 0.5) * SPACING + (SPACING / 2 - BUFFER), (y + 0.5) * SPACING + (SPACING / 2 - BUFFER));
	glVertex2i((x + 0.5) * SPACING - (SPACING / 2 - BUFFER), (y + 0.5) * SPACING + (SPACING / 2 - BUFFER));
	glEnd();
}

void drawPath(int x, int y, int e, int s, int w, int n, struct rgb color) {
	glColor3f(color.r, color.g, color.b);
	if (e != 0) {
		glBegin(GL_QUADS);
		glVertex2i((x + e) * SPACING - (SPACING / 2 - BUFFER), (y + 0.5) * SPACING - (SPACING / 2 - BUFFER));
		glVertex2i((x + e) * SPACING + (SPACING / 2 - BUFFER), (y + 0.5) * SPACING - (SPACING / 2 - BUFFER));
		glVertex2i((x + e) * SPACING + (SPACING / 2 - BUFFER), (y + 0.5) * SPACING + (SPACING / 2 - BUFFER));
		glVertex2i((x + e) * SPACING - (SPACING / 2 - BUFFER), (y + 0.5) * SPACING + (SPACING / 2 - BUFFER));
		glEnd();
	}
	if (s != 0) {
		glBegin(GL_QUADS);
		glVertex2i((x + 0.5) * SPACING - (SPACING / 2 - BUFFER), (y + s) * SPACING - (SPACING / 2 - BUFFER));
		glVertex2i((x + 0.5) * SPACING + (SPACING / 2 - BUFFER), (y + s) * SPACING - (SPACING / 2 - BUFFER));
		glVertex2i((x + 0.5) * SPACING + (SPACING / 2 - BUFFER), (y + s) * SPACING + (SPACING / 2 - BUFFER));
		glVertex2i((x + 0.5) * SPACING - (SPACING / 2 - BUFFER), (y + s) * SPACING + (SPACING / 2 - BUFFER));
		glEnd();
	}
	if (w != 0 && x > 0) {
		glBegin(GL_QUADS);
		glVertex2i((x) * SPACING - (SPACING / 2 - BUFFER), (y + 0.5) * SPACING - (SPACING / 2 - BUFFER));
		glVertex2i((x) * SPACING + (SPACING / 2 - BUFFER), (y + 0.5) * SPACING - (SPACING / 2 - BUFFER));
		glVertex2i((x) * SPACING + (SPACING / 2 - BUFFER), (y + 0.5) * SPACING + (SPACING / 2 - BUFFER));
		glVertex2i((x) * SPACING - (SPACING / 2 - BUFFER), (y + 0.5) * SPACING + (SPACING / 2 - BUFFER));
		glEnd();
	}
	if (n != 0 && y > 0) {
		glBegin(GL_QUADS);
		glVertex2i((x + 0.5) * SPACING - (SPACING / 2 - BUFFER), (y) * SPACING - (SPACING / 2 - BUFFER));
		glVertex2i((x + 0.5) * SPACING + (SPACING / 2 - BUFFER), (y) * SPACING - (SPACING / 2 - BUFFER));
		glVertex2i((x + 0.5) * SPACING + (SPACING / 2 - BUFFER), (y) * SPACING + (SPACING / 2 - BUFFER));
		glVertex2i((x + 0.5) * SPACING - (SPACING / 2 - BUFFER), (y) * SPACING + (SPACING / 2 - BUFFER));
		glEnd();
	}
}

int randDir(int x, int y) {
	int valid[4];
	int count = 0;
	if (map[x + 1][y].visited != 1 && x < GRID - 1) {
		valid[count] = 0;
		count++;
	}
	if (map[x][y + 1].visited != 1 && y < GRID - 1) {
		valid[count] = 1;
		count++;
	}
	if (map[x - 1][y].visited != 1 && x > 0) {
		valid[count] = 2;
		count++;
	}
	if (map[x][y - 1].visited != 1 && y > 0) {
		valid[count] = 3;
		count++;
	}
	if (count == 0) return -1;
	int dir = rand() % count;
	return valid[dir];
}

void reset(void) {
	for (int i = 0; i < GRID; i++) {
		for (int j = 0; j < GRID; j++) {
			map[i][j] = defCell;
		}
	}
	wx = GRID/2;
	wy = GRID/2;
	map[wx][wy].visited = 1;
	step = 0;
	maxstep = 0;
	filled = 0;
}

int walk(void) {
	switch (randDir(wx, wy))
	{
	case 0:
		map[wx][wy].e = 1;
		wx++;
		map[wx][wy].visited = 1;
		map[wx][wy].dist = step;
		path[step] = 0;
		step++;
		return 1;
	case 1:
		map[wx][wy].s = 1;
		wy++;
		map[wx][wy].visited = 1;
		map[wx][wy].dist = step;
		path[step] = 1;
		step++;
		return 1;
	case 2:
		map[wx][wy].w = -1;
		wx--;
		map[wx][wy].visited = 1;
		map[wx][wy].dist = step;
		path[step] = 2;
		step++;
		return 1;
	case 3:
		map[wx][wy].n = -1;
		wy--;
		map[wx][wy].visited = 1;
		map[wx][wy].dist = step;
		path[step] = 3;
		step++;
		return 1;
	case -1:
		return 0;
	}
}

void backtrack(void) {
	switch (path[step - 1])
	{
	case 0:
		wx--;
		step--;
		return;
	case 1:
		wy--;
		step--;
		return;
	case 2:
		wx++;
		step--;
		return;
	case 3:
		wy++;
		step--;
		return;
	case -1:
		return;
	}
}

void display() {

	for (int i = 0; i < GRID; i++) {
		for (int j = 0; j < GRID; j++) {
			if (map[i][j].visited == 1) {
				drawCell(i, j, HSLtoRGB((struct hsl) {1-map[i][j].dist/maxstep, 1, 0.5}));
				drawPath(i, j, map[i][j].e, map[i][j].s, map[i][j].w, map[i][j].n, HSLtoRGB((struct hsl) { 1. - map[i][j].dist/maxstep, 1, 0.5}));
			}
			else {
				drawCell(i, j, (struct rgb) {0,0,0.1});
			}
		}
	}

	drawCell(0, 0, (struct rgb) { 0.1, 0.9, 0.1 });
	drawCell(GRID - 1, GRID - 1, (struct rgb) {0.9, 0.1, 0.1});

	while (filled < GRID*GRID-1) {
	//for (int i = 0; i < 1/*GRID*GRID/200*/; i++) {
		if (walk() != 1 && filled < GRID*GRID - 1) {
			backtrack();
		}
		else filled++;
		if (step > maxstep) {
			maxstep = step;
		}
	}

	glutSwapBuffers();
	glutPostRedisplay();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void resize(int w, int h) {
	glutReshapeWindow(WIDTH, HEIGHT);
}

void init() {
	glClearColor(0, 0, 0, 0);
	gluOrtho2D(0, WIDTH, HEIGHT, 0);
	srand(time(NULL));
	reset();
}

void ButtonDown(unsigned char key, int x, int y) {
	if (key == 'r') {
		reset();
	}
	glutPostRedisplay();
}

void main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Maze Generator. Press 'r' to reset.");
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(ButtonDown);
	glutMainLoop();
}