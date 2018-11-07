// newPlayerTest.cpp - Alan Morgan

// Fix Depth looks. - Image - CHECK
// Update things to be able to be INVISIBLE! (Thing handler?) - CHECK - Kind of, just remove from Data Mode.
// Remove from middle. - DONE
// Fix Grid Glitch. - DONE :)
// Move Needed enemy stuff to living.

// Create Enemy That stays still and punches facing down, w/rectangle, no image yet.
// Add image to enemy.
// Rename sssThingHandler to thingHandler
// Create Wall
// Remove TBO.
// Fix Collision glitch.
// Create Enemy spawner. Don't spawn near walls!
// Create Menu.
// Add Music.

#include "../constants.h"
#include "../graphics.h"
#include "../input.h"
#include "../timer.h"
#include "../sssThingHandler.h"
#include "../helperGL.h"
#include "../view.h"
#include "../grid.h"
#include "../imageHandler.h"
#include "../image.h"
#include "../background.h"
#include "../thing.h"
#include "../living.h"
#include "../player.h"

// Separated from main, so that destructors may get called before ending GL/SDL.
void runProgram(Graphics & g) {
	Input input;
	View v;
	// Not this yet
	Grid grid;
	Timer t;
	ImageHandler im (SPRITE_SHEET, 0);
	Background bg;
	im.parseStrip(0, 0, 70, 70, 16, 8, 0, 0);

	// Test sss thing handler - 20 capacity
	SSS_ThingHandler sss(25);
	// Friction
	float friction = .3;
	// Player and Rectangle Speed based on FPS
	const float RS = 5 * 16 / FPS;
	// used for wall speed;
	float recSpeed = RS;

	// We will create a bunch of testing rectangles, rect is the player.
	//Rectangle rect(10, 10, 16, 16);
	Player rect;
	rect.setXY(20, 5);
	rect.setImageHandler(&im);

	Rectangle push(200, 200, 64, 64);

	GLuint rectIndex = sss.addThing();

	//Image rectImg(&im, 2);
	//rectImg.dataMode(&sss, rectIndex);
	rect.dataMode(&sss, rectIndex);
	push.dataMode(&sss, sss.addThing());

	rect.setIX(-70 / 2 + 16 / 2);
	rect.setIY(-70 / 2 + 16 / 2);

	// These are all the wall rectangles (the hollow ones).
	unsigned rectSize = 9;
	Thing rectangles[9];

	// Set rectangles to datamode
	for (unsigned i = 0; i < rectSize; ++i) {
		unsigned tmpIndex = sss.addThing();
		//rectangles[i].setImageHandler(&im);
		rectangles[i].setXYWH(-50, i * 90, 70, 70),
		rectangles[i].setZ(Z_RECT);
		rectangles[i].dataMode(&sss, sss.addThing());
	}
	// Keep the mouse within the window.
	input.mouseCapture();

	// Loop forever.
	bool loopFlag = false;
	while(!loopFlag) {
		t.reset();
		input.update();
		loopFlag = input.isQuit();

		// Sync the grid with the mouse location.
		grid.update(input);
		rect.input(input);

		// ESC Key
		if (input.pressed(input.KEY_ESC)) {
			input.isMouseCaptured() ? input.mouseFree() : input.mouseCapture();
			recSpeed *= -1;
		}

		// SPACE Key
		if (input.down(input.KEY_SPA)) {
			for (unsigned i = 0; i < rectSize; ++i) {
				rectangles[i].setHS(recSpeed);
				rectangles[i].setVS(recSpeed);
			}

		} else {
			for (unsigned i = 0; i < rectSize; ++i) {
				rectangles[i].setHS(rectangles[i].getHS() * friction);
				rectangles[i].setVS(rectangles[i].getVS() * friction);
			}
		}


		// Collision player with walls.
		for (unsigned i = 0; i < rectSize; ++i)
		{
			if ( rect.willIntersect(rectangles[i])) {
				rect.collision(rectangles[i]);
			}
		}

		// DEBUG
		std::cerr << "Y: " << rect.getY() + rect.getH() << " | RY: " << rectangles[0].getY() << "\n";

		// ASDW Keys
		rect.updateHand(grid);


		// Collision player with push
		if (push.isIntersecting(rect)) {
			rect.hurt(1);
			//rect.addHS(-rect.getHS() * (32*2 / FPS));
			//rect.addVS(-rect.getVS() * (32*2 / FPS));
			//push.addHS(rect.getHS() * (32*4 / FPS));
			//push.addVS(rect.getVS() * (32*4 / FPS));
			float acc = 32 * 13 / FPS;
			float cx = rect.getCX() - push.getCX();
			float cy = rect.getCY() - push.getCY();
			float angle = atan(cy / cx);
			float dx = abs(cos(angle) * acc);
			float dy = abs(sin(angle) * acc);

			if (cx < 0.0f)
				rect.addHS(-dx + push.getHS());
			else
				rect.addHS( dx + push.getHS());

			if (cy < 0.0f)
				rect.addVS(-dy + push.getVS());
			else
				rect.addVS( dy + push.getVS());
		}

		if (push.isIntersecting(rect.getHandRect())) {
			float acc = 32 * 9 / FPS;
			float cx = push.getCX() - rect.getHandRect().getCX();
			float cy = push.getCY() - rect.getHandRect().getCY();
			float angle = atan(cy / cx);
			float dx = abs(cos(angle) * acc);
			float dy = abs(sin(angle) * acc);

			if (cx < 0.0f)
				push.addHS(-dx + rect.getHandRect().getHS());
			else
				push.addHS(dx + rect.getHandRect().getHS());

			if (cy < 0.0f)
				push.addVS(-dy + rect.getHandRect().getVS());
			else
				push.addVS(dy + rect.getHandRect().getVS());
		}

		push.addHS(-push.getHS() * friction);
		push.addVS(-push.getVS() * friction);

		// Move EVERYTHING hehe
		sss.updatePositions();

		// Center View
		v.center(rect, true);
		glEnable( GL_BLEND );
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bg.drawBackground(&v);
		g.beginFrameBuffer();

		//glEnable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		sss.drawThings(v.getViewMatrix(), 0, im.getSheetWidth(), im.getSheetHeight());
		g.endFrameBuffer();

		// Draw the grid. And mouse.
		grid.draw();

		// This is what puts those above drawings into action.
		g.update();

		// Timer stuff! The sleep method sleeps the thread. We want to
		// sleep in order for us to have exactly the frame rate if this
		// is 30 frames per second, then 1 second / 30 is how much time
		// each frame should take. Subtract that by the amount of time
		// this loop took and you know how much time to sleep. Pausing
		// your program can help out your cpu, but remember that the
		// lower framerates can be choppy, and the higher ones may be
		// too hard for your computer to handle.
		if (t.getTicks() < 1000 / FPS)
			g.sleep(1000 / FPS - t.getTicks());
	}
}

int main(int argc, char ** argv) {
	// Program starts here.
	Graphics g;
	if (g.init())
		return 1;

	g.createFrameBuffer(VS_FRAMEBUFFER, FS_FRAMEBUFFER);

	runProgram(g);

	g.deleteFrameBuffer();

	g.endit();
	return 0;
}
