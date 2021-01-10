#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

// Helper Methods
int get_index(image im, int x, int y, int c);

float get_pixel(image im, int x, int y, int c)
{
    // Do bound checking on c?
    if (c < 0 || c >= im.c) return 0;

    // Use Clamp padding
    x = MIN(MAX(0,x), im.w - 1);
    y = MIN(MAX(0,y), im.h -1);

    return im.data[get_index(im, x, y, c)];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    // Check bounds
    if (x < 0 || x >= im.w || y < 0 || y >= im.h || c < 0 || c >= im.c) return;

    // modify value
    im.data[get_index(im, x, y, c)] = v;
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    // use memcpy to copy data array
    memcpy(copy.data, im.data, im.w * im.h * im.c * sizeof(float));

    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);
    int x, y;
    for (x = 0; x < im.w; x++) 
    {
        for (y = 0; y < im.h; y++)
        {
            // using fomula Y' = 0.299 R' + 0.587 G' + .114 B'
            float r = get_pixel(im, x, y, 0);
            float g = get_pixel(im, x, y, 1);
            float b = get_pixel(im, x, y, 2);
            set_pixel(gray, x, y, 0, 0.299 * r + 0.587 * g + 0.114 * b);
        }
    }
    return gray;
}

void shift_image(image im, int c, float v)
{
    // validate parameters?
    assert(c >= 0 && c < im.c);
    int x, y;
    for (x = 0; x < im.w; x++)
    {
        for (y = 0; y < im.h; y++)
        {
            float old = get_pixel(im, x, y, c);
            set_pixel(im, x, y, c, old + v);
        }
    }
}

void clamp_image(image im)
{
    int x, y, c;
    for (c = 0; c < im.c; c++)
    {
        for (x = 0; x < im.w; x++)
        {
            for (y = 0; y < im.h; y++)
            {
                float old = get_pixel(im, x, y, c);
                set_pixel(im, x, y, c, MIN(MAX(0.0, old), 1.0));
            }
        }
    }
}


// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im)
{
    // TODO Fill this in
}

void hsv_to_rgb(image im)
{
    // TODO Fill this in
}

// Index Helper function, 
// finds the index given an image, x and y
int get_index(image im, int x, int y, int c)
{
    return x + im.w * y + im.w * im.h * c;
}
