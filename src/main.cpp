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
	
	while(!glfwWindowShouldClose(window)) {
		// I guess this clears the buffer contents from the previous frame	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// size x, size y, colour format, data type, data
		glDrawPixels(100, 100, GL_RGB, GL_UNSIGNED_BYTE, red_square);
		
		// swap the buffer we drew to with the visible buffer	
		glfwSwapBuffers(window);	
			
		glfwWaitEvents();
	}

	return 0;
}
