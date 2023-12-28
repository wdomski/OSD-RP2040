#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"

#include "eprintfs.h"
#include "fonts.h"

#include "i2c-slave.h"

const uint LED_PIN = 25;
const uint DIM_PIN = 12;

uint32_t counter;

spi_inst_t *OSD_spi = spi1;
uint dma_spi_tx;
dma_channel_config dma_config;

// Pins
const uint8_t OSD_mosi_pin = 15; // 19- spi0
int ret;

uint8_t line_top_1[80 * 8];
uint8_t line_top_2[80 * 8];
uint8_t line_bottom_1[80 * 8];
uint8_t line_bottom_2[80 * 8];

uint8_t *line_buffers[] =
    {line_top_1, line_top_2, line_bottom_1, line_bottom_2};

uint8_t buffer[1024] = {0x55, 0xff, 0xff};

#define LINE_SATRT_OFFSET 10
volatile uint32_t line_lengths[4];
uint32_t line_starts[4] = {35 + LINE_SATRT_OFFSET, 45 + LINE_SATRT_OFFSET, 200 + LINE_SATRT_OFFSET, 210 + LINE_SATRT_OFFSET};

volatile int flag;
volatile uint32_t sync_pulse;
volatile uint32_t last_time_sync_pulse;
volatile uint32_t last_time_frame;
volatile int flag_sync_pulse;
volatile int time_passed;

volatile int line_counter;

int time_counter;

#define FRAME_GPIO_Pin 7
#define LINE_GPIO_Pin 14

#define FRAME_Event (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL)
#define LINE_Event (GPIO_IRQ_EDGE_FALL)

void isr_frame_gpio(void);

void isr_line_gpio(void);

void start_dma_transfer(void *spi, void *dma,
                        uint8_t *buffer, uint32_t length, uint32_t current_line,
                        uint32_t start_line);

void isr_dma_handler(void);

volatile int flag;

bool timer_line_callback(struct repeating_timer *t);

struct repeating_timer timer;
uint32_t previous_osd_state;

absolute_time_t current_time;
absolute_time_t last_time_osd;
absolute_time_t last_time_osd_enabled;

int main()
{
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    stdio_init_all();

    // enable OSD
    OSD_REG_OSD_ENABLED = 1;
    previous_osd_state = 1;

    // enable I2C slave
    i2c_slave_setup();

    // configure LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // configure OSD pins
    // DIM pin for darker background on screen
    gpio_init(DIM_PIN);
    // gpio_set_dir(DIM_PIN, GPIO_IN);
    // gpio_set_pulls(DIM_PIN, false, false);
    gpio_set_dir(DIM_PIN, GPIO_OUT);
    gpio_put(DIM_PIN, 0);

    // SPI configuration to dispatch lines on screen
    ret = spi_init(OSD_spi, 11 * 1000 * 1000);

    spi_set_format(OSD_spi, 8, SPI_CPOL_1, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(OSD_mosi_pin, GPIO_FUNC_SPI);

    dma_spi_tx = dma_claim_unused_channel(true);
    dma_config = dma_channel_get_default_config(dma_spi_tx);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_dreq(&dma_config, spi_get_dreq(OSD_spi, true));

    dma_channel_set_irq0_enabled(dma_spi_tx, true);
    irq_set_exclusive_handler(DMA_IRQ_0, isr_dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // line width is 50 characters
    for (int i = 0; i < 4; ++i)
    {
        line_lengths[i] = esprintf((char *)buffer,
                                   "                                                  ");
        font_print_line((uint8_t *)font_small, buffer, line_lengths[i], line_buffers[i]);
    }

    gpio_add_raw_irq_handler(FRAME_GPIO_Pin, isr_frame_gpio);
    gpio_add_raw_irq_handler(LINE_GPIO_Pin, isr_line_gpio);
    gpio_set_irq_enabled(FRAME_GPIO_Pin, FRAME_Event, true);
    gpio_set_irq_enabled(LINE_GPIO_Pin, LINE_Event, true);
    irq_set_enabled(IO_IRQ_BANK0, true);

    while (1)
    {
        current_time = get_absolute_time();
        if (absolute_time_diff_us(last_time_osd, current_time) > 100000)
        {
            last_time_osd = current_time;

            line_lengths[0] = esprintf((char *)buffer, "  BATT: %2d.%02dV RSSI: %3d AD: %3d T %02d:%02d:%02d      ",
                                       OSD_REG_BATTERY / 1000, (OSD_REG_BATTERY % 1000) / 10, OSD_REG_RSSI,
                                       OSD_REG_NAV_HOME_ANGLE_DEVIATION,
                                       OSD_REG_GPS_TIME / 10000, OSD_REG_GPS_TIME % 10000 / 100, OSD_REG_GPS_TIME % 100);
            font_print_line((uint8_t *)font_small, buffer, line_lengths[0], line_buffers[0]);

            line_lengths[2] = esprintf((char *)buffer,
                                       "  ALT: %4d.%d VEL: %3d.%d DIST: %5d.%d DIST G: %5d.%d      ",
                                       OSD_REG_GPS_ALTITUDE / 1000, (OSD_REG_GPS_ALTITUDE % 1000) / 100,
                                       OSD_REG_GPS_VELOCITY / 1000, (OSD_REG_GPS_VELOCITY % 1000) / 100,
                                       OSD_REG_GPS_DISTANCE / 1000, (OSD_REG_GPS_DISTANCE % 1000) / 100,
                                       OSD_REG_GPS_DISTANCE_ON_GROUND / 1000, (OSD_REG_GPS_DISTANCE_ON_GROUND % 1000) / 100);
            font_print_line((uint8_t *)font_small, buffer, line_lengths[2], line_buffers[2]);

            int32_t latitude = OSD_REG_GPS_LATITUDE;
            uint8_t latitude_sign = 'N';
            if (latitude < 0)
            {
                latitude_sign = 'S';
                latitude = -latitude;
            }
            int32_t longitude = OSD_REG_GPS_LONGITUDE;
            uint8_t longitude_sign = 'E';
            if (longitude < 0)
            {
                longitude_sign = 'W';
                longitude = -longitude;
            }
            line_lengths[3] = esprintf((char *)buffer,
                                       "  LAT: %2d.%06d %c LONG: %3d.%06d %c HEAD: %3d      ",
                                       latitude / 1000000, latitude % 1000000, latitude_sign,
                                       longitude / 1000000, longitude % 1000000, longitude_sign,
                                       OSD_REG_GPS_HEADING);

            font_print_line(font_small[0], buffer, line_lengths[3], line_buffers[3]);
        }

        current_time = get_absolute_time();
        if (absolute_time_diff_us(last_time_osd_enabled, current_time) > 100000)
        {
            last_time_osd_enabled = current_time;
            // change in state
            if (OSD_REG_OSD_ENABLED != previous_osd_state)
            {
                if (OSD_REG_OSD_ENABLED != 0)
                {
                    gpio_set_irq_enabled(FRAME_GPIO_Pin, FRAME_Event, true);
                    gpio_set_irq_enabled(LINE_GPIO_Pin, LINE_Event, true);
                }
                else
                {
                    gpio_set_irq_enabled(FRAME_GPIO_Pin, FRAME_Event, false);
                    gpio_set_irq_enabled(LINE_GPIO_Pin, LINE_Event, false);
                    gpio_set_dir(OSD_mosi_pin, GPIO_IN);
                    gpio_disable_pulls(OSD_mosi_pin);
                    gpio_set_function(OSD_mosi_pin, GPIO_FUNC_NULL);
                }
                previous_osd_state = OSD_REG_OSD_ENABLED;
            }
        }
    }
}

bool timer_line_callback(struct repeating_timer *t)
{
    gpio_xor_mask(1 << LED_PIN);
    uint8_t state = gpio_get(LINE_GPIO_Pin);
    ++line_counter;

    for (int i = 0; i < 4; ++i)
    {
        if (line_counter >= line_starts[i] && line_counter < line_starts[i] + 8)
        {
            // the order is no coincidence
            // first start DMA transfer and then use dimming
            // otherwise the sync can be lost
            start_dma_transfer(OSD_spi, (void *)dma_spi_tx, line_buffers[i],
                               line_lengths[i], line_counter, line_starts[i]);
        }
        else
        {
            // gpio_set_dir(DIM_PIN, GPIO_IN);
        }
    }
    return true;
}

void isr_frame_gpio(void)
{
    if (gpio_get_irq_event_mask(FRAME_GPIO_Pin) & FRAME_Event)
    {
        gpio_acknowledge_irq(FRAME_GPIO_Pin, FRAME_Event);
        if (line_counter >= 150)
        {
            line_counter = 0;
        }
    }
}

void isr_line_gpio(void)
{
    if (gpio_get_irq_event_mask(LINE_GPIO_Pin) & LINE_Event)
    {
        gpio_acknowledge_irq(LINE_GPIO_Pin, LINE_Event);

        gpio_xor_mask(1 << LED_PIN);
        uint8_t state = gpio_get(LINE_GPIO_Pin);
        ++line_counter;

        for (int i = 0; i < 4; ++i)
        {
            if (line_counter >= line_starts[i] && line_counter < line_starts[i] + 8)
            {
                // the order is no coincidence
                // first start DMA transfer and then use dimming
                // otherwise the sync can be lost
                start_dma_transfer(OSD_spi, (void *)dma_spi_tx, line_buffers[i],
                                   line_lengths[i], line_counter, line_starts[i]);
            }
            else
            {
                // gpio_set_dir(DIM_PIN, GPIO_IN);
            }
        }
    }
}

void start_dma_transfer(void *spi, void *dma,
                        uint8_t *buffer, uint32_t length, uint32_t current_line,
                        uint32_t start_line)
{
    if (!dma_channel_is_busy(dma_spi_tx))
    {
        // enable dim background
        // gpio_set_dir(DIM_PIN, GPIO_OUT);
        // gpio_put(DIM_PIN, 0);
        gpio_put(DIM_PIN, 0);

        gpio_set_function(OSD_mosi_pin, GPIO_FUNC_SPI);
        dma_channel_configure((uint)dma, &dma_config, &spi_get_hw((spi_inst_t *)spi)->dr, (buffer + length * (current_line % start_line)), length, true);
    }
}

void isr_dma_handler(void)
{
    dma_hw->ints0 = 1u << dma_spi_tx;
    // disable dim GPIO to remove background
    // gpio_set_dir(DIM_PIN, GPIO_IN);
    gpio_put(DIM_PIN, 1);

    gpio_set_dir(OSD_mosi_pin, GPIO_IN);
    gpio_disable_pulls(OSD_mosi_pin);
    gpio_set_function(OSD_mosi_pin, GPIO_FUNC_NULL);

    flag = 0;
}
