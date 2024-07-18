#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>
#include <vector>

// glm::mat4 projection;
int screenWidth = 640;
int screenHeight = 480;

const char* vs = R"(
    #version 330 core

    layout (location = 0) in vec3 pos;

    uniform mat4 projection;
    uniform mat4 model;

    void main() {
        gl_Position =  projection * model * vec4(pos, 1.0);
    }

)";

const char* fs = R"(
    #version 330 core

    out vec4 FragColor;

    void main() {
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }

)";

bool stopMovement = false;

void wallCircleCollision(float x, float y, float radius) {
    float wallx1 = 0.0f;
    float wallx2 = 640.0f;
    float wally1 = 0.0f;
    float wally2 = 480.0f;

    if(x - radius < wallx1) {
        
    }

    if(x + radius > wallx2) {

    }

    if(y - radius < wally1) {

    }

    if(y + radius > wally2) {

    }

}

struct Line {
    float length;
    float vertices[12];
    glm::vec3 position;
    glm::mat4 model;
};

std::vector<Line>lines;

Line createLine(float length) {
    float vertices[] = {
        0.0f, 0.0f, 0.0f,
        length, 0.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
        length,  1.0f, 0.0f
    };

    Line line = {length};

    // std::cout << sizeof(line.vertices) << "\n";
    // std::cout << sizeof(line.vertices) / sizeof(float) << "\n";

    for(int i = 0; i < (sizeof(line.vertices) / sizeof(float)); ++i) {
        line.vertices[i] = vertices[i];
    }
    
    line.position = glm::vec3(0.0f, 0.0f, 0.0f);
    line.model = glm::mat4(1.0f);

    return line;
}

void setLinePosition(Line* line, glm::vec3 newPosition) {
    line->position = newPosition;
}

int linesCount = 0;
float linesX = 0.0f;
float linesY = 0.0f;

void legacyLineDraw(float x, float y, float length, GLuint shaderProgram) {
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);

    // float vertices[] = {
    //     x, y, 0.0f,
    //     x * length, y, 0.0f,
    //     x, y, 0.0f,
    //     x * length,  y, 0.0f
    // };

    float lineHeight = 1.0f;

    float lx = 200.0f;
    float ly = 200.0f;

    float fx = lx + x * (lx + length - lx);
    float fy = ly + y * (ly + length - ly);

    // std::cout << "fx: " << fx << " fy: " << fy << "\n";

    float vertices[] = {
        lx, ly, 0.0f,
        fx, fy, 0.0f,
        lx, ly + lineHeight, 0.0f,
        fx, fy + lineHeight, 0.0f
    };
    // float vertices[] = {
    //     100.0f, 100.0f, 0.0f,
    //     (100.0f + length * x), (100.0f * y), 0.0f,
    //     100.0f, 101.0f, 0.0f,
    //     (100.0f + length * x), ((100.0f * y) + 1.0f), 0.0f
    // };

    GLuint VAO, VBO;
    int stride = 3;
    int offset = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    int verticeSize = sizeof(vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticeSize, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), offset == 0 ? nullptr : (void*)(offset * sizeof(float)));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, -10.0f, 10.0f);

    int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    int modelLocation = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}


struct Circle {
    float radius;
    float vCount;
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    glm::vec3 position;
    glm::vec3 velocity;
    glm::mat4 model;
    GLuint VAO, VBO, EBO;
    float mass;
};

void genCircleVertices(Circle* circle) {
    float angle = 360.0f / circle->vCount;

    int triangleCount = circle->vCount - 2;

    std::vector<glm::vec3> temp;
    // positions
    for (int i = 0; i < circle->vCount; i++) {
        float currentAngle = angle * i;
        float x = circle->radius * cos(glm::radians(currentAngle));
        float y = circle->radius * sin(glm::radians(currentAngle));
        float z = 0.0f;

        circle->vertices.push_back(glm::vec3(x, y, z));
    }

    // push indexes of each triangle points
    for (int i = 0; i < triangleCount; i++) {
        circle->indices.push_back(0);
        circle->indices.push_back(i + 1);
        circle->indices.push_back(i + 2);
    }
}

void genGLAttributes(Circle* circle) {
    glGenBuffers(1, &circle->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, circle->VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * circle->vertices.size(), &circle->vertices[0], GL_STATIC_DRAW); // FOR CIRCLE

    glGenVertexArrays(1, &circle->VAO);
    glBindVertexArray(circle->VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &circle->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circle->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * circle->indices.size(), &circle->indices[0], GL_STATIC_DRAW);

}

Circle createCircle(float radius, float vertexCount) {
    std::vector<glm::vec3> vertices1;
    std::vector<unsigned int> indices1;

    Circle circle = {radius, vertexCount, vertices1, indices1};

    circle.position = glm::vec3(0.0f, 0.0f, 0.0f);
    circle.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    circle.model = glm::mat4(1.0f);

    circle.mass = 1.0f;

    genCircleVertices(&circle);

    return circle;
}

void setPosition(Circle* circle, glm::vec3 newPosition) {
    circle->position = newPosition;
}

void setVelocity(Circle* circle, glm::vec3 newVelocity) {
    circle->velocity = newVelocity;
}

void applyTransform(Circle* circle) {
    circle->model = glm::translate(circle->model, circle->position);
}

void resetTransform(Circle* circle) {
    circle->model = glm::mat4(1.0f);
}

void moveCircle(Circle* circle, float speedX) {
    if(stopMovement == true) return;
    // circle->position.x = position.x + speedX;
    // setPosition(circle, glm::vec3(circle->position.x + speedX, circle->position.y, circle->position.z));
    glm::vec3 newPosition = circle->position + circle->velocity;

    setPosition(circle, newPosition);
}

bool circlesCollide(float x1, float y1, float rad1, float x2, float y2, float rad2) {
    // Calculate distance between centers of the circles
    float dx = x1 - x2;
    float dy = y1 - y2;

    float distance = std::sqrt((dx * dx + dy * dy));

    // Check if the distance is less than or equal to the sum of the radii
    if (distance < rad1 + rad2) {
        return true; // Collision detected
    } else {
        return false; // No collision
    }
}

void circleCollision(Circle* circle1, Circle* circle2) {
    float x1 = circle1->position.x;
    float y1 = circle1->position.y;
    float rad1 = circle1->radius;
    float c1m = circle1->mass;
    glm::vec3 v1 = circle1->velocity;
    float x2 = circle2->position.x;
    float y2 = circle2->position.y;
    float rad2 = circle2->radius;
    float c2m = circle2->mass;
    glm::vec3 v2 = circle2->velocity;


    if(circlesCollide(x1, y1, rad1, x2, y2, rad2)) {
        std::cout << "collide\n";
        if(stopMovement == true) return;

        // stopMovement = true;
        // circle1->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        // circle2->velocity = glm::vec3(0.0f, 0.0f, 0.0f);

        float dx = x2 - x1;
        float dy = y2 - y1;
        float distance = std::sqrt((dx * dx + dy * dy));
        float nx = dx / distance;  // normalized x-component of the normal vector
        float ny = dy / distance;  // normalized y-component of the normal vector

        float overlap = rad1 + rad2 - distance;

        /* std::cout << "old1: " << circle1->position.x << " " << circle1->position.y << "\n";
        std::cout << "old2: " << circle2->position.x << " " << circle2->position.y << "\n"; */

        glm::vec3 deltaP = glm::vec3((overlap * 0.5) * nx, (overlap * 0.5) * ny, 0.0f);

        setPosition(circle1, (circle1->position - deltaP));
        setPosition(circle2, (circle2->position + deltaP));

        /* std::cout << "new1: " << circle1->position.x << " " << circle1->position.y << "\n";
        std::cout << "new2: " << circle2->position.x << " " << circle2->position.y << "\n"; */

        distance = rad1 + rad2;

        std::cout << "overlap: " << overlap << "\n";

        // std::cout << nx << " " << ny << "\n";
        linesCount = 1;
        linesX = nx;
        linesY = ny;

        float dvx = v2.x - v1.x;
        float dvy = v2.y - v1.y;
        float dot_product = dvx * nx + dvy * ny;

        // std::cout << "dp: " << dot_product << "\n";

        // 2 * (m1 * m3) * dp
        // -------------------
        // (m1 * m2) * distane

        float impulse = (2.0f * (c1m * c2m) * dot_product) / ((c1m + c2m) * distance);
        // std::cout << impulse << " impulse \n";

        circle1->velocity.x += impulse * nx / c1m;
        circle1->velocity.y += impulse * ny / c1m;

        // circle1->velocity.x *= 100.0f;
        // circle1->velocity.y *= 100.0f;

        circle2->velocity.x -= impulse * nx / c2m;
        circle2->velocity.y -= impulse * ny / c2m;

        // circle2->velocity.x *= 5.0f;
        // circle2->velocity.y *= 5.0f;

        // std::cout << "x: " << circle1->velocity.x << " y: " << circle1->velocity.y << "\n";



        // Calculate relative velocity in the normal direction
        // float dvx = c2->vx - c1->vx;
        // float dvy = c2->vy - c1->vy;
        // float dot_product = dvx * nx + dvy * ny;

        // // Calculate the impulse (change in momentum)
        // float impulse = (2.0 * (c1->mass * c2->mass) * dot_product) / ((c1->mass + c2->mass) * distance);

        // // Update velocities
        // c1->vx += impulse * nx / c1->mass;
        // c1->vy += impulse * ny / c1->mass;
        // c2->vx -= impulse * nx / c2->mass;
        // c2->vy -= impulse * ny / c2->mass;
        
    }
}

// Function to compile shaders
unsigned int compileShader(unsigned int type, const char* source);

// Function to create shader program
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource);

int main() {
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(screenWidth, screenHeight, "GLM Ortho Projection Example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // GLEW initialization code
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    GLuint shaderProgram = createShaderProgram(vs, fs);

    Line line = createLine(10.0f);

    // float vertices[] = {
    //     0.0f, 0.0f, 0.0f,
    //     100.0f, 0.0f, 0.0f,
    //     50.0f,  100.0f, 0.0f
    // };

    std::vector<Circle> circles;

    Circle circle = createCircle(50, 120);
    setPosition(&circle, glm::vec3(50.0f, 290.0f, 0.0f));
    setVelocity(&circle, glm::vec3(1.0f, 0.0f, 0.0f));
    applyTransform(&circle);

    genGLAttributes(&circle);


    Circle circle2 = createCircle(50, 120);
    setPosition(&circle2, glm::vec3(320.0f, 240.0f, 0.0f));
    // setVelocity(&circle2, glm::vec3(-1.0f, 0.0f, 0.0f));
    applyTransform(&circle2);

    genGLAttributes(&circle2);

    circles.push_back(circle);
    circles.push_back(circle2);

    glUseProgram(shaderProgram);

    glm::mat4 projection = glm::mat4(1.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
        glViewport(0, 0, screenWidth, screenHeight);

        glfwPollEvents();
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, -10.0f, 10.0f);

        int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        for (Circle& circle : circles) {
            // std::cout << circle.position.x << "\n";
            applyTransform(&circle);

            int modelLocation = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(circle.model));

            glBindVertexArray(circle.VAO);
            glDrawElements(GL_TRIANGLES, circle.indices.size(), GL_UNSIGNED_INT, 0);

            resetTransform(&circle);
        }

        moveCircle(&circles[0], 1.0f);
        moveCircle(&circles[1], 1.0f);

        circleCollision(&circles[0], &circles[1]);

        if(linesCount > 0) {
            // legacyLineDraw(0.5f, 0.8660f, 100.0f, shaderProgram);
            legacyLineDraw(linesX, linesY, 100.0f, shaderProgram);
        }


        glfwSwapBuffers(window);

    }

    glfwTerminate();
    return 0;
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
    }

    return shader;
}

// Function to create shader program
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader linking error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}