#include <functional>
#include <iostream>
#include <string>

#include <emscripten.h>
#include <SDL.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>

// Shader sources
const GLchar* vertexSource =
    "attribute vec4 position;                     \n"
    "void main()                                  \n"
    "{                                            \n"
    "  gl_Position = vec4(position.xyz, 1.0);     \n"
    "}                                            \n";
const GLchar* fragmentSource =
    "precision mediump float;\n"
    "void main()                                  \n"
    "{                                            \n"
    "  gl_FragColor[0] = gl_FragCoord.x/960.0;    \n"
    "  gl_FragColor[1] = gl_FragCoord.y/900.0;    \n"
    "  gl_FragColor[2] = 0.5;                     \n"
    "}                                            \n";

// an example of something we will control from the javascript side
bool background_is_black = true;

// the function called by the javascript code
extern "C" void EMSCRIPTEN_KEEPALIVE toggle_background_color() { background_is_black = !background_is_black; }

std::function<void()> loop;
void main_loop() { loop(); }

std::string QueryCapability() {
  return "GL_VENDOR = " +
         std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))) +
         "\nGL_RENDERER = " +
         std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER))) +
         "\nGL_VERSION = " +
         std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))) +
         "\nGL_EXTENSIONS = " +
         std::string(
             reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS))) +
         "\n";
}

int main()
{
    SDL_Window *window = nullptr;
    SDL_CreateWindowAndRenderer(1024, 768, 0, &window, nullptr);

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    std::cout << "context = " << QueryCapability() << "\n";

    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {
        1.0f, 1.0f, // first
        1.0f, -1.0f, // second
        0.0f, -1.0f,

        // second triangle
        0.0f, 0.5f,
        0.0f, -0.8f,
        -0.5f, 1.0f,
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    loop = [&]
    {
        // move a vertex
        const uint32_t milliseconds_since_start = SDL_GetTicks();
        const uint32_t milliseconds_per_loop = 3000;
        float transit_factor = milliseconds_since_start % milliseconds_per_loop / float(milliseconds_per_loop);
        vertices[0] = ( milliseconds_since_start % milliseconds_per_loop ) / float(milliseconds_per_loop) * 2 - 1.0f;
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Clear the screen
        if( background_is_black )
            glClearColor(transit_factor, transit_factor * transit_factor * 2.3f, 0.0f, 1.0f);
        else
            glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a triangle from the 3 vertices
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SDL_GL_SwapWindow(window);
    };

    emscripten_set_main_loop(main_loop, 0, true);

    return EXIT_SUCCESS;
}
