#ifndef OLX_MAINLOOP_H
#define OLX_MAINLOOP_H

extern bool afterCrash;

void doMainLoop();

bool handleSDLEvents(bool wait);

#endif // OLX_MAINLOOP_H
