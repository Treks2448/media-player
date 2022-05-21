#include <GLFW/glfw3.h>
#include <stdio.h>

int main(int argc, const char** argv) {
	GLFWwindow* window;

	if (!glfwInit()) {
		printf("Couldn't initialize GLFW.");
		return 1;
	}

	window = glfwCreateWindow(640, 480, "Hello World!", NULL, NULL);

	if (!window) {
		printf("Couldn't open GLFW window.");
		return 1;
	}
	
	glfwMakeContextCurrent(window);
	
	unsigned char* red_square = new unsigned char[100 * 100 * 3];
	for (int iy = 0; iy < 100; iy++) {	
		for (int ix = 0; ix < 100; ix++) {
			red_square[iy * 100 * 3 + ix * 3 + 0] = 0xff;
			red_square[iy * 100 * 3 + ix * 3 + 1] = 0x00;
			red_square[iy * 100 * 3 + ix * 3 + 2] = 0x00;
		}
	}
	
	// Generate a texture handle
	GLuint texHandle;
	glGenTextures(1, &texHandle);	
	// bind GL_TEXTURE_2D to texHandle 
	glBindTexture(GL_TEXTURE_2D, texHandle);
	
	// no idea what this stuff does... 
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// load data into the texture (handle, level, type, x-size, y-size, border, input-type, data-type, data)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, red_square);
		
	while(!glfwWindowShouldClose(window)) {
		// I guess this clears the buffer contents from the previous frame	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// get window size	
		int windowWidth, windowHeight;
		glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
		
		glMatrixMode(GL_PROJECTION); // describes how the coordinate space is projected onto the screen
		glLoadIdentity();
		// Sets an orthographic projection. Sets projection coordinates to fit the window 
		// (left, right, bottom, top, nearVal, farVal)
		glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
		
		glMatrixMode(GL_MODELVIEW); // describes how transformations occur in the coordinate space

		// Render
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texHandle);
		glBegin(GL_QUADS);
			glTexCoord2d(0, 0); glVertex2i(0, 0);
			glTexCoord2d(1, 0); glVertex2i(100, 0);
			glTexCoord2d(1, 1); glVertex2i(100, 100);
			glTexCoord2d(0, 1); glVertex2i(0, 100);
		glEnd();
		glDisable(GL_TEXTURE_2D);
			

		// swap the back buffer with the front buffer so that it is rendered on screen
		glfwSwapBuffers(window);	
			
		glfwWaitEvents();
	}

	return 0;
}
