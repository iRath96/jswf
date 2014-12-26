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

uint16_t minFrameCount = 0;
uint16_t currentFrame = 0;
std::vector<flash::Document *> documents;
render::Context ctx;

void clearScreen() {
  memset(ctx.buffer, 0xff, ctx.w * ctx.h * 4);
}

void renderCurrentFrame() {
  clearScreen();
  for(auto document = documents.begin(); document != documents.end(); ++document) {
    ctx.document = *document;
    render::renderFrame((*document)->frames[currentFrame], ctx);
  }
}

#define PENGUIN_FILENAME "/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/penguin.swf"
void renderSWF(std::string filename) {
  std::ifstream file(filename);
  std::stringstream s;
  s << file.rdbuf();
  
  io::StringReader *reader = new io::StringReader(s.str());
  flash::Document *document = new flash::Document(reader);
  
#define MIN(a, b) ((a) > (b) ? (b) : (a))
  minFrameCount = documents.empty() ? document->header.frameCount : MIN(minFrameCount, document->header.frameCount);
#undef MIN
  
  documents.push_back(document);
  
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
  ctx.matrix.tx = 85 * 20 * ctx.matrix.sx; // 138 for character
  ctx.matrix.ty = 105 * 20 * ctx.matrix.sy; // 111 for character
  
  if(true & 0) {
    ctx.matrix.sx = ctx.matrix.sy = 2;
    ctx.matrix.tx = ctx.matrix.ty = 0;
  }
  
  // fixed (bugs from swf2svg)
  // -  2121.swf  [gradient]
  // -   110.swf  [general failure]
  // -  2129.swf  [gradient]
  // -  2020.swf  [shape incorrect]
  // -  3032.swf  [shape missing], also: animations!
  // -  5485.swf  [?]
  // - 15045.swf  [mask]
  // -  5220.swf  [transformation]
  // -  5223.swf  [general failure]
  // -  1122.swf  [general failure]
  // -  1353.swf  [general failure]
  // -  3226.swf  [mask]
  // - 24215.swf  [mask]
  // -   118.swf  [gradient]
  
  // bugs:
  // -  3138.swf  [lines] (before: [general failure])
  // - 24042.swf  [lines]
  // - 24216.swf  [lines]
  // -  5481.swf  [lines] (before: [general failure])
  
  if(false) {
    for(int i = 0; i < 16; ++i)
      renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/24215.swf");
    return 0;
  }
  
  //renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/9123.swf"); // 9123 is also cool
  //renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/character.swf");
  //renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/sprite-swfs/puffle_gold_walk.swf");
  //renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/sprite-swfs/puffle_blue1023_walk.swf");
  
  //renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/room-swfs/town.swf");
  
  //renderSWF("test.swf");
  renderSWF(PENGUIN_FILENAME);
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/2129.swf"); // face
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/1025.swf"); // hair
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/839.swf");
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/728.swf");
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/3222.swf"); // scarf
  renderSWF("/Users/alex/Desktop/Desktop/pcl/swf2svg/swfs/6237.swf");
  
  glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  SDL_GL_SwapWindow(window);
  //return 0;
  
  // Efficiency:
  // TODO:2014-12-25:alex:Shape::polygonize is 91.3% of Document::read, 59.4% of polygonize is vector::erase
  // TODO:2014-12-25:alex:renderCurrentFrame is 94.7% of renderSWF, Document::Document is 5.1%
  // TODO:2014-12-25:alex:calculateIntersections2 is 77.5% of render::renderShape.
  
  SDL_Event event;
  while(true) {
    while(SDL_PollEvent(&event)) {
      printf("%d vs %d vs %d\n", event.type, SDL_QUIT, SDL_KEYDOWN);
      printf("%d\n", event.key.keysym.sym);
      //if(event.type == 16842757) goto exitMainLoop;
      if(event.type == 65538)
        currentFrame = (currentFrame + 1) % minFrameCount;
    }
    
    if(minFrameCount > 1) {
      renderCurrentFrame();
      glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
      SDL_GL_SwapWindow(window);
    }
    
    SDL_Delay(1000/24);
  }
  
exitMainLoop:
  SDL_GL_DeleteContext(glcontext);
  SDL_Quit();
  
  return 0;
}
