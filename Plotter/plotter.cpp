#include <stdlib.h>
#include <gl/freeglut.h>

int main(int argc, char* args[]){
	glutInit(&argc, args);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1000, 700);
	glutInitWindowPosition(30, 30);
	glutCreateWindow("Graph Plotter");
	InitializeGlutCallbacks();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glutMainLoop();
	return 0;
}