#ifndef RESIZABLE_IMAGE_H
#define RESIZABLE_IMAGE_H


#include <Fl/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Check_Button.H>
#include <FL/fl_draw.H>

class ResizableImageBox : public Fl_Box {
public:
    ResizableImageBox(int X, int Y, int W, int H, const char* L = 0) 
        : Fl_Box(X, Y, W, H, L) {}

    void resize(int X, int Y, int W, int H) override {
        Fl_Box::resize(X, Y, W, H);
        
        if (image()) {
            image()->scale(w(), h(), 1, 1); 
        }
    }
};

class MonitorAspectOutlineImage : public ResizableImageBox {
public:
    bool isActive = false;

    MonitorAspectOutlineImage(int X, int Y, int W, int H, const char* L = 0) 
        : ResizableImageBox(X, Y, W, H, L) {}

    void TogglePreviewFromCheckbox(Fl_Check_Button* w) {
        isActive = (bool)w->value();
        this->redraw();
    }

protected:
    void draw() override {
        ResizableImageBox::draw();
        
        if (this->isActive && image()) {
            // (0, 0.751573) ----- (1, 0.750322)
            //       |                   |
            //       |                   |
            //       |                   |
            // (0, 0.248427) ----- (1, 0.249678)

            // the monitor has wonky UVs, use the average of the Vs
            //   0.7509475
            // - 0.2490525
            // = 0.5018950 // height

            // 1-0.7509475 = 0.2490525

            int centerX = x() + w() / 2;
            int centerY = y() + h() / 2;
            
            int image_size = std::min(w(), h());

            int outline_width = image_size;
            int outline_height = (int)(image_size * 0.501895f);

            fl_color(FL_RED);
            fl_line_style(FL_SOLID, 2);
            // very nice...n't
            fl_rect(centerX - image_size / 2, (int)((centerY - image_size / 2) + image_size * 0.2490525f), outline_width, outline_height);
            fl_line_style(0);
        }
    }
};

#endif