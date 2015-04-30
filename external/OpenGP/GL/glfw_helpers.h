// Copyright (C) 2014 - Andrea Tagliasacchi
#include "check_error_gl.h"

#pragma once
#include "GL/glew.h" ///< must be included before GLFW
#include "GL/glfw.h"
#include "shader_helpers.h"

/// Convenience constants
static const int ONE = 1;
static const bool DONT_NORMALIZE = false;
static const bool DONT_TRANSPOSE = false;
static const int ZERO_STRIDE = 0;
static const void* ZERO_BUFFER_OFFSET = 0;

namespace opengp{

static int _width = 640;
static int _height = 480;
static void (*_display)(void) = NULL;

inline void glfwInitWindowSize(int width, int height){
    _width = width;
    _height = height;
}

inline int glfwCreateWindow(const char* title = NULL){
    // GLFW Initialization
    if( !glfwInit() ){
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return EXIT_FAILURE;
    }    
    
    /// Hint GLFW that we would like an OpenGL 4.0 context (at least)
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 4);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    /// Hint for multisampling
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
    
    /// Attempt to open the window: fails if required version unavailable
    /// @note Intel GPUs do not support OpenGL 3.0
    if( !glfwOpenWindow(_width, _height, 0,0,0,0, 32,0, GLFW_WINDOW ) ){
        fprintf( stderr, "Failed to open OpenGL 4 GLFW window.\n" );
        glfwTerminate();
        return EXIT_FAILURE;
    }

    /// Outputs the OpenGL version & set title
    int major, minor, revision;
    glfwGetGLVersion(&major, &minor, &revision);
    if(title == NULL){
        std::ostringstream strout;
        strout << "GLFW OpenGL " << major << "." << minor << "." << revision << std::endl;    
        std::string _title = strout.str();
        glfwSetWindowTitle(_title.c_str());
    } else {
        glfwSetWindowTitle(title);
    }
    
    // GLEW Initialization (must have a context)
    glewExperimental = true;
    if( glewInit() != GLEW_NO_ERROR ){
        
        fprintf( stderr, "Failed to initialize GLEW\n"); 
        return EXIT_FAILURE;
    }
    
    ///--- get rid of GL_INVALID_ENUM error generated by GLFW
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        if(error!=GL_INVALID_ENUM){
            fprintf(stderr, "!!!ERROR: glfwCreateWindow failed");
            exit(0);
        }
    }
    check_error_gl();          
    
    return EXIT_SUCCESS;
}


/// @see glutDisplayFunc
inline void glfwDisplayFunc(void (*display)(void)){    
    _display = display;
}

/// @see glutMainLoop
inline void glfwMainLoop(){
    assert(_display!=NULL);
    
    /// Render loop & keyboard input
    while(glfwGetKey(GLFW_KEY_ESC)!=GLFW_PRESS && glfwGetWindowParam(GLFW_OPENED)){
        _display();
        glfwSwapBuffers();
    }
    
    /// Close OpenGL window and terminate GLFW
    glfwTerminate();
}

/// @todo document texture loading 
inline GLuint load_texture_targa(const std::string& path){
    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(ONE, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Read the file, call glTexImage2D with the right parameters
    glfwLoadTexture2D(path.c_str(), 0);

    // Nice trilinear filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Return the ID of the texture we just created
    return textureID;
}


/// @see http://r3dux.org/2012/07/a-simple-glfw-fps-counter/
inline float update_title_fps(std::string theWindowTitle = "NONE", float theTimeInterval = 1.0)
{
    // Static values which only get initialised the first time the function runs
    static float t0Value       = glfwGetTime(); // Set the initial time to now
    static int    fpsFrameCount = 0;             // Set the initial FPS frame count to 0
    static float fps           = 0.0;           // Set the initial FPS value to 0.0
    
    // Get the current time in seconds since the program started (non-static, so executed every time)
    float currentTime = glfwGetTime();
    
    // Ensure the time interval between FPS checks is sane (low cap = 0.1s, high-cap = 10.0s)
    // Negative numbers are invalid, 10 fps checks per second at most, 1 every 10 secs at least.
    if (theTimeInterval < 0.1)
    {
        theTimeInterval = 0.1;
    }
    if (theTimeInterval > 10.0)
    {
        theTimeInterval = 10.0;
    }
    
    // Calculate and display the FPS every specified time interval
    if ((currentTime - t0Value) > theTimeInterval)
    {
        // Calculate the FPS as the number of frames divided by the interval in seconds
        fps = (float)fpsFrameCount / (currentTime - t0Value);
        
        // If the user specified a window title to append the FPS value to...
        if (theWindowTitle != "NONE")
        {
            // Convert the fps value into a string using an output stringstream
            std::ostringstream stream;
            stream << fps;
            std::string fpsString = stream.str();
            
            // Append the FPS value to the window title details
            theWindowTitle += " | FPS: " + fpsString;
            
            // Convert the new window title to a c_str and set it
            const char* pszConstString = theWindowTitle.c_str();
            glfwSetWindowTitle(pszConstString);
        }
        else // If the user didn't specify a window to append the FPS to then output the FPS to the console
        {
            std::cout << "FPS: " << fps << std::endl;
        }
        
        // Reset the FPS frame counter and set the initial time to be now
        fpsFrameCount = 0;
        t0Value = glfwGetTime();
    }
    else // FPS calculation time interval hasn't elapsed yet? Simply increment the FPS frame counter
    {
        fpsFrameCount++;
    }
    
    // Return the current FPS - doesn't have to be used if you don't want it!
    return fps;
}

} // opengp::
