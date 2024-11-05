#pragma once

#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>

#include <math.h>

// I2C addresses
extern const uint8_t OPT4048_ADDR;
extern const uint8_t OPT4048_ADDR_GND;
extern const uint8_t OPT4048_ADDR_VDD;
extern const uint8_t OPT4048_ADDR_SDA;
extern const uint8_t OPT4048_ADDR_SCL;

// register map
extern const uint8_t OPT4048_REG_CH0;
extern const uint8_t OPT4048_REG_CH1;
extern const uint8_t OPT4048_REG_CH2;
extern const uint8_t OPT4048_REG_CH3;
extern const uint8_t OPT4048_REG_THRES;
extern const uint8_t OPT4048_REG_CFG;
extern const uint8_t OPT4048_FLAGS_REG;
extern const uint8_t OPT4048_REG_DID;

// device ID
extern const uint16_t OPT4048_DID;

// configuration options
enum opt4048_range {
    R2_2KLUX    = 0,
    R4_5KLUX    = 1,
    R9KLUX      = 2,
    R18KLUX     = 3,
    R36KLUX     = 4,
    R72KLUX     = 5,
    R144KLUX    = 6,
    R_AUTO      = 12
};

enum opt4048_conv_time {
    T600US  = 0,
    T1MS    = 1,
    T1_8MS  = 2,
    T3_4MS  = 3,
    T6_5MS  = 4,
    T12_7MS = 5,
    T25MS   = 6,
    T50MS   = 7,
    T100MS  = 8,
    T200MS  = 9,
    T400MS  = 10,
    T800MS  = 11
};

enum opt4048_operating_mode {
    POWER_DOWN                = 0,
    FORCE_AUTO_RANGE_ONE_SHOT = 1,
    ONE_SHOT                  = 2,
    CONTINUOUS                = 3
};

enum opt4048_fault_count {
    ONE_FAULT = 0,
    TWO_FAULTS = 1,
    FOUR_FAULTS = 2,
    EIGHT_FAULTS = 3
};

enum opt4048_threshold_ch {
    CH0 = 0,
    CH1 = 1,
    CH2 = 2,
    CH3 = 3
};

enum opt4048_int_cfg {
    SMBUS_ALERT                = 0,
    INT_PIN_READY_NEXT_CHANNEL = 1,
    INT_PIN_READY_ALL_CHANNELS = 3
};

// Data structures for inside the library
struct {
    uint8_t exponent; // 4 bits
    uint32_t result;  // 20 bits
    uint8_t counter;  // 4 bits
    uint8_t crc;      // 4 bits
} typedef OPT4048_CH_MAP;  // 32 bits

struct {
    uint8_t threshold_l_exp;
    uint16_t threshold_l_result;
    uint8_t threshold_h_exp;
    uint16_t threshold_h_result;
} OPT4048_THRES_MAP;  // 32 bits


// LSB & MSB in all structs is swapped :(
// TODO move struct on inside to outside AKA remove union
// create unpack and pack functions for each struct
struct {
    bool QWAKE;
    opt4048_range RANGE;
    opt4048_conv_time CONVERSION_TIME;
    opt4048_operating_mode OPERATING_MODE;
    bool LATCH;
    bool INT_POL;
    opt4048_fault_count FAULT_COUNT;
    opt4048_threshold_ch THRESHOLD_CH_SEL;
    bool INT_DIR;
    opt4048_int_cfg INT_CFG;
    bool I2C_BURST;
} typedef OPT4048_CONFIGURATION; // 32 bits

struct {
    bool OVERLOAD_FLAG;  
    bool CONVERSION_READY_FLAG;
    bool FLAG_H;
    bool FLAG_L;
} typedef OPT4048_FLAGS; // 16 bits

// data structures for outside this file 
struct {
    OPT4048_CH_MAP ch0;
    OPT4048_CH_MAP ch1;
    OPT4048_CH_MAP ch2;
    OPT4048_CH_MAP ch3;
} typedef opt4048_data;

struct {
    double X;
    double Y;
    double Z;
    double x;
    double y;
    double z;
    double l;
} typedef color_xyz;

struct {
    double r;
    double g;
    double b;
} typedef color_rgb;

struct {
    double l;
    double a;
    double b;
} typedef color_lab;

double min(double, double);
double max(double, double);
color_xyz rawToXYZ(opt4048_data);
color_rgb xyzToRGB(color_xyz);
color_lab xyzToLAB(color_xyz);
double xyzToCCT(color_xyz);

class OPT4048 {
public:
    OPT4048(i2c_inst_t*, uint8_t);

    opt4048_data read();
    color_xyz readInXYZ();
    color_rgb readInRGB();
    color_lab readInLAB();

    // standard config
    void setRange(opt4048_range);
    void setConversionTime(opt4048_conv_time);
    void setOperatingMode(opt4048_operating_mode);
    void setThresholdChannel(opt4048_threshold_ch);
    void setFaultCount(opt4048_fault_count);
    void setQuickWake(bool);
    
    // interrupts
    void setIntConfig(opt4048_int_cfg);
    void setIntPolarity(bool);
    void setIntDirection(bool);
    void setLatch(bool);

private:
    void setI2CBurst(bool);

    uint16_t readFrom(uint8_t);
    void writeTo(uint8_t, uint16_t);

    void readConfig();
    void writeConfig();

    bool crcValid(OPT4048_CH_MAP);

    uint32_t packConfig(OPT4048_CONFIGURATION);
    OPT4048_CONFIGURATION unpackConfig(uint32_t);

    OPT4048_CH_MAP unpackChannel(uint32_t);

    OPT4048_CONFIGURATION config;
    i2c_inst_t* i2cn;
    uint8_t addr;
};