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

#include "SDL/SDL.h"

using namespace jswf;

int main(int argc, char *argv[]) {
  std::cout << "Loading file." << std::endl;
  
  std::ifstream file("test.swf");
  std::stringstream s;
  s << file.rdbuf();
  
  std::cout << s.str() << std::endl;
  
  io::StringReader *reader = new io::StringReader(s.str());
  flash::Document document(reader);
  
#pragma mark Render
  
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Surface *screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
  
  render::Context ctx;
  
  ctx.document = &document;
  ctx.w = screen->w;
  ctx.h = screen->h;
  ctx.buffer = (uint32_t *)screen->pixels;
  
  render::renderFrame(document.frames[0], ctx);
  
  SDL_Flip(screen);
  
  SDL_Event event;
  while(SDL_WaitEvent(&event)) {
    if(event.type == SDL_QUIT) break;
    if(event.type == SDL_KEYDOWN)
      if(event.key.keysym.sym == SDLK_ESCAPE) break;
  }
  
  SDL_Quit();
  
  return 0;
}
