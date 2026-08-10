#include "gui_callback.h"

#include "main.h"
#include "gui.h"
#include "gui_generated.h"

#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_RGB_Image.H>
#include <iostream>

void SkinSelectorCallback(Fl_Widget* w) {
    unsigned char oldSelectedSkin = selectedSkin;
    selectedSkin = (unsigned char)PreviewSkinSelector->value();

    if (oldSelectedSkin != selectedSkin) {
        selectedFrame = 1;
        isAnimating = false;
    }

    UpdateInterface();
}

void FrameSelectorCallback(Fl_Widget* w) {
    selectedFrame = (unsigned int)PreviewFrameSelector->value();
    UpdateInterface();
}

void AnimatedCheckboxCallback(Fl_Widget* w) {
    skins[selectedSkin].animated = AnimatedCheckbox->value();
    isAnimating = false;
    UpdateInterface(); // update frame controls visibility
}

void FPSInputCallback(Fl_Widget* w) {
    skins[selectedSkin].fps = (int)FPSInput->value();
}

void ScanlineCheckboxCallback(Fl_Widget* w) {
    skins[selectedSkin].scanline = ScanlineCheckbox->value();
}

void ScrollingCheckboxCallback(Fl_Widget* w) {
    skins[selectedSkin].scrolling = ScrollingCheckbox->value();
    UpdateInterface(); // update scroll controls visibility
}

void HorizontalScrollingCheckboxCallback(Fl_Widget* w) {
    skins[selectedSkin].horizontalScrolling = HorizontalScrollingCheckbox->value();
}

void ScrollRateInputCallback(Fl_Widget* w) {
    skins[selectedSkin].scrollingRate = (float)ScrollRateInput->value();
}

void PoseSelectorCallback(Fl_Widget* w) {
    angled = PoseSelector->value() == 0;
}

void ModelNameTextInputCallback(Fl_Widget* w) {
    modelName = ModelNameTextInput->value();
}


void AddSkinCallback(Fl_Widget* w) {
    if (skins.size() >= MAX_SKINS) {
        return;
    }

    AddNewSkin();

    // TODO: bad to do this twice, idk how to fix
    UpdateInterface();
    selectedSkin = (unsigned char)PreviewSkinSelector->maximum();
    selectedFrame = 1;
    UpdateInterface();
}

void RemoveCurrentSkinCallback(Fl_Widget* w) {
    RemoveSkin(selectedSkin);

    if (skins.size() == 0) {
        AddNewSkin();
    }

    // Use loop to avoid repetition
    while (selectedSkin > skins.size() - 1) {
        selectedSkin -= 1;
    }

    selectedFrame = 1;
    UpdateInterface(); // also calls PreviewSkinSelector->value
}

void AddFrameCallback(Fl_Widget* w) {
    Skin* currentSkin = &skins[selectedSkin];

    if (currentSkin->frames.size() >= MAX_FRAMES) {
        return;
    }

    AddEmptyFrameToSkin(currentSkin);

    UpdateInterface();
    selectedFrame = (unsigned int)PreviewFrameSelector->maximum();
    UpdateInterface();
}

void RemoveCurrentFrameCallback(Fl_Widget* w) {
    Skin* currentSkin = &skins[selectedSkin];
    if (currentSkin->previewImages.size() >= selectedFrame)
        currentSkin->previewImages.erase(currentSkin->previewImages.begin() + selectedFrame - 1);

    RemoveFrameFromSkin(currentSkin, selectedFrame - 1);

    if (currentSkin->frames.size() == 0) {
        AddEmptyFrameToSkin(currentSkin);
        currentSkin->width = 0;
        currentSkin->height = 0;
    }

    // Use loop to avoid repetition
    while (selectedFrame > currentSkin->frames.size()) {
        selectedFrame -= 1;
    }

    UpdateInterface(); // also calls PreviewFrameSelector->value
}

void LoadImageCallback(Fl_Widget* w) {
    Fl_Native_File_Chooser fnfc;

    fnfc.title("Pick a file");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fnfc.filter("Images\t*.{png,bmp,jpg,jpeg,tga,webp,vtf}\n");
    
    switch ( fnfc.show() ) {
        case -1:
        { // ERROR
            printf("ERROR: %s\n", fnfc.errmsg());
            break;
        }
        case  1:
        { // CANCEL
            break;
        }
        default:
        { // FILE CHOSEN
            LoadImageForCurrentFrame(fnfc.filename());
            Skin* currentSkin = &skins[selectedSkin];

            if (!currentSkin->imageLoaded[selectedFrame - 1]) {
                fl_alert("The selected image failed to load!");
                break;
            }
            
            RGBATexture* frame = &currentSkin->frames[selectedFrame - 1];
            const uchar* pixel_data = reinterpret_cast<const uchar*>(frame->data());

            Fl_RGB_Image* base_img = new Fl_RGB_Image(pixel_data, currentSkin->width, currentSkin->height, 4);
            int image_size = std::min(PreviewImage->w(), PreviewImage->h());
            Fl_Image* resized = base_img->copy(image_size, image_size);
            delete base_img;

            int targetIndex = selectedFrame - 1;
            if (static_cast<size_t>(targetIndex) >= currentSkin->previewImages.size()) {
                currentSkin->previewImages.resize(targetIndex + 1);
            }

            delete currentSkin->previewImages[targetIndex];
            currentSkin->previewImages[targetIndex] = resized;

            UpdateInterface();
            break;
        }
    }
}

void PlayAnimationCallback(Fl_Widget* w) {
    bool hasAnyImages = false;
    for (bool loaded : skins[selectedSkin].imageLoaded)
        hasAnyImages |= loaded;
    
    if (!hasAnyImages) {
        isAnimating = false;
        return UpdateInterface();
    }

    isAnimating = !isAnimating;
    UpdateInterface();
    
    if (isAnimating)
        Fl::add_timeout(1.0 / skins[selectedSkin].fps, AnimationCallback, nullptr);
}

void ExportCallback(Fl_Widget* w) {
    Fl_Native_File_Chooser fnfc;

    fnfc.title("Pick the output folder");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    fnfc.options(Fl_Native_File_Chooser::NEW_FOLDER);
    
    switch ( fnfc.show() ) {
        case -1:
        { // ERROR
            printf("ERROR: %s\n", fnfc.errmsg());
            break;
        }
        case  1:
        { // CANCEL
            break;
        }
        default:
        { // FILE CHOSEN
            ExportModel((std::filesystem::path)fnfc.filename());
        }
    }
}