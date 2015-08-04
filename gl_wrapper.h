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
    
    class frame_buffer;
    class render_target;
    class texture;
    class attrib_buffer;
    
    class program;
    
    
    
    
    
    class texture
    {
        idtype tex_id;
    public:
        enum tex_type {rgba8=0, rgb8, rgb32f};
        
        
        
        texture()
        {
            glGenTextures(1, &tex_id);
        }
        
        void bind()
        {
            glBindTexture(GL_TEXTURE_2D, tex_id);
        }
        
        void make_storage(unsigned w, unsigned h, tex_type t, int level = 0, const char * data = 0);
        
        ~texture()
        {
            if(tex_id)
                glDeleteTextures(1, &tex_id);
        }
        friend class frame_buffer;
    };
    
    
    struct dynamic_tex_type_traits
    {
        int internal_format;
        int format;
        int type;
    };
    
    template<const texture::tex_type t>
    struct tex_type_traits
    {
        enum i_f { internal_format = GL_RGBA4 };
        enum fmt { format = GL_RGBA };
        enum f_t { type = GL_BYTE };
        
        constexpr static  dynamic_tex_type_traits dynamic{internal_format, format, type};
    };
    template <>
    struct tex_type_traits <texture::rgba8>
    {
        enum i_f { internal_format = GL_RGBA8_OES };
        enum fmt { format = GL_RGBA };
        enum f_t { type = GL_BYTE };
        constexpr static  dynamic_tex_type_traits dynamic{internal_format, format, type};
    };
    template<>
    struct tex_type_traits<texture::rgb8>
    {
        enum i_f { internal_format = GL_RGBA8_OES };
        enum fmt { format = GL_RGBA };
        enum f_t { type = GL_BYTE };
        constexpr static  dynamic_tex_type_traits dynamic{internal_format, format, type};
    };
    template<>
    struct tex_type_traits<texture::rgb32f>
    {
        enum i_f { internal_format = GL_RGB32F_EXT };
        enum fmt { format = GL_RGBA };
        enum f_t { type = GL_FLOAT };
        constexpr static  dynamic_tex_type_traits dynamic{internal_format, format, type};
    };
    
     inline void texture::make_storage(unsigned w, unsigned h, tex_type t, int level, const char * data)
     {
         static dynamic_tex_type_traits dynamics[]={
             tex_type_traits<rgba8>::dynamic,
             tex_type_traits<rgb8>::dynamic,
             tex_type_traits<rgb32f>::dynamic
         };
         
         
         glTexImage2D(GL_TEXTURE_2D, level, dynamics[t].internal_format, w, h, 0, dynamics[t].format, dynamics[t].type, data);
     }
    
    class render_target
    {
        idtype rt_id;
    public:
        render_target()
        {
            glGenRenderbuffers(1, &rt_id);
            
        }
        
        void make_storage(unsigned w, unsigned h, idtype kind)
        {
            glRenderbufferStorage(GL_RENDERBUFFER, kind, w, h);
        }
        
        
        ~render_target()
        {
            if(rt_id)
                glDeleteRenderbuffers(1, &rt_id);
        }
        friend class frame_buffer;
    };
    
    class frame_buffer
    {
        idtype fb_id;
    public:
        enum attachment_place{color=GL_COLOR_ATTACHMENT0, depth=GL_DEPTH_ATTACHMENT, stencil=GL_STENCIL_ATTACHMENT};
        frame_buffer()
        {
            glGenFramebuffers(1, &fb_id);
        }
        ~frame_buffer()
        {
            if(fb_id)
                glDeleteFramebuffers(1, &fb_id);
        }
        
        void bind()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
        }
       
        void attach(const render_target& rt,attachment_place pl)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, pl, GL_RENDERBUFFER, rt.rt_id);
        }
        
       
        void attach(const texture& t,attachment_place pl, unsigned level=0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
            glFramebufferTexture2D(GL_FRAMEBUFFER, pl, GL_TEXTURE_2D, t.tex_id, level);
        }
        
        bool status()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
            return status == GL_FRAMEBUFFER_COMPLETE;
        }
    };
    
    

    
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
