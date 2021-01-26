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
    // Find the sum of all values in im for each channel
    int x, y, c;
    float sum[3] = {0, 0, 0};
    for (c = 0; c < im.c; c++) {
        for (x = 0; x < im.w; x++) {
            for (y = 0; y < im.h; y++) {
                sum[c] += get_pixel(im, x, y, c);
            }
        }
    }
    // now divide each value by the sum for that channel
    for (c = 0; c < im.c; c++) {
        for (x = 0; x < im.w; x++) {
            for (y = 0; y < im.h; y++) {
                float val = get_pixel(im, x, y, c);
                set_pixel(im, x, y, c, val / sum[c]);
            }
        }
    }
}

image make_box_filter(int w)
{
    image im = make_image(w, w, 1);
    int x, y;
    // fill with 1's then normalize
    for (y = 0; y < im.h; y++) {
        for (x = 0; x < im.w; x++) {
            set_pixel(im, x, y, 0, 1.0f);
        }
    }
    l1_normalize(im);
    return im;
}

image convolve_image(image im, image filter, int preserve)
{
    // assert the method is called correctly
    assert(filter.c == 1 || filter.c == im.c);
    int x, y, c;
    c = preserve ? im.c : 1;

    // Setup output and cords for filter offsets
    tuple* fCords = filterCords(filter);
    image output = make_image(im.w, im.h, c);

    // Apply filter to each pixel in im
    for (x = 0; x < output.w; x++) {
        for (y = 0; y < output.h; y++) {
            for (c = 0; c < im.c; c++) {
                // if only 1 filter channel use 0 else stay in sync
                int filterChannel = filter.c ? 0 : c;
                float finalPix = convolvePixel(im, filter, x, y, filterChannel, c, fCords);
                if (preserve) {
                    set_pixel(output, x, y, c, finalPix);
                } else {
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
    image im = make_image(3,3,1);
    set_pixel(im, 1, 0, 0, -1);
    set_pixel(im, 0, 1, 0, -1);
    set_pixel(im, 1, 1, 0, 5);
    set_pixel(im, 2, 1, 0, -1);
    set_pixel(im, 1, 2, 0, -1);
    return im;
}

image make_emboss_filter()
{
    image im = make_image(3,3,1);
    set_pixel(im, 0, 0, 0, -2);
    set_pixel(im, 1, 0, 0, -1);
    set_pixel(im, 0, 1, 0, -1);
    set_pixel(im, 1, 1, 0, 1);
    set_pixel(im, 2, 1, 0, 1);
    set_pixel(im, 1, 2, 0, 1);
    set_pixel(im, 2, 2, 0, 2);
    return im;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: TODO

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: TODO

image make_gaussian_filter(float sigma)
{
    // calculate size, ensure it is an odd integer
    int size = 6 * sigma;
    if (size % 2 == 0) size += 1;
    image im = make_image(size, size, 1);

    // we calculate around the center, so get offset coords
    tuple* offCords = filterCords(im);
    printf("%d, %d\n", offCords[0].x, offCords[0].y);
    for (int i = 0; i < size * size; i++) {
        float x = offCords[i].x;
        float y = offCords[i].y;
        float num = exp(-1.0f * (x * x + y * y) / (2.0f * sigma * sigma));
        float den = TWOPI * sigma * sigma;
        // TODO: this is bad, we should be abstracting and using 
        // set pixel instead of modifying contents directly...
        im.data[i] = num / den;
    }
    l1_normalize(im);
    return im;
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
