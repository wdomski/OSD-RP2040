#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"

#include "eprintfs.h"
#include "fonts.h"
#include "fonts_utils.h"
#include "osd.h"
#include "osd-comm.h"
#include "osd-param.h"
#include "utils.h"

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

#define OSD_LINE_TOP_1 35
#define OSD_LINE_BOTTOM_1 215

uint8_t buffer[1024] = {0x55, 0xff, 0xff};

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

void start_dma_transfer_line(void *spi, void *dma,
    uint8_t *buffer, uint32_t length);

void isr_dma_handler(void);

volatile int flag;

void fill_with_blanks(uint8_t *buffer, int32_t length);

struct repeating_timer timer;
uint32_t previous_osd_state;

absolute_time_t current_time;
absolute_time_t last_time_osd;
absolute_time_t last_time_osd_enabled;
absolute_time_t last_time_stats;

uint8_t stats_counter;

int osd_params_screen_cleaned;

uint8_t osd_param_format[128];

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

    spi_set_format(OSD_spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    gpio_set_function(OSD_mosi_pin, GPIO_FUNC_SPI);

    dma_spi_tx = dma_claim_unused_channel(true);
    dma_config = dma_channel_get_default_config(dma_spi_tx);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_dreq(&dma_config, spi_get_dreq(OSD_spi, true));

    dma_channel_set_irq0_enabled(dma_spi_tx, true);
    irq_set_exclusive_handler(DMA_IRQ_0, isr_dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    gpio_add_raw_irq_handler(FRAME_GPIO_Pin, isr_frame_gpio);
    gpio_add_raw_irq_handler(LINE_GPIO_Pin, isr_line_gpio);
    gpio_set_irq_enabled(FRAME_GPIO_Pin, FRAME_Event, true);
    gpio_set_irq_enabled(LINE_GPIO_Pin, LINE_Event, true);
    irq_set_enabled(IO_IRQ_BANK0, true);

    while (1)
    {
        current_time = get_absolute_time();
        if (absolute_time_diff_us(last_time_stats, current_time) > 400000)
        {
            last_time_stats = current_time;
            printf("Frame: %d, OSD: %d\r\n Batt: %d.%02d, T %02ld:%02ld:%02ld SAT: %2d\r\n Param: %d, Index: %d, Value: %ld\r\n", 
                    stats_counter++,
                    osd_data->osd_enabled,
                    osd_data->battery / 1000, (osd_data->battery % 1000) / 10,
                    OSD_REG_GPS_TIME / 10000, OSD_REG_GPS_TIME % 10000 / 100, OSD_REG_GPS_TIME % 100,
                                    OSD_REG_GPS_SAT_NUMBER,
                   osd_data->param_config.mode, osd_data->param_config.index, osd_data->param_config.value);
        }
        current_time = get_absolute_time();
        if (absolute_time_diff_us(last_time_osd, current_time) > 100000)
        {
            last_time_osd = current_time;

            uint32_t line_length;

            line_length = esprintf((char *)buffer, "T %02d:%02d:%02d SAT: %2d",
                                    OSD_REG_GPS_TIME / 10000, OSD_REG_GPS_TIME % 10000 / 100, OSD_REG_GPS_TIME % 100,
                                    OSD_REG_GPS_SAT_NUMBER);                                
            fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
            font_print_line((uint8_t *)font_small, buffer, line_length, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, OSD_LINE_TOP_1));

            switch (osd_data->param_config.mode)
            {
            case 0:
            {
                if(osd_params_screen_cleaned == 0){
                    // clear screen for params
                    fill_with_blanks(buffer, OSD_LINE_LENGTH);
                    font_print_line((uint8_t *)font_small, buffer, OSD_LINE_LENGTH, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, 100));
                    font_print_line((uint8_t *)font_small, buffer, OSD_LINE_LENGTH, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, 110));
                    osd_params_screen_cleaned = 1;
                }
                break;
            }
            case 1:
            {
                osd_params_screen_cleaned = 0;
                if (osd_data->param_config.index < OSD_PARAM_COUNT)
                {
                    const char *param_name = osd_param_names[osd_data->param_config.index];
                    line_length = esprintf((char *)buffer, "%s (%d)  ", param_name, osd_data->param_config.index);
                    fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
                    font_print_line((uint8_t *)font_small, buffer, OSD_LINE_LENGTH, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, 100));
                    int fraction = osd_param_fraction[osd_data->param_config.index];
                    uint8_t sign = osd_data->param_config.value < 0 ? '-' : ' ';
                    if (fraction > 1)
                    {
                        uint8_t power = 1;
                        int tmp = fraction;
                        for(int i = 0; i< 5; ++i)
                        {
                            tmp /= 10;
                            if(tmp == 1){
                                break;
                            }
                            ++power;
                        }
                        esprintf((char *) osd_param_format, "     %%c%%d.%%0%dd       ", power);
                        line_length = esprintf((char *)buffer, (const char *)osd_param_format, sign, abs_int32(osd_data->param_config.value / fraction), abs_int32(osd_data->param_config.value % fraction));
                    }
                    else
                    {
                        line_length = esprintf((char *)buffer, "     %d", osd_data->param_config.value);
                    }
                    fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
                    font_print_line((uint8_t *)font_small, buffer, OSD_LINE_LENGTH, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, 110));
                }
                else
                {
                    line_length = esprintf((char *)buffer, "    PARAM NOT FOUND (%d)  ", osd_data->param_config.index);
                    fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
                    font_print_line((uint8_t *)font_small, buffer, OSD_LINE_LENGTH, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, 100));
                    line_length = 0;
                    fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
                    font_print_line((uint8_t *)font_small, buffer, OSD_LINE_LENGTH, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, 110));                    
                }

                break;
            }
            case 2:
            {
                osd_params_screen_cleaned = 0;
                if (osd_data->param_config.index < OSD_PARAM_COUNT)
                {
                    const char *param_name = osd_param_names[osd_data->param_config.index];
                    line_length = esprintf((char *)buffer, "%s (%d)  ", param_name, osd_data->param_config.index);
                    fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
                    font_print_line((uint8_t *)font_small, buffer, OSD_LINE_LENGTH, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, 100));
                    int fraction = osd_param_fraction[osd_data->param_config.index];
                    if (fraction > 1)
                    {
                        uint8_t power = 1;
                        int tmp = fraction;
                        for(int i = 0; i< 5; ++i)
                        {
                            tmp /= 10;
                            if(tmp == 1){
                                break;
                            }
                            ++power;
                        }
                        esprintf((char *) osd_param_format, ">>>  %%c%%d.%%0%dd       ", power);                        
                        uint8_t sign = osd_data->param_config.value < 0 ? '-' : ' ';
                        line_length = esprintf((char *)buffer, (const char *)osd_param_format, sign, abs_int32(osd_data->param_config.value / fraction), abs_int32(osd_data->param_config.value % fraction));
                    }
                    else
                    {
                        line_length = esprintf((char *)buffer, ">>>  %d", osd_data->param_config.value);
                    }
                    fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
                    font_print_line((uint8_t *)font_small, buffer, OSD_LINE_LENGTH, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, 110));                    
                }
                else
                {
                    line_length = esprintf((char *)buffer, "    PARAM NOT FOUND (%d)  ", osd_data->param_config.index);
                    fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
                    font_print_line((uint8_t *)font_small, buffer, OSD_LINE_LENGTH, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, 100));
                    line_length = 0;
                    fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
                    font_print_line((uint8_t *)font_small, buffer, OSD_LINE_LENGTH, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, 110));
                }
            }

            default:
                break;
            }

            line_length = esprintf((char *)buffer, "%2d.%02dV",
                                   OSD_REG_BATTERY / 1000, (OSD_REG_BATTERY % 1000) / 10);
            font_print_line_scale((uint8_t *)font_small, buffer, line_length, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS + 50, get_line(frame_buffer, 180), 2, 2);
            line_length = esprintf((char *)buffer, "R %3d ",
                                   OSD_REG_RSSI);
            font_print_line_scale((uint8_t *)font_small, buffer, line_length, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS + 50, get_line(frame_buffer, 200), 2, 2);
            line_length = esprintf((char *)buffer, "A %3d ",
                                   OSD_REG_NAV_HOME_ANGLE_DEVIATION);
            font_print_line_scale((uint8_t *)font_small, buffer, line_length, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS + 50, get_line(frame_buffer, 220), 2, 2);

            if(OSD_REG_AUTOPILOT_ENABLED == 0){
                line_length = esprintf((char *)buffer, "MODE: MANUAL               ");
            }
            else{
                line_length = esprintf((char *)buffer, "MODE: AUTO CTRL: %3d.%d", 
                                        OSD_REG_CTRL_SIGNAL / 1000, (OSD_REG_CTRL_SIGNAL % 1000) / 100);
            }
            // fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
            font_print_line((uint8_t *)font_small, buffer, line_length, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, OSD_LINE_BOTTOM_1-30));            

            line_length = esprintf((char *)buffer,
                                       "VVEL: %3d.%d ALT: %4d.%d  ",
                                       OSD_REG_VERTICAL_SPEED / 1000, (OSD_REG_VERTICAL_SPEED % 1000) / 100,
                                       OSD_REG_GPS_ALTITUDE / 1000, (OSD_REG_GPS_ALTITUDE % 1000) / 100);
            // fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
            font_print_line((uint8_t *)font_small, buffer, line_length, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, OSD_LINE_BOTTOM_1-20));            

            line_length = esprintf((char *)buffer,
                                       "VEL: %3d.%d DIST: %5d.%d DIST G: %5d.%d      ",
                                       OSD_REG_GPS_VELOCITY / 1000, (OSD_REG_GPS_VELOCITY % 1000) / 100,
                                       OSD_REG_GPS_DISTANCE / 1000, (OSD_REG_GPS_DISTANCE % 1000) / 100,
                                       OSD_REG_GPS_DISTANCE_ON_GROUND / 1000, (OSD_REG_GPS_DISTANCE_ON_GROUND % 1000) / 100);
            fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
            font_print_line((uint8_t *)font_small, buffer, line_length, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, OSD_LINE_BOTTOM_1-10));

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
            line_length = esprintf((char *)buffer,
                                       "LAT: %2d.%06d %c LONG: %3d.%06d %c HEAD: %3d   ",
                                       latitude / 1000000, latitude % 1000000, latitude_sign,
                                       longitude / 1000000, longitude % 1000000, longitude_sign,
                                       OSD_REG_GPS_HEADING);
            fill_with_blanks(buffer + line_length, OSD_LINE_LENGTH - line_length);
            font_print_line(font_small[0], buffer, line_length, OSD_LINE_LENGTH, FRAME_BUFFER_X_OFFSET_CHARS, get_line(frame_buffer, OSD_LINE_BOTTOM_1));
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

        if(line_counter >= FRAME_BUFFER_Y_OFFSET_LINES && line_counter < FRAME_BUFFER_Y_OFFSET_LINES + OSD_LINES){
            int offset = (line_counter - FRAME_BUFFER_Y_OFFSET_LINES ) * OSD_LINE_LENGTH;
            start_dma_transfer_line(OSD_spi, (void *)dma_spi_tx, &frame_buffer[offset], OSD_LINE_LENGTH);
        }
        ++line_counter;
    }
}

void start_dma_transfer_line(void *spi, void *dma,
                        uint8_t *buffer, uint32_t length)
{
    if (!dma_channel_is_busy(dma_spi_tx))
    {
        // enable dim background
        // gpio_set_dir(DIM_PIN, GPIO_OUT);
        // gpio_put(DIM_PIN, 0);
        gpio_put(DIM_PIN, 0);

        gpio_set_function(OSD_mosi_pin, GPIO_FUNC_SPI);
        dma_channel_configure((uint)dma, &dma_config, &spi_get_hw((spi_inst_t *)spi)->dr, buffer, length, true);
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
