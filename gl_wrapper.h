//
//  gl_wrapper.h
//  floppy gl wrapper
//
//  Created by Mateusz Bugaj on 17.04.2015.
//  Copyright (c) 2015 Mateusz Bugaj. All rights reserved.
//

#ifndef __floppy__gl_wrapper__
#define __floppy__gl_wrapper__

#include  <OpenGLES/ES2/glext.h>

#include <vector>
#include <string>
namespace gl {
    typedef GLuint idtype;
    enum shader_enum{vertex=GL_VERTEX_SHADER, fragment=GL_FRAGMENT_SHADER} ;
    
    class render_target;
    class texture;
    class attrib_buffer;
    
    class program;
    
    template<const shader_enum type>
    class shader
    {
        idtype shader_id;
        friend class program;
    public:
        enum shader_t {shader_type=type};
        shader()
        {
            shader_id = glCreateShader(type);
        }
        
        void source(const char * src)
        {
            glShaderSource(shader_id, 1, &src, 0);
            glCompileShader(shader_id);
        }
        std::pair<bool, std::string> status()
        {
            std::string retLog;
            GLint logLength;
            glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                auto logAlloc=std::vector<GLchar>(logLength);
                GLchar *log = logAlloc.data();
                glGetShaderInfoLog(shader_id, logLength, &logLength, log);
                retLog=logAlloc.data();
            }
            
            GLint status;
            glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
            if (status == GL_FALSE) {
                glDeleteShader(shader_id);
                shader_id=0;
            }
            
            return std::make_pair(shader_id!=0, retLog);
        }
        
        ~shader()
        {
            if(shader_id)
                glDeleteShader(shader_id);
        }
        
    };
    class program
    {
        idtype program_id;
    public:
        program()
        {
            program_id = glCreateProgram();
        }
        template<const shader_enum type>
        void attach(const shader<type>& sh)
        {
            glAttachShader(sh.shader_id, type);
        }
        
        template<const shader_enum type>
        void detach(const shader<type>& sh)
        {
            glDetachShader(program_id,sh.shader_id);
        }
        
        
        
        
        void bind_attrib_name(const char * name, idtype nr)
        {
            glBindAttribLocation(program_id, nr, name);
        }
        
        void link()
        {
            glLinkProgram(program_id);
        }
        
        std::pair<bool, std::string> link_status()
        {
            std::string retLog;
            
            GLint logLength;
            glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                auto logAlloc=std::vector<GLchar>(size_t(logLength));
                GLchar *log = logAlloc.data();
                glGetProgramInfoLog(program_id, logLength, &logLength, log);
                retLog=logAlloc.data();
            }
            
            GLint status;
            glGetProgramiv(program_id, GL_LINK_STATUS, &status);
            if (status == GL_FALSE) {
                glDeleteProgram(program_id);
                program_id = 0;
            }
            
            return std::make_pair(program_id!=0, retLog);
        }
        
        
        void validate()
        {
            glValidateProgram(program_id);
        }
        
        std::pair<bool, std::string> validate_status()
        {
            std::string retLog;
            
            GLint logLength;
            glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                auto logAlloc=std::vector<GLchar>( size_t(logLength));
                GLchar *log = logAlloc.data();
                glGetProgramInfoLog(program_id, logLength, &logLength, log);
                retLog=logAlloc.data();
            }
            
            GLint status;
            glGetProgramiv(program_id, GL_VALIDATE_STATUS, &status);
            if (status == GL_FALSE) {
                glDeleteProgram(program_id);
                program_id = 0;
            }
            
            return std::make_pair(program_id!=0, retLog);
        }
        
        idtype uniform_loc(const char * name)
        {
            return glGetUniformLocation(program_id, name);
        }
        
        
    };
    
}
/*
 
 
 void init_a_program_and_deps()
 {
 unsigned program, vertex_shader, fragment_shader;
 
 program = glCreateProgram();
 const char * vertex_shader_src="", * fragment_shader_source="";
 
 {//compile
 //glShaderBinary(	GLsizei n,
 //   const GLuint *shaders,
 // GLenum binaryformat,
 //  const void *binary,
 //   GLsizei length);
 //   vertex_shader = glCreateShader(GL_VERTEX_SHADER);
 glShaderSource(vertex_shader, 1, &vertex_shader_src, 0);
 glCompileShader(vertex_shader);
 
 GLint logLength;
 glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logLength);
 if (logLength > 0) {
 GLchar *log = (GLchar *)new char[logLength];
 glGetShaderInfoLog(vertex_shader, logLength, &logLength, log);
 delete []log;
 }
 
 GLint status;
 glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
 if (status == GL_FALSE) {
 glDeleteShader(vertex_shader);
 }
 }
 
 glAttachShader(program, vertex_shader);
 
 // Attach fragment shader to program.
 glAttachShader(program, fragment_shader);
 
 // Bind attribute locations.
 // This needs to be done prior to linking.
 glBindAttribLocation(program, 0, "position");
 glBindAttribLocation(program, 1, "normal");
 
 {
 GLint status;
 glLinkProgram(program);
 
 GLint logLength;
 glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
 if (logLength > 0) {
 GLchar *log = (GLchar *)new char[logLength];
 glGetProgramInfoLog(program, logLength, &logLength, log);
 delete []log;
 }
 
 glGetProgramiv(program, GL_LINK_STATUS, &status);
 if (status == GL_FALSE) {
 if (vertex_shader) {
 glDeleteShader(vertex_shader);
 vertex_shader = 0;
 }
 if (program) {
 glDeleteProgram(program);
 program = 0;
 }
 }
 }
 
 // Get uniform locations.
 unsigned UNIFORM_MODELVIEWPROJECTION_MATRIX = glGetUniformLocation(program, "modelViewProjectionMatrix");
 unsigned UNIFORM_NORMAL_MATRIX = glGetUniformLocation(program, "normalMatrix");
 
 // Release vertex and fragment shaders.
 if (vertex_shader) {
 glDetachShader(program, vertex_shader);
 glDeleteShader(vertex_shader);
 }
 
 {
 GLint logLength, status;
 
 glValidateProgram(program);
 glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
 if (logLength > 0) {
 GLchar *log = (GLchar *)new char[logLength];
 glGetProgramInfoLog(program, logLength, &logLength, log);
 
 delete [](log);
 }
 
 glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
 if (status == GL_FALSE) {
 }
 }
 
 //alternative
 {
 const GLchar *vertexSourceText = " ... vertex shader GLSL source code ... ";
 const GLchar *fragmentSourceText = " ... fragment shader GLSL source code ... ";
 
 // Compile and link the separate vertex shader program, then read its uniform variable locations
 unsigned _vertexProgram = glCreateShaderProgramvEXT(GL_VERTEX_SHADER, 1, &vertexSourceText);
 unsigned _uniformModelViewProjectionMatrix = glGetUniformLocation(_vertexProgram, "modelViewProjectionMatrix");
 unsigned _uniformNormalMatrix = glGetUniformLocation(_vertexProgram, "normalMatrix");
 
 // Compile and link the separate fragment shader program (which uses no uniform variables)
 unsigned _fragmentProgram =  glCreateShaderProgramvEXT(GL_FRAGMENT_SHADER, 1, &fragmentSourceText);
 unsigned _ppo;
 // Construct a program pipeline object and configure it to use the shaders
 glGenProgramPipelinesEXT(1, &_ppo);
 glBindProgramPipelineEXT(_ppo);
 glUseProgramStagesEXT(_ppo, GL_VERTEX_SHADER_BIT_EXT, _vertexProgram);
 glUseProgramStagesEXT(_ppo, GL_FRAGMENT_SHADER_BIT_EXT, _fragmentProgram);
 }
 
 }*/



#endif /* defined(__floppy__gl_wrapper__) */
