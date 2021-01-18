#include <math.h>
#include "image.h"

float nn_interpolate(image im, float x, float y, int c)
{
    // simply round the x,y values
    return get_pixel(im, roundf(x), roundf(y), c);
}

// Helper function, calculates the coordinates and calls the passed
// Interpolation Function
image interpolate_helper(image im, int w, int h, 
    float (*func)(image, float, float, int))
{
    // Make a new image
    image resized = make_image(w, h, im.c);
    
    // what is the scale and offset?
    float scaleX = 1.0f + (2.0f * (im.w - w)) / (2.0f * w);
    float offX = (im.w - w) / ((2.0f * w));
    float scaleY = 1.0f + (2.0f * (im.h - h)) / (2.0f * h);
    float offY = (im.h - h) / ((2.0f * h));
    
    // Loop through each px
    int c, x, y;
    for (c = 0; c < resized.c; c++) {
        for (x = 0; x < resized.w; x++) {
            for (y = 0; y < resized.h; y++) {
                // call the passed interpolation function
                float nearest = func(
                    im, 
                    x * scaleX + offX,
                    y * scaleY + offY, 
                    c
                );
                set_pixel(resized, x, y, c, nearest);
            }
        }
    }
    return resized;
}

image nn_resize(image im, int w, int h)
{
    return interpolate_helper(im, w, h, &nn_interpolate);
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    // get the values for the 4 corners
    // use floor to guaraanteeeeeeeeeeee rounding down
    float xf = floor(x);
    float yf = floor(y);
    float v1 = get_pixel(im, xf, yf, c);
    float v2 = get_pixel(im, xf + 1, yf, c);
    float v3 = get_pixel(im, xf, yf + 1, c);
    float v4 = get_pixel(im, xf + 1, yf + 1, c);

    // find the distances from the given x,y to the 4 corners
    float d1 = x - xf;
    float d2 = xf + 1 - x;
    float d3 = y - yf;
    float d4 = yf + 1 - y;

    // calculate the interpolation
    return (v1 * d2 + v2 * d1) * d4 + (v3 * d2 + v4 * d1) * d3;
}

image bilinear_resize(image im, int w, int h)
{
    return interpolate_helper(im, w, h, &bilinear_interpolate);
}

