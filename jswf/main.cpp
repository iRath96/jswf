//
//  main.cpp
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "StringReader.h"
#include "Document.h"
#include "Header.h"
#include "Tags.h"
#include "Render.h"

#include <SDL2/SDL.h>
#include <OpenGL/gl.h>

using namespace jswf;

uint16_t currentFrame = 0;
flash::Document *document = NULL;
render::Context ctx;

void clearScreen() {
  memset(ctx.buffer, 0xff, ctx.w * ctx.h * 4);
}

void renderCurrentFrame() {
  clearScreen();
  render::renderFrame(document->frames[currentFrame], ctx);
}

#define PENGUIN_FILENAME "/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/penguin.swf"
void renderSWF(std::string filename) {
  std::ifstream file(filename);
  std::stringstream s;
  s << file.rdbuf();
  
  if(document != NULL) delete document;
  
  io::StringReader *reader = new io::StringReader(s.str());
  document = new flash::Document(reader);
  ctx.document = document;
  
  if(filename == PENGUIN_FILENAME) {
    flash::ColorTransform &ct = document->frames[0].displayList[1].colorTransform;
    ct.rM = ct.gM = ct.bM = 0;
    ct.rA = 0xff;
    ct.gA = 0xcc;
    ct.bA = 0x00;
  }
  
  renderCurrentFrame();
}

int main(int argc, char *argv[]) {
#pragma mark Render
  
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Window *window = SDL_CreateWindow(
                                        "jswf",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        550, 400,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
  
  int width, height;
  char title[128];
  
  SDL_GL_GetDrawableSize(window, &width, &height);
  sprintf(title, "jswf [%d x %d]", width, height);
  SDL_SetWindowTitle(window, title);
  
  // Create an OpenGL context associated with the window.
  SDL_GLContext glcontext = SDL_GL_CreateContext(window);
  
  // now you can make GL calls.
  glClearColor(1,1,1,1);
  glClear(GL_COLOR_BUFFER_BIT);
  
  uint32_t *pixels = new uint32_t[width * height];
  
  ctx.w = width;
  ctx.h = height;
  ctx.buffer = (uint32_t *)pixels;
  
  ctx.matrix.sx = ctx.matrix.sy = 4;
  ctx.matrix.tx = 100 * 20 * ctx.matrix.sx; // 85
  ctx.matrix.ty = 100 * 20 * ctx.matrix.sy; // 125
  
  if(true & 0) {
    ctx.matrix.sx = ctx.matrix.sy = 2;
    ctx.matrix.tx = ctx.matrix.ty = 0;
  }
  
  //renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/character.swf");
  
  renderSWF("test.swf");
  /*renderSWF(PENGUIN_FILENAME);
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/2129.swf"); // face
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/1025.swf"); // hair
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/839.swf");
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/728.swf");
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/3222.swf"); // scarf
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/6237.swf");*/
  
  glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  
  SDL_GL_SwapWindow(window);
  
  SDL_Event event;
  while(true) {
    while(SDL_PollEvent(&event)) {
      printf("%d vs %d vs %d\n", event.type, SDL_QUIT, SDL_KEYDOWN);
      printf("%d\n", event.key.keysym.sym);
      if(event.type == 16842757) goto exitMainLoop;
      if(event.type == 65538)
        ++currentFrame;
    }
    
    renderCurrentFrame();
    glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    SDL_GL_SwapWindow(window);
    
    SDL_Delay(1000/24);
  }
  
exitMainLoop:
  SDL_GL_DeleteContext(glcontext);
  SDL_Quit();
  
  return 0;
}
