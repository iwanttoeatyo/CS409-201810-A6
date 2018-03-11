/*
 *
 *	Main.cpp
 *	Assignment 3
 *	Author: Chris Boyd, 200225231
 *
 *	Description:
 *		An OpenGL game using ObjLibrary and a modified ObjShader for shadows and lighting.
 *		Opens a freeglut window, loads in the Obj shader models. Reads a world file and loads a world.
 *		Draws to a depth texture. Renders to the screen with lighting and shadows.
 *		Generates Height maps for 5 different disk types. Creates models using triangle lists in vbos.
 *		Loads in a rod and ring at the position of each disk center. Rings move around the map to a new
 *		disk center when they reach one they choose a new disk target to move towards.
 *
 *		Rings and rods are worth points. The player can collect them for points.
 *
 *
 *		KEY BINDINGS:
 *		[Tab]: Loads the next file in the world folder.
 *		WASD: Move/Strafe
 *		Arrow keys: Move
 *		Left mouse button: Hold down to look around
 *		Right mouse button: Hold down to turn.
 *		Shift: Hold to move and turn faster.
 *		Left/Right mouse buttons: Hold down to move forward.
 *	    O: Hold to change to overview camera.
 *		H: to toggle between full and half speed time scale.
 *		L: Toggles Lighting
 *
 *
 */

#include <cstdlib>
#include <string>
#include <iostream>
#include <ctime>

 //GLM inclues
#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"

//GLUT / OBJ inclues
#include "lib/GetGlutWithShaders.h"
#include "lib/ObjLibrary/ObjShader.h"

#include "lib/ObjLibrary/LightingManager.h"


//My includes

#include "Sleep.h"
#include "MathHelper.h"
#include "Random.h"
#include "Game.h"
#include "PerformanceCounter.h"
#include "LineRenderer.h"
#include "TextRenderer.h"
#include "Globals.h"
#include "main.h"


using namespace std;
using namespace ObjLibrary;

using MathHelper::M_PI;
using MathHelper::M_PI_2;


int main(int argc, char* argv[])
{
	srand(unsigned(time(nullptr)));
	Random::init();

	glutInitWindowSize(g_win_width, g_win_height);
	glutInitWindowPosition(0, 0);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB | GLUT_MULTISAMPLE);
	glutCreateWindow("Disk Game");

	// add shader initialization here
	const int gl3w_err = gl3wInit();
	if (gl3w_err != 0)
	{
		// Problem: gl3wInit failed, something is seriously wrong.
		cerr << "gl3wInit failed, aborting." << endl;
		exit(1);
	}
	printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));

	//Keyboard input
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);

	//Mouse input
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);

	//Update
	glutIdleFunc(update);
	//Reshape
	glutReshapeFunc(reshape);
	//Render
	glutDisplayFunc(display);
	//Load models and set up lighting
	init();

	glutMainLoop();
	return 1;
}



void init()
{
	glClearColor(0.2f, 0.4f, 0.6f, 0.0f);

	glEnable(GL_DEPTH_TEST);

	g_text_renderer.init();
	g_line_renderer.init();


	game.init();
	g_depth_texture.init();

	g_time_counter.start();
}




//Read keyboard input
void keyboard(unsigned char key, int x, int y)
{
	//Convert lower case to uppercase
	if (key >= 'a' && key <= 'z')
		key = key - 'a' + 'A';

	//These should only happen on key down not hold
	switch (key)
	{
	case 'H':
		//Time Scale to half speed
		if (!g_key_pressed['H'])
		{
			if (g_time_scale == 1.0f)
				g_time_scale = 0.25f;
			else
				g_time_scale = 1.0f;
		}
		break;

		//Tab cycle between worlds on win32
#ifdef _WIN32
	case '\t':
		if (!g_key_pressed['\t'])
			game.destroyIntoNextWorld();
		break;
#endif
	case 'L':
		LightingManager::setEnabled(!LightingManager::isEnabled());
		break;
	case 27: // on [ESC]
		exit(0); // normal exit
	default:;
	}

	g_key_pressed[key] = true;

	g_key_pressed[KEY_SHIFT] = (glutGetModifiers() == GLUT_ACTIVE_SHIFT);
	g_key_pressed[KEY_LEFT_ALT] = (glutGetModifiers() == GLUT_ACTIVE_ALT);

	//Alt-Enter will change to and from fullscreen
	if (g_key_pressed[KEY_LEFT_ALT] && g_key_pressed[13] && g_fullscreen_toggle_allowed)
	{
		g_full_screen = !g_full_screen;
		g_fullscreen_toggle_allowed = false;

		if (g_full_screen)
			glutFullScreen();
		else
		{
			g_win_width = 1280;
			g_win_height = 960;
			glutReshapeWindow(g_win_width, g_win_height);
			glutPositionWindow(0, 0);
		}
	}
}

//Read keyboard input on key up
void keyboardUp(unsigned char key, int x, int y)
{
	//Convert all keys to uppercase
	if (key >= 'a' && key <= 'z')
		key = key - 'a' + 'A';

	//Unpress the key
	g_key_pressed[key] = false;


	//Fix sticky key with shift
	if (key == '?' || key == '/')
	{
		g_key_pressed['?'] = false;
		g_key_pressed['/'] = false;
	}

	switch (key)
	{
	case 13:
		g_fullscreen_toggle_allowed = true;
		break;
	default:;
	}

	//Set special keys
	g_key_pressed[KEY_SHIFT] = (glutGetModifiers() == GLUT_ACTIVE_SHIFT);
	g_key_pressed[KEY_LEFT_ALT] = (glutGetModifiers() == GLUT_ACTIVE_ALT);

}

//Read keyboard input on key down
void special(const int special_key, int x, int y)
{
	switch (special_key)
	{
	case GLUT_KEY_SHIFT_L:
		g_key_pressed[KEY_SHIFT] = true;
		break;
	case GLUT_KEY_UP:
		g_key_pressed[KEY_UP_ARROW] = true;
		break;
	case GLUT_KEY_DOWN:
		g_key_pressed[KEY_DOWN_ARROW] = true;
		break;
	case GLUT_KEY_LEFT:
		g_key_pressed[KEY_LEFT_ARROW] = true;
		break;
	case GLUT_KEY_RIGHT:
		g_key_pressed[KEY_RIGHT_ARROW] = true;
		break;
	case GLUT_KEY_ALT_L:
		g_key_pressed[KEY_LEFT_ALT] = false;
		break;
	default:;
	}

}

//Read keyboard input on key up
void specialUp(const int special_key, int x, int y)
{
	switch (special_key)
	{
	case GLUT_KEY_UP:
		g_key_pressed[KEY_UP_ARROW] = false;
		break;
	case GLUT_KEY_DOWN:
		g_key_pressed[KEY_DOWN_ARROW] = false;
		break;
	case GLUT_KEY_LEFT:
		g_key_pressed[KEY_LEFT_ARROW] = false;
		break;
	case GLUT_KEY_RIGHT:
		g_key_pressed[KEY_RIGHT_ARROW] = false;
		break;
	case GLUT_KEY_END:
		g_key_pressed[KEY_END] = false;
		break;
	case GLUT_KEY_ALT_L:
		g_key_pressed[KEY_LEFT_ALT] = false;
		break;
	default:;
	}

	//Set special keys
	g_key_pressed[KEY_SHIFT] = (glutGetModifiers() == GLUT_ACTIVE_SHIFT);
	g_key_pressed[KEY_LEFT_ALT] = (glutGetModifiers() == GLUT_ACTIVE_ALT);
}

//Read mouse move input when a button is held
void mouseMove(const int x, const int y)
{
	//Add up mouse movements until they are processed.
	g_mouse_dx += x - g_mouse_x;
	g_mouse_dy += y - g_mouse_y;
	g_mouse_x = x;
	g_mouse_y = y;

	//Warp the pointer back to its original spot if it moved
	if (g_mouse_x != g_mouse_locked_x || g_mouse_y != g_mouse_locked_y)
	{
		glutWarpPointer(g_mouse_locked_x, g_mouse_locked_y);
		g_mouse_x = g_mouse_locked_x;
		g_mouse_y = g_mouse_locked_y;
	}

}

//Read mouse click input
void mouseButton(const int button, const int state, const int x, const int y)
{

	//Hides mouse when held
	switch (button)
	{
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			g_key_pressed[MOUSE_RIGHT] = true;
			glutSetCursor(GLUT_CURSOR_NONE);
		} else
		{
			g_key_pressed[MOUSE_RIGHT] = false;
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
		break;
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			g_key_pressed[MOUSE_LEFT] = true;
			glutSetCursor(GLUT_CURSOR_NONE);
		} else
		{
			g_key_pressed[MOUSE_LEFT] = false;
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
		break;
	default:;
	}

	//Reset mouse coords on mouse click
	g_mouse_x = x;
	g_mouse_y = y;
	g_mouse_locked_x = x;
	g_mouse_locked_y = y;
}



void update()
{
	//Get time since last frame
	g_delta_time = g_time_counter.getAndReset();

	//Multiply delta time by time_scale to slow or speed up game
	const double scaled_time = g_delta_time * g_time_scale;

	g_elapsed_time_nanoseconds += long long(g_delta_time * 1000 + 0.5);
	g_update_lag += scaled_time;

	if (g_update_lag > FRAME_TIME_UPDATE * g_time_scale)
	{
		while (g_update_lag > FRAME_TIME_UPDATE * g_time_scale)
		{
			game.update(FRAME_TIME_UPDATE * g_time_scale);
			g_update_lag -= FRAME_TIME_UPDATE * g_time_scale;
			g_update_count++;
		}
	}else if (g_delta_time < FRAME_TIME_DISPLAY)
	{
		std::cout << "  slept: " << (FRAME_TIME_DISPLAY - g_delta_time);
		sleepms(FRAME_TIME_DISPLAY - g_delta_time);
		g_delta_time += (FRAME_TIME_DISPLAY - g_delta_time);
	}
	
	//Update the animations every frame
	game.updateAnimations(scaled_time);
	g_display_count++;

	//Update the fps display, but not too fast or it is unreadable
	if (g_display_count % unsigned(FPS_DISPLAY / 10) == 0)
	{
		const float alpha = 0.5;
		//Exponential smoothing for display rate
		g_display_fps = alpha * g_display_fps + (1 - alpha) * (1000.0 / g_delta_time);

		//Average update rate
		g_update_fps = (double(g_update_count) / (double(g_elapsed_time_nanoseconds)/1000000.0));
	}

	
	glutPostRedisplay();
}

//Resizes window and adjusts the projection matrix
void reshape(const int w, const int h)
{
	glViewport(0, 0, w, h);
	g_win_width = w;
	g_win_height = h;

	const float aspect_ratio = float(w) / float(h);

	// calculate the projection matrix
	g_projection_matrix = glm::perspective(glm::radians(float(FOV)),
		aspect_ratio,
		float(CLIP_NEAR),
		float(CLIP_FAR));

	glutPostRedisplay();
}

void display()
{
	game.display();
	glutSwapBuffers();
}
