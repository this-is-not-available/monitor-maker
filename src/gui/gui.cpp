#include "gui.h"

#include "main.h"
#include <iostream>
#include <FL/Fl_JPEG_Image.H>

bool isAnimating = false;

uchar empty_image_data[4] = {0,0,0,0};
Fl_RGB_Image* emptyImage = new Fl_RGB_Image(empty_image_data, 1, 1, 4);

// Sets a widget to be visibility based on a bool
void SetVisible(Fl_Widget* w, bool visible) {
    if (visible) {
        w->show();
    } else{
        w->hide();
    }
}

// Loads the current selected skin's selected frame into the preview and updates PreviewFrameSelector
void UpdatePreviewImage() {
    Skin currentSkin = skins[selectedSkin];

    // Annoying to have to do this here too
    // since it's already used in UpdateInterface() which also calls this function
    // but it otherwise wouldn't update if we are animating
    PreviewFrameSelector->value(selectedFrame);


    if (currentSkin.imageLoaded[selectedFrame - 1]) {
        Fl_Image * preview = currentSkin.previewImages[selectedFrame - 1];
        PreviewImage->image(preview);
        PreviewImage->label("");
    }
    else {
        PreviewImage->image(emptyImage);
        PreviewImage->label("No Image Loaded");
    }

    PreviewImage->redraw();
}

// Update entire interface to reflect current state of the application
void UpdateInterface() {
    Skin currentSkin = skins[selectedSkin];

    size_t totalFrames = 0;
    for (const Skin& skin : skins) {
        totalFrames += skin.frames.size();
    }

    bool isAnimated = currentSkin.animated;
    bool isScrolling = currentSkin.scrolling;

    // Sidebar
    ResolutionDisplay->copy_label(std::format("Resolution: {}x{}", currentSkin.width, currentSkin.height).c_str());
    SkinFrameCountDisplay->copy_label(std::format("Frames in selected skin: {}", currentSkin.frames.size()).c_str());
    AnimatedCheckbox->value(isAnimated);
    ScanlineCheckbox->value(currentSkin.scanline);
    ScrollingCheckbox->value(isScrolling);
    ScrollRateInput->value(currentSkin.scrollingRate);
    HorizontalScrollingCheckbox->value(currentSkin.horizontalScrolling);
    FPSInput->value(currentSkin.fps);
    SkinCountDisplay->copy_label(std::format("Skins: {}", skins.size()).c_str());
    TotalFrameCountDisplay->copy_label(std::format("Total frames: {}", totalFrames).c_str());

    ModelNameTextInput->value(modelName.c_str());
    //PoseSelector->value(angled ? 0 : 1);

    SetVisible(ExportButton, p2Found);
    SetVisible(ScrollRateInput, isScrolling);
    SetVisible(HorizontalScrollingCheckbox, isScrolling);
    SetVisible(FPSInput, isAnimated);

    // Right side
    PreviewSkinSelector->maximum((double)(skins.size() - 1));
    PreviewSkinSelector->minimum(0);
    PreviewSkinSelector->value(selectedSkin);
    PreviewFrameSelector->maximum((double)currentSkin.frames.size());
    PreviewFrameSelector->minimum(1);
    PreviewFrameSelector->value(selectedFrame);

    SetVisible(PreviewFrameSelector, isAnimated);
    SetVisible(AddFrameButton, isAnimated);
    SetVisible(RemoveFrameButton, isAnimated);
    SetVisible(PlayAnimation, isAnimated);

    if (isAnimating)
        PlayAnimation->label("@||");
    else
        PlayAnimation->label("►");
    
    UpdatePreviewImage();
}

// Callback to cycle next frame in the skin's animation and schedule it for next frame
void AnimationCallback(void* data) {
    if (isAnimating) {
        selectedFrame %= skins[selectedSkin].frames.size();
        selectedFrame += 1;
        UpdatePreviewImage();
        Fl::repeat_timeout(1.0 / skins[selectedSkin].fps, AnimationCallback, data);
    }
}