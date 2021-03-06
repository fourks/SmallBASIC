// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <time.h>
#include "ui/form_ui.h"
#include "ui/utils.h"
#include "platform/sdl/display.h"
#include "common/device.h"

extern common::Graphics *graphics;

//
// Canvas implementation
//
Canvas::Canvas() :
  _canvas(NULL),
  _clip(NULL) {
}

Canvas::~Canvas() {
  if (_canvas != NULL) {
    SDL_FreeSurface(_canvas);
  }
  delete _clip;
  _canvas = NULL;
  _clip = NULL;
}

bool Canvas::create(int w, int h) {
  logEntered();
  _w = w;
  _h = h;
  int bpp;
  Uint32 rmask, gmask, bmask, amask;
  SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_RGB565, 
                             &bpp, &rmask, &gmask, &bmask, &amask);
  _canvas = SDL_CreateRGBSurface(0, w, h, bpp, rmask, gmask, bmask, amask);
  return _canvas != NULL;
}

pixel_t *Canvas::getLine(int y) { 
  pixel_t *pixels = (pixel_t *)_canvas->pixels;
  return pixels + (y * _w); 
}

void Canvas::setClip(int x, int y, int w, int h) {
  delete _clip;
  if (x != 0 || y != 0 || _w != w || _h != h) {
    _clip = new SDL_Rect();
    _clip->x = x;
    _clip->y = y;
    _clip->w = x + w;
    _clip->h = y + h;
  } else {
    _clip = NULL;
  }
}

//
// Graphics implementation
//
Graphics::Graphics(SDL_Window *window) : common::Graphics(),
  _window(window) {
}

Graphics::~Graphics() {
  logEntered();
}

bool Graphics::construct(const char *font, const char *boldFont) {
  logEntered();

  _surface = SDL_GetWindowSurface(_window);
  SDL_GetWindowSize(_window, &_w, &_h);
  bool result = false;
  if (loadFonts(font, boldFont)) {
    _screen = new Canvas();
    if (_screen && _screen->create(getWidth(), getHeight())) {
      _drawTarget = _screen;
      maSetColor(DEFAULT_BACKGROUND);
      result = true;
    }
  }
  return result;
}

void Graphics::redraw() {
  SDL_Surface *src = ((Canvas *)_screen)->_canvas;
  SDL_Rect srcrect;
  srcrect.x = 0;
  srcrect.y = 0;
  srcrect.w = _screen->_w;
  srcrect.h = _screen->_h;

  SDL_Rect dstrect;
  dstrect.x = 0;
  dstrect.y = 0;
  dstrect.w = _w;
  dstrect.h = _h;

  SDL_BlitSurface(src, &srcrect, _surface, &dstrect);
  SDL_UpdateWindowSurface(_window);
}

void Graphics::resize() {
  delete _screen;
  _screen = new ::Canvas();
  _screen->create(getWidth(), getHeight());
  _drawTarget = NULL;
}

bool Graphics::loadFonts(const char *font, const char *boldFont) {
  return (!FT_Init_FreeType(&_fontLibrary) &&
          loadFont(font, _fontFace) &&
          loadFont(boldFont, _fontFaceB));
}

bool Graphics::loadFont(const char *filename, FT_Face &face) {
  bool result = !FT_New_Face(_fontLibrary, filename, 0, &face);
  trace("load font %s = %d", filename, result);
  return result;
}

//
// maapi implementation
//
void maUpdateScreen(void) {
  ((::Graphics *)graphics)->redraw();
}


