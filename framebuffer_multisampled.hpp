#pragma once

#include "framebuffer.h"

#define USING_MULTISAMPLE 1

class Framebuffer_multisampled : public Framebuffer{
public:
   Framebuffer_multisampled(){
   }

   void bind() {
      glViewport(0, 0, _width, _height);
      #if USING_MULTISAMPLE
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo_multi);
      #else
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo_single);
      #endif
   }

   void unbind() {
      #if USING_MULTISAMPLE
      glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo_multi); // Make sure your multisampled FBO is the read framebuffer
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo_single);   // Make sure no FBO is set as the draw framebuffer
      glDrawBuffer(GL_BACK);
      glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
      #endif
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   int init(int image_width, int image_height, bool use_interpolation = false) {
      this->_width = image_width;
      this->_height = image_height;

      glGenFramebuffers(1, &_fbo_multi);
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo_multi);

      glGenTextures(1, &_color_tex_multi);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _color_tex_multi);

      glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, _width, _height, true );
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 /*location = 0*/, GL_TEXTURE_2D_MULTISAMPLE, _color_tex_multi, 0 /*level*/);

      glGenRenderbuffers(1, &_depth_rb_multi);
      glBindRenderbuffer(GL_RENDERBUFFER, _depth_rb_multi);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_rb_multi);

      glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT32, _width, _height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
         std::cerr << "!!!ERROR: Framebuffer not OK :(" << std::endl;
      glBindFramebuffer(GL_FRAMEBUFFER, 0); ///< avoid pollution

      //////////////////////////////////single sampled frambuffer//////////////////////////////////7
      glGenFramebuffers(1, &_fbo_single);
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo_single);

      glGenTextures(1, &_color_tex_single);
      glBindTexture(GL_TEXTURE_2D, _color_tex_single);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if(use_interpolation){
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      } else {
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      }

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); ///< how to load from buffer

      glBindTexture(GL_TEXTURE_2D, 0);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 /*location = 0*/, GL_TEXTURE_2D, _color_tex_single, 0 /*level*/);

      #if !USING_MULTISAMPLE
      glGenRenderbuffers(1, &_depth_rb_single);
      glBindRenderbuffer(GL_RENDERBUFFER, _depth_rb_single);

      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, _width, _height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_rb_single);
      #endif

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
         std::cerr << "!!!ERROR: Framebuffer not OK :(" << std::endl;
      glBindFramebuffer(GL_FRAMEBUFFER, 0); ///< avoid pollution

      #if USING_MULTISAMPLE
      return _color_tex_single;
      #else
      return _color_tex_single;
      #endif
   }

   void cleanup() {
      glDeleteTextures(1, &_color_tex_single);
      glDeleteTextures(1, &_color_tex_multi);
      glDeleteRenderbuffers(1, &_depth_rb_single);
      glDeleteRenderbuffers(1, &_depth_rb_multi);
      glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
      glDeleteFramebuffers(1, &_fbo_single);
      glDeleteFramebuffers(1, &_fbo_multi);
   }

   GLuint get_texture(){
      return _color_tex_multi;
   }

protected:
   bool _init;
   int _width;
   int _height;
   GLuint _fbo_multi;
   GLuint _fbo_single;
   GLuint _depth_rb_multi;
   GLuint _depth_rb_single;
   GLuint _color_tex_multi;
   GLuint _color_tex_single;
};
