/**
 * Estevan Seyfried, eseyfried
 * Max Sutters, msutters
 */

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
    assert (c >= 0 && c < im.c);

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
    assert(im.c == 3);
    int x, y;
    for (x = 0; x < im.w; x++) {
        for (y = 0; y < im.h; y++) {
            // Get RGB vals
            float R = get_pixel(im, x, y, 0);
            float G = get_pixel(im, x, y, 1);
            float B = get_pixel(im, x, y, 2);
        
            // Get Value from Max of RGB
            float V = three_way_max(R, G, B);

            // Calculate saturation from C = V - m, S = C / V
            float m = three_way_min(R, G, B);
            float C = V - m;
            float S = C / V;

            // Calculate Hue from Relative Ratios
            float H;
            if (C == 0.0) {
                H = 0.0;
            } else if (V == R) {
                H = (G - B) / C;
            } else if (V == G) {
                H = (B - R) / C + 2;
            } else {
                H = (R - G) / C + 4;
            }

            // check if negative and loop around
            if (H < 0.0) {
                H = H / 6 + 1;
            } else {
                H = H / 6;
            }

            // assign the new values
            set_pixel(im, x, y, 0, H);
            set_pixel(im, x, y, 1, S);
            set_pixel(im, x, y, 2, V);
        }
    }
}

void hsv_to_rgb(image im)
{
    assert(im.c == 3);
    int x, y;
    for (x = 0; x < im.w; x++) {
        for (y = 0; y < im.h; y++) {
            // Get HSV vals (all fields 0 <= field <= 1)
            float H = get_pixel(im, x, y, 0);
            float S = get_pixel(im, x, y, 1);
            float V = get_pixel(im, x, y, 2);
        
            // Init RGB values
            float R = V, G = V, B = V;

            // if H is defined, find RGB values            
            if (H != 0.0) {
                float C = V * S;
                float m = V - C;

                // undo the / 6
                H *= 6.0;

                // different formulas depending on H
                if (H < 1) {
                    // V = R, B < G, B = m
                    G = H * C + m;
                    B = m;   
                } else if (H < 2){
                    // V = G, B < R, B = m
                    R = (2 - H) * C + m;
                    B = m;
                } else if (H < 3) {
                    // V = G, R < B, R = m
                    B = (H - 2) * C + m;
                    R = m;
                } else if (H < 4) {
                    // V = B, R < G, R = m
                    G = (4 - H) * C + m;
                    R = m;
                } else if (H < 5) {
                    // V = B, G < R, G = m
                    R = (H - 4) * C + m;
                    G = m;
                } else {
                    // H <= 6, V = R, G < B, G = m
                    B = m - (H - 6) * C;
                    G = m;
                }
            }
            // assign the new values
            set_pixel(im, x, y, 0, R);
            set_pixel(im, x, y, 1, G);
            set_pixel(im, x, y, 2, B);
        }
    }
}

// Index Helper function, 
// finds the index given an image, x and y
int get_index(image im, int x, int y, int c)
{
    return x + im.w * y + im.w * im.h * c;
}
