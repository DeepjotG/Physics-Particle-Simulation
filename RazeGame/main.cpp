#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader_s.h>
#include <Camera.h>
#include <Particle.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <vector>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
    GLsizei length, const char* message, const void* userParam);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


Camera camera;
bool firstMouse = true;
float lastX = 400.0f;  // Initialize with some default value (center of the window)
float lastY = 300.0f;  // Initialize with some default value (center of the window)

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


/*
vector<float> createTriangleVertices(float radius) {
    const float PI = 3.1415926f;
    const float H_ANGLE = PI / 180 * 72;    // 72 degree = 360 / 5
    const float V_ANGLE = atanf(1.0f / 2);  // elevation = 26.565 degree

    std::vector<float> vertices(12 * 3);    // array of 12 vertices (x,y,z)
    int i1, i2;                             // indices
    float z, xy;                            // coords
    float hAngle1 = -PI / 2 - H_ANGLE / 2;  // start from -126 deg at 1st row
    float hAngle2 = -PI / 2;                // start from -90 deg at 2nd row

    // the first top vertex at (0, 0, r)
    vertices[0] = 0;
    vertices[1] = 0;
    vertices[2] = radius;

    // compute 10 vertices at 1st and 2nd rows
    for (int i = 1; i <= 5; ++i)
    {
        i1 = i * 3;         // index for 1st row
        i2 = (i + 5) * 3;   // index for 2nd row

        z = radius * sinf(V_ANGLE);            // elevaton
        xy = radius * cosf(V_ANGLE);            // length on XY plane

        vertices[i1] = xy * cosf(hAngle1);      // x
        vertices[i2] = xy * cosf(hAngle2);
        vertices[i1 + 1] = xy * sinf(hAngle1);  // y
        vertices[i2 + 1] = xy * sinf(hAngle2);
        vertices[i1 + 2] = z;                   // z
        vertices[i2 + 2] = -z;

        // next horizontal angles
        hAngle1 += H_ANGLE;
        hAngle2 += H_ANGLE;
    }

    // the last bottom vertex at (0, 0, -r)
    i1 = 11 * 3;
    vertices[i1] = 0;
    vertices[i1 + 1] = 0;
    vertices[i1 + 2] = -radius;

    return vertices;
}

void addVertices(vector<float>& vertices, const float* v1, const float* v2, const float* v3) {
    vertices.insert(vertices.end(), v1, v1 + 3);
    vertices.insert(vertices.end(), v2, v2 + 3);
    vertices.insert(vertices.end(), v3, v3 + 3);
}

void addIndices(vector<unsigned int>& Indices, unsigned int i1, unsigned int i2, unsigned int i3)
{   
    Indices.push_back(i1);
    Indices.push_back(i2);
    Indices.push_back(i3);
};


void interpolateVertex(const float* v1, const float* v2, float alpha, float radius, float* newV) {
    newV[0] = (1 - alpha) * v1[0] + alpha * v2[0];
    newV[1] = (1 - alpha) * v1[1] + alpha * v2[1];
    newV[2] = (1 - alpha) * v1[2] + alpha * v2[2];

    float length = sqrt(newV[0] * newV[0] + newV[1] * newV[1] + newV[2] * newV[2]);

    newV[0] = (newV[0] / length) * radius;
    newV[1] = (newV[1] / length) * radius;
    newV[2] = (newV[2] / length) * radius;

}

void subdivisionIscosphere(float subdivision, vector<float>& vertices, vector<unsigned int>& indices, float radius) {
    std::vector<float> tmpVertices;
    std::vector<unsigned int> tmpIndices;
    std::vector<float> newVs((subdivision + 1) * (subdivision + 2) / 2 * 3);
    const float* v1, * v2, * v3;          // ptr to original vertices of a triangle
    float newV1[3], newV2[3], newV3[3]; // new vertex positions
    unsigned int index = 0;
    int i, j, k;
    float a;                            // lerp alpha
    unsigned int i1, i2;                // indices

    // copy prev arrays
    tmpVertices = vertices;
    tmpIndices = indices;
    vertices.clear();
    indices.clear();

    // iterate each triangle of icosahedron
    for (i = 0; i < tmpIndices.size(); i += 3)
    {
        // get 3 vertices of a triangle of icosahedron
        v1 = &tmpVertices[tmpIndices[i] * 3];
        v2 = &tmpVertices[tmpIndices[i + 1] * 3];
        v3 = &tmpVertices[tmpIndices[i + 2] * 3];

        // add top vertex (x,y,z)
        newVs.clear();
        newVs.insert(newVs.end(), v1, v1 + 3);

        // find new vertices by subdividing edges
        for (j = 1; j <= subdivision; ++j)
        {
            a = (float)j / subdivision;    // lerp alpha

            // find 2 end vertices on the edges of the current row
            //          v1           //
            //         / \           // if N = 3,
            //        *---*          // lerp alpha = 1 / N
            //       / \ / \         //
            // newV1*---*---* newV2  // lerp alpha = 2 / N
            //     / \newV3/ \       //
            //    v2--*---*---v3     //
            interpolateVertex(v1, v2, a, radius, newV1);
            interpolateVertex(v1, v3, a, radius, newV2);
            for (k = 0; k <= j; ++k)
            {
                if (k == 0)      // new vertex on the left edge, newV1
                    newVs.insert(newVs.end(), newV1, newV1 + 3);
                else if (k == j) // new vertex on the right edge, newV2
                    newVs.insert(newVs.end(), newV2, newV2 + 3);
                else            // new vertices between newV1 and newV2
                {
                    a = (float)k / j;
                    interpolateVertex(newV1, newV2, a, radius, newV3);
                    newVs.insert(newVs.end(), newV3, newV3 + 3);
                }
            }
        }

        // compute sub-triangles from new vertices
        for (j = 1; j <= subdivision; ++j)
        {
            for (k = 0; k < j; ++k)
            {
                // indices
                i1 = (j - 1) * j / 2 + k; // index from prev row
                i2 = j * (j + 1) / 2 + k; // index from curr row

                v1 = &newVs[i1 * 3];
                v2 = &newVs[i2 * 3];
                v3 = &newVs[(i2 + 1) * 3];
                addVertices(vertices, v1, v2, v3);

                // add indices
                addIndices(indices, index, index + 1, index + 2);

                index += 3; // next index

                // if K is not the last, add adjacent triangle
                if (k < (j - 1))
                {
                    i2 = i1 + 1; // next of the prev row
                    v2 = &newVs[i2 * 3];
                    addVertices(vertices, v1, v3, v2);
                    addIndices(indices, index, index + 1, index + 2);
                    index += 3;
                }
            }
        }
    }
}

*/

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);


    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);



    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("Shaders/shader.vs", "Shaders/shader.fs"); // you can name your shader files however you like

    //std::vector<float> vertices = createTriangleVertices(0.1f);
    /*
    float vertices[] = {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
    };
    */

    vector<float> vertices;

    vector<unsigned int> Indices = {
        0,1,2,
        0,2,3,
        0,3,4,
        0,4,5,
        0,5,1,

        1, 6, 2, 
        2, 6, 7, 
        2, 7, 3, 
        3, 7, 8, 
        3, 8, 4, 
        4, 8, 9, 
        4, 9, 5, 
        5, 9, 10, 
        5, 10, 1, 
        1, 10, 6,
        6, 11, 7,  
        
        7, 11, 8,  
        8, 11, 9,  
        9, 11, 10, 
        10, 11, 6 };

    Particle Iscosphere(1.0f, 6.0f, vertices, Indices);
    Iscosphere.vertices = Iscosphere.createTriangleVertices();
    Iscosphere.subdivisionIscosphere();


    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), Iscosphere.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() *sizeof(unsigned int), Iscosphere.indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);

    glPointSize(2);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {   

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render the triangle
        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);


        glBindVertexArray(VAO);
        //glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);
        glDrawArrays(GL_POINTS, 0, Iscosphere.vertices.size() / 3);
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}





// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = camera.MovementSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);  // Move forward

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime); // Move backward

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);    // Move left

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);   // Move right
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}


