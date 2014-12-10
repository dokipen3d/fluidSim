#include "GLShaderObject.h"
#include <iostream>

using namespace std;

const char* GLShaderObject::openGLErrorString( GLenum _errorCode )
{
    // Only 3.2+ Core and ES 2.0+ errors, no deprecated strings like stack underflow etc.
    if      (_errorCode == GL_INVALID_ENUM)                  { return "GL_INVALID_ENUM"; }
    else if (_errorCode == GL_INVALID_VALUE)                 { return "GL_INVALID_VALUE"; }
    else if (_errorCode == GL_INVALID_OPERATION)             { return "GL_INVALID_OPERATION"; }
    else if (_errorCode == GL_INVALID_FRAMEBUFFER_OPERATION) { return "GL_INVALID_FRAMEBUFFER_OPERATION"; }
    else if (_errorCode == GL_OUT_OF_MEMORY)                 { return "GL_OUT_OF_MEMORY"; }
    else if (_errorCode == GL_NO_ERROR)                      { return "GL_NO_ERROR"; }
    else {
        return "unknown error";
    }
}

void GLShaderObject::CheckGLError(std::string str)
{
    error = glGetError();

    if( error != GL_NO_ERROR )
    {

        printf( "Error! %s %s\n", str.c_str(), openGLErrorString( error ) );
    }
}

GLShaderObject::GLShaderObject()
{
    mProgramID = 0;
    //cout << "in shader constructor" << endl;
    
}

GLShaderObject::~GLShaderObject()
{
    freeProgram();
    //cout << "in shader destructor" << endl;

}


bool GLShaderObject::loadProgram(const char *vertex_path, const char *fragment_path)
{
    //Success flag
    GLint programSuccess = GL_TRUE;

    std::string vertShaderStr = readFile(vertex_path);
    const char *vertShaderSrc = vertShaderStr.c_str();

    std::string fragShaderStr = readFile(fragment_path);
    const char *fragShaderSrc = fragShaderStr.c_str();

    



    //Generate program
    mProgramID = glCreateProgram();

    

    //Create vertex shader
    GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
    //Get vertex source
    
    //Set vertex source
    glShaderSource( vertexShader, 1, &vertShaderSrc, NULL );

    //Compile vertex source
    glCompileShader( vertexShader );
    //Check vertex shader for errors
    GLint vShaderCompiled = GL_FALSE;
    glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &vShaderCompiled );
    if( vShaderCompiled != GL_TRUE )
    {
        printf( "Unable to compile vertex shader %d!\n", vertexShader );
        printShaderLog( vertexShader );
        return false;
    }

    //Attach vertex shader to program
    glAttachShader( mProgramID, vertexShader );


    //Create fragment shader
    GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );


    //Get fragment source
    
    //Set fragment source
    glShaderSource( fragmentShader, 1, &fragShaderSrc, NULL );
    //Compile fragment source
    glCompileShader( fragmentShader );
    //Check fragment shader for errors
    GLint fShaderCompiled = GL_FALSE;
    glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled );
    if( fShaderCompiled != GL_TRUE )
    {
        printf( "Unable to compile fragment shader %d!\n", fragmentShader );
        printShaderLog( fragmentShader ); return false;
    }

    //Attach fragment shader to program
    glAttachShader( mProgramID, fragmentShader );



    //Link program
    glLinkProgram( mProgramID );

    

    //Check for errors
    glGetProgramiv( mProgramID, GL_LINK_STATUS, &programSuccess );
    if( programSuccess != GL_TRUE )
    {
        printf( "Error linking program %d!\n", mProgramID );
        printProgramLog( mProgramID );
        return false;
    }



    return true;


}

GLint GLShaderObject::getAttributeLocation(std::string name)
{
    return glGetAttribLocation(mProgramID, name.c_str());
    CheckGLError("oi");
}



void GLShaderObject::freeProgram()
{

    glDeleteProgram(mProgramID);

}

void GLShaderObject::setTextureUnits()
{

    tex1UniformLoc = glGetUniformLocation(mProgramID, "texBack");
    tex2UniformLoc = glGetUniformLocation(mProgramID, "texFront");
    glUniform1i(tex1UniformLoc, 0);
    glUniform1i(tex2UniformLoc, 1);



}

bool GLShaderObject::bind()
{
    error = GL_NO_ERROR;
    //Use shader
    //cout << "about to use program " << endl;
    glUseProgram( mProgramID );
    //cout << "using program " << endl;

    //Check for error
    error = glGetError();
    if( error != GL_NO_ERROR )
    {
        printf( "Error binding shader! %s\n", openGLErrorString( error ) );
        //printf( "Error binding shader! \n" );

        printProgramLog( mProgramID );
        return false;
    }

    //setTextureUnits();

    error = glGetError();
    if( error != GL_NO_ERROR )
    {
        //printf( "Error setting texture units! %s\n", gluErrorString( error ) );
        printf( "Error setting texture units! \n" );

        printProgramLog( mProgramID );
        return false;
    }
    //else printf( "setting texture units! %s\n", gluErrorString( error ) );


    return true;

}

void GLShaderObject::unbind()
{
    //Use default program
    glUseProgram( 0 );
}

GLuint GLShaderObject::getProgramID()
{
    return mProgramID;
}





void GLShaderObject::printProgramLog( GLuint program)
{
    //Make sure name is shader
    if( glIsProgram( program ) )
    {
        //Program log length
        int infoLogLength = 0;
        int maxLength = infoLogLength;

        //Get info string length
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );

        //Allocate string
        char* infoLog = new char[ maxLength ];

        //Get info log
        glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
        if( infoLogLength > 0 )
        {
            //Print Log
            printf( "%s\n", infoLog );
        }

        //Deallocate string
        delete[] infoLog;
    }

    else { printf( "Name %d is not a program\n", program ); }
}



void GLShaderObject::printShaderLog(GLuint shader)
{
    //Make sure name is shader
    if( glIsShader( shader ) )
    {
        //Shader log length
        int infoLogLength = 0;
        int maxLength = infoLogLength;

        //Get info string length
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );

        //Allocate string
        char* infoLog = new char[ maxLength ];

        //Get info log
        glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
        if( infoLogLength > 0 )
        {
            //Print Log
            printf( "%s\n", infoLog );
        }

        //Deallocate string
        delete[] infoLog;
    }

    else
    {
        printf( "Name %d is not a shader\n", shader );
    }


}


std::string GLShaderObject::readFile(const char *filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);
    
    if(!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }
    
    std::string line = "";
    while(!fileStream.eof()) {
        std::getline(fileStream, line);
        //content.append(line + "n");
        content.append(line + "\n");

    }
    
    fileStream.close();
    return content;
}














