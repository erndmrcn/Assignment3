#include <iostream>
#include "parser.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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
int width, height;
static void errorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void cameraInit()
{
    parser::Camera camera = scene.camera;
    // camera settings
    // viewport
    glViewport(0, 0, camera.image_width, camera.image_height);
    glMatrixMode(GL_MODELVIEW);
    /* Set camera position */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    scene.camera.far_distance, scene.camera.near_distance, scene.camera.position.x, 
    scene.camera.position.y, scene.camera.position.z, scene.camera.image_width, scene.camera.image_height,
    scene.camera.up.x, scene.camera.up.y, scene.camera.up.z, scene.camera.gaze.x,
    scene.camera.gaze.y, scene.camera.gaze.z; 
    gluLookAt(camera.position.x, camera.position.y, camera.position.z,
    (camera.gaze.x * camera.near_distance + camera.position.x),
    (camera.gaze.y * camera.near_distance + camera.position.y),
    (camera.gaze.z * camera.near_distance + camera.position.z),
    camera.up.x, camera.up.y, camera.up.z);
    /* Set projection frustrum */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(camera.near_plane.x, camera.near_plane.y, camera.near_plane.z, camera.near_plane.w, camera.near_distance, camera.far_distance);
}

void turnOn()
{
    // turnOn lights
    int lSize = scene.point_lights.size();
    for(int i = 0; i<lSize; i++)
    {
        parser::PointLight pointLight = scene.point_lights[i];

        GLfloat color[] = {pointLight.intensity.x, pointLight.intensity.y, pointLight.intensity.z, 1.0f};
        GLfloat position[] = {pointLight.position.x, pointLight.position.y, pointLight.position.z, 1.0f};
        GLfloat ambient[] = {scene.ambient_light.x, scene.ambient_light.y, scene.ambient_light.z, 1.0f};

        glLightfv(GL_LIGHT0+i, GL_POSITION, position);
        glLightfv(GL_LIGHT0+i, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0+i, GL_DIFFUSE, color);
        glLightfv(GL_LIGHT0+i, GL_SPECULAR, color);
    }

}

void drawMeshes()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    int mSize = scene.meshes.size();
    for(int i = 0; i<mSize; i++)
    {
        parser::Mesh mesh = scene.meshes[i];
        parser::Material material = scene.materials[mesh.material_id-1];

        GLfloat ambientColor[] = {material.ambient.x, material.ambient.y, material.ambient.z, 1.0f};
        GLfloat diffuseColor[] = {material.diffuse.x, material.diffuse.y, material.diffuse.z, 1.0f};
        GLfloat specularColor[] = {material.specular.x, material.specular.y, material.specular.z, 1.0f};
        GLfloat phongExponent[] = {material.phong_exponent};

        // transformations
        glPushMatrix();
        int tSize = mesh.transformations.size();
        for(int j = tSize - 1; j>=0; j--)
        {
            parser::Transformation transformation = mesh.transformations[i];
            // translation
            if(transformation.transformation_type == "Translation")
            {
                parser::Vec3f translate = scene.translations[transformation.id - 1];
                glTranslatef(translate.x, translate.y, translate.z);
            }
            // rotation
            if(transformation.transformation_type == "Rotation")
            {
                parser::Vec4f rotation = scene.rotations[transformation.id - 1];
                glRotatef(rotation.x, rotation.y, rotation.z, rotation.w);
            }
            //scaling
            if(transformation.transformation_type == "Scaling")
            {
                parser::Vec3f scaling = scene.scalings[transformation.id - 1];
                glScalef(scaling.x, scaling.y, scaling.z);
            }
        }
        int fSize = mesh.faces.size();
        for(int j = 0; j<fSize; j++)
        {
            // polygon mode
            if(mesh.mesh_type == "Solid")
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            else if(mesh.mesh_type == "Wireframe")
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }


            // material colors
            glMaterialfv(GL_FRONT, GL_AMBIENT, ambientColor);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor); 
            glMaterialfv(GL_FRONT, GL_SPECULAR, specularColor);
            glMaterialfv(GL_FRONT, GL_SHININESS, phongExponent);
            
            // faces and begin gl_triangles
            glBegin(GL_TRIANGLES);
            
            parser::Face face = mesh.faces[j];
            parser::Vec3f vertex0;
            parser::Vec3f vertex1;
            parser::Vec3f vertex2;
            parser::Vec3f normal;
            parser::Vec3f b;
            parser::Vec3f a;

            vertex0 = scene.vertex_data[face.v0_id - 1];
            vertex1 = scene.vertex_data[face.v1_id - 1];
            vertex2 = scene.vertex_data[face.v2_id - 1];

            // vertex0
            a.x = vertex1.x - vertex0.x;
            a.y = vertex1.y - vertex0.y;
            a.z = vertex1.z - vertex0.z;
            b.x = vertex2.x - vertex0.x;
            b.y = vertex2.y - vertex0.y;
            b.z = vertex2.z - vertex0.z;
            normal.x = a.y*b.z - a.z*b.y;
            normal.y = a.z*b.x - a.x*b.z;
            normal.z = a.x*b.y - a.y*b.x;
            glNormal3f(normal.x, normal.y, normal.z);
            glVertex3f(vertex0.x, vertex0.y, vertex0.z);
            // vertex1
            a.x = vertex0.x - vertex1.x;
            a.y = vertex0.y - vertex1.y;
            a.z = vertex0.z - vertex1.z;
            b.x = vertex2.x - vertex1.x;
            b.y = vertex2.y - vertex1.y;
            b.z = vertex2.z - vertex1.z;
            normal.x = a.y*b.z - a.z*b.y;
            normal.y = a.z*b.x - a.x*b.z;
            normal.z = a.x*b.y - a.y*b.x;
            glNormal3f(normal.x, normal.y, normal.z);
            glVertex3f(vertex1.x, vertex1.y, vertex1.z);
            // vertex2
            a.x = vertex0.x - vertex2.x;
            a.y = vertex0.y - vertex2.y;
            a.z = vertex0.z - vertex2.z;
            b.x = vertex1.x - vertex2.x;
            b.y = vertex1.y - vertex2.y;
            b.z = vertex1.z - vertex2.z;
            normal.x = a.y*b.z - a.z*b.y;
            normal.y = a.z*b.x - a.x*b.z;
            normal.z = a.x*b.y - a.y*b.x;
            glNormal3f(normal.x, normal.y, normal.z);
            glVertex3f(vertex2.x, vertex2.y, vertex2.z);

            glEnd();
        }

        glPopMatrix();
    }
}

int main(int argc, char* argv[]) {
    scene.loadFromXml(argv[1]);

    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW\n" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    win = glfwCreateWindow(scene.camera.image_width, scene.camera.image_height, "CENG477 - HW3", NULL, NULL);

    if (!win) {
        // if some error occured exit
        std::cout << "Failed to open GLFW window.\n" << std::endl;
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
    glClearColor(0, 0, 0, 1);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    // initialize camera and scene

    // set camera 
    // read scene into buffers
    // do lights
    // draw

    // enable lights
    glEnable(GL_LIGHTING);
    int lSize = scene.point_lights.size();
    for(int i = 0; i<lSize; i++)
    {
        glEnable(GL_LIGHT0+i);
    }
    // instead of waitEvents use pollEvents
    while(!glfwWindowShouldClose(win)) {
        
        cameraInit();
        turnOn();
        drawMeshes();
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
