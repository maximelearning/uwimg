#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

typedef struct{
    int x,y;
} tuple;

// Helper function to create array of offset coords
tuple* filterCords(image im) {
    int size = im.w * im.h;
    tuple* res = calloc(size, sizeof(tuple));
    int maxOff = im.w / 2;
    for (int i = 0; i < size; i++) {
        res[i].x = i % im.w - maxOff;
        res[i].y = i / im.w - maxOff;
    }
    return res;
}

// helper method applys filter to one pixel
float convolvePixel(image im, image filter, int x, int y, int filterChannel, 
    int imageChannel, tuple* cords) {
    float res = 0;
    int fChanOffset = filter.h * filter.w * filterChannel;
    // find the sum at each corresponding pixel in the filter
    for (int i = 0; i < filter.w * filter.w; i++) {
        // Get the original pixel at filter location
        float orig = get_pixel(im, x + cords[i].x, y + cords[i].y, imageChannel);
        // multiply by filter
        orig *= filter.data[i + fChanOffset];
        // add to final result
        res += orig;
    }
    return res;
}

void l1_normalize(image im)
{
    // find the normal of each pixel
    int pixels = im.w * im.h;
    float normal = 1.0f / pixels;
    int x, y, c;
    for (c = 0; c < im.c; c++) {
        for (x = 0; x < im.w; x++) {
            for (y = 0; y < im.h; y++) {
                set_pixel(im, x, y, c, normal);
            }
        }
    }
}

image make_box_filter(int w)
{
    image filter = make_image(w, w, 1);
    l1_normalize(filter);
    return filter;
}

image convolve_image(image im, image filter, int preserve)
{
    // assert the method is called correctly
    assert(filter.c == 1 || filter.c == im.c);
    int x, y, c;
    if (preserve == 1) {
        // if preserve, keep channels
        c = im.c;
    } else {
        // else sum to 1 channel
        c = 1;
    }
    // Get a corresponding array of cords for the filter 
    tuple* fCords = filterCords(filter);
    // calculate for each pixel of image
    image output = make_image(im.w, im.h, c);

    for (x = 0; x < output.w; x++) {
        for (y = 0; y < output.h; y++) {
            float finalPix;
            if (preserve == 1) {
                for (c = 0; c < im.c; c++) {
                    if (filter.c == 1) {
                        // only 1 filter chanel use 1 for each pass
                        finalPix = convolvePixel(im, filter, x, y, 0, c, fCords);
                        set_pixel(output, x, y, c, finalPix);
                    } else {
                        // keep filter channel and image channel in sync
                        finalPix = convolvePixel(im, filter, x, y, c, c, fCords);
                        set_pixel(output, x, y, c, finalPix);
                    }
                }
            } else {
                // We need to output one channel
                for (c = 0; c < im.c; c++) {
                    if (filter.c == 1) {
                        // only 1 filter chanel use 1 for each pass
                        finalPix = convolvePixel(im, filter, x, y, 0, c, fCords);
                        
                    } else {
                        // keep filter channel and image channel in sync
                        finalPix = convolvePixel(im, filter, x, y, c, c, fCords);
                        set_pixel(output, x, y, c, finalPix);
                    }
                    // make_image() clears out data, so we can safely read
                    float prev = get_pixel(output, x, y, 0);
                    set_pixel(output, x, y, 0, finalPix + prev);
                }

            }
        }
    }

    free(fCords);
    return output;
}

image make_highpass_filter()
{
    image im = make_image(3,3,1);
    set_pixel(im, 1, 0, 0, -1);
    set_pixel(im, 0, 1, 0, -1);
    set_pixel(im, 1, 1, 0, 4);
    set_pixel(im, 2, 1, 0, -1);
    set_pixel(im, 1, 2, 0, -1);
    return im;
}

image make_sharpen_filter()
{
    // TODO
    return make_image(1,1,1);
}

image make_emboss_filter()
{
    // TODO
    return make_image(1,1,1);
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: TODO

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: TODO

image make_gaussian_filter(float sigma)
{
    // TODO
    return make_image(1,1,1);
}

image add_image(image a, image b)
{
    // TODO
    return make_image(1,1,1);
}

image sub_image(image a, image b)
{
    // TODO
    return make_image(1,1,1);
}

image make_gx_filter()
{
    // TODO
    return make_image(1,1,1);
}

image make_gy_filter()
{
    // TODO
    return make_image(1,1,1);
}

void feature_normalize(image im)
{
    // TODO
}

image *sobel_image(image im)
{
    // TODO
    return calloc(2, sizeof(image));
}

image colorize_sobel(image im)
{
    // TODO
    return make_image(1,1,1);
}
