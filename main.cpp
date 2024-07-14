#include <GLFW/glfw3.h>
#include <cmath>

void drawCircle(float centerX, float centerY, float radius, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(centerX, centerY);  // Center of circle
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i * (2.0f * M_PI / segments);
        float x = centerX + radius * std::cos(angle);
        float y = centerY + radius * std::sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 0.0f, 0.0f);  // Red color
    drawCircle(0.0f, 0.0f, 0.5f, 30);  // Example: Circle centered at (0,0) with radius 0.5
}

int main() {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Circle Example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
