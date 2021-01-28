/**
 * Estevan Seyfried:    estevans
 * Maxime Sutters:      msutters
 */

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
// Answer: Highpass is used to find edges, which rely on imformation from more than one channel, therefore
// we should not preserve and collapse the info into one channel since we only care about the difference and not color. 
//
// Sharpen is a combination of highpass and identity, and is used mainly for image manipulation and visual effect,
// we usually preserve to keep the color.
//
// Emboss does not have much use for computer vision and is used as a visual effect, so we usually preserve to keep the color. 

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: We should apply clamp post-proccessing for all the above filters since the convolving operation may 
// result in values outside the allowable range. 

image make_gaussian_filter(float sigma)
{
    // calculate size, ensure it is an odd integer
    int size = 6 * sigma;
    if (size % 2 == 0) size += 1;
    image im = make_image(size, size, 1);

    // we calculate around the center, so get offset coords
    tuple* offCords = filterCords(im);
    for (int i = 0; i < size * size; i++) {
        float x = offCords[i].x;
        float y = offCords[i].y;
        float num = exp(-1.0f * (x * x + y * y) / (2.0f * sigma * sigma));
        float den = TWOPI * sigma * sigma;
        // TODO: this is bad, we should be abstracting and using 
        // set_pixel() instead of modifying contents directly...
        im.data[i] = num / den;
    }

    free(offCords);

    l1_normalize(im);
    return im;
}

image add_image(image a, image b)
{
    // Make sure same size
    assert(a.h == b.h && a.w == b.w && a.c == b.c);
    image im = make_image(a.w, a.h, a.c);
    int size = a.w * a.h * a.c;
    for (int i = 0; i < size; i++) {
        im.data[i] = a.data[i] + b.data[i];
    }
    return im;
}

image sub_image(image a, image b)
{
    // Make sure same size
    assert(a.h == b.h && a.w == b.w && a.c == b.c);
    image im = make_image(a.w, a.h, a.c);
    int size = a.w * a.h * a.c;
    for (int i = 0; i < size; i++) {
        im.data[i] = a.data[i] - b.data[i];
    }
    return im;
}

image make_gx_filter()
{
    image im = make_image(3, 3, 1);
    im.data[0] = -1.0f;
    im.data[2] = 1.0f;
    im.data[3] = -2.0f;
    im.data[5] = 2.0f;
    im.data[6] = -1.0f;
    im.data[8] = 1.0f;
    return im;
}

image make_gy_filter()
{
    image im = make_image(3, 3, 1);
    im.data[0] = -1.0f;
    im.data[1] = -2.0f;
    im.data[2] = -1.0f;
    im.data[6] = 1.0f;
    im.data[7] = 2.0f;
    im.data[8] = 1.0f;
    return im;
}

void feature_normalize(image im)
{
    assert(im.w > 0 && im.h > 0 && im.c > 0);
    // find min and max
    float min = im.data[0];
    float max = im.data[0];
    int size = im.w * im.h * im.c;
    for (int i = 0; i < size; i++) {
        min = MIN(min, im.data[i]);
        max = MAX(max, im.data[i]);
    }
    float range = max - min;
    for (int i = 0; i < size; i++) {
        im.data[i] = (range == 0.0f) ? 0.0f : (im.data[i] - min) / range;
    }
}

image *sobel_image(image im)
{
    // allocate space for images
    image* result = calloc(2, sizeof(image));
    // result store for gradient magnitude
    result[0] = make_image(im.w, im.h, 1);
    // result store for gradient direction
    result[1] = make_image(im.w, im.h, 1);

    image gx = convolve_image(im, make_gx_filter(), 0);
    image gy = convolve_image(im, make_gy_filter(), 0);

    // note: only returning one channel of data, so don't need im.c
    int size = gx.w * gx.h * gx.c; // = gy.w * gy.h * gy.c
    for (int i = 0; i < size; i++) {
        result[0].data[i] = sqrtf(powf(gx.data[i], 2) + powf(gy.data[i], 2));
        result[1].data[i] = atan2f(gy.data[i], gx.data[i]);
    }

    return result;
}

image colorize_sobel(image im)
{
    // get sobel image
    image* sim = sobel_image(im);

    // normalize channels of sobel image
    feature_normalize(sim[0]);
    feature_normalize(sim[1]);

    // note: want to manipulate RGB values, so 3 channels in the result image
    image result = make_image(sim[0].w, sim[0].h, 3);
    for (int x = 0; x < result.w; x++) {
        for (int y = 0; y < result.h; y++) {
            // calculate and set hue with angle (note: angle is same thing as gradient direction)
            float hue = get_pixel(sim[1], x, y, 0);
            set_pixel(result, x, y, 0, hue);
            // calculate and set saturation and value (note: satVal comes from magnitude)
            float satVal = get_pixel(sim[0], x, y, 0);
            set_pixel(result, x, y, 1, satVal);
            set_pixel(result, x, y, 2, satVal);
        }
    }
    free(sim);

    hsv_to_rgb(result);

    image f = make_gaussian_filter(1);
    image blurred_result = convolve_image(result, f, 1); 
    blurred_result = add_image(add_image(blurred_result, result), result);
    clamp_image(blurred_result);

    return blurred_result;
}
