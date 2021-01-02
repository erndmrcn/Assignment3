#include <iostream>
#include "parser.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

//////-------- Global Variables -------/////////
GLuint gpuVertexBuffer;
GLuint gpuNormalBuffer;
GLuint gpuIndexBuffer;

// never free a pointer that GL library returns 
// they are automatically handled 

// Sample usage for reading an XML scene file
parser::Scene scene;
static GLFWwindow* win = NULL;

static void errorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void drawMeshes(parser::Mesh mesh, int s)
{
    // Use VBO
    static bool firstTime = true;
    static int times = 0;
    
    static int vertexPosDataInBytes;
    
    int totalSize = 0;

    if(firstTime)
    {
        times++;
        if(times == s)
            firstTime = false;
        
        // these function enable us to change vertex_buff and color_buff
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        int fSize = mesh.faces.size();
        std::vector<GLuint> indices;
        for(int i; i<fSize; i++)
        {
            parser::Face face = mesh.faces[i];
            indices.push_back(face.v0_id-1);
            indices.push_back(face.v1_id-1);
            indices.push_back(face.v2_id-1);
        }
        
        std::vector<GLfloat> vertexPos;
        for(int i = 0; i<fSize; i=i)
        {
            parser::Vec3f temp = scene.vertex_data[indices[i]];    
            vertexPos.push_back(temp.x);
            vertexPos.push_back(temp.y);
            vertexPos.push_back(temp.z);
        }
        
        GLuint vertexAttribBuffer, indexBuffer;

		glGenBuffers(1, &vertexAttribBuffer);
		glGenBuffers(1, &indexBuffer);

		// std::assert(vertexAttribBuffer > 0 && indexBuffer > 0);

		glBindBuffer(GL_ARRAY_BUFFER, vertexAttribBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		vertexPosDataInBytes = sizeof(vertexPos);

		int indexDataSizeInBytes = sizeof(indices);
		
		glBufferData(GL_ARRAY_BUFFER, vertexPosDataInBytes, 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertexPosDataInBytes, static_cast<void*> (vertexPos.data()));
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, static_cast<void*> (indices.data()), GL_STATIC_DRAW);

    }

    // initialization is done only for the first time
    parser::Vec3f ambient = scene.materials[mesh.material_id-1].ambient;
    parser::Vec3f diffuse = scene.materials[mesh.material_id-1].diffuse;
    parser::Vec3f specular = scene.materials[mesh.material_id-1].specular;
    GLfloat spec = scene.materials[mesh.material_id-1].phong_exponent;

    GLfloat ambColor [ 4 ] = {ambient.x, ambient.y, ambient.z, 1.0 };
    GLfloat diffColor [ 4 ] = {diffuse.x, diffuse.y, diffuse.z, 1.0 };
    GLfloat specColor [ 4 ] = {specular.x, specular.y, specular.z, 1.0 };
    GLfloat specExp [ 1 ] = {spec};
    glMaterialfv ( GL_FRONT , GL_AMBIENT , ambColor ) ;
    glMaterialfv ( GL_FRONT , GL_DIFFUSE , diffColor ) ;
    glMaterialfv ( GL_FRONT , GL_SPECULAR , specColor ) ;
    glMaterialfv ( GL_FRONT , GL_SHININESS , specExp ) ;
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawElements(GL_TRIANGLES, totalSize*3, GL_UNSIGNED_INT, 0);
}

void display()
{
    glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    int mSize = scene.meshes.size();
    for(int i = 0; i<mSize; i++)
    {
        // transformations
        parser::Mesh mesh = scene.meshes[i];
        drawMeshes(mesh, mSize);
    }
}

int main(int argc, char* argv[]) {
    scene.loadFromXml(argv[1]);
    
    /*
        mesh.material_id
        mesh.faces
        mesh.transformations
        mesh.mesh-type
    
    */
    
    // read meshes into the buffers 

    // scene is read
    // use V(ertex)B(uffer)O(bjects) to get better performance
    // only meshes
    // do transformations after rendered correctly
    // culling

    glfwSetErrorCallback(errorCallback);
    
    // before initialization functions are not thread-safe
    // library initialization
    if (!glfwInit()) {
        std::cout << "An error occured while initializing" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // disallow resize -> glfwWindowHint(GLFW_RESIZABLE, 0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    // for full-screen
    // glfwCreateWindow(scene.camera.image_width, scene.camera.image_height, "CENG477 - HW3", glfwGetPrimaryMonitor(), NULL); 
    // creates a window
    win = glfwCreateWindow(scene.camera.image_width, scene.camera.image_height, "CENG477 - HW3", NULL, NULL);
    // to get window position -> 
    // int xpos, ypos;
    // glfwGetWindowPos(win, &xpos, &ypos);
    // set windows position -> glfwSetWindowPos(win, 1920, 1024);

    // starts with full-screen mode 
    // glfwMaximizeWindow(win);

    // to hide the window
    // glfwHideWindow(win);

    // for transparency if OS supports
    // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    if (!win) {
        // if some error occured exit
        std::cout << "An error occured while creating window!" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(win);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
            exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(win, keyCallback);

    // instead of waitEvents use pollEvents
    while(!glfwWindowShouldClose(win)) {
        
        display();

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    // destroys the window created at the beginning
    glfwDestroyWindow(win);

    // library termination 
    glfwTerminate();

    exit(EXIT_SUCCESS);

    return 0;
}
