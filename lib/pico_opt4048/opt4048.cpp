#include "./opt4048.h"

const uint8_t OPT4048_ADDR      = 0b1000100;
const uint8_t OPT4048_ADDR_GND  = OPT4048_ADDR;
const uint8_t OPT4048_ADDR_VDD  = 0b1000101;
const uint8_t OPT4048_ADDR_SDA  = 0b1000110;
const uint8_t OPT4048_ADDR_SCL  = 0b1000111;
const uint8_t OPT4048_REG_CH0   = 0x00;
const uint8_t OPT4048_REG_CH1   = 0x02;
const uint8_t OPT4048_REG_CH2   = 0x04;
const uint8_t OPT4048_REG_CH3   = 0x06;
const uint8_t OPT4048_REG_THRES = 0x08;
const uint8_t OPT4048_REG_CFG   = 0x0A;
const uint8_t OPT4048_FLAGS_REG = 0x0C;
const uint8_t OPT4048_REG_DID   = 0x11;
const uint16_t OPT4048_DID      = 0x0821;

const uint64_t conv_times[12] = {600, 1000, 1800, 3400, 6500, 12700, 25000, 50000, 100000, 200000, 400000, 800000};

double min(double a, double b) {
    return (a < b) ? a : b;
}

double max(double a, double b) {
    return (a > b) ? a : b;
}


OPT4048::OPT4048(i2c_inst_t* port, uint8_t address) {
    // save I2C instance for future use
    i2cn = port;
    addr = address;

    // initialize OPT4048 & read Device ID
    uint16_t device_id = readFrom(OPT4048_REG_DID);
    printf("0x%02X @ 0x%02X : 0x%04X\n", addr, OPT4048_REG_DID, device_id);

    if (device_id == OPT4048_DID) {
        printf("Identified OPT4048 successfully\n");
    } else {
        printf("Could not identify OPT4048 - entering error loop\n");
        while(1) {}
    }

    readConfig();
    setOperatingMode(CONTINUOUS);
    readConfig();
}

opt4048_data OPT4048::read() {
    opt4048_data channels;
    uint8_t channel_data[16];
    i2c_write_blocking(i2cn, addr, 0, 1, true);
    int nbytes = i2c_read_blocking(i2cn, addr, channel_data, 16, false);

#ifdef DEBUG
    printf("Bytes read: %d\n", nbytes);
#endif

    uint32_t ch0 = (channel_data[0] << 24) | (channel_data[1] << 16) | (channel_data[2] << 8) | channel_data[3];
    uint32_t ch1 = (channel_data[4] << 24) | (channel_data[5] << 16) | (channel_data[6] << 8) | channel_data[7];
    uint32_t ch2 = (channel_data[8] << 24) | (channel_data[9] << 16) | (channel_data[10] << 8) | channel_data[11];
    uint32_t ch3 = (channel_data[12] << 24) | (channel_data[13] << 16) | (channel_data[14] << 8) | channel_data[15];
    channels.ch0 = unpackChannel(ch0);
    channels.ch1 = unpackChannel(ch1);
    channels.ch2 = unpackChannel(ch2);
    channels.ch3 = unpackChannel(ch3);

#ifdef DEBUG
    printf("ALL: 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", channel_data[0], channel_data[1], channel_data[2], channel_data[3], channel_data[4], channel_data[5], channel_data[6], channel_data[7], channel_data[8], channel_data[9], channel_data[10], channel_data[11], channel_data[12], channel_data[13], channel_data[14], channel_data[15]);
    printf("CH0 exp: %d\tresult: %d\tcounter: %d\tcrc: %d\n", channels.ch0.exponent, channels.ch0.result, channels.ch0.counter, channels.ch0.crc);
    printf("CH1 exp: %d\tresult: %d\tcounter: %d\tcrc: %d\n", channels.ch1.exponent, channels.ch1.result, channels.ch1.counter, channels.ch1.crc);
    printf("CH2 exp: %d\tresult: %d\tcounter: %d\tcrc: %d\n", channels.ch2.exponent, channels.ch2.result, channels.ch2.counter, channels.ch2.crc);
    printf("CH3 exp: %d\tresult: %d\tcounter: %d\tcrc: %d\n", channels.ch3.exponent, channels.ch3.result, channels.ch3.counter, channels.ch3.crc);

    printf("\n");
#endif

    return channels;
}

color_xyz OPT4048::readInXYZ() {
    color_xyz color = rawToXYZ(read());

#ifdef DEBUG
    printf("[%02.3f, %02.3f, %02.3f, %02.3f]\n\n", color.x, color.y, color.z, color.l);
#endif

    return color;
}

color_rgb OPT4048::readInRGB() {
    color_rgb rgb = xyzToRGB(readInXYZ());

#ifdef DEBUG
    printf("[%02.3f, %02.3f, %02.3f]\t", rgb.r, rgb.g, rgb.b);

    printf("#%02X%02X%02X\n\n",
        (int) max(0, min(255, (rgb.r * 255))),
        (int) max(0, min(255, (rgb.g * 255))),
        (int) max(0, min(255, (rgb.b * 255))));
#endif

    return rgb;
}

void OPT4048::readConfig() {
    uint16_t high = readFrom(OPT4048_REG_CFG);
    uint16_t low = readFrom(OPT4048_REG_CFG + 1);
    config = unpackConfig((high << 16) | low);

#ifdef DEBUG
    printf("OPT4048: read config:\n");
    printf("QWAKE:\t\t%s\n", config.QWAKE ? "true" : "false");
    printf("RANGE:\t\t0x%x\n", config.RANGE);
    printf("CONV_TIME:\t0x%x\n", config.CONVERSION_TIME);
    printf("OP_MODE:\t0x%x\n", config.OPERATING_MODE);
    printf("LATCH:\t\t%s\n", config.LATCH ? "true" : "false");
    printf("FAULT_COUNT\t0x%x\n", config.FAULT_COUNT);
    printf("INT_POL:\t%s\n", config.INT_POL ? "active high" : "active low");
    printf("INT_DIR:\t%s\n", config.INT_DIR ? "output" : "input");
    printf("INT_CFG:\t%x\n", config.INT_CFG);
    printf("I2C_BURST:\t%s\n", config.I2C_BURST ? "Enabled": "Disabled");
    printf("THRES_CH:\t%x\n\n", config.THRESHOLD_CH_SEL);
#endif
}

void OPT4048::writeConfig() {
    uint32_t data = packConfig(config);
    uint16_t high = (data >> 16) & 0xFFFF;
    uint16_t low = data & 0xFFFF;
    writeTo(OPT4048_REG_CFG, high);
    writeTo(OPT4048_REG_CFG + 1, low);
}

uint16_t OPT4048::readFrom(uint8_t reg_addr) {
    uint8_t read_data[2];
    int error = i2c_write_blocking(i2cn, addr, &reg_addr, 1, true);
    error = i2c_read_blocking(i2cn, addr, read_data, 2, false);

    uint16_t return_value = (read_data[0] << 8) | read_data[1];

    return return_value;
}

void OPT4048::writeTo(uint8_t reg_addr, uint16_t value) {
    uint8_t write_data[3] = {reg_addr, (uint8_t) ((value >> 8) & 0xFF), (uint8_t) (value & 0xFF)};
    i2c_write_blocking(i2cn, addr, write_data, 3, false);
}

bool OPT4048::crcValid(OPT4048_CH_MAP) {
    // Calculate CRC for a given channel
    return true;
}

// register pack/unpack functions
uint32_t OPT4048::packConfig(OPT4048_CONFIGURATION cfg) {
    // pack cfg into bits for I2C transmit
    uint16_t high;
    high = (((uint16_t) cfg.QWAKE & 0b1) << 15) | high;
    high = (((uint16_t) cfg.RANGE & 0b1111) << 10) | high;
    high = (((uint16_t) cfg.CONVERSION_TIME & 0b1111) << 6) | high;
    high = (((uint16_t) cfg.OPERATING_MODE & 0b11) << 4) | high;
    high = (((uint16_t) cfg.LATCH & 0b1) << 3) | high;
    high = (((uint16_t) cfg.INT_POL & 0b1) << 2) | high;
    high = ((uint16_t) cfg.FAULT_COUNT & 0b11) | high;

    uint16_t low;
    low = 0b1000000000000000;
    low = (((uint16_t) cfg.THRESHOLD_CH_SEL & 0b11) << 5) | low;
    low = (((uint16_t) cfg.INT_DIR & 0b1) << 4) | low;
    low = (((uint16_t) cfg.INT_CFG & 0b11) << 2) | low;
    low = ((uint16_t) cfg.I2C_BURST & 0b1) | low;

    return (high << 16) | low;
}

OPT4048_CONFIGURATION OPT4048::unpackConfig(uint32_t data) {
    uint16_t high, low;
    OPT4048_CONFIGURATION cfg;

    high = data >> 16;
    low = data & 0xFFFF;

    cfg.QWAKE = ((high >> 15) & 0b1);
    cfg.RANGE = (opt4048_range) ((high >> 10) & 0b1111);
    cfg.CONVERSION_TIME = (opt4048_conv_time) ((high >> 6) & 0b1111);
    cfg.OPERATING_MODE = (opt4048_operating_mode) ((high >> 4) & 0b11);
    cfg.LATCH = ((high >> 3) & 0b1);
    cfg.INT_POL = ((high >> 2) & 0b1);
    cfg.FAULT_COUNT = (opt4048_fault_count) (high & 0b11);

    cfg.THRESHOLD_CH_SEL = (opt4048_threshold_ch) ((low >> 5) & 0b11);
    cfg.INT_DIR = ((low >> 4) & 0b1);
    cfg.INT_CFG = (opt4048_int_cfg) ((low >> 2) & 0b11);
    cfg.I2C_BURST = (low & 0b1);

    return cfg;
}

OPT4048_CH_MAP OPT4048::unpackChannel(uint32_t data) {
    uint16_t low, high;
    OPT4048_CH_MAP channel;

    high = data >> 16;
    low = data & 0xFFFF;

    channel.exponent = (high >> 12) & 0xF;
    channel.result = ((high & 0xFFF) << 8) | ((low >> 8) & 0xFF);
    channel.counter = (low >> 4) & 0xF;
    channel.crc = low & 0xF;

    return channel;
}

void OPT4048::setRange(opt4048_range range) {
    config.RANGE = range;
    writeConfig();
}

void OPT4048::setConversionTime(opt4048_conv_time ctime) {
    config.CONVERSION_TIME = ctime;
    writeConfig();
}

void OPT4048::setFaultCount(opt4048_fault_count fault_c) {
    config.FAULT_COUNT = fault_c;
    writeConfig();
}

void OPT4048::setIntConfig(opt4048_int_cfg cfg) {
    config.INT_CFG = cfg;
    writeConfig();
}

void OPT4048::setIntDirection(bool dir) {
    config.INT_DIR = dir;
    writeConfig();
}

void OPT4048::setIntPolarity(bool polarity) {
    config.INT_POL = polarity;
    writeConfig();
}

void OPT4048::setLatch(bool latch) {
    config.LATCH = latch;
    writeConfig();
}

void OPT4048::setOperatingMode(opt4048_operating_mode opmode) {
    config.OPERATING_MODE = opmode;
    writeConfig();
}

void OPT4048::setQuickWake(bool qwake) {
    config.QWAKE = qwake;
    writeConfig();
}

void OPT4048::setThresholdChannel(opt4048_threshold_ch ch) {
    config.THRESHOLD_CH_SEL = ch;
    writeConfig();
}