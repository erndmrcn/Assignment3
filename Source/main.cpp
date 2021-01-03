#include <iostream>
#include <chrono>
#include <string.h>
#include "parser.h"
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//////-------- Global Variables -------/////////
GLuint gpuVertexBuffer;
GLuint gpuNormalBuffer;
GLuint gpuIndexBuffer;
char gRendererInfo[512] = { 0 };
char gWindowTitle[512] = { 0 };
// never free a pointer that GL library returns 
// they are automatically handled 

// Sample usage for reading an XML scene file
parser::Scene scene;
static GLFWwindow* win = NULL;
int width, height;
std::vector<parser::Vec3f> normals;

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
    glLoadIdentity();
    gluLookAt(camera.position.x, camera.position.y, camera.position.z,
    (camera.gaze.x * camera.near_distance + camera.position.x),
    (camera.gaze.y * camera.near_distance + camera.position.y),
    (camera.gaze.z * camera.near_distance + camera.position.z),
    camera.up.x, camera.up.y, camera.up.z);
    /* Set projection frustrum */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // gluPerspective((GLdouble) 30, (GLdouble) (camera.near_plane.y - camera.near_plane.x) / (camera.near_plane.w - camera.near_plane.z), (GLdouble) camera.near_distance, (GLdouble) camera.far_distance);
    glFrustum(camera.near_plane.x, camera.near_plane.y, camera.near_plane.z, camera.near_plane.w, 
    camera.near_distance, camera.far_distance);
}

void turnOn()
{
    GLfloat ambient[] = {scene.ambient_light.x, scene.ambient_light.y, scene.ambient_light.z, 1.0f};
    // turnOn lights
    int lSize = scene.point_lights.size();
    for(int i = 0; i<lSize; i++)
    {
        parser::PointLight pointLight = scene.point_lights[i];

        GLfloat color[] = {pointLight.intensity.x, pointLight.intensity.y, pointLight.intensity.z, 1.0f};
        GLfloat position[] = {pointLight.position.x, pointLight.position.y, pointLight.position.z, 1.0f};
        glLightfv(GL_LIGHT0+i, GL_POSITION, position);
        glLightfv(GL_LIGHT0+i, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0+i, GL_DIFFUSE, color);
        glLightfv(GL_LIGHT0+i, GL_SPECULAR, color);
    }
}

void calculateNormals()
{
    int vSize = scene.vertex_data.size();
    for(int i = 0; i<vSize; i++)
    {
        parser::Vec3f temp;
        temp.x = 0.0f;
        temp.y = 0.0f;
        temp.z = 0.0f;
        normals.push_back(temp);
    }

    int mSize = scene.meshes.size();
    for(int i = 0; i<mSize; i++)
    {
        parser::Mesh mesh = scene.meshes[i];
        int fSize = mesh.faces.size();
        for(int j = 0; j<fSize; j++)
        {
            parser::Face face = mesh.faces[j];
            // vertex0
            parser::Vec3f vertex0 = scene.vertex_data[face.v0_id-1];
            // vertex1
            parser::Vec3f vertex1 = scene.vertex_data[face.v1_id-1];
            // vertex2
            parser::Vec3f vertex2 = scene.vertex_data[face.v2_id-1];
            
            parser::Vec3f normal0;
            parser::Vec3f normal1;
            parser::Vec3f normal2;
            parser::Vec3f b;
            parser::Vec3f a;
            // normal0
            a.x = vertex1.x - vertex0.x;
            a.y = vertex1.y - vertex0.y;
            a.z = vertex1.z - vertex0.z;
            b.x = vertex2.x - vertex0.x;
            b.y = vertex2.y - vertex0.y;
            b.z = vertex2.z - vertex0.z;
            normal0.x = a.y*b.z - a.z*b.y;
            normal0.y = a.z*b.x - a.x*b.z;
            normal0.z = a.x*b.y - a.y*b.x;
            normals[face.v0_id-1].x += normal0.x;
            normals[face.v0_id-1].y += normal0.y;
            normals[face.v0_id-1].z += normal0.z;
            // normal1
            a.x = vertex2.x - vertex1.x;
            a.y = vertex2.y - vertex1.y;
            a.z = vertex2.z - vertex1.z;
            b.x = vertex0.x - vertex1.x;
            b.y = vertex0.y - vertex1.y;
            b.z = vertex0.z - vertex1.z;
            normal1.x = a.y*b.z - a.z*b.y;
            normal1.y = a.z*b.x - a.x*b.z;
            normal1.z = a.x*b.y - a.y*b.x;
            normals[face.v1_id-1].x += normal1.x;
            normals[face.v1_id-1].y += normal1.y;
            normals[face.v1_id-1].z += normal1.z;
            // normal2
            a.x = vertex0.x - vertex2.x;
            a.y = vertex0.y - vertex2.y;
            a.z = vertex0.z - vertex2.z;
            b.x = vertex1.x - vertex2.x;
            b.y = vertex1.y - vertex2.y;
            b.z = vertex1.z - vertex2.z;
            normal2.x = a.y*b.z - a.z*b.y;
            normal2.y = a.z*b.x - a.x*b.z;
            normal2.z = a.x*b.y - a.y*b.x;
            normals[face.v2_id-1].x += normal2.x;
            normals[face.v2_id-1].y += normal2.y;
            normals[face.v2_id-1].z += normal2.z;
        }
    }
//     for(int i = 0; i<vSize; i++)
//     {
//         float len = sqrtf(normals[i].x*normals[i].x+ normals[i].y*normals[i].y + normals[i].z + normals[i].z);
//         normals[i].x /= len;
//         normals[i].y /= len;
//         normals[i].z /= len;
//     }
}

void drawMeshes()
{
    static int framesRendered = 0;
	static std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_NORMALIZE);

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
            parser::Transformation transformation = mesh.transformations[j];
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
            if(scene.culling_enabled)
            {
                glEnable(GL_CULL_FACE);
                glFrontFace(GL_CCW);
                if(scene.culling_face)
                {
                    glCullFace(GL_FRONT);
                    if(mesh.mesh_type == "Solid")
                    {
                        glPolygonMode(GL_BACK, GL_FILL);
                    }
                    else if(mesh.mesh_type == "Wireframe")
                    {
                        glPolygonMode(GL_BACK, GL_LINE);
                    }
                }
                else if(!scene.culling_face)
                {
                    glCullFace(GL_BACK);
                    glFrontFace(GL_CCW);
                    if(mesh.mesh_type == "Solid")
                    {
                        glPolygonMode(GL_FRONT, GL_FILL);
                    }
                    else if(mesh.mesh_type == "Wireframe")
                    {
                        glPolygonMode(GL_FRONT, GL_LINE);
                    }
                }
            }
            else if(!scene.culling_enabled)
            {
                glDisable(GL_CULL_FACE);
                glFrontFace(GL_CCW);
                if(mesh.mesh_type == "Solid")
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
                else if(mesh.mesh_type == "Wireframe")
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
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
            parser::Vec3f normal0;
            parser::Vec3f normal1;
            parser::Vec3f normal2;
            parser::Vec3f b;
            parser::Vec3f a;

            vertex0 = scene.vertex_data[face.v0_id - 1];
            vertex1 = scene.vertex_data[face.v1_id - 1];
            vertex2 = scene.vertex_data[face.v2_id - 1];
            normal0 = normals[face.v0_id - 1];
            normal1 = normals[face.v1_id - 1];
            normal2 = normals[face.v2_id - 1];
            // vertex0
            glNormal3f(normal0.x, normal0.y, normal0.z);
            glVertex3f(vertex0.x, vertex0.y, vertex0.z);
            
            // vertex1
            glNormal3f(normal1.x, normal1.y, normal1.z);
            glVertex3f(vertex1.x, vertex1.y, vertex1.z);
            
            // vertex2
            glNormal3f(normal2.x, normal2.y, normal2.z);
            glVertex3f(vertex2.x, vertex2.y, vertex2.z);

            glEnd();
        }
        glPopMatrix();
    }
     ++framesRendered;

	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsedTime = end - start;
	if (elapsedTime.count() > 1.)
	{
		start = std::chrono::system_clock::now();

		std::stringstream stream;
		stream << (framesRendered/(float) 60);
		framesRendered = 0;
        
		strcpy(gWindowTitle, gRendererInfo);
		strcat(gWindowTitle, stream.str().c_str());
		strcat(gWindowTitle, " FPS]");

		glfwSetWindowTitle(win, gWindowTitle);
	}
}

int main(int argc, char* argv[]) {
    scene.loadFromXml(argv[1]);
    float len = sqrtf(scene.camera.gaze.x*scene.camera.gaze.x + scene.camera.gaze.y*scene.camera.gaze.y + scene.camera.gaze.z*scene.camera.gaze.z);
    if (len == 0.0f)
        len = 1.0f;
    scene.camera.gaze.x /= len;
    scene.camera.gaze.y /= len;
    scene.camera.gaze.z /= len;
    std::cout << "OpenGL version: " << glGetString(GL_RENDERER) << " " << glGetString(GL_VERSION)<< std::endl;
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
    glClearColor(scene.background_color.x, scene.background_color.y, scene.background_color.z, 1);
    strcpy(gRendererInfo, "CENG477 - HW3[");

    glfwSetWindowTitle(win, gRendererInfo);

    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    // initialize camera and scene

    // set camera 
    // read scene into buffers
    // do lights
    // draw
    calculateNormals();
    // enable lights
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
