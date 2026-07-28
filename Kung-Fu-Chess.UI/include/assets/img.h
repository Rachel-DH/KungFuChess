#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <filesystem>

class Img {
public:
    Img();
    
    // Loads the image at path, optionally resizing to size (keep_aspect fits the longer side); returns *this for chaining.
    Img& read(const std::string& path,
              const std::pair<int, int>& size = {},
              bool keep_aspect = false,
              int interpolation = cv::INTER_AREA);
    
    // Draws this image onto other_img with its top-left corner at (x, y).
    void draw_on(Img& other_img, int x, int y);
    
    // Draws txt with its baseline at (x, y), scaled by font_size.
    void put_text(const std::string& txt, int x, int y, double font_size,
                  const cv::Scalar& color = cv::Scalar(255, 255, 255, 255),
                  int thickness = 1);

    // Draws an unfilled rectangle outline inset to stay fully within (x, y, width, height); OpenCV centers stroke thickness on the path, so this insets by thickness/2 first to avoid bleeding past the given bounds.
    void draw_rect(int x, int y, int width, int height,
                    const cv::Scalar& color, int thickness = 2);
    
    // Displays the image in its own window.
    void show();

    // Returns a deep copy of this image; mutating the copy (e.g. via draw_on) never
    // affects the original, unlike a plain copy which would share the underlying cv::Mat.
    Img clone() const;

    // Returns the underlying OpenCV Mat.
    const cv::Mat& get_mat() const { return img; }

    // True once an image has been successfully read.
    bool is_loaded() const { return !img.empty(); }

private:
    cv::Mat img;
}; 