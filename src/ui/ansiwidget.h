// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef ANSIWIDGET_H
#define ANSIWIDGET_H

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#if defined(_MOSYNC)
  #include <maapi.h>
#else
  #include "ui/maapi.h"
#endif

#include "ui/StringLib.h"
#include "ui/screen.h"
#include "ui/interface.h"

#define MAX_PENDING 250
#define MAX_PENDING_GRAPHICS 25
#define MAX_SCREENS 8
#define SYSTEM_SCREENS 6

using namespace strlib;

// base implementation for all buttons
struct Widget : public Shape {
  Widget(int bg, int fg, int x, int y, int w, int h);
  virtual ~Widget() {}
  virtual void clicked(IButtonListener *listener, int x, int y) = 0;
  virtual bool overlaps(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  void drawButton(const char *caption, int x, int y, int w, int h, bool pressed);
  void drawLink(const char *caption, int x, int y, int sw, int chw);
  int getBackground(int buttonColor);

  bool _pressed;
  int _bg, _fg;
};

// base implementation for all internal buttons
struct Button : public Widget {
  Button(Screen *screen, const char *action, const char *label,
         int x, int y, int w, int h);
  virtual ~Button() {}
  void clicked(IButtonListener *listener, int x, int y);

  String _action;
  String _label;
};

// internal text button
struct TextButton : public Button {
  TextButton(Screen *screen, const char *action, const char *label,
             int x, int y, int w, int h) :
  Button(screen, action, label, x, y, w, h) {}
  void draw(int x, int y, int bw, int cw) {
    drawLink(_label.c_str(), x, y, bw, cw);
  }
};

// internal block button
struct BlockButton : public Button {
  BlockButton(Screen *screen, const char *action, const char *label,
              int x, int y, int w, int h) :
  Button(screen, action, label, x, y, w, h) {}
  void draw(int x, int y, int bw, int cw) {
    drawButton(_label.c_str(), x, y, _width, _height, _pressed);
  }
};

// base implementation for all external buttons
struct FormWidget : public Widget, IFormWidget {
  FormWidget(Screen *screen, int x, int y, int w, int h);
  virtual ~FormWidget();

  void setListener(IButtonListener *listener) { this->_listener = listener; }
  Screen *getScreen() { return _screen; }
  void clicked(IButtonListener *listener, int x, int y);

  IFormWidgetListModel *getList() const { return NULL; }
  void setText(const char *text) {}
  bool edit(int key) { return false; }

  int getX() { return this->_x; }
  int getY() { return this->_y; }
  int getW() { return this->_width; }
  int getH() { return this->_height; }
  void setX(int x) { this->_x = x; }
  void setY(int y) { this->_y = y; }
  void setW(int w) { this->_width = w; }
  void setH(int h) { this->_height = h; }

protected:
  Screen *_screen;
  IButtonListener *_listener;
};

struct FormButton : public FormWidget {
  FormButton(Screen *screen, const char *caption, int x, int y, int w, int h);
  virtual ~FormButton() {}

  const char *getText() const { return _caption.c_str(); }
  void draw(int x, int y, int sw, int chw) {
    drawButton(_caption.c_str(), x, y, _width, _height, _pressed);
  }
  void clicked(IButtonListener *listener, int x, int y);
  void setText(const char *text) { _caption = text; }

private:
  String _caption;
};

struct FormLabel : public FormWidget {
  FormLabel(Screen *screen, const char *caption, int x, int y, int w, int h);
  virtual ~FormLabel() {}

  const char *getText() const { return _caption.c_str(); }
  void draw(int x, int y, int sw, int chw) {
    drawButton(_caption.c_str(), x, y, _width, _height, false);
  }
  void setText(const char *text) { _caption = text; }

private:
  String _caption;
};

struct FormLink : public FormWidget {
  FormLink(Screen *screen, const char *link, int x, int y, int w, int h);
  virtual ~FormLink() {}

  const char *getText() const { return _link.c_str(); }
  void draw(int x, int y, int sw, int chw) {
    drawLink(_link.c_str(), x, y, sw, chw);
  }

private:
  String _link;
};

struct FormLineInput : public FormWidget {
  FormLineInput(Screen *screen, char *buffer, int maxSize,
                int x, int y, int w, int h);
  virtual ~FormLineInput() {}

  void close();
  void draw(int x, int y, int sw, int chw);
  bool edit(int key);
  const char *getText() const { return _buffer; }
  void setText(const char *text) {}

private:
  char *_buffer;
  int _maxSize;
  int _scroll;
};

struct FormList : public FormWidget {
  FormList(Screen *screen, IFormWidgetListModel *model,
           int x, int y, int w, int h);
  virtual ~FormList() {}

  IFormWidgetListModel *getList() const { return _model; }
  const char *getText() const { return _model->getTextAt(_model->selected()); }
  void optionSelected(int index);

protected:
  IFormWidgetListModel *_model;
  int _topIndex;
  int _activeIndex;
};

struct FormDropList : public FormList {
  FormDropList(Screen *screen, IFormWidgetListModel *model,
               int x, int y, int w, int h);
  void clicked(IButtonListener *listener, int x, int y);
  void draw(int dx, int dy, int sw, int chw);
  bool overlaps(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  virtual ~FormDropList() {}

private:
  void drawList(int dx, int dy);
  int _listWidth;
  int _listHeight;
  int _visibleRows;
  bool _listActive;
};

struct FormListBox : public FormList {
  FormListBox(Screen *screen, IFormWidgetListModel *model,
              int x, int y, int w, int h);
  void clicked(IButtonListener *listener, int x, int y);
  void draw(int dx, int dy, int sw, int chw);
  bool overlaps(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  virtual ~FormListBox() {}
};

struct AnsiWidget {
  explicit AnsiWidget(IButtonListener *listener, int width, int height);
  ~AnsiWidget();

  void beep() const;
  void clearScreen() { _back->clear(); }
  bool construct();
  IFormWidget *createButton(char *caption, int x, int y, int w, int h);
  IFormWidget *createLabel(char *caption, int x, int y, int w, int h);
  IFormWidget *createLineInput(char *buffer, int maxSize, int x, int y, int w, int h);
  IFormWidget *createLink(char *caption, int x, int y, int w, int h);
  IFormWidget *createListBox(IFormWidgetListModel *model, int x, int y, int w, int h);
  IFormWidget *createDropList(IFormWidgetListModel *model, int x, int y, int w, int h);
  void draw();
  void drawOverlay(bool vscroll) { _back->drawOverlay(vscroll); }
  void drawImage(MAHandle image, int x, int y, int sx, int sy, int w, int h);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  void edit(IFormWidget *formWidget, int c);
  void flush(bool force, bool vscroll=false, int maxPending = MAX_PENDING);
  void flushNow() { if (_front) _front->drawBase(false); }
  int  getBackgroundColor() { return _back->_bg; }
  int  getColor() { return _back->_fg; }
  int  getFontSize() { return _fontSize; }
  int  getPixel(int x, int y);
  void getScroll(int &x, int &y) { _back->getScroll(x, y); }
  int  getHeight() { return _height; }
  int  getWidth()  { return _width; }
  int  getX() { return _back->_curX; }
  int  getY() { return _back->_curY; }
  int  textHeight(void);
  bool optionSelected(int index);
  void print(const char *str);
  void redraw();
  void reset();
  void resize(int width, int height);
  void setColor(long color);
  void setDirty() { _back->setDirty(); }
  void setAutoflush(bool autoflush) { _autoflush = autoflush; }
  void setFontSize(int fontSize);
  void setPixel(int x, int y, int c);
  void setTextColor(long fg, long bg);
  void setXY(int x, int y) { _back->_curX=x; _back->_curY=y; }
  void setScrollSize(int scrollSize);
  bool pointerTouchEvent(MAEvent &event);
  bool pointerMoveEvent(MAEvent &event);
  void pointerReleaseEvent(MAEvent &event);

private:
  void createLabel(char *&p);
  Widget *createLink(char *&p, bool formLink, bool button);
  Widget *createLink(const char *action, const char *text,
                     bool formLink, bool button);
  void createOptionsBox(char *&p);
  bool doEscape(char *&p, int textHeight);
  void doSwipe(int start, bool moveDown, int distance, int maxScroll);
  void drawActiveButton();
  List<String *> *getItems(char *&p);
  void handleEscape(char *&p, int textHeight);
  void removeScreen(char *&p);
  void screenCommand(char *&p);
  bool setActiveButton(MAEvent &event, Screen *screen);
  Screen *selectScreen(char *&p);
  void showAlert(char *&p);
  void swapScreens();

  Screen *_screens[MAX_SCREENS];
  Screen *_back;   // screen being painted/written
  Screen *_front;  // screen to display
  Screen *_focus;  // screen with the active button
  int _width;      // device screen width
  int _height;     // device screen height
  int _fontSize;   // font height based on screen size
  int _xTouch;     // touch x value
  int _yTouch;     // touch y value
  int _xMove;      // touch move x value
  int _yMove;      // touch move y value
  int _touchTime;  // last move time
  bool _swipeExit; // last touch-down was swipe exit
  bool _autoflush; // flush internally
  IButtonListener *_buttonListener;
  Widget *_activeButton;
};

#endif // ANSIWIDGET_H
