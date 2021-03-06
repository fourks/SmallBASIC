// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include "ui/StringLib.h"
#include "ui/ansiwidget.h"

struct System : public IButtonListener {
  System();
  virtual ~System();

  void buttonClicked(const char *action);
  int getPen(int code);
  char *getText(char *dest, int maxSize);
  bool isActive() { return _state != kInitState && _state != kDoneState; }
  bool isBack() { return _state == kBackState; }
  bool isBreak() { return _state >= kBreakState; }
  bool isClosing() { return _state >= kClosingState; }
  bool isInitial() { return _state == kInitState; }
  bool isModal() { return _state == kModalState; }
  bool isRestart() { return _state == kRestartState; }
  bool isRunning() { return _state == kRunState || _state == kModalState; }
  bool isSystemScreen() { return _systemScreen; }
  char *readSource(const char *fileName);
  void setBack();
  void setRunning(bool running);
  void systemPrint(const char *msg, ...);

  AnsiWidget *_output;
  virtual MAEvent processEvents(int waitFlag) = 0;
  virtual void setExit(bool quit) = 0;
  virtual char *loadResource(const char *fileName);

protected:
  MAEvent getNextEvent() { return processEvents(1); }
  void handleEvent(MAEvent event);
  void handleMenu(int menuId);
  void resize();
  void runMain(const char *mainBasPath);
  void runOnce(const char *startupBas);
  void setPath(const char *filename);
  bool setParentPath();
  void setDimensions();
  void showCompletion(bool success);
  void showError();
  void checkLoadError();
  void showSystemScreen(bool showSrc);
  void showMenu();

  enum { 
    kInitState = 0,// thread not active
    kActiveState,  // thread activated
    kRunState,     // program is running
    kRestartState, // running program should restart
    kModalState,   // retrieving user input inside program
    kConnState,    // retrieving remote program source
    kBreakState,   // running program should abort
    kBackState,    // back button detected 
    kClosingState, // thread is terminating
    kDoneState     // thread has terminated
  } _state;

  strlib::String _loadPath;
  int _lastEventTime;
  int _eventTicks;
  int _touchX;
  int _touchY;
  int _touchCurX;
  int _touchCurY;
  int _initialFontSize;
  int _fontScale;
  int _overruns;
  bool _systemMenu;
  bool _systemScreen;
  bool _mainBas;
  bool _buttonPressed;
  char *_programSrc;
};

#endif
