#pragma once

#define USING_MULTISAMPLE 1

class Framebuffer{
public:
   Framebuffer(){

   }
   
   virtual void bind() = 0;

   virtual void unbind() = 0;

   virtual int init(int image_width, int image_height, bool use_interpolation = false) = 0;

   virtual void cleanup() = 0;

   virtual GLuint get_texture() = 0;

protected:
   bool _init;
   bool multisampled;
   int _width;
   int _height;
   GLuint _fbo_multi;
   GLuint _fbo_single;
   GLuint _depth_rb_multi;
   GLuint _depth_rb_single;
   GLuint _color_tex_multi;
   GLuint _color_tex_single;
};
