#include "gui_callback.h"

#include "main.h"
#include "gui.h"
#include "gui_generated.h"

#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_RGB_Image.H>
#include <iostream>

// Callback for currently selected skin spinner
void SkinSelectorCallback(Fl_Widget* w) {
    unsigned char oldSelectedSkin = selectedSkin;
    selectedSkin = (unsigned char)PreviewSkinSelector->value();

    if (oldSelectedSkin != selectedSkin) {
        selectedFrame = 1;
        isAnimating = false;
    }

    UpdateInterface();
}

// Callback for currently selected frame spinner
void FrameSelectorCallback(Fl_Widget* w) {
    selectedFrame = (unsigned int)PreviewFrameSelector->value();
    UpdateInterface();
}

// Callback for animation toggle checkbox
void AnimatedCheckboxCallback(Fl_Widget* w) {
    skins[selectedSkin].animated = AnimatedCheckbox->value();
    isAnimating = false;
    selectedFrame = 1;
    UpdateInterface(); // update frame controls visibility
}

// Callback for animation FPS spinner
void FPSInputCallback(Fl_Widget* w) {
    skins[selectedSkin].fps = (int)FPSInput->value();
}

// Callback for horizontal scanline effect toggle checkbox
void ScanlineCheckboxCallback(Fl_Widget* w) {
    skins[selectedSkin].scanline = ScanlineCheckbox->value();
}

// Callback for skin scrolling toggle checkbox
void ScrollingCheckboxCallback(Fl_Widget* w) {
    skins[selectedSkin].scrolling = ScrollingCheckbox->value();
    UpdateInterface(); // update scroll controls visibility
}

// Callback for skin horizontal scrolling toggle checkbox
void HorizontalScrollingCheckboxCallback(Fl_Widget* w) {
    skins[selectedSkin].horizontalScrolling = HorizontalScrollingCheckbox->value();
}

// Callback for skin scroll rate spinner
void ScrollRateInputCallback(Fl_Widget* w) {
    skins[selectedSkin].scrollingRate = (float)ScrollRateInput->value();
}

// Callback for model pose dropdown menu
void PoseSelectorCallback(Fl_Widget* w) {
    angled = PoseSelector->value() == 0;
}

// Callback for model name and path input
void ModelNameTextInputCallback(Fl_Widget* w) {
    modelName = ModelNameTextInput->value();
}


// Callback for new skin button
void AddSkinCallback(Fl_Widget* w) {
    if (skins.size() >= MAX_SKINS) {
        return;
    }

    AddNewSkin();

    // TODO: bad to do this twice, idk how to fix without duplicating maximum calculation
    UpdateInterface();
    selectedSkin = (unsigned char)PreviewSkinSelector->maximum();
    selectedFrame = 1;
    UpdateInterface();
}

// Callback for remove skin button
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

// Callback for add new frame button
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

// Callback for remove frame button
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

// Callback for load image button
void LoadImageCallback(Fl_Widget* w) {
    Fl_Native_File_Chooser fnfc;

    fnfc.title("Pick a file");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fnfc.filter("All\t*.{png,bmp,jpg,jpeg,tga,webp,vtf,gif,apng}\n"
                "Images\t*.{png,bmp,jpg,jpeg,tga,webp,vtf}\n"
                "Animated Images\t*.{gif,apng,vtf}");
    
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
            int numAddedFrames = LoadImageForCurrentFrame(fnfc.filename());
            Skin* currentSkin = &skins[selectedSkin];

            for (int i = 0; i < numAddedFrames; i++) {
                int targetIndex = i + selectedFrame - 1;
                if (!currentSkin->imageLoaded[targetIndex]) {
                    fl_alert("The selected image failed to load!");
                    break;
                }
                
                // Get image pixels
                RGBATexture* frame = &currentSkin->frames[targetIndex];
                const uchar* pixel_data = reinterpret_cast<const uchar*>(frame->data());

                // Resize
                Fl_RGB_Image* base_img = new Fl_RGB_Image(pixel_data, currentSkin->width, currentSkin->height, 4);
                int image_size = std::min(PreviewImage->w(), PreviewImage->h());
                Fl_Image* resized = base_img->copy(image_size, image_size);
                delete base_img;

                // Grow array to fit frame
                if (static_cast<size_t>(targetIndex) >= currentSkin->previewImages.size()) {
                    currentSkin->previewImages.resize(targetIndex + 1);
                }

                if (i > 0) {
                    currentSkin->previewImages.insert(currentSkin->previewImages.begin() + targetIndex, resized);
                } else {
                    // Store frame
                    delete currentSkin->previewImages[targetIndex];
                    currentSkin->previewImages[targetIndex] = resized;
                }
            }

            if (numAddedFrames > 1) {
                currentSkin->animated = true;
            }

            // Display
            UpdateInterface();
            break;
        }
    }
}

// Callback for animation playback button
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