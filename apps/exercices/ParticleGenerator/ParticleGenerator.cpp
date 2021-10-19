//#############################################################################
//  File:      ParticleGenerator.cpp
//  Purpose:   Core profile OpenGL application with a colored cube with
//             GLFW as the OS GUI interface (http://www.glfw.org/).
//  Date:      October 2021
//  Authors:   Affolter Marc
//  License:   This software is provided under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <GL/gl3w.h>    // OpenGL headers
#include <GLFW/glfw3.h> // GLFW GUI library
#include <SLMat4.h>     // 4x4 matrix class
#include <SLVec3.h>     // 3D vector class
#include <glUtils.h>    // Basics for OpenGL shaders, buffers & textures

//-----------------------------------------------------------------------------
//! Struct definition for vertex attribute position and color
struct VertexPC
{
    SLVec4f p; // vertex position [x,y,z]
    SLCol4f c; // vertex color [r,g,b]
    SLVec2f t; // vertex texture coord. [s,t]

    // Setter method
    void set(float posX,
             float posY,
             float posZ,
             float posW,
             float colorR,
             float colorG,
             float colorB,
             float colorA,
             float textureX,
             float textureY)
    {
        p.set(posX, posY, posZ, posW);
        c.set(colorR, colorG, colorB, colorA);
    }
};

//! Struct definition for vertex attributes
struct VertexPNT
{
    SLVec3f p; // vertex position [x,y,z]
    SLVec3f n; // vertex normal [x,y,z]
    SLVec2f t; // vertex texture coord. [s,t]
};
//! Struct definition for particle attribute position, velocity, color and life
struct Particle
{
    SLVec3f p; // particle position [x,y,z]
    SLVec3f v; // particle velocity [x,y,z]
    SLCol4f c; // particle color [r,g,b,a]
    float s; // particle scale 
    float life;

    Particle()
      : p(0.0f), v(0.0f), c(1.0f),s(1.0f), life(0.0f) {}
};
//-----------------------------------------------------------------------------
// GLobal application variables
static GLFWwindow* window;       //!< The global GLFW window handle
static SLstring    _projectRoot; //!< Directory of executable
static SLint       _scrWidth;    //!< Window width at start up
static SLint       _scrHeight;   //!< Window height at start up

static SLMat4f _viewMatrix;       //!< 4x4 view matrix
static SLMat4f _modelMatrix;      //!< 4x4 model matrix
static SLMat4f _projectionMatrix; //!< 4x4 projection matrix

static GLuint _vao  = 0; //!< ID of the vertex array object
static GLuint _vboV = 0; //!< ID of the VBO for vertex attributes
static GLuint _vboI = 0; //!< ID of the VBO for vertex index array

static GLuint _numV = 0; //!< NO. of vertices
static GLuint _numI = 0; //!< NO. of vertex indexes for triangles

vector<Particle> particles;  //!< List of particles
const int AMOUNT = 500; //!< Amount of particles

static float  _camZ;                   //!< z-distance of camera
static float  _rotX, _rotY;            //!< rotation angles around x & y axis
static int    _deltaX, _deltaY;        //!< delta mouse motion
static int    _startX, _startY;        //!< x,y mouse start positions
static int    _mouseX, _mouseY;        //!< current mouse position
static bool   _mouseLeftDown;          //!< Flag if mouse is down
static GLuint _modifiers = 0;          //!< modifier bit flags
const GLuint  NONE       = 0;          //!< constant for no modifier
const GLuint  SHIFT      = 0x00200000; //!< constant for shift key modifier
const GLuint  CTRL       = 0x00400000; //!< constant for control key modifier
const GLuint  ALT        = 0x00800000; //!< constant for alt key modifier

static GLuint _shaderVertID = 0; //! vertex shader id
static GLuint _shaderFragID = 0; //! fragment shader id
static GLuint _shaderProgID = 0; //! shader program id
static GLuint _textureID    = 0; //!< texture id

// Attribute & uniform variable location indexes
static GLint _pLoc;   //!< attribute location for vertex position
static GLint _nLoc;   //!< attribute location for vertex normal
static GLint _cLoc;   //!< attribute location for vertex color
static GLint _oLoc;   //!< attribute location for vertex offset
static GLint _sLoc;   //!< attribute location for vertex offset
static GLint _tLoc;   //!< attribute location for vertex texture coord
static GLint _gLoc;   //!< uniform location for gamma value
static GLint _mvpLoc; //!< uniform location for modelview-projection matrix

static GLint _texture0Loc; //!< uniform location for texture 0

//-----------------------------------------------------------------------------
void buildBox()
{
    // create C arrays on heap
    // Define the vertex position and colors as an array of structure
    // We define the colors with the same components as the cubes corners.
    _numV              = 4;
    VertexPC* vertices = new VertexPC[_numV];
    vertices[0].set(1, 1, 1, 1, 1, 1, 1, 1, 0, 1); //LT
    vertices[1].set(1, 0, 1, 1, 1, 0, 1, 1, 0, 0); //LB
    vertices[2].set(0, 0, 1, 1, 0, 0, 1, 1, 1, 0); //RB
    vertices[3].set(0, 1, 1, 1, 0, 1, 1, 1, 1, 1); //RT

    // Define the triangle indexes of the cubes vertices
    _numI           = 6;
    GLuint* indices = new GLuint[_numI];
    int     n       = 0;
    indices[n++]    = 0;
    indices[n++]    = 2;
    indices[n++]    = 1;
    indices[n++]    = 0;
    indices[n++]    = 3;
    indices[n++]    = 2; // Near

    // Generate the OpenGL vertex array object
    glUtils::buildVAO(_vao,
                      _vboV,
                      _vboI,
                      vertices,
                      (GLint)_numV,
                      sizeof(VertexPC),
                      indices,
                      (GLint)_numI,
                      sizeof(GL_UNSIGNED_INT),
                      (GLint)_shaderProgID,
                      _pLoc,
                      _cLoc,
                      -1,
                      _tLoc);

    // delete data on heap. The VBOs are now on the GPU
    delete[] vertices;
    delete[] indices;
}
void buildSquare()
{
    // create vertex array for interleaved position, normal and texCoord
    //                  Position,  Normal, texCrd,
    _numV = 4;

    // clang-format off
    float vertices[] = {1, 0, 1, 0,-1, 0, 1, 0, // Vertex 0
                        1, 1, 1, 0,-1, 0, 1, 1, // Vertex 1
                        0, 1, 1, 0,-1, 0, 0, 1, // Vertex 2
                        0, 0, 1, 0,-1, 0, 0, 0}; // Vertex 3
    // clang-format on
    // create index array for GL_TRIANGLES
    _numI            = 6;
    GLuint indices[] = {0, 1, 2, 0, 2, 3};

    // Generate the OpenGL vertex array object
    glUtils::buildVAO(_vao,
                      _vboV,
                      _vboI,
                      vertices,
                      (GLint)_numV,
                      sizeof(VertexPNT),
                      indices,
                      (GLint)_numI,
                      sizeof(GL_UNSIGNED_INT),
                      (GLint)_shaderProgID,
                      _pLoc,
                      _nLoc,
                      _tLoc);

    // The vertices and indices are on the stack memory and get deleted at the
    // end of the block.
}
void initParticles()
{
    // create vertex array for interleaved position, normal and texCoord
    //                  Position,  Normal, texCrd,
    _numV = 4;

    // clang-format off
    float vertices[] = {1, 0, 1, 0,-1, 0, 1, 0, // Vertex 0
                        1, 1, 1, 0,-1, 0, 1, 1, // Vertex 1
                        0, 1, 1, 0,-1, 0, 0, 1, // Vertex 2
                        0, 0, 1, 0,-1, 0, 0, 0}; // Vertex 3
    // clang-format on
    // create index array for GL_TRIANGLES
    _numI            = 6;
    GLuint indices[] = {0, 1, 2, 0, 2, 3};

    // Generate the OpenGL vertex array object
    glUtils::buildVAO(_vao,
                      _vboV,
                      _vboI,
                      vertices,
                      (GLint)_numV,
                      sizeof(VertexPNT),
                      indices,
                      (GLint)_numI,
                      sizeof(GL_UNSIGNED_INT),
                      (GLint)_shaderProgID,
                      _pLoc,
                      _nLoc,
                      _tLoc);

    // create this->amount default particle instances
    for(unsigned int i = 0; i < AMOUNT; ++i)
    {
        particles.push_back(Particle());
    }
}
//-----------------------------------------------------------------------------
/*!
onInit initializes the global variables and builds the shader program. It
should be called after a window with a valid OpenGL context is present.
*/
void onInit()
{
    // backwards movement of the camera
    _camZ = -3;

    // Mouse rotation parameters
    _rotX = _rotY = 0;
    _deltaX = _deltaY = 0;
    _mouseLeftDown    = false;

    // Load textures
    _textureID = glUtils::buildTexture(_projectRoot + "/data/images/textures/woodcontainer.jpg");

    // Load, compile & link shaders
    _shaderVertID = glUtils::buildShader(_projectRoot + "/data/shaders/Particle.vert", GL_VERTEX_SHADER);
    _shaderFragID = glUtils::buildShader(_projectRoot + "/data/shaders/Particle.frag", GL_FRAGMENT_SHADER);
    _shaderProgID = glUtils::buildProgram(_shaderVertID, _shaderFragID);

    // Activate the shader program
    glUseProgram(_shaderProgID);

    // Get the variable locations (identifiers) within the program
    _pLoc   = glGetAttribLocation(_shaderProgID, "a_position");
    _nLoc   = glGetAttribLocation(_shaderProgID, "a_normal");
    _tLoc   = glGetAttribLocation(_shaderProgID, "a_texCoord");
    _gLoc   = glGetUniformLocation(_shaderProgID, "u_oneOverGamma");
    _mvpLoc = glGetUniformLocation(_shaderProgID, "u_mvpMatrix");
    _cLoc        = glGetUniformLocation(_shaderProgID, "color");
    _oLoc        = glGetUniformLocation(_shaderProgID, "offset");
    _sLoc        = glGetUniformLocation(_shaderProgID, "scale");
    _texture0Loc = glGetUniformLocation(_shaderProgID, "u_matTextureDiffuse0");

    //buildBox();
    //buildSquare();
    initParticles();
    /*particles[0].life = 2.0f;
    particles[0].c.r = 1.0f;
    particles[0].c.g = 1.0f;
    particles[0].c.b = 0.0f;
    particles[0].c.a = 1.0f;
    particles[0].p.x  = 0.0f;
    particles[0].p.y  = 0.5f;
    particles[0].p.z  = 0.0f;
    particles[1].life = 2.0f;*/

    glClearColor(0.5f, 0.5f, 0.5f, 1); // Set the background color
    glEnable(GL_DEPTH_TEST);           // Enables depth test
    glEnable(GL_CULL_FACE);            // Enables the culling of back faces
    GETGLERROR;
}

// stores the index of the last particle used (for quick access to next dead particle)
unsigned int lastUsedParticle = 0;
unsigned int firstUnusedParticle()
{
    // first search from last used particle, this will usually return almost instantly
    for (unsigned int i = lastUsedParticle; i < AMOUNT; ++i)
    {
        if (particles[i].life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }
    // otherwise, do a linear search
    for (unsigned int i = 0; i < lastUsedParticle; ++i)
    {
        if (particles[i].life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }
    // all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
    lastUsedParticle = 0;
    return 0;
}

void respawnParticle(Particle& particle, SLVec3f offset)
{
    float random      = (rand() % 100) / 100.0f;
    float rColor      = 0.5f + ((rand() % 100) / 100.0f);
    particle.p =  offset;
    particle.p.x += random;
    particle.c    = SLVec4f(rColor, rColor, rColor, 1.0f);
    particle.life     = 5.0f;
    particle.s = ((float)rand() / (RAND_MAX));
    particle.v = SLVec3f(0,0.2,0);
}
void spawnParticles(unsigned int newParticles, SLVec3f offset)
{
    // add new particles
    for (unsigned int i = 0; i < newParticles; ++i)
    {
        int unusedParticle = firstUnusedParticle();
        respawnParticle(particles[unusedParticle], offset);
    }
}

void updateParticles(float dt)
{
    // update all particles
    for (unsigned int i = 0; i < AMOUNT; ++i)
    {
        Particle& p = particles[i];
        p.life-= dt; // reduce life
        if (p.life > 0.0f)
        { // particle is alive, thus update
            p.p += p.v * dt;
            p.c.a -= dt * 2.5f;
        }
    }
}
//-----------------------------------------------------------------------------
/*!
onClose is called when the user closes the window and can be used for proper
deallocation of resources.
*/
void onClose(GLFWwindow* window)
{
    // Delete shaders & programs on GPU
    glDeleteShader(_shaderVertID);
    glDeleteShader(_shaderFragID);
    glDeleteProgram(_shaderProgID);

    // Delete arrays & buffers on GPU
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vboV);
    glDeleteBuffers(1, &_vboI);
}
//-----------------------------------------------------------------------------
/*!
onPaint does all the rendering for one frame from scratch with OpenGL.
*/
bool onPaint()
{
    //1) Clear the color & depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //2a) View transform: move the coordinate system away from the camera
    _viewMatrix.identity();
    _viewMatrix.translate(0, 0, _camZ);

    //2b) Model transform: rotate the coordinate system increasingly
    _viewMatrix.rotate(_rotX + _deltaX, 1, 0, 0);
    _viewMatrix.rotate(_rotY + _deltaY, 0, 1, 0);

    //3) Model transform: move the cube so that it rotates around its center
    _modelMatrix.identity();
    _modelMatrix.translate(-0.5f, -0.5f, -0.5f);

    //4) Build the combined modelview-projection matrix
    SLMat4f mvp(_projectionMatrix);
    SLMat4f mv(_viewMatrix);
    mv.multiply(_modelMatrix);
    mvp.multiply(mv);

    //6) Activate the shader program and pass the uniform variables to the shader
    glUseProgram(_shaderProgID);
    glUniformMatrix4fv(_mvpLoc, 1, 0, (float*)&mvp);
    glUniform1f(_gLoc, 1.0f);
    glUniform1i(_texture0Loc, 0);
    //glUniform4f(_cLoc,1,1,0,1);

    // Enable all of the vertex attribute arrays
    glEnableVertexAttribArray((GLuint)_pLoc);
    glEnableVertexAttribArray((GLuint)_nLoc);
    glEnableVertexAttribArray((GLuint)_tLoc);

    //7a) Activate the vertex array
    glBindVertexArray(_vao);

    //7b) Activate the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vboI);

    // Activate Texture
    glBindTexture(GL_TEXTURE_2D, _textureID);

    // For VBO only offset instead of data pointer
    GLsizei stride  = sizeof(VertexPNT);
    GLsizei offsetN = sizeof(SLVec3f);
    GLsizei offsetT = sizeof(SLVec3f) + sizeof(SLVec3f);
    glVertexAttribPointer((GLuint)_pLoc,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          stride,
                          nullptr);
    glVertexAttribPointer((GLuint)_nLoc,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          stride,
                          (void*)(size_t)offsetN);
    glVertexAttribPointer((GLuint)_tLoc,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          stride,
                          (void*)(size_t)offsetT);
    glUniform1f(_sLoc, 1.0f);
    // use additive blending to give it a 'glow' effect
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    int count = 0;
    for (Particle particle : particles)
    {
        count++;
        if (particle.life > 0.0f)
        {
            glUniform1f(_sLoc, particle.s);
            glUniform4f(_cLoc, particle.c.r, particle.c.g, particle.c.b, particle.c.a);
            glUniform3f(_oLoc, particle.p.x, particle.p.y, particle.p.z);
            //7c) Draw cube with triangles by indexes
            glDrawElements(GL_TRIANGLES, (GLint)_numI, GL_UNSIGNED_INT, nullptr);
            
        }
    }
    // don't forget to reset to default blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    

    // Disable the vertex arrays
    glDisableVertexAttribArray((GLuint)_pLoc);
    glDisableVertexAttribArray((GLuint)_nLoc);
    glDisableVertexAttribArray((GLuint)_tLoc);

    //8) Fast copy the back buffer to the front buffer. This is OS dependent.
    glfwSwapBuffers(window);
    GETGLERROR;

    // Return true to get an immediate refresh
    return true;
}
//-----------------------------------------------------------------------------
/*!
onResize: Event handler called on the resize event of the window. This event
should called once before the onPaint event. Do everything that is dependent on
the size and ratio of the window.
*/
void onResize(GLFWwindow* window, int width, int height)
{
    float w = (float)width;
    float h = (float)height;

    // define the projection matrix
    _projectionMatrix.perspective(50, w / h, 0.01f, 10.0f);

    // define the viewport
    glViewport(0, 0, width, height);

    GETGLERROR;

    onPaint();
}
//-----------------------------------------------------------------------------
/*!
Mouse button down & release eventhandler starts and end mouse rotation
*/
void onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    SLint x = _mouseX;
    SLint y = _mouseY;

    _mouseLeftDown = (action == GLFW_PRESS);
    if (_mouseLeftDown)
    {
        _startX = x;
        _startY = y;

        // Renders only the lines of a polygon during mouse moves
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        _rotX += _deltaX;
        _rotY += _deltaY;
        _deltaX = 0;
        _deltaY = 0;

        // Renders filled polygons
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}
//-----------------------------------------------------------------------------
/*!
Mouse move eventhandler tracks the mouse delta since touch down (_deltaX/_deltaY)
*/
void onMouseMove(GLFWwindow* window, double x, double y)
{
    _mouseX = (int)x;
    _mouseY = (int)y;

    if (_mouseLeftDown)
    {
        _deltaY = (int)x - _startX;
        _deltaX = (int)y - _startY;
        onPaint();
    }
}
//-----------------------------------------------------------------------------
/*!
Mouse wheel eventhandler that moves the camera forward or backwards
*/
void onMouseWheel(GLFWwindow* window, double xscroll, double yscroll)
{
    if (_modifiers == NONE)
    {
        _camZ += (SLfloat)Utils::sign(yscroll) * 0.1f;
        onPaint();
    }
}
//-----------------------------------------------------------------------------
/*!
Key action eventhandler handles key down & release events
*/
void onKey(GLFWwindow* window, int GLFWKey, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (GLFWKey)
        {
            case GLFW_KEY_ESCAPE:
                onClose(window);
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case GLFW_KEY_LEFT_SHIFT: _modifiers = _modifiers | SHIFT; break;
            case GLFW_KEY_RIGHT_SHIFT: _modifiers = _modifiers | SHIFT; break;
            case GLFW_KEY_LEFT_CONTROL: _modifiers = _modifiers | CTRL; break;
            case GLFW_KEY_RIGHT_CONTROL: _modifiers = _modifiers | CTRL; break;
            case GLFW_KEY_LEFT_ALT: _modifiers = _modifiers | ALT; break;
            case GLFW_KEY_RIGHT_ALT: _modifiers = _modifiers | ALT; break;
        }
    }
    else if (action == GLFW_RELEASE)
    {
        switch (GLFWKey)
        {
            case GLFW_KEY_LEFT_SHIFT: _modifiers = _modifiers ^ SHIFT; break;
            case GLFW_KEY_RIGHT_SHIFT: _modifiers = _modifiers ^ SHIFT; break;
            case GLFW_KEY_LEFT_CONTROL: _modifiers = _modifiers ^ CTRL; break;
            case GLFW_KEY_RIGHT_CONTROL: _modifiers = _modifiers ^ CTRL; break;
            case GLFW_KEY_LEFT_ALT: _modifiers = _modifiers ^ ALT; break;
            case GLFW_KEY_RIGHT_ALT: _modifiers = _modifiers ^ ALT; break;
        }
    }
}
//-----------------------------------------------------------------------------
/*!
Error callback handler for GLFW.
*/
void onGLFWError(int error, const char* description)
{
    fputs(description, stderr);
}

//-----------------------------------------------------------------------------
/*!
The C main procedure running the GLFW GUI application.
*/
int main(int argc, char* argv[])
{
    _projectRoot = SLstring(SL_PROJECT_ROOT);

    // Initialize the platform independent GUI-Library GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(onGLFWError);

    // Enable fullscreen anti aliasing with 4 samples
    glfwWindowHint(GLFW_SAMPLES, 4);

    //You can enable or restrict newer OpenGL context here (read the GLFW documentation)
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);
#else
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    _scrWidth  = 640;
    _scrHeight = 480;

    // Create the GLFW window
    window = glfwCreateWindow(_scrWidth,
                              _scrHeight,
                              "Color Cube",
                              nullptr,
                              nullptr);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Get the current GL context. After this you can call GL
    glfwMakeContextCurrent(window);

    // Init OpenGL access library gl3w
    if (gl3wInit() != 0)
    {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        exit(-1);
    }

    // Check errors before we start
    GETGLERROR;

    glUtils::printGLInfo();

    // Set number of monitor refreshes between 2 buffer swaps
    glfwSwapInterval(1);

    // Prepare all OpenGL stuff
    onInit();

    // Call resize once for correct projection
    onResize(window, _scrWidth, _scrHeight);

    // Set GLFW callback functions
    glfwSetKeyCallback(window, onKey);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onMouseMove);
    glfwSetScrollCallback(window, onMouseWheel);
    glfwSetWindowCloseCallback(window, onClose);

    // Event loop
    float lastFrame = glfwGetTime();
    float currentFrame = 0.0f;
    float deltaTime = 0.0f;
    float timingRespawn = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();
        deltaTime    = currentFrame - lastFrame;
        lastFrame    = currentFrame;
        timingRespawn += deltaTime;
        if (timingRespawn > 1) {
            spawnParticles(2, SLVec3f(0, 0.2, 0));
            timingRespawn = 0.0f;
        }
        updateParticles(deltaTime);

        // if no updated occurred wait for the next event (power saving)
        if (!onPaint())
            glfwWaitEvents();
        else
            glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(0);
}
//-----------------------------------------------------------------------------
