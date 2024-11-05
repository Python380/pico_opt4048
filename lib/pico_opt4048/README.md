# Pico OPT4048 library

- `OPT4048` class
    - Constructor: `OPT4048(i2c_inst_t i2c, uint8_t address)` - Takes the I2C system device and address of the OPT4048 & initializes the OPT4048 sensor. Will enter a death loop if initialization fails.
    - `opt4048_data read()` - Reads the most basic data possible, returning raw data. This needs processed using conversion functions listed later.
    - `color_xyz readInXYZ()` - Reads the data in CIE xyz. This returns color as CIExyz and brightness in lux.
    - `color_rgb readInRGB()` - Reads the data in CIE sRGB. This is standard RGB - NOTE: data returned ranges from 0-1 & is a double. Further conversion required to get to 8-bit or 12-bit color.
    - `set*(___)` - sets the specified configuration field to the specified value. See TI datasheet linked below.
- `color_xyz rawToXYZ(opt4048_data)` - takes raw data returned from `OPT4048::read()` and converts it to CIE XYZ. This data is the input for all other conversion functions.
- `color_rgb xyzToRGB(color_xyz)` - Converts XYZ color to RGB color.

### Texas Instruments - "OPT4048 High Speed High Precision Tristimulus XYZ Color Sensor"
https://www.ti.com/lit/gpn/OPT4048
