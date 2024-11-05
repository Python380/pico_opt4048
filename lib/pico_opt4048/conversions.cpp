#include "opt4048.h"

// Color conversion matrix from:
//   Paper: Texas Instruments, "OPT4048 High Speed High Precision Tristimulus XYZ Color Sensor"
//   Section: 9.2.4 "Application Curves"
//   Subtitle: Matrix values for color determination
//   Date updated: December 2022
const double m[4][4] = {
    { 0.0002348929920, -0.0000189652390,  0.0000120811684, 0.0000000000000 },
    { 0.0000407467441,  0.0001989582020, -0.0000158848115, 0.0021500000000 },
    { 0.0000928619404, -0.0000169739553,  0.0006740215200, 0.0000000000000 },
    { 0.0000000000000,  0.0000000000000,  0.0000000000000, 0.0000000000000 }
};

const double s[3][3] = {
    {  3.2406255, -1.5372080, -0.4986286 },
    { -0.9689307,  1.8757561,  0.0415175 },
    {  0.0557101, -0.2040211,  1.0569959 }
};

double max(double a, double b, double c) {
    // If A is greater than B, return A, otherwise B; then compare result with C
    double x = (a > b) ? a : b;
    return (x > c) ? x : c;
}

// converts a single channel of RGB to a channel of sRGB
double linearRGBToSRGB(double channel) {
    if (channel <= 0.0031308) {
        return 12.92 * channel;
    } else {
        return 1.055 * pow(channel, 1 / 2.4) - 0.055;
    }
}

color_xyz rawToXYZ(opt4048_data channels) { // , OPT4048_CONFIGURATION config
    color_xyz color;

    // Convert result (mantissa in the datasheet) & exponent to a 28-bit number
    uint32_t ch0 = channels.ch0.result << channels.ch0.exponent;
    uint32_t ch1 = channels.ch1.result << channels.ch1.exponent;
    uint32_t ch2 = channels.ch2.result << channels.ch2.exponent;
    uint32_t ch3 = channels.ch3.result << channels.ch3.exponent;

    // Matrix multiplication from TI datasheet
    // TODO SHOULD SIMPLIFY; NO REASON TO CALCULATE LUX AT SAME TIME
    color.X = ch0 * m[0][0] + ch1 * m[1][0] + ch2 * m[2][0] + ch3 * m[3][0];
    color.Y = ch0 * m[0][1] + ch1 * m[1][1] + ch2 * m[2][1] + ch3 * m[3][1];
    color.Z = ch0 * m[0][2] + ch1 * m[1][2] + ch2 * m[2][2] + ch3 * m[3][2];
    color.l = ch0 * m[0][3] + ch1 * m[1][3] + ch2 * m[2][3] + ch3 * m[3][3];

    // TODO convert all channels to real lux and not value lux

    // Normalization for xyz from XYZ
    color.x = color.X / (color.X + color.Y + color.Z);
    color.y = color.Y / (color.X + color.Y + color.Z);
    color.z = color.Z / (color.X + color.Y + color.Z);

    return color;
}

color_rgb xyzToRGB(color_xyz input_color) {
    color_rgb color;

    // Matrix multiplication https://en.wikipedia.org/wiki/SRGB#The_sRGB_transfer_function_.28.22gamma.22.29
    color.r = s[0][0] * input_color.x + s[0][1] * input_color.y + s[0][2] * input_color.z;
    color.g = s[1][0] * input_color.x + s[1][1] * input_color.y + s[1][2] * input_color.z;
    color.b = s[2][0] * input_color.x + s[2][1] * input_color.y + s[2][2] * input_color.z;

    // Delinearlization
    color.r = linearRGBToSRGB(color.r);
    color.g = linearRGBToSRGB(color.g);
    color.b = linearRGBToSRGB(color.b);

    return color;
}

double xyzToCCT(color_xyz xyz) {
    double n = (xyz.x - 0.3320) / (0.1858 - xyz.y);
    return (437 * pow(n, 3)) + (3601 * pow(n, 2)) + (6861 * n) + 5517;
}