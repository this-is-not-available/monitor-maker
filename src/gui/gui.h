#ifndef GUI_H
#define GUI_H


#include "gui_callback.h"
#include "gui_generated.h"

void UpdateInterface();
void AnimationCallback(void* data);

extern bool isAnimating;

#endif