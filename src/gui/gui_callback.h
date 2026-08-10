#ifndef GUI_CALLBACK_H
#define GUI_CALLBACK_H


#include "gui_generated.h"

void SkinSelectorCallback(Fl_Widget* w);
void FrameSelectorCallback(Fl_Widget* w);
void AnimatedCheckboxCallback(Fl_Widget* w);
void FPSInputCallback(Fl_Widget* w);

void ScanlineCheckboxCallback(Fl_Widget* w);
void ScrollingCheckboxCallback(Fl_Widget* w);
void HorizontalScrollingCheckboxCallback(Fl_Widget* w);
void ScrollRateInputCallback(Fl_Widget* w);

void ModelNameTextInputCallback(Fl_Widget* w);
void PoseSelectorCallback(Fl_Widget* w);

void AddSkinCallback(Fl_Widget* w);
void RemoveCurrentSkinCallback(Fl_Widget* w);
void AddFrameCallback(Fl_Widget* w);
void RemoveCurrentFrameCallback(Fl_Widget* w);

void LoadImageCallback(Fl_Widget* w);
void PlayAnimationCallback(Fl_Widget* w);
void ExportCallback(Fl_Widget* w);

#endif