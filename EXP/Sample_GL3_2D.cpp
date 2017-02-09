#include <iostream>
#include<stdlib.h>
#include <cmath>
#include <fstream>
#include <vector>
#include<map>
#include <ao/ao.h>
#include <mpg123.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define BITS 8

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;
struct Galaxy {
	string name;
	float x,y;
	glm::mat4 mvp;
	glm::mat4 model;
	VAO* object;
	int direction; //0 for clockwise and 1 for anticlockwise for animation
	float dx;
	float dy;
	float angleRotated;
	int keyPressed;
	float x1,y1, x2,y2, x3,y3, x4,y4;
	int colorNumber;

	float distBwCenters1; // these two are for the check collision function;
	double alpha1;

};
mpg123_handle *mh;
unsigned char *buffer;
size_t buffer_size;
size_t done;
int err;

int driver;
ao_device *dev;

ao_sample_format format;
int channels, encoding;
long rate;

void audio_init() {
    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size= 3000;
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    mpg123_open(mh, "./breakout.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
}

void audio_play() {
    /* decode and play */
    if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
        ao_play(dev, (char*) buffer, done);
    else mpg123_seek(mh, 0, SEEK_SET);
}

void audio_close() {
    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
}
typedef struct Galaxy Sprite;
map <string, Galaxy> laserObjects;
map <string, Galaxy> bucketObjects;
map <string, Galaxy> fallingObjects;
map <string, Galaxy> mirrorObjects;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float trans_increments=0;
double newRotatedAngle=0;
int count=0;
int gameOverFlag=0,mirrorCollisionFlag=0,flag=0,normalBehaviourFlag=1, oneTimeFlag=0, seg_flag=0,lane1Flag=0, lane2Flag=0,lane3Flag=0, lane4Flag=0, coinAppearFlag1=0,collisionFlag;
VAO *triangle, *linea, *lineb, *linec, *lined, *linee, *linef, *lineg, *lineh, *linei, *linej, *linek, *linel, *linem, *linen, *linez;
double timeNow, last_updated_time,lastTime=0;
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float move_trans=0;
float rectangle_movement=0;
int grandScore=0;
float speedFactor=0,laserSpeedFactor=0;
float zoom_camera=1,x_change=0,y_change=0;
//rectangle["laser"].name="laser1";
//rectangle["bullet"].name="bullet1";

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

void createLaser(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, string name)
{
	GLfloat vertex_buffer_data [] = {
		x1,y1,0, // vertex 1 -4, -1
		x2,y2,0, // vertex 2 -2,-1
		x3,y3,0, // vertex 3 -2, 0.5

		x4,y4,0, // vertex 3  -4, 0.5
		x1,y1,0, // vertex 4  -4, -1
		x3,y3,0,  // vertex 1  -2,0.5

		-4+3.5-1,0.4,0,
		-4+3.5-1,0.2,0,
		-3.75+3.5,0.4,0,

		-3.75+3.5,0.2,0,
		-3.75+3.5,0.4,0,
		-4+3.5-1,0.2,0,

		-3.75+3.5,0.4,0,
		-3.75+3.5,0.2,0,
		-3.5+3.5,0.2,0,

		-4+3.5-1,-0.2,0,
		-4+3.5-1,-0.4,0,
		-3.75+3.5,-0.2,0,

		-4+3.5-1,-0.4,0,
		-3.75+3.5,-0.4,0,
		-3.75+3.5,-0.2,0,

		-3.75+3.5,-0.2,0,
		-3.75+3.5,-0.4,0,
		-3.5+3.5,-0.2,0,
	};

	GLfloat color_buffer_data [] = {
		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1

		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1

		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1

		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1

		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1

		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1

		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1

		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1
		75/255.0,89/255.0,181/255.0, // color 1
	};
	laserObjects["laser"].object = create3DObject(GL_TRIANGLES, 24, vertex_buffer_data, color_buffer_data, GL_FILL);

}
void createRectangle (float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, string name)
{
	GLfloat color_buffer_data[1000];
	// GL3 accepts only Triangles. Quads are not supported
	GLfloat vertex_buffer_data [] = {
		x1,y1,0, // vertex 1 -4, -1
		x2,y2,0, // vertex 2 -2,-1
		x3,y3,0, // vertex 3 -2, 0.5

		x4,y4,0, // vertex 3  -4, 0.5
		x1,y1,0, // vertex 4  -4, -1
		x3,y3,0,  // vertex 1  -2,0.5
	};
	if (name=="greeny")
		GLfloat color_buffer_data []={
			41/255.0f,194/255.0f,50/255.0f,
			15/255.0f,249/255.0f,29/255.0f,
			15/255.0f,249/255.0f,29/255.0f,
			41/255.0f,194/255.0f,50/255.0f,
			15/255.0f,249/255.0f,29/255.0f,
			15/255.0f,249/255.0f,29/255.0f,
		};
	if(name=="reddy")
		GLfloat color_buffer_data[]={
			200/255.0f, 48/255.0f, 48/255.0f,
			255/255.0f,72/255.0f,72/255.0f,
			255/255.0f,72/255.0f,72/255.0f,
			200/255.0f, 48/255.0f, 48/255.0f,
			255/255.0f,72/255.0f,72/255.0f,
			255/255.0f,72/255.0f,72/255.0f,
		};
	if(name=="beamy")
	{
		laserObjects["beam"].x1=x1;
		laserObjects["beam"].x2=x2;
		laserObjects["beam"].x3=x3;
		laserObjects["beam"].x4=x4;
		laserObjects["beam"].y1=y1;
		laserObjects["beam"].y2=y2;
		laserObjects["beam"].y3=y3;
		laserObjects["beam"].y4=y4;
		laserObjects["beam"].model=glm::mat4(1.0f);

		GLfloat color_buffer_data[]={
			256,0,0,
			256,0,0,
			256,0,0,
			256,0,0,
			256,0,0,
			256,0,0,
		};
	}
	if(name=="brick1" || name=="brick2" || name=="brick3" || name=="brick4" || name=="brick5")
	{
		int colorFillFlag=rand()%3+1;
		int powerUps=rand()%10+1;
		if(colorFillFlag==1)
		{
			fallingObjects[name].colorNumber=1;
			GLfloat color_buffer_data[]={
				193/255.0f,49/255.0f,49/255.0f,
				1,0,0,
				1,0,0,
				1,0,0,
				193/255.0f,49/255.0f,49/255.0f, //red coloured bricks
				1,0,0, };
		}
		else if(colorFillFlag==2)
		{
			fallingObjects[name].colorNumber=2;
			GLfloat color_buffer_data[]={
				40/255.0f,164/255.0f,108/255.0f,
				0,1,139/255.0f,
				0,1,139/255.0f, // green coloured bricks
				0,1,139/255.0f,
				40/255.0f,164/255.0f,108/255.0f,
				0,1,139/255.0f,
			};
		}

		else if(colorFillFlag==3)
		{
			fallingObjects[name].colorNumber=3;
			GLfloat color_buffer_data[]={
				0,0,0,
				0,0,0,
				0,0,0,
				0,0,0,
				0,0,0,
				0,0,0,
			};
		}
		if(powerUps==1)
		{
			fallingObjects[name].colorNumber=11;
			GLfloat color_buffer_data[]={
				189/255.0,189/255.0f,70/255.0f,
				247/255.0f,247/255.0f,12/255.0f, // yellow coloured bricks
				247/255.0f,247/255.0f,12/255.0f,
				189/255.0,189/255.0f,70/255.0f,
				247/255.0f,247/255.0f,12/255.0f,
				247/255.0f,247/255.0f,12/255.0f,
			};
		}
		if(powerUps==2)
		{
			fallingObjects[name].colorNumber=12;
			GLfloat color_buffer_data[]={
				35/255.0f, 190/255.0f,181/255.0f,
				40/255.0f,250/255.0f,238/255.0f,
				40/255.0f,250/255.0f,238/255.0f,
				35/255.0f, 190/255.0f,181/255.0f,
				40/255.0f,250/255.0f,238/255.0f,
				40/255.0f,250/255.0f,238/255.0f,
			};
		}
	}

	// create3DObject creates and returns a handle to a VAO that can be used later
	if(name=="greeny")
		bucketObjects["bucket1"].object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	else if(name=="reddy")
		bucketObjects["bucket2"].object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	else if(name=="beamy")
		laserObjects["beam"].object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	else if(name=="brick1")
		fallingObjects["brick1"].object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	else if(name=="brick2")
		fallingObjects["brick2"].object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	else if(name=="brick3")
		fallingObjects["brick3"].object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	else if(name=="brick4")
		fallingObjects["brick4"].object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	else if(name=="brick5")
		fallingObjects["brick5"].object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	else if(name=="coin1")
		fallingObjects["coin1"].object = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
//float vertical_mov=0;
float fraction_rotated=0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_UP:
				laserObjects["laser"].dy=0;
				break;
			case GLFW_KEY_DOWN:
				laserObjects["laser"].dy=0;
				break;
			case GLFW_KEY_A:
				bucketObjects["bucket1"].dx=0;
				break;
			case GLFW_KEY_D:
				bucketObjects["bucket1"].dx=0;
				break;
			case GLFW_KEY_LEFT:
				bucketObjects["bucket2"].dx=0;
				break;
			case GLFW_KEY_RIGHT:
				bucketObjects["bucket2"].dx=0;
				break;
			case GLFW_KEY_Q:
				fraction_rotated=0;
				//laserObjects["laser"].angleRotated+=-(M_PI/180.0f);
				break;
			case GLFW_KEY_E:
				fraction_rotated=0;
				//	laserObjects["laser"].angleRotated+=(M_PI/180.0f);
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_UP:
				laserObjects["laser"].dy=0.1;
				break;
			case GLFW_KEY_DOWN:
				laserObjects["laser"].dy=-0.1;
				break;
			case GLFW_KEY_A:
				bucketObjects["bucket1"].dx=-0.1;
				break;
			case GLFW_KEY_D:
				bucketObjects["bucket1"].dx=0.1;
				break;
			case GLFW_KEY_LEFT:
				bucketObjects["bucket2"].dx=-0.1;
				break;
			case GLFW_KEY_RIGHT:
				bucketObjects["bucket2"].dx=0.1;
				break;
			case GLFW_KEY_SPACE:
				laserObjects["beam"].keyPressed=1;
				seg_flag=1;
				if(oneTimeFlag==0)
					last_updated_time=glfwGetTime();
				break;
			case GLFW_KEY_Q:
				fraction_rotated=-(M_PI/180.0f);
				break;
			case GLFW_KEY_E:
				fraction_rotated=(M_PI/180.0f);
				break;
				//			case GLFW_KEY_Q:
				//				laserObjects["laser"].angleRotated=(M_PI/180.0f);
				//				break;
				//			case GLFW_KEY_E:
				//				laserObjects["laser"].angleRotated=-(M_PI/180.0f);
				//				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	//	switch (key) {
	////		case 'Q':
	//		case 'q':
	//			quit(window);
	//			break;
	//		default:
	//			break;
	//	}
}
int left_button_clicked=0,right_button_clicked=0;
double mouse_x,mouse_y, mouse_x_old, mouse_y_old;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	glfwGetCursorPos(window, &mouse_x, &mouse_y);
	mouse_x=(mouse_x/1200.0)*8-4;

	//	cout << "mouse_x:" << mouse_x << "mouse_y:" << mouse_y << endl;
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS)
			{
				left_button_clicked=1;
				if(mouse_x > (bucketObjects["bucket1"].x1+bucketObjects["bucket1"].x3)/2)
					bucketObjects["bucket1"].dx=0.1;
				else if(mouse_x < (bucketObjects["bucket1"].x1+bucketObjects["bucket1"].x3)/2)
					bucketObjects["bucket1"].dx=-0.1;
			}
			else if(action==GLFW_RELEASE)
			{
				left_button_clicked=0;
				bucketObjects["bucket1"].dx=0;
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_PRESS)
			{
				right_button_clicked=1;
				if(mouse_x > (bucketObjects["bucket2"].x1+bucketObjects["bucket2"].x3)/2)
					bucketObjects["bucket2"].dx=0.1;
				else if(mouse_x < (bucketObjects["bucket2"].x1+bucketObjects["bucket2"].x3)/2)
					bucketObjects["bucket2"].dx=-0.1;
			}
			else if(action==GLFW_RELEASE)
			{
				right_button_clicked=0;
				bucketObjects["bucket2"].dx=0;
			}
			break;
		default:
			break;
	}
}
void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset==-1) {
		zoom_camera /= 1.1; //make it bigger than current size
	}
	else if(yoffset==1){
		zoom_camera *= 1.1; //make it bigger than current size
	}
	if (zoom_camera<=1) {
		zoom_camera = 1;
	}
	if (zoom_camera>=4) {
		zoom_camera=4;
	}
	if(x_change-4.0f/zoom_camera<-4)
		x_change=-4+4.0f/zoom_camera;
	else if(x_change+4.0f/zoom_camera>4)
		x_change=4-4.0f/zoom_camera;
	if(y_change-4.0f/zoom_camera<-4)
		y_change=-4+4.0f/zoom_camera;
	else if(y_change+4.0f/zoom_camera>4)
		y_change=4-4.0f/zoom_camera;
	Matrices.projection = glm::ortho((float)(-4.0f/zoom_camera+x_change), (float)(4.0f/zoom_camera+x_change), (float)(-4.0f/zoom_camera+y_change), (float)(4.0f/zoom_camera+y_change), 0.1f, 500.0f);
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	//    Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}


// Creates the triangle object used in this sample code
void createLine (float x1, float y1, float x2, float y2, string name)
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	GLfloat vertex_buffer_data [] = {
		x1,y1,0, // vertex 0
		x2,y2,0, // vertex 1
		(x1+x2)/2.0,(y1+y2)/2.0,0, // vertex 2
	};
	GLfloat color_buffer_data[]={
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
	};
	if(name=="mirror1" || name=="mirror2")
	{
		mirrorObjects[name].x1=x1;
		mirrorObjects[name].x2=x2;
		mirrorObjects[name].y1=y1;
		mirrorObjects[name].y2=y2;
	}

	// create3DObject creates and returns a handle to a VAO that can be used later
	if(name=="mirror1")
		mirrorObjects["mirror1"].object = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="mirror2")
		mirrorObjects["mirror2"].object = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linea")
		linea = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="lineb")
		lineb = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linec")
		linec= create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="lined")
		lined = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linee")
		linee = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linef")
		linef = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="lineg")
		lineg = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="lineh")
		lineh = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linei")
		linei = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linej")
		linej = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linek")
		linek = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linel")
		linel = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linem")
		linem = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linen")
		linen = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	if(name=="linez")
		linez = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);

	//triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}


// Creates the rectangle object used in this sample code

/* Edit this function according to your assignment */
void initialise()
{

	bucketObjects["bucket1"].model=glm::mat4(1.0f);
	laserObjects["laser"].model=glm::mat4(1.0f);
	bucketObjects["bucket2"].model=glm::mat4(1.0f);
	laserObjects["beam"].model=glm::mat4(1.0f);
	mirrorObjects["mirror1"].mvp=glm::mat4(1.0f);
}
float speed=0.0f;
float value_of_y1;
void display()
{
	int onesplace=abs(grandScore)%10;
	int tensplace=abs((grandScore/10))%10;
	//	cout << "onesplace:" << onesplace << "tensplace:" << tensplace << endl;
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
	Matrices.projection = glm::ortho((float)(-4.0f/zoom_camera+x_change), (float)(4.0f/zoom_camera+x_change), (float)(-4.0f/zoom_camera+y_change), (float)(4.0f/zoom_camera+y_change), 0.1f, 500.0f);
	//	Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
	glm::mat4 VP = Matrices.projection * Matrices.view;
	glm::mat4 privateMVP=VP*glm::mat4(1.0f);
	if(grandScore<0)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linez);
	}

	if(tensplace==0)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linea);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineb);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linec);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linee);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linef);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineg);
	}
	if(tensplace==1)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linec);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineg);
	}
	if(tensplace==2)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linea);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linec);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lined);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linee);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linef);
	}
	if(tensplace==3)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linea);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linec);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lined);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linef);
	}
	if(tensplace==4)
	{
		draw3DObject(lineb);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lined);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linec);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineg);
	}
	if(tensplace==5)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linea);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineb);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lined);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linef);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineg);

	}
	if(tensplace==6)
	{

		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linea);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineb);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lined);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linee);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linef);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineg);
	}
	if(tensplace==7)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linea);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linec);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineg);
	}
	if(tensplace==8)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linea);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineb);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linec);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lined);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linee);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linef);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineg);
	}
	if(tensplace==9)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linea);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineb);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linec);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lined);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linef);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineg);
	}

	/*	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linea);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineb);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linec);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lined);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linee);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linef);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineg);


		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineh);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linei);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linej);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linek);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linel);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linem);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linen); */
	if(onesplace==0)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineh);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linei);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linej);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linel);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linem);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linen);

	}
	if(onesplace==1)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linej);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linen);
	}


	if(onesplace==2)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineh);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linej);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linek);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linel);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linem);
	}

	if(onesplace==3)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineh);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linej);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linek);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linem);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linen);

	}

	if(onesplace==4)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linei);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linej);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linen);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linek);

	}

	if(onesplace==5)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linei);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineh);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linem);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linen);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linek);

	}

	if(onesplace==6)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineh);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linei);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linek);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linel);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linem);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linen);

	}
	if(onesplace==7)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineh);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linej);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linen);
	}

	if(onesplace==8)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineh);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linei);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linej);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linek);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linel);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linem);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linen);

	}

	if(onesplace==9)
	{
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(lineh);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linei);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linej);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linek);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linem);
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &privateMVP[0][0]);
		draw3DObject(linen);
	}



}
int checkCollision(Galaxy beam, double theta)
{

	float widthOfBrick=abs(fallingObjects["brick1"].x1-fallingObjects["brick1"].x2); //same for all the bricks
	float widthOfBeam=abs(beam.x1-beam.x2);

	float centerOfBeamX=(beam.x1+beam.x3)/2.0-3.5; float centerOfBeamY= (beam.y1+beam.y3)/2.0 + laserObjects["laser"].y1;
	for(map < string,Galaxy>::iterator it1=fallingObjects.begin(); it1!=fallingObjects.end();it1++)
	{
		string brick1=it1->first;

		float centerOfBrick1X=(fallingObjects[brick1].x1+fallingObjects[brick1].x3)/2.0; float centerOfBrick1Y= (fallingObjects[brick1].y1+fallingObjects[brick1].y3)/2.0;
		float centerOfBrick2X=(fallingObjects[brick1].x1+fallingObjects[brick1].x3)/2.0; float centerOfBrick2Y= (fallingObjects[brick1].y1+fallingObjects[brick1].y3)/2.0;

		fallingObjects[brick1].alpha1=atan((centerOfBrick1Y-centerOfBeamY)/(centerOfBrick1X-centerOfBeamX));

		fallingObjects[brick1].distBwCenters1=sqrt((centerOfBeamX-centerOfBrick1X)*(centerOfBeamX-centerOfBrick1X) + (centerOfBeamY-centerOfBrick1Y)*(centerOfBeamY-centerOfBrick1Y));

		if(((widthOfBrick/cos(theta)) + widthOfBeam*cos(fallingObjects[brick1].alpha1-theta))/2.0>fallingObjects[brick1].distBwCenters1)
		{
			if(fallingObjects[brick1].name=="brick1")
			{
				if(fallingObjects[brick1].colorNumber==3)// if black brick is hit, increase the grandScore
					grandScore++;
				else if(fallingObjects[brick1].colorNumber==1 || fallingObjects[brick1].colorNumber==2)
					grandScore--; //if red or green flag is hit, decrease the grandScore
				else if(fallingObjects[brick1].colorNumber==11)
					speedFactor+=0.02;
				else if(fallingObjects[brick1].colorNumber==12)
					laserSpeedFactor+=0.001;
				return 1;
			}
			if(fallingObjects[brick1].name=="brick2")
			{
				if(fallingObjects[brick1].colorNumber==3)
					grandScore++;
				else if(fallingObjects[brick1].colorNumber==1 || fallingObjects[brick1].colorNumber==2)
					grandScore--;
				else if(fallingObjects[brick1].colorNumber==11)
					speedFactor+=0.02;
				else if(fallingObjects[brick1].colorNumber==12)
					laserSpeedFactor+=0.001;
				return 2;
			}
			if(fallingObjects[brick1].name=="brick3")
			{
				if(fallingObjects[brick1].colorNumber==3)
					grandScore++;
				else if(fallingObjects[brick1].colorNumber==1 || fallingObjects[brick1].colorNumber==2)
					grandScore--;
				else if(fallingObjects[brick1].colorNumber==11)
					speedFactor+=0.02;
				else if(fallingObjects[brick1].colorNumber==12)
					laserSpeedFactor+=0.001;
				return 3;
			}
			if(fallingObjects[brick1].name=="brick4")
			{
				if(fallingObjects[brick1].colorNumber==3)
					grandScore++;
				else if(fallingObjects[brick1].colorNumber==1 || fallingObjects[brick1].colorNumber==2)
					grandScore--;
				else if(fallingObjects[brick1].colorNumber==11)
					speedFactor+=0.02;
				else if(fallingObjects[brick1].colorNumber==12)
					laserSpeedFactor+=0.001;
				return 4;
			}
			if(fallingObjects[brick1].name=="brick5")
			{
				if(fallingObjects[brick1].colorNumber==3)
					grandScore++;
				else if(fallingObjects[brick1].colorNumber==1 || fallingObjects[brick1].colorNumber==2)
					grandScore--;
				else if(fallingObjects[brick1].colorNumber==11)
					speedFactor+=0.02;
				else if(fallingObjects[brick1].colorNumber==12)
					laserSpeedFactor+=0.001;
				return 5;
			}
			else
				return 0;
		}
	}
}
int checkReflectionCondition(Galaxy beam)
{
	//putting coordinates in equation of line of mirror.
	int sign1, sign2, sign3, sign4, majorSign1, majorSign2;
	beam.x1+=-3.5;
	beam.x2+=-3.5;
	beam.y1+=laserObjects["laser"].y1;
	beam.y2+=laserObjects["laser"].y1;
	for(map < string,Galaxy>::iterator it=mirrorObjects.begin(); it!=mirrorObjects.end();it++)
	{
		string mirror1=it->first;
		newRotatedAngle=2*mirrorObjects[mirror1].angleRotated-beam.angleRotated;

		//equation of the mirror
		if(((mirrorObjects[mirror1].y1-mirrorObjects[mirror1].y2)/(mirrorObjects[mirror1].x1-mirrorObjects[mirror1].x2))*(beam.x1-mirrorObjects[mirror1].x1) -beam.y1 + mirrorObjects[mirror1].y1>=0)
			sign1=1;
		else
			sign1=-1;
		if(((mirrorObjects[mirror1].y1-mirrorObjects[mirror1].y2)/(mirrorObjects[mirror1].x1-mirrorObjects[mirror1].x2))*(beam.x2-mirrorObjects[mirror1].x1) -beam.y2 + mirrorObjects[mirror1].y1>=0)
			sign2=1;
		else
			sign2=-1;
		if(sign1*sign2==-1)
			majorSign1=1; //confirmed that laser extremes are on the  opp. sides of the mirror.

		//eqution of laserBeam.

		if(((beam.y2-beam.y1)/(beam.x2-beam.x1))*(mirrorObjects[mirror1].y1-beam.x1)-mirrorObjects[mirror1].y1+beam.y1>=0)
			sign3=1;
		else
			sign3=-1;
		if(((beam.y2-beam.y1)/(beam.x2-beam.x1))*(mirrorObjects[mirror1].y2-beam.x1)-mirrorObjects[mirror1].y2+beam.y1>=0)
			sign4=1;
		else
			sign4=-1;
		if(sign3*sign4==-1)
			majorSign2=1; // confirmed that the mirror is opp. side of the laser.
		if(majorSign1*majorSign2==1)
		{
			if(mirrorObjects[mirror1].name=="m1")
				return 1;
			//return the info that the laser has collided with the first mirror
			else if(mirrorObjects[mirror1].name=="m2")
				return 2;  // return the info that the laser has collided with the second mirror
		}
	}
	return 0;
}

void draw ()
{
	/* Render the scene with openGL */
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);
	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);
	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;
	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix

	/* Render your scene */

	/*
	   glm::mat4 translateTriangle = glm::translate (glm::vec3(vertical_mov-0.5f, 0.0f,0.0f)); // glTranslatef
	   glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(1,0,1));  // rotate about vector (1,0,1)
	   glm::mat4 triangleTransform = translateTriangle;// * rotateTriangle;
	   Matrices.model *= triangleTransform;
	   MVP = VP * Matrices.model; // MVP = p * V * M
	 */

	//  Don't change unless you are sure!!
	//	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	//	draw3DObject(triangle);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();

	if(glfwGetTime()-last_updated_time>8)
		seg_flag=0;
	timeNow=glfwGetTime();
	if(laserObjects["beam"].keyPressed==1)
	{
		if(oneTimeFlag==0)
		{
			createRectangle(-0.3,0,0.3,0,0.3,0.05,-0.3,0.05,"beamy");
			seg_flag=1;
			oneTimeFlag=1;
			laserObjects["beam"].keyPressed=0;
			value_of_y1=laserObjects["laser"].y1;
			//		cout << "value of y1 " << value_of_y1 << endl;
		}
		if(timeNow-last_updated_time>=1)
		{
			last_updated_time=timeNow;
			oneTimeFlag=0;
			speed=0;
		}
	}

	if(seg_flag==1)
	{

		if(normalBehaviourFlag==1)
			laserObjects["beam"].angleRotated=laserObjects["laser"].angleRotated;
		if(laserObjects["beam"].x1-3.5>4 || laserObjects["beam"].x1-3.5<-4 || laserObjects["beam"].x2-3.5>4 || laserObjects["beam"].x2-3.5<-4 || laserObjects["beam"].y1+laserObjects["laser"].y1>4 || laserObjects["beam"].y1+laserObjects["laser"].y1<-4)
		{
			normalBehaviourFlag=1;
			laserObjects["beam"].x1=-4;
			laserObjects["beam"].x2=-4;
			laserObjects["beam"].x3=-4;
			laserObjects["beam"].x4=-4;
			laserObjects["beam"].y1=-4;
			laserObjects["beam"].y2=-4;
			laserObjects["beam"].y3=-4;
			laserObjects["beam"].y4=-4;
		}

		laserObjects["beam"].x1+=speed*cos(laserObjects["beam"].angleRotated);
		laserObjects["beam"].x2+=speed*cos(laserObjects["beam"].angleRotated);
		laserObjects["beam"].x3+=speed*cos(laserObjects["beam"].angleRotated);
		laserObjects["beam"].x4+=speed*cos(laserObjects["beam"].angleRotated);

		laserObjects["beam"].y1+=speed*sin(laserObjects["beam"].angleRotated);
		laserObjects["beam"].y2+=speed*sin(laserObjects["beam"].angleRotated);
		laserObjects["beam"].y3+=speed*sin(laserObjects["beam"].angleRotated);
		laserObjects["beam"].y4+=speed*sin(laserObjects["beam"].angleRotated);
		speed=speed+0.006+laserSpeedFactor;

		//	cout << laserObjects["laser"].y1 << endl;
		//	cout << "value of y1 " << value_of_y1 << endl;

		laserObjects["beam"].model=glm::mat4(1.0f);
		glm::mat4 translateRectangle3=glm::translate(glm::vec3(-3.5,value_of_y1,0));
		glm::mat4 translateRectangle13=glm::translate(glm::vec3(laserObjects["beam"].x1,laserObjects["beam"].y1,0));
		glm::mat4 rotateRectangle = glm::rotate(laserObjects["beam"].angleRotated, glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		laserObjects["beam"].model*=translateRectangle13*translateRectangle3*rotateRectangle;
		laserObjects["beam"].mvp=VP*laserObjects["beam"].model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &laserObjects["beam"].mvp[0][0]);
		draw3DObject(laserObjects["beam"].object);
	}

	float f, num;
	if(lane1Flag==0)
	{
		lane1Flag=1;
		fallingObjects["brick1"].model=glm::mat4(1.0f);
		num=-(rand()%7+17)/10.0;
		fallingObjects["brick1"].x1=num-0.15;
		fallingObjects["brick1"].x2=num+0.15;
		fallingObjects["brick1"].x3=num+0.15;
		fallingObjects["brick1"].x4=num-0.15;
		fallingObjects["brick1"].y1=3.2;
		fallingObjects["brick1"].y2=3.2;
		fallingObjects["brick1"].y3=4;
		fallingObjects["brick1"].y4=4;
		createRectangle(fallingObjects["brick1"].x1,fallingObjects["brick1"].y1, fallingObjects["brick1"].x2, fallingObjects["brick1"].y2, fallingObjects["brick1"].x3, fallingObjects["brick1"].y3, fallingObjects["brick1"].x4, fallingObjects["brick1"].y4,"brick1");
	}

	if(lane1Flag==1)
	{
		f=-2;
		fallingObjects["brick1"].y1+=-0.01-speedFactor;
		fallingObjects["brick1"].y2+=-0.01-speedFactor;
		fallingObjects["brick1"].y3+=-0.01-speedFactor;
		fallingObjects["brick1"].y4+=-0.01-speedFactor;
		if(fallingObjects["brick1"].y1<-2)
		{
			lane1Flag=0;
			if(bucketObjects["bucket1"].x1<=fallingObjects["brick1"].x1 && bucketObjects["bucket1"].x2 >= fallingObjects["brick1"].x2)
			{
				if(fallingObjects["brick1"].colorNumber==bucketObjects["bucket1"].colorNumber)
					grandScore++;
			}
			if(bucketObjects["bucket2"].x1<=fallingObjects["brick1"].x1 && bucketObjects["bucket2"].x2 >= fallingObjects["brick1"].x2)
			{
				if(fallingObjects["brick1"].colorNumber==bucketObjects["bucket2"].colorNumber)
					grandScore++;
			}
			if(((bucketObjects["bucket1"].x1<=fallingObjects["brick1"].x1 && bucketObjects["bucket1"].x2 >= fallingObjects["brick1"].x2)  ||(bucketObjects["bucket2"].x1<=fallingObjects["brick1"].x1 && bucketObjects["bucket2"].x2 >= fallingObjects["brick1"].x2)) && fallingObjects["brick1"].colorNumber==3)
				grandScore--;


		}

		//	cout << "Hello!"<< endl;


		glm::mat4 translateRectangle4=glm::translate(glm::vec3(0,-0.01-speedFactor,0));
		fallingObjects["brick1"].model*=translateRectangle4;
		fallingObjects["brick1"].mvp=VP*fallingObjects["brick1"].model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &fallingObjects["brick1"].mvp[0][0]);
		draw3DObject(fallingObjects["brick1"].object);
	}

	if(lane2Flag==0)
	{
		lane2Flag=1;
		fallingObjects["brick2"].model=glm::mat4(1.0f);
		num=-(rand()%7+7)/10.0;
		fallingObjects["brick2"].x1=num-0.15;
		fallingObjects["brick2"].x2=num+0.15;
		fallingObjects["brick2"].x3=num+0.15;
		fallingObjects["brick2"].x4=num-0.15;
		fallingObjects["brick2"].y1=3.2;
		fallingObjects["brick2"].y2=3.2;
		fallingObjects["brick2"].y3=4;
		fallingObjects["brick2"].y4=4;
		createRectangle(fallingObjects["brick2"].x1,fallingObjects["brick2"].y1, fallingObjects["brick2"].x2, fallingObjects["brick2"].y2, fallingObjects["brick2"].x3, fallingObjects["brick2"].y3, fallingObjects["brick2"].x4, fallingObjects["brick2"].y4,"brick2");

	}
	if(lane2Flag==1)
	{
		f=-2;
		fallingObjects["brick2"].y1+=-0.02-speedFactor;
		fallingObjects["brick2"].y2+=-0.02-speedFactor;
		fallingObjects["brick2"].y3+=-0.02-speedFactor;
		fallingObjects["brick2"].y4+=-0.02-speedFactor;
		if(fallingObjects["brick2"].y1<-2)
		{
			lane2Flag=0;
			if(bucketObjects["bucket1"].x1<=fallingObjects["brick2"].x1 && bucketObjects["bucket1"].x2 >= fallingObjects["brick2"].x2)
			{
				if(fallingObjects["brick2"].colorNumber==bucketObjects["bucket2"].colorNumber)
					grandScore++;
			}
			if(bucketObjects["bucket2"].x1<=fallingObjects["brick2"].x1 && bucketObjects["bucket2"].x2 >= fallingObjects["brick2"].x2)
			{
				if(fallingObjects["brick2"].colorNumber==bucketObjects["bucket2"].colorNumber)
					grandScore++;
			}
			if(((bucketObjects["bucket1"].x1<=fallingObjects["brick2"].x1 && bucketObjects["bucket1"].x2 >= fallingObjects["brick2"].x2)  ||(bucketObjects["bucket2"].x1<=fallingObjects["brick2"].x1 && bucketObjects["bucket2"].x2 >= fallingObjects["brick2"].x2)) && fallingObjects["brick2"].colorNumber==3)
				grandScore--;
		}

		glm::mat4 translateRectangle5=glm::translate(glm::vec3(0,-0.02-speedFactor,0));
		fallingObjects["brick2"].model*=translateRectangle5;
		fallingObjects["brick2"].mvp=VP*fallingObjects["brick2"].model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &fallingObjects["brick2"].mvp[0][0]);
		draw3DObject(fallingObjects["brick2"].object);
	}


	if(lane3Flag==0)
	{
		lane3Flag=1;
		fallingObjects["brick3"].model=glm::mat4(1.0f);
		num=(rand()%7+7)/10.0;
		fallingObjects["brick3"].x1=num-0.15;
		fallingObjects["brick3"].x2=num+0.15;
		fallingObjects["brick3"].x3=num+0.15;
		fallingObjects["brick3"].x4=num-0.15;
		fallingObjects["brick3"].y1=3.2;
		fallingObjects["brick3"].y2=3.2;
		fallingObjects["brick3"].y3=4;
		fallingObjects["brick3"].y4=4;
		createRectangle(fallingObjects["brick3"].x1,fallingObjects["brick3"].y1, fallingObjects["brick3"].x2, fallingObjects["brick3"].y2, fallingObjects["brick3"].x3, fallingObjects["brick3"].y3, fallingObjects["brick3"].x4, fallingObjects["brick3"].y4,"brick3");

	}
	if(lane3Flag==1)
	{
		f=-2;
		fallingObjects["brick3"].y1+=-0.04-speedFactor;
		fallingObjects["brick3"].y2+=-0.04-speedFactor;
		fallingObjects["brick3"].y3+=-0.04-speedFactor;
		fallingObjects["brick3"].y4+=-0.04-speedFactor;
		if(fallingObjects["brick3"].y1<-2)
		{
			lane3Flag=0;
			if(bucketObjects["bucket1"].x1<=fallingObjects["brick3"].x1 && bucketObjects["bucket1"].x2 >= fallingObjects["brick3"].x2)
			{
				if(fallingObjects["brick3"].colorNumber==bucketObjects["bucket1"].colorNumber)
					grandScore++;
			}
			if(bucketObjects["bucket2"].x1<=fallingObjects["brick3"].x1 && bucketObjects["bucket2"].x2 >= fallingObjects["brick3"].x2)
			{
				if(fallingObjects["brick3"].colorNumber==bucketObjects["bucket2"].colorNumber)
					grandScore++;
			}
			if(((bucketObjects["bucket1"].x1<=fallingObjects["brick3"].x1 && bucketObjects["bucket1"].x2 >= fallingObjects["brick3"].x2)  ||(bucketObjects["bucket2"].x1<=fallingObjects["brick3"].x1 && bucketObjects["bucket2"].x2 >= fallingObjects["brick3"].x2)) && fallingObjects["brick3"].colorNumber==3)
				grandScore--;
		}

		glm::mat4 translateRectangle15=glm::translate(glm::vec3(0,-0.04-speedFactor,0));
		fallingObjects["brick3"].model*=translateRectangle15;
		fallingObjects["brick3"].mvp=VP*fallingObjects["brick3"].model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &fallingObjects["brick3"].mvp[0][0]);
		draw3DObject(fallingObjects["brick3"].object);
	}


	if(lane4Flag==0)
	{
		lane4Flag=1;
		fallingObjects["brick4"].model=glm::mat4(1.0f);
		num=(rand()%7+17)/10.0;
		fallingObjects["brick4"].x1=num-0.15;
		fallingObjects["brick4"].x2=num+0.15;
		fallingObjects["brick4"].x3=num+0.15;
		fallingObjects["brick4"].x4=num-0.15;
		fallingObjects["brick4"].y1=3.2;
		fallingObjects["brick4"].y2=3.2;
		fallingObjects["brick4"].y3=4;
		fallingObjects["brick4"].y4=4;
		createRectangle(fallingObjects["brick4"].x1,fallingObjects["brick4"].y1, fallingObjects["brick4"].x2, fallingObjects["brick4"].y2, fallingObjects["brick4"].x3, fallingObjects["brick4"].y3, fallingObjects["brick4"].x4, fallingObjects["brick4"].y4,"brick4");

	}
	if(lane4Flag==1)
	{
		f=-2;
		fallingObjects["brick4"].y1+=-0.03-speedFactor;
		fallingObjects["brick4"].y2+=-0.03-speedFactor;
		fallingObjects["brick4"].y3+=-0.03-speedFactor;
		fallingObjects["brick4"].y4+=-0.03-speedFactor;
		if(fallingObjects["brick4"].y1<-2)
		{
			lane4Flag=0;
			if(bucketObjects["bucket1"].x1<=fallingObjects["brick4"].x1 && bucketObjects["bucket1"].x2 >= fallingObjects["brick4"].x2)
			{
				if(fallingObjects["brick4"].colorNumber==bucketObjects["bucket1"].colorNumber)
					grandScore++;
			}
			if(bucketObjects["bucket2"].x1<=fallingObjects["brick4"].x1 && bucketObjects["bucket2"].x2 >= fallingObjects["brick4"].x2)
			{
				if(fallingObjects["brick4"].colorNumber==bucketObjects["bucket2"].colorNumber)
					grandScore++;
			}
			if(((bucketObjects["bucket1"].x1<=fallingObjects["brick4"].x1 && bucketObjects["bucket1"].x2 >= fallingObjects["brick4"].x2)  ||(bucketObjects["bucket2"].x1<=fallingObjects["brick4"].x1 && bucketObjects["bucket2"].x2 >= fallingObjects["brick4"].x2)) && fallingObjects["brick4"].colorNumber==3)
				grandScore--;
		}

		glm::mat4 translateRectangle25=glm::translate(glm::vec3(0,-0.03-speedFactor,0));
		fallingObjects["brick4"].model*=translateRectangle25;
		fallingObjects["brick4"].mvp=VP*fallingObjects["brick4"].model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &fallingObjects["brick4"].mvp[0][0]);
		draw3DObject(fallingObjects["brick4"].object);
	}




	//	cout << laserObjects["beam"].object;
	//	glm::mat4 translate1=glm::translate(glm::vec3(3.75,0,0));
	//	glm::mat4 translate2=glm::translate(glm::vec3(-3.75,0,0));
	laserObjects["laser"].model=glm::mat4(1.0f);
	laserObjects["laser"].y1+=laserObjects["laser"].dy;
	laserObjects["laser"].y2+=laserObjects["laser"].dy;
	laserObjects["laser"].y3+=laserObjects["laser"].dy;
	laserObjects["laser"].y4+=laserObjects["laser"].dy;
	laserObjects["laser"].angleRotated+=fraction_rotated;

	if(laserObjects["laser"].angleRotated>=-(M_PI/6.0f) && laserObjects["laser"].angleRotated <=(M_PI/6.0f) && laserObjects["laser"].y1>=-2)
	{
		glm::mat4 translateRectangle = glm::translate (glm::vec3(-3.5,laserObjects["laser"].y1,0));
		glm::mat4 rotateRectangle = glm::rotate(laserObjects["laser"].angleRotated, glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		laserObjects["laser"].model*=translateRectangle*rotateRectangle;
		laserObjects["laser"].mvp = VP * laserObjects["laser"].model;
	}
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &laserObjects["laser"].mvp[0][0]);
	draw3DObject(laserObjects["laser"].object);


	bucketObjects["bucket1"].x1+=bucketObjects["bucket1"].dx;
	bucketObjects["bucket1"].x2+=bucketObjects["bucket1"].dx;
	bucketObjects["bucket1"].x3+=bucketObjects["bucket1"].dx;
	bucketObjects["bucket1"].x4+=bucketObjects["bucket1"].dx;
	if(bucketObjects["bucket1"].x1>=-3.8 && bucketObjects["bucket1"].x2<=3.8)
	{
		glm::mat4 translateRectangle1=glm::translate(glm::vec3(bucketObjects["bucket1"].dx,0,0));
		bucketObjects["bucket1"].model*=translateRectangle1;
		bucketObjects["bucket1"].mvp=VP*bucketObjects["bucket1"].model;
	}
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &bucketObjects["bucket1"].mvp[0][0]);
	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(bucketObjects["bucket1"].object);

	bucketObjects["bucket2"].x1+=bucketObjects["bucket2"].dx;
	bucketObjects["bucket2"].x2+=bucketObjects["bucket2"].dx;
	bucketObjects["bucket2"].x3+=bucketObjects["bucket2"].dx;
	bucketObjects["bucket2"].x4+=bucketObjects["bucket2"].dx;
	if(bucketObjects["bucket2"].x1>=-3.8 && bucketObjects["bucket2"].x2<=3.8)
	{
		glm::mat4 translateRectangle2=glm::translate(glm::vec3(bucketObjects["bucket2"].dx,0,0));
		bucketObjects["bucket2"].model*=translateRectangle2;
		bucketObjects["bucket2"].mvp=VP*bucketObjects["bucket2"].model;
	}
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &bucketObjects["bucket2"].mvp[0][0]);
	draw3DObject(bucketObjects["bucket2"].object);

	collisionFlag=checkCollision(laserObjects["beam"],laserObjects["laser"].angleRotated);

	//	cout << collisionFlag << endl;
	//	cout << endl;
	if(collisionFlag==1)
	{
		laserObjects["beam"].x1=-4;
		laserObjects["beam"].y1=-4;
		laserObjects["beam"].x2=-4;
		laserObjects["beam"].y2=-4;
		laserObjects["beam"].x3=-4;
		laserObjects["beam"].y3=-4;
		laserObjects["beam"].x4=-4;
		laserObjects["beam"].y4=-4;
		collisionFlag=0;
		lane1Flag=0;
		seg_flag=0;
	}

	if(collisionFlag==2)
	{
		laserObjects["beam"].x1=-4;
		laserObjects["beam"].y1=-4;
		laserObjects["beam"].x2=-4;
		laserObjects["beam"].y2=-4;
		laserObjects["beam"].x3=-4;
		laserObjects["beam"].y3=-4;
		laserObjects["beam"].x4=-4;
		laserObjects["beam"].y4=-4;
		collisionFlag=0;
		lane2Flag=0;
		seg_flag=0;

	}
	if(collisionFlag==3)
	{
		laserObjects["beam"].x1=-4;
		laserObjects["beam"].y1=-4;
		laserObjects["beam"].x2=-4;
		laserObjects["beam"].y2=-4;
		laserObjects["beam"].x3=-4;
		laserObjects["beam"].y3=-4;
		laserObjects["beam"].x4=-4;
		laserObjects["beam"].y4=-4;
		collisionFlag=0;
		lane3Flag=0;
		seg_flag=0;

	}

	if(collisionFlag==4)
	{
		laserObjects["beam"].x1=-4;
		laserObjects["beam"].y1=-4;
		laserObjects["beam"].x2=-4;
		laserObjects["beam"].y2=-4;
		laserObjects["beam"].x3=-4;
		laserObjects["beam"].y3=-4;
		laserObjects["beam"].x4=-4;
		laserObjects["beam"].y4=-4;
		collisionFlag=0;
		lane4Flag=0;
		seg_flag=0;

	}

	//	if(glfwGetTime()-lastTime>0.1)
	//	{
	//		lastTime=glfwGetTime();
	mirrorCollisionFlag=checkReflectionCondition(laserObjects["beam"]);
	//	}
	//	cout << mirrorCollisionFlag << endl;
	if(mirrorCollisionFlag==1)
	{
		normalBehaviourFlag=0;
		if(glfwGetTime()-lastTime>0.15)
		{
			laserObjects["beam"].angleRotated=newRotatedAngle;
			lastTime=glfwGetTime();
		}
	}
	else if(mirrorCollisionFlag==2)
	{
		normalBehaviourFlag=0;
		if(glfwGetTime()-lastTime>0.15)
		{

			laserObjects["beam"].angleRotated=newRotatedAngle;
			lastTime=glfwGetTime();
		}
	}

	mirrorObjects["mirror1"].model=glm::mat4(1.0f);
	mirrorObjects["mirror1"].mvp=VP*mirrorObjects["mirror1"].model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &mirrorObjects["mirror1"].mvp[0][0]);
	draw3DObject(mirrorObjects["mirror1"].object);

	mirrorObjects["mirror2"].model=glm::mat4(1.0f);
	mirrorObjects["mirror2"].mvp=VP*mirrorObjects["mirror2"].model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &mirrorObjects["mirror2"].mvp[0][0]);
	draw3DObject(mirrorObjects["mirror2"].object);

	display(); //display the grandScore in seven-segment display.
	//	cout <<"Grandscore:"<< grandScore << endl;

	// Increment angles

	//camera_rotation_angle++; // Simulating camera rotation
	// triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	//  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Brick-Breaker", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetScrollCallback(window, mousescroll); // mouse scroll


	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	laserObjects["laser"].name="lasy";
	laserObjects["beam"].name="beamy";
	bucketObjects["bucket1"].name="greeny";
	bucketObjects["bucket2"].name="reddy";
	bucketObjects["bucket1"].colorNumber=2;
	bucketObjects["bucket2"].colorNumber=1;
	laserObjects["laser"].angleRotated=0;

	fallingObjects["brick1"].name="brick1"; //initialising the names of all the bricks, need it in the checkCollison() function.
	fallingObjects["brick2"].name="brick2";
	fallingObjects["brick3"].name="brick3";
	fallingObjects["brick4"].name="brick4";
	fallingObjects["brick5"].name="brick5";

	mirrorObjects["mirror1"].name="m1"; // initialising the names and their rotated angle of all the mirrors, mirrors having angle greater than 90 degree are stored as 90-theta whereas the ones with the lesser angle are stored as theta;
	mirrorObjects["mirror1"].angleRotated=(-M_PI/4.0f);
	mirrorObjects["mirror2"].name="m2";
	mirrorObjects["mirror2"].angleRotated=(M_PI/3.0f);




	/* Objects should be created before any other gl function and shaders */
	// Create the models
	initialise();
	//createLine (); // Generate the VAO, VBOs, vertices data & copy into the array buffer


	createLaser(-4+3.5,-0.2, -3+3.5, -0.2, -3+3.5, 0.2, -4+3.5, 0.2,laserObjects["laser"].name);

	createRectangle(-2,-4,-1,-4,-1,-2,-2,-2,bucketObjects["bucket1"].name);
	createRectangle(0,-4,1,-4,1,-2,0,-2,bucketObjects["bucket2"].name);
	createLine(-0.45,3.9,0.45,2.9,"mirror1"); //the mirror is yet not appearing on the screen, has to work upon that.
	createLine(-0.45,-1.5588,0.45,0,"mirror2");


	createLine(-3.5,3.8,-3.3,3.8,"linea"); // for all the corresponding line segments, used the logic for seven-segment display.
	createLine(-3.5,3.8,-3.5,3.6,"lineb");
	createLine(-3.3,3.8,-3.3,3.6,"linec");
	createLine(-3.5,3.6,-3.3,3.6,"lined");
	createLine(-3.5,3.6,-3.5,3.4,"linee");
	createLine(-3.5,3.4,-3.3,3.4,"linef");
	createLine(-3.3,3.6,-3.3,3.4,"lineg");

	createLine(-3.1,3.8,-2.9,3.8,"lineh");
	createLine(-3.1,3.8,-3.1,3.6,"linei");
	createLine(-2.9,3.8,-2.9,3.6,"linej");
	createLine(-3.1,3.6,-2.9,3.6,"linek");
	createLine(-3.1,3.6,-3.1,3.4,"linel");
	createLine(-3.1,3.4,-2.9,3.4,"linem");
	createLine(-2.9,3.6,-2.9,3.4,"linen");

	createLine(-3.9,3.6,-3.5,3.6,"linez"); //for the neg sign
	//createLine(
	//	createRectangle(-3, -0.1,-2,-0.1, -2, 0.1, -3, 0.1, "beamy");

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );

	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (90/255.0f,90/255.0f,90/255.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1200;
	int height = 800;

	GLFWwindow* window = initGLFW(width, height);
	laserObjects["laser"].x1=-4;
	laserObjects["laser"].x2=-3;
	laserObjects["laser"].x3=-3;
	laserObjects["laser"].x4=-4;
	laserObjects["laser"].y1=-0.2;
	laserObjects["laser"].y2=-0.2;
	laserObjects["laser"].y3=0.2;
	laserObjects["laser"].y4=0.2;

	bucketObjects["bucket1"].x1=-2;
	bucketObjects["bucket1"].x2=-1;
	bucketObjects["bucket1"].x3=-1;
	bucketObjects["bucket1"].x4=-2;
	bucketObjects["bucket1"].y1=-4;
	bucketObjects["bucket1"].y2=-4;
	bucketObjects["bucket1"].y3=-2;
	bucketObjects["bucket1"].y4=-2;

	bucketObjects["bucket2"].x1=0;
	bucketObjects["bucket2"].x2=1;
	bucketObjects["bucket2"].x3=1;
	bucketObjects["bucket2"].x4=0;
	bucketObjects["bucket2"].y1=-4;
	bucketObjects["bucket2"].y2=-4;
	bucketObjects["bucket2"].y3=-2;
	bucketObjects["bucket2"].y4=-2;

	initGL (window, width, height);
	audio_init();
	double last_update_time = glfwGetTime(), current_time;


	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();
		audio_play();
		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}

		if(grandScore <= -10)
		{glfwTerminate();exit(EXIT_SUCCESS);}

		//	(sleep);
	}

	audio_close();
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
