#ifndef UGUI_SIM_H_
#define UGUI_SIM_H_

#include "ugui.h"

typedef struct
{
    int      width;
    int      height;
    int      screenMultiplier;
    int      screenMargin;
    uint32_t windowBackColor;
} simcfg_t;

/* Platform-independent key codes for GUI_HandleKey */
#define GUI_KEY_UP        0
#define GUI_KEY_DOWN      1
#define GUI_KEY_LEFT      2
#define GUI_KEY_RIGHT     3
#define GUI_KEY_PAGEUP    4
#define GUI_KEY_PAGEDOWN  5
#define GUI_KEY_HOME      6
#define GUI_KEY_END       7

simcfg_t* GUI_SimCfg(void);
void      GUI_Setup(UG_DEVICE *device);
void      GUI_Process(void);
void      GUI_HandleKey(int key);

#endif /* UGUI_SIM_H_ */