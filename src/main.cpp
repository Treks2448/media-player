#include <GLFW/glfw3.h>
#include <stdio.h>

int loadFrame(const char* filename, int* width, int* height, unsigned char** data);

int main(int argc, const char** argv) {
	unsigned char* red_square;
	int width;
	int height;

	if (!loadFrame("/home/igor/Videos/a.mp4", &width, &height, &red_square)) {
	    printf("Couldn't load frame\n");
	    return 1;
	}

	GLFWwindow* window;

	if (!glfwInit()) {
		printf("Couldn't initialize GLFW.\n");
		return 1;
	}

	window = glfwCreateWindow(width, height, "Hello World!", NULL, NULL);

	if (!window) {
		printf("Couldn't open GLFW window.\n");
		return 1;
	}
	
	glfwMakeContextCurrent(window);	
	
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, red_square);
		
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
		// For some reason the video is up-side-down so I have swapped top and bottom
		glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
		
		glMatrixMode(GL_MODELVIEW); // describes how transformations occur in the coordinate space

		// Render
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texHandle);
		glBegin(GL_QUADS);
			glTexCoord2d(0, 0); glVertex2i(0, 0);
			glTexCoord2d(1, 0); glVertex2i(windowWidth, 0);
			glTexCoord2d(1, 1); glVertex2i(windowWidth, windowHeight);
			glTexCoord2d(0, 1); glVertex2i(0, windowHeight);
		glEnd();
		glDisable(GL_TEXTURE_2D);
			

		// swap the back buffer with the front buffer so that it is rendered on screen
		glfwSwapBuffers(window);	
			
		glfwWaitEvents();
	}

	return 0;
}
