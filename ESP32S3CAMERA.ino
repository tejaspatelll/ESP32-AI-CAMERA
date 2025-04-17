/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */
#include "esp_camera.h"
#include "esp_sleep.h" // Include for deep sleep functionality
#include "driver/rtc_io.h" // Include for RTC IO control
#include <lvgl.h>
#include <demos/lv_demos.h>
#include <Arduino_GFX_Library.h>
#include "bsp_cst816.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>  // Add for HTTPS support
#include <qrcode.h>
#include "esp_wifi.h"
#include "secrets.h" // <-- Contains WiFi, API keys, and other secrets. DO NOT COMMIT THIS FILE.


// Root certificate for api.imgur.com
// Let's Encrypt R11 Certificate
const char* imgur_root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIGIzCCBQugAwIBAgISBg+1wLip2QRa8ioFDLgwY9FdMA0GCSqGSIb3DQEBCwUA\n" \
"MDMxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQwwCgYDVQQD\n" \
"EwNSMTEwHhcNMjUwMzIyMTM0OTAzWhcNMjUwNjIwMTM0OTAyWjAcMRowGAYDVQQD\n" \
"ExFhcGlkb2NzLmltZ3VyLmNvbTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoC\n" \
"ggIBAJMWhhZyd9iI5MdUJMkH4l9MMqosv088/joLdLU0kN2B9Qtk9GD2FbUrdKHN\n" \
"Fiau+8OxuVB7ZoECz5dDO304F/wwx8p8WXof2UwXCos5S6bIh9MR7zT5qmZ/rOnm\n" \
"VwjeCEaNd62cU+l4p2CkRv0FjvHcF+GxZ0VLLz8MwEWUixd5dYiwoEgrbPLmb0W7\n" \
"4tVcAmB+G5HB7R30vMcKGyM+0xoI047EJMEy6Hnlgjh5RRaLtnMz/azaoIpWoyWq\n" \
"NCnoRjeVKNQgQpp2sF1OCcuq6efwBcXskckRuMONtsd7HyLKzevfy6qo5Kp75Wtp\n" \
"dOBhnWbthUjCahDQX3dxkz2Zr09tQ4Xzp3lHEk9yprTg4SVlttTn1ZvEzfil2tWg\n" \
"jJsoZ3pNQ7hluBF1LyA6v99qzcsabWklGQO8dRhoH9V51wXd33MPWXeUVi6JqbJD\n" \
"4KXxWqUV2LxaiYVmY8BsUErL0liyuiWJNrTaXBsr+5fDKizDVw138Xwf7ByhPD/F\n" \
"Aw8TKNKwXJsXOO82FWBlmBRX6QRptox37sQfT6HhhJ4ue4HCcB66/gbyqTcfDt+l\n" \
"A3c05QA/UQh9cDoAlV/sat5oolI0qBMdU1Up8tnAk5E8mYImn+HEr2laUBx5GuYb\n" \
"ZoeT3Q04ktZ9vnqVsJ5i5dTXSmHCkzxm4H0yfTOmKUUoqqIzAgMBAAGjggJGMIIC\n" \
"QjAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMC\n" \
"MAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFHiqXiU+B3HgpiMj2pOOy48Hc98eMB8G\n" \
"A1UdIwQYMBaAFMXPRqTq9MPAemyVxC2wXpIvJuO5MFcGCCsGAQUFBwEBBEswSTAi\n" \
"BggrBgEFBQcwAYYWaHR0cDovL3IxMS5vLmxlbmNyLm9yZzAjBggrBgEFBQcwAoYX\n" \
"aHR0cDovL3IxMS5pLmxlbmNyLm9yZy8wHAYDVR0RBBUwE4IRYXBpZG9jcy5pbWd1\n" \
"ci5jb20wEwYDVR0gBAwwCjAIBgZngQwBAgEwLgYDVR0fBCcwJTAjoCGgH4YdaHR0\n" \
"cDovL3IxMS5jLmxlbmNyLm9yZy85NC5jcmwwggEFBgorBgEEAdZ5AgQCBIH2BIHz\n" \
"APEAdwCi4wrkRe+9rZt+OO1HZ3dT14JbhJTXK14bLMS5UKRH5wAAAZW+UtLdAAAE\n" \
"AwBIMEYCIQDX9/KxYpPWDmX8YhBZxjoX3zTsRD/v0drt0N8WR8LMYQIhAMiJ1joH\n" \
"2CAwZU/RzbLY2AvlL+vn3JIHg+JtxN5cjVbkAHYAzPsPaoVxCWX+lZtTzumyfCLp\n" \
"hVwNl422qX5UwP5MDbAAAAGVvlLTBwAABAMARzBFAiEAxW/cob+O2OYF0JvBgI0Y\n" \
"67/Qokd0K2mmo0lU81XS3r8CIAomDVSGmH/QoLysm1V84x4ebuvXsOXDndkNUlR5\n" \
"uF/UMA0GCSqGSIb3DQEBCwUAA4IBAQBg3SR9vi4s9o7ndxM05j562W9JU0ekETpY\n" \
"+rRijY7NuCiEU3vxgmgtJproKFY7xddltF2lr5ZzcNzsu5CpTvkgG8t/ZdQFpc4e\n" \
"b351iRWBGIX5TVGLni7d4ijCbbsPIBDsuNwaSUSqV8I1dBVhOtJO3x5BViJM/bGI\n" \
"paEjhLqFhUlpXp4d+m0JcOwDBg9UFKiDKcR/QcaSL9rkj3Vz6rc4gHcr3pTf7L37\n" \
"vX1OvZ491oRe7hXpxp/SUGlZWFjJ5QuxW3/VwzmzmW6yoNwMgFxFt4Yrk0fXnGCR\n" \
"jzEtL1S46hOCIjoqbea37hxZZZ3nabXlvIbAf8l3gB65GS62EbiC\n" \
"-----END CERTIFICATE-----\n";

/*To use the built-in examples and demos of LVGL uncomment the includes below respectively.
 *You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for the demos `lvgl/demos` to `lvgl/src/demos`.
 Note that the `lv_examples` library is for LVGL v7 and you shouldn't install it for this version (since LVGL v8)
 as the examples and demos are now part of the main LVGL library. */

// #include <examples/lv_examples.h>
// #include <demos/lv_demos.h>

#define PWDN_GPIO_NUM 17   //power down is not used
#define RESET_GPIO_NUM -1  //software reset will be performed
#define XCLK_GPIO_NUM 8
#define SIOD_GPIO_NUM 21
#define SIOC_GPIO_NUM 16

#define Y9_GPIO_NUM 2
#define Y8_GPIO_NUM 7
#define Y7_GPIO_NUM 10
#define Y6_GPIO_NUM 14
#define Y5_GPIO_NUM 11
#define Y4_GPIO_NUM 15
#define Y3_GPIO_NUM 13
#define Y2_GPIO_NUM 12
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 4
#define PCLK_GPIO_NUM 9

#define BOOT_BUTTON_PIN GPIO_NUM_0 // BOOT button is usually GPIO 0
#define CAPTURE_BUTTON_PIN 20  // New physical capture button on PIN 20

#define EXAMPLE_PIN_NUM_LCD_SCLK 39
#define EXAMPLE_PIN_NUM_LCD_MOSI 38
#define EXAMPLE_PIN_NUM_LCD_MISO 40
#define EXAMPLE_PIN_NUM_LCD_DC 42
#define EXAMPLE_PIN_NUM_LCD_RST -1
#define EXAMPLE_PIN_NUM_LCD_CS 45
#define EXAMPLE_PIN_NUM_LCD_BL 1
#define EXAMPLE_PIN_NUM_TP_SDA 48
#define EXAMPLE_PIN_NUM_TP_SCL 47

#define LEDC_FREQ 5000
#define LEDC_TIMER_10_BIT 10

#define EXAMPLE_LCD_ROTATION 1
#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 320


/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = new Arduino_ESP32SPI(
  EXAMPLE_PIN_NUM_LCD_DC /* DC */, EXAMPLE_PIN_NUM_LCD_CS /* CS */,
  EXAMPLE_PIN_NUM_LCD_SCLK /* SCK */, EXAMPLE_PIN_NUM_LCD_MOSI /* MOSI */, EXAMPLE_PIN_NUM_LCD_MISO /* MISO */);

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_ST7789(
  bus, EXAMPLE_PIN_NUM_LCD_RST /* RST */, EXAMPLE_LCD_ROTATION /* rotation */, true /* IPS */,
  EXAMPLE_LCD_H_RES /* width */, EXAMPLE_LCD_V_RES /* height */);


/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

/*******************************************************************************
 * Please config the touch panel in touch.h
 ******************************************************************************/
// #include "touch.h"

uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;
lv_disp_draw_buf_t draw_buf;
lv_color_t *disp_draw_buf;
lv_disp_drv_t disp_drv;

static SemaphoreHandle_t lvgl_api_mux = NULL;

lv_obj_t *img_camera;
lv_obj_t *capture_btn;
lv_obj_t *status_label;

// Camera control flags
static bool is_capturing = true;  // Start with live feed

// Store the last captured frame 
static camera_fb_t *captured_pic = NULL;
static lv_img_dsc_t stored_img_dsc;
static uint8_t* rotated_display_buffer = NULL; // Buffer for rotated image in response panel

// OpenAI API configuration
const char* openai_endpoint = "https://api.openai.com/v1/chat/completions";

// Imgur image management 
String imgurDeleteHash = ""; // Store the Imgur delete hash for later deletion

// Global UI elements
lv_obj_t *response_panel;
lv_obj_t *response_label;
lv_obj_t *close_btn;
bool is_response_visible = false;

// Processing state
int processing_stage = 0; // 0 = idle, 1 = waiting for frame, 2 = processing

// Function declarations
void process_image_task(void *pvParameters);
String getOpenAIResponse(camera_fb_t *fb);
String getOpenAIResponseWithUrl(const String& imageUrl);
bool connectToWiFi();

// RTC memory to store sleep state
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR bool just_woke_up = false; // Flag to track wake-up state

// Constants for WiFi management
const int WIFI_RECHECK_INTERVAL = 5000; // 5 seconds
const int MIN_RSSI = -80;                // Minimum acceptable RSSI
const int MAX_RECONNECT_ATTEMPTS = 5;

// WiFi stability enhancement
unsigned long lastWifiCheckTime = 0;       // For the watchdog timer
bool wifiRequestInProgress = false;        // Flag to prevent multiple simultaneous requests
int consecutiveFailedRequests = 0;         // Counter for failed requests to detect persistent issues

// --- Additions for Prompt Selection ---

// Array of short names for the dropdown menu
const char* prompt_names[] = {
    "Describe Scene",
    "Identify Objects",
    "Read Text",
    "Is it Food?",
    "Pantry Check",
     "Suggest Recipe",
    "Suggest Activity",
    "Witty Roast",
    "Rate Outfit",
    "Quick Task Suggestion"
};

// Array of the corresponding full prompts for the OpenAI API
const char* prompt_texts[] = {
    // Describe Scene
    "Briefly describe the scene in this image in one or two simple sentences suitable for a basic display. Focus on the main subject.",
    // Identify Objects
    "List the main, distinct objects you see in this image. Use simple terms separated by commas. Max 5 objects.",
    // Read Text
    "Read any clear text visible in the image. If none is clear, say 'No clear text found'. Keep it short.",
    // Is it Food?
    "Analyze this image. Is the primary item likely food? Give a very short 'Yes/No' style answer and a 2-3 word reason (e.g., 'Yes, looks like fruit', 'No, looks like a tool').",
    // Pantry Check
    "Image: Identify food items. Suggest simple recipe if possible. Output: 'Pantry:[List], Recipe:[Name/None]'. Keep response very short.",
   // Suggest Recipe
    "Analyze the image and identify the main food ingredients. Suggest a simple recipe using these ingredients. Response format: 'Ingredients: [List], Recipe: [Name]'. Keep it concise, avoid emojis, no complex formatting and ensure it fits a small display.",
    // Suggest Activity
    "Look at the image and suggest a fun activity based on its content. Response format: 'Activity: [Idea]'. Ensure the suggestion is relevant, actionable, and fits a basic display without emojis or complex formatting.",
    // Witty Roast
    "Create a short, witty joke based on the image. Use a strong setup and punchline in one sentence. Keep it lighthearted, avoid emojis or special characters, and ensure it fits a small display.",
    // Rate Outfit
    "Analyze the outfit in the image and provide a rating (1 to 10) with a brief reason with suggestions for improvement (e.g., '7/10, stylish but mismatched colors'). Keep it concise and suitable for a basic display.",
    // Quick Task Suggestion
    "Based on this image, suggest one quick task to complete related to its content (e.g., 'Organize desk', 'Wash dishes'). Ensure the response is actionable, concise, and fits a small display."
};

// Calculate the number of prompts
const int num_prompts = sizeof(prompt_names) / sizeof(prompt_names[0]);

// Variable to store the index of the currently selected prompt
int current_prompt_index = 0; // Default to the first prompt

// Global variable for the dropdown widget
lv_obj_t *prompt_dropdown;

// --- End of Additions for Prompt Selection ---

bool lvgl_lock(int timeout_ms) {
  // Convert timeout in milliseconds to FreeRTOS ticks
  // If `timeout_ms` is set to -1, the program will block until the condition is met
  const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTakeRecursive(lvgl_api_mux, timeout_ticks) == pdTRUE;
}

void lvgl_unlock(void) {
  xSemaphoreGiveRecursive(lvgl_api_mux);
}


#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf) {
  Serial.printf(buf);
  Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
#else
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
#endif
  lv_disp_flush_ready(disp_drv);
}

// Global pointer to store the canvas buffer for freeing later
lv_color_t* qr_canvas_buffer = NULL;

// Function to show API response panel
void show_response_panel(const char* response_text, const String& imageUrl = "") {
    // Update info bar when showing response
    if (lvgl_lock(-1)) {
        lv_label_set_text(status_label, "AI response received!");
        lvgl_unlock();
    }
    Serial.println("Showing response panel");
    
    // Clean up any previous QR canvas buffer before creating a new panel
    if (qr_canvas_buffer != NULL) {
        // Serial.println("Freeing previous QR canvas buffer..."); // Commented out as non-essential
        heap_caps_free(qr_canvas_buffer); // Assuming heap_caps_malloc was used
        // free(qr_canvas_buffer); // Use if malloc was used
        qr_canvas_buffer = NULL;
    }
    // Clean up any previous rotated display buffer
    if (rotated_display_buffer != NULL) {
        Serial.println("Freeing previous rotated display buffer...");
        free(rotated_display_buffer);
        rotated_display_buffer = NULL;
    }
    
    if (lvgl_lock(-1)) {
        // Temporarily hide the dropdown while showing response panel
        if (prompt_dropdown != NULL && lv_obj_is_valid(prompt_dropdown)) {
            lv_obj_add_flag(prompt_dropdown, LV_OBJ_FLAG_HIDDEN);
            // Serial.println("Temporarily hiding dropdown"); // Commented out as non-essential
        }
        
        // Create response panel (container)
        response_panel = lv_obj_create(lv_scr_act());
        lv_obj_set_size(response_panel, screenWidth - 40, screenHeight - 40);
        lv_obj_align(response_panel, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(response_panel, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_radius(response_panel, 10, LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(response_panel, 10, LV_STATE_DEFAULT);
        lv_obj_set_flex_flow(response_panel, LV_FLEX_FLOW_COLUMN); // Arrange children vertically
        lv_obj_set_flex_align(response_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollbar_mode(response_panel, LV_SCROLLBAR_MODE_AUTO); // Allow scrolling if content overflows

        // Create image container for captured image - Make it SQUARE
        lv_obj_t *img_container = lv_obj_create(response_panel);
        
        // Calculate square container size based only on panel width
        int container_size = 120; // panel width - padding
        // int max_height_for_image = (lv_obj_get_height(response_panel) - 60) / 2; // Roughly half, leaving space for text/btn/QR
        // if (container_size > max_height_for_image) {
        //     container_size = max_height_for_image;
        // }
        if (container_size < 50) container_size = 50; // Minimum size

        lv_obj_set_size(img_container, container_size * 4/3, container_size); // Set square size
        lv_obj_set_style_pad_all(img_container, 0, LV_STATE_DEFAULT); // No padding within the square container
        lv_obj_set_style_radius(img_container, 5, LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(img_container, 1, LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(img_container, lv_color_hex(0xCCCCCC), LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(img_container, true, 0); // Clip content (image) to container bounds
        lv_obj_set_scrollbar_mode(img_container, LV_SCROLLBAR_MODE_OFF); // Explicitly disable scrolling for image container
        lv_obj_clear_flag(img_container, LV_OBJ_FLAG_CLICKABLE);   // Not clickable at all
        lv_obj_add_flag(img_container, LV_OBJ_FLAG_SCROLLABLE);    // LVGL docs: "Disables scrolling completely"
        lv_obj_add_flag(img_container, LV_OBJ_FLAG_GESTURE_BUBBLE); // (Optional) Don't handle gestures

        // Create image object for captured image
        lv_obj_t *captured_img = lv_img_create(img_container);
        if (captured_pic != NULL) {
            int img_w = captured_pic->width;
            int img_h = captured_pic->height;
            uint8_t* display_buf_ptr = captured_pic->buf;
            
            // --- Rotate the image for display if it's RGB565 --- 
            if (captured_pic->format == PIXFORMAT_RGB565) {
                Serial.println("Rotating image for display panel...");
                img_w = captured_pic->height; // Swapped dimensions
                img_h = captured_pic->width;
                size_t rotated_size = img_w * img_h * 2;
                rotated_display_buffer = (uint8_t*)malloc(rotated_size);
                if (rotated_display_buffer) {
                    rotate_rgb565_90ccw(captured_pic->buf, rotated_display_buffer, captured_pic->width, captured_pic->height);
                    display_buf_ptr = rotated_display_buffer; // Use the rotated buffer
                } else {
                    Serial.println("Failed to allocate memory for display rotation! Showing original.");
                    img_w = captured_pic->width; // Revert dimensions
                    img_h = captured_pic->height;
                    display_buf_ptr = captured_pic->buf; // Use original buffer
                }
            }
            // --- End Rotation --- 
              
            // Use stored_img_dsc which already has the captured image data
            stored_img_dsc.header.always_zero = 0;
            stored_img_dsc.data = display_buf_ptr; // Use potentially rotated buffer
            stored_img_dsc.header.w = img_w; // Use potentially rotated width
            stored_img_dsc.header.h = img_h; // Use potentially rotated height
            // stored_img_dsc.data_size = img_w * img_h * 2; // data_size is not strictly needed if data ptr is set
            stored_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
            
            lv_img_set_src(captured_img, &stored_img_dsc);
            
            // --- Calculate Square Crop, Zoom, and Offset --- 
            int crop_size = min(img_w, img_h);
            float zoom_factor = (float)container_size * 1.33 / crop_size;
            lv_coord_t lv_zoom = (lv_coord_t)(256 * zoom_factor);
            lv_img_set_zoom(captured_img, lv_zoom);

            // Calculate offsets to center the square crop within the container
            // Offset = container_center - (image_crop_center * zoom)
            lv_coord_t offset_x = container_size * 1.33 / 2 - (img_w / 2.0f) * zoom_factor;
            lv_coord_t offset_y = container_size / 2 - (img_h / 2.0f) * zoom_factor;

            lv_img_set_offset_x(captured_img, offset_x);
            lv_img_set_offset_y(captured_img, offset_y);
            // --- End Crop --- 

            // Align the image widget itself within the container (should be centered by default)
            lv_obj_align(captured_img, LV_ALIGN_CENTER, 0, 0);
        }

        // Create response text label (takes remaining space)
        response_label = lv_label_create(response_panel);
        lv_obj_set_width(response_label, lv_pct(100)); // Use 100% width of parent
        lv_obj_set_height(response_label, LV_SIZE_CONTENT); // Auto height based on text
        lv_label_set_long_mode(response_label, LV_LABEL_LONG_WRAP);
        
        // Set text style: color black, use default font for better character support
        lv_obj_set_style_text_color(response_label, lv_color_hex(0x000000), LV_STATE_DEFAULT);
        
        // Use a font that supports a wide range of characters
        lv_obj_set_style_text_font(response_label, &lv_font_montserrat_14, LV_STATE_DEFAULT);
        
        // Set the text
        // Serial.printf("Setting response text (first 50 chars): %.*s\n", 50, response_text); // Commented out as non-essential
        lv_label_set_text(response_label, response_text);
        
        // Add some space between label and button
        lv_obj_set_style_pad_bottom(response_label, 10, LV_STATE_DEFAULT);

        // Create close button (at the bottom)
        close_btn = lv_btn_create(response_panel);
        lv_obj_set_size(close_btn, 80, 40);
        lv_obj_t *close_label_btn = lv_label_create(close_btn);
        lv_label_set_text(close_label_btn, "Close");
        lv_obj_center(close_label_btn);
        lv_obj_add_event_cb(close_btn, close_btn_event_cb, LV_EVENT_CLICKED, NULL);
        
        // --- Add QR Code for Image URL using Canvas ---
        if (!imageUrl.isEmpty()) {
            // Serial.println("Generating QR Code..."); // Commented out as non-essential
            QRCode qrcode;
            // Allocate buffer for QR code data. Version 5 (37x37) is often enough for URLs.
            // Increase version if URLs are very long. Max version depends on library & memory.
            const int qr_version = 5;
            uint8_t qrcodeBytes[qrcode_getBufferSize(qr_version)]; 
            // Corrected call: Cast string to uint8_t* and add length
            if (qrcode_initBytes(&qrcode, qrcodeBytes, qr_version, ECC_LOW, (uint8_t*)imageUrl.c_str(), imageUrl.length()) == 0) {
                Serial.printf("QR Code Size: %d x %d\n", qrcode.size, qrcode.size);

                // Define module size (pixels per QR code module)
                const int module_size = 3; 
                // Calculate canvas size
                int qr_canvas_size = qrcode.size * module_size;
                Serial.printf("Canvas Size: %d x %d\n", qr_canvas_size, qr_canvas_size);

                // Create canvas object
                lv_obj_t * qr_canvas = lv_canvas_create(response_panel);
                if (!qr_canvas) {
                    Serial.println("Error: Failed to create QR canvas!");
                } else {
                    // Allocate buffer for the canvas - check allocation
                    // Use heap_caps_malloc for potentially larger allocations on ESP32
                    lv_color_t* cbuf = (lv_color_t*)heap_caps_malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(qr_canvas_size, qr_canvas_size), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                    if (!cbuf) {
                        Serial.println("Error: Failed to allocate buffer for QR canvas! (SPIRAM/Internal)");
                        // Try internal memory as fallback
                        cbuf = (lv_color_t*)malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(qr_canvas_size, qr_canvas_size));
                    }
                    
                    if (!cbuf) {
                         Serial.println("Error: Failed to allocate buffer for QR canvas! (All attempts failed)");
                         lv_obj_del(qr_canvas); // Clean up canvas object if buffer allocation fails
                    } else {
                        // --- Store buffer pointer globally --- 
                        qr_canvas_buffer = cbuf;
                        // --- --- 

                        lv_canvas_set_buffer(qr_canvas, cbuf, qr_canvas_size, qr_canvas_size, LV_IMG_CF_TRUE_COLOR);
                        
                        // Fill canvas background with white
                        lv_canvas_fill_bg(qr_canvas, lv_color_hex(0xFFFFFF), LV_OPA_COVER);

                        // Draw QR code modules
                        lv_draw_rect_dsc_t rect_dsc;
                        lv_draw_rect_dsc_init(&rect_dsc);
                        rect_dsc.bg_color = lv_color_hex(0x000000); // Black modules
                        rect_dsc.radius = 0;
                        rect_dsc.border_width = 0;

                        for (uint8_t y = 0; y < qrcode.size; y++) {
                            for (uint8_t x = 0; x < qrcode.size; x++) {
                                if (qrcode_getModule(&qrcode, x, y)) {
                                    // Draw a rectangle for each black module
                                    lv_canvas_draw_rect(qr_canvas, x * module_size, y * module_size, module_size, module_size, &rect_dsc);
                                }
                            }
                        }
                        // Serial.println("QR Code drawn to canvas"); // Commented out as non-essential
                        
                        // Align canvas in the flex layout (center horizontally)
                        lv_obj_align(qr_canvas, LV_ALIGN_CENTER, 0, 0); 
                        // Add padding above the canvas for spacing from the button
                        lv_obj_set_style_pad_top(qr_canvas, 15, 0); 
                    }
                }
            } else {
                 Serial.printf("Error: Failed to initialize QR code generator for URL (maybe too long?): %s\n", imageUrl.c_str());
            }
        }
        // --- End QR Code ---
        
        // Explicitly invalidate to force redraw
        if (response_label != NULL) {
          lv_obj_invalidate(response_label);
        }
        lv_obj_invalidate(response_panel);
        
        // Show the panel
        is_response_visible = true;
        
        lvgl_unlock();
        // Serial.printf("Response panel displayed, free heap: %d bytes\n", ESP.getFreeHeap()); // Commented out as non-essential
    } else {
        Serial.println("Failed to lock LVGL mutex");
    }
}

// Close button event handler
static void close_btn_event_cb(lv_event_t * e) {
    if (lvgl_lock(-1)) {
        // Show status message immediately when close is pressed
        lv_label_set_text(status_label, "Closing response panel...");
        lvgl_unlock();
    }
    if (lvgl_lock(-1)) {
        lv_label_set_text(status_label, "Closed response panel. Ready.");
        if (response_panel != NULL) {
            // ... existing code ...
            // --- Free the QR canvas buffer using the global pointer --- 
            if (qr_canvas_buffer != NULL) {
                 Serial.println("Freeing QR canvas buffer via global pointer...");
                 // Use heap_caps_free if allocated with heap_caps_malloc, otherwise use free
                 heap_caps_free(qr_canvas_buffer); 
                 // free(qr_canvas_buffer); // Use this if you are sure malloc was used
                 qr_canvas_buffer = NULL; // Reset the global pointer
            } else {
                 // This might happen if the panel was closed before QR code was generated or if buffer allocation failed
                 Serial.println("QR canvas buffer pointer was NULL during close.");
            }
            // --- --- 

            // --- Free the rotated display buffer --- 
            if (rotated_display_buffer != NULL) {
                Serial.println("Freeing rotated display buffer...");
                free(rotated_display_buffer);
                rotated_display_buffer = NULL;
            }
            // --- --- 

            // Delete the Imgur image if we have a delete hash
            if (!imgurDeleteHash.isEmpty()) {
                Serial.println("Attempting to delete Imgur image...");
                if (deleteImgurImage()) {
                    Serial.println("Successfully deleted Imgur image");
                } else {
                    Serial.println("Failed to delete Imgur image");
                }
            }

            lv_obj_del(response_panel);  // Delete the panel completely (this also deletes children like the canvas)
            response_panel = NULL;
            response_label = NULL;
            close_btn = NULL;
            is_response_visible = false;
            
            // Resume live view - the main task will handle updating the image
            is_capturing = true;
            lv_label_set_text(status_label, "Touch IMAGE to capture");

            // Return the captured frame buffer if it exists
            if (captured_pic != NULL) {
                Serial.println("Returning captured frame buffer");
                esp_camera_fb_return(captured_pic);
                captured_pic = NULL;
            }

            // Ensure the camera image object is brought to the front
            if (img_camera != NULL) {
              lv_obj_move_foreground(img_camera);
            }
            // Also ensure status label is in front
            if (status_label != NULL) {
              lv_obj_move_foreground(status_label);
            }
            
            // Ensure dropdown is visible and in front after closing response panel
            if (prompt_dropdown != NULL && lv_obj_is_valid(prompt_dropdown)) {
              lv_obj_clear_flag(prompt_dropdown, LV_OBJ_FLAG_HIDDEN);
              lv_obj_move_foreground(prompt_dropdown);
              Serial.println("Restored dropdown visibility");
            }
        }
        lvgl_unlock();
        Serial.println("Response panel closed, resuming live feed.");
    }
}

// Event handler for the prompt dropdown
static void prompt_select_event_cb(lv_event_t * e) {
    lv_obj_t * dropdown = lv_event_get_target(e);
    if (lvgl_lock(-1)) { // Lock LVGL access
        current_prompt_index = lv_dropdown_get_selected(dropdown);
        Serial.printf("Prompt selection changed to index: %d (%s)\n",
                      current_prompt_index, prompt_names[current_prompt_index]);
        lvgl_unlock(); // Unlock LVGL access
    }
}

// Event handler to stop click events from propagating through dropdown
static void dropdown_click_event_cb(lv_event_t * e) {
    // Stop event propagation to prevent triggering camera capture
    lv_event_stop_bubbling(e);
    Serial.println("Dropdown clicked, event stopped.");
}

// Update the img_click_event_cb function
static void img_click_event_cb(lv_event_t * e) {
    Serial.println("IMG CLICK EVENT TRIGGERED!");
    
    if (is_response_visible) {
        // Ignore clicks on image if response panel is shown
        Serial.println("Response visible, ignoring click on background image.");
        return;
    }
    
    // If we're in processing stage, ignore this click
    if (processing_stage > 0) {
        Serial.println("Already processing, ignoring click");
        return;
    }
    
    // --- This is the core logic triggered ONLY by a click on img_camera ---
    is_capturing = !is_capturing;
    Serial.printf("Click on image toggled capture state: %s\n", is_capturing ? "capturing" : "paused");
    
    if (!is_capturing) {
        // Just paused - update status and start capture process
        if (lvgl_lock(-1)) {
            lv_label_set_text(status_label, "Capturing image...");
            lvgl_unlock();
            Serial.println("Status changed to 'Capturing image...'");
        }
        // Set the processing stage to waiting for frame
        processing_stage = 1;
        Serial.println("Processing stage set to 1 - waiting for frame");
    } else {
        // Resuming live feed
        if (lvgl_lock(-1)) {
            lv_label_set_text(status_label, "Touch IMAGE to capture");
            lvgl_unlock();
            Serial.println("Status changed to 'Touch IMAGE to capture'");
        }
        // Free the captured frame if it exists
        if (captured_pic != NULL) {
            Serial.println("Freeing previously captured frame on resume");
            esp_camera_fb_return(captured_pic);
            captured_pic = NULL;
        }
        // Reset processing stage
        processing_stage = 0;
        Serial.println("Processing stage reset to 0");
    }
    // --- End of core logic ---
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
  uint16_t touchpad_x;
  uint16_t touchpad_y;
  bool current_touch;

  bsp_touch_read(); // Read touch data
  current_touch = bsp_touch_get_coordinates(&touchpad_x, &touchpad_y); // Get coordinates if touched

  if (current_touch) {
    data->point.x = touchpad_x;
    data->point.y = touchpad_y;
    data->state = LV_INDEV_STATE_PRESSED; // Report as pressed
  } else {
    data->state = LV_INDEV_STATE_RELEASED; // Report as released
    // Serial.println("Touch Released"); // Optional: for debugging
  }

  // Keep the logic that triggers processing if waiting for a frame
  if (processing_stage == 1 && captured_pic != NULL) {
    // Move to processing stage
    processing_stage = 2;

    // Update status
    if (lvgl_lock(-1)) {
      lv_label_set_text(status_label, "Analyzing image...");
      lvgl_unlock();
    }

    Serial.println("Starting image processing with captured frame");

    // Create a task to process the image
    TaskHandle_t processTask;
    xTaskCreate(process_image_task, "process_image", 32768, NULL, 3, &processTask);
  }
}

// Task to process the image without blocking the UI
void process_image_task(void *pvParameters) {
  Serial.printf("Processing task started, free heap: %d bytes\n", ESP.getFreeHeap());
  
  // Make sure WiFi is connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, attempting to reconnect...");
    if (connectToWiFi()) {
      Serial.println("WiFi reconnected successfully");
    } else {
      Serial.println("WiFi reconnection failed");
      if (lvgl_lock(-1)) {
    lv_label_set_text(status_label, "Error: WiFi connection failed.");
    lvgl_unlock();
}
show_response_panel("Error: WiFi connection failed. Please try again.", ""); // Pass empty URL
      processing_stage = 0;
      vTaskDelete(NULL);
      return;
    }
  }
  
  if (captured_pic != NULL) {
    // Clear any previously uploaded image first
    if (!imgurDeleteHash.isEmpty()) {
    if (lvgl_lock(-1)) {
        lv_label_set_text(status_label, "Deleting previous Imgur image...");
        lvgl_unlock();
    }
    Serial.println("Clearing previous Imgur image before new upload...");
    if (deleteImgurImage()) {
        if (lvgl_lock(-1)) {
            lv_label_set_text(status_label, "Previous Imgur image deleted!");
            lvgl_unlock();
        }
        Serial.println("Successfully cleared previous Imgur image");
    } else {
        if (lvgl_lock(-1)) {
            lv_label_set_text(status_label, "Failed to delete previous Imgur image.");
            lvgl_unlock();
        }
        Serial.println("Failed to clear previous Imgur image, continuing with new upload");
    }
}
    
    // First, prepare the image for upload (including rotation)
    if (lvgl_lock(-1)) {
        lv_label_set_text(status_label, "Preparing image for upload..."); // Update status
        lvgl_unlock();
    }
    Serial.println("Preparing image for upload...");
    String imageUrl;

    // --- START ROTATION & PREPARATION LOGIC ---
    uint8_t* buffer_to_process = captured_pic->buf;
    size_t size_to_process = captured_pic->len;
    int width_to_process = captured_pic->width;
    int height_to_process = captured_pic->height;
    pixformat_t format_to_process = captured_pic->format;
    uint8_t* temp_rotated_buffer = NULL; // To hold rotated RGB565 if needed

    // Remove rotation logic
    // --- END ROTATION & PREPARATION LOGIC ---

    // --- START UPLOAD LOGIC (using potentially rotated buffer) ---
    if (format_to_process == PIXFORMAT_JPEG) {
      // If the format is JPEG, upload directly
      if (lvgl_lock(-1)) {
        lv_label_set_text(status_label, "Uploading JPEG...");
        lvgl_unlock();
      }
      imageUrl = uploadImageToImgur(buffer_to_process, size_to_process);
    } else if (format_to_process == PIXFORMAT_RGB565) {
      // Convert the RGB565 to JPEG first
      uint8_t *jpg_buf = NULL;
      size_t jpg_size = 0;
      
      if (lvgl_lock(-1)) {
        lv_label_set_text(status_label, "Converting to JPEG...");
        lvgl_unlock();
      }
      Serial.println("Converting RGB565 to JPEG...");
      // Use the original buffer dimensions here
      if (fmt2jpg(buffer_to_process, size_to_process, width_to_process, height_to_process, 
                  PIXFORMAT_RGB565, 90, &jpg_buf, &jpg_size)) {
        
        Serial.printf("Converted to JPEG: %d bytes\n", jpg_size);
        if (lvgl_lock(-1)) {
          lv_label_set_text(status_label, "Uploading...");
          lvgl_unlock();
        }
        imageUrl = uploadImageToImgur(jpg_buf, jpg_size);
        free(jpg_buf); // Free the JPEG buffer after uploading
      } else {
        Serial.println("Failed to convert to JPEG");
        if (lvgl_lock(-1)) {
          lv_label_set_text(status_label, "Error: Failed to convert image to JPEG.");
          lvgl_unlock();
        }
        show_response_panel("Error: Failed to convert image to JPEG format.", ""); // Pass empty URL
        processing_stage = 0;
        vTaskDelete(NULL);
        return;
      }
    } else {
         Serial.printf("Unsupported format for upload: %d\n", format_to_process);
         if (lvgl_lock(-1)) {
             lv_label_set_text(status_label, "Error: Unsupported image format.");
             lvgl_unlock();
         }
         show_response_panel("Error: Unsupported image format for upload.", "");
         processing_stage = 0;
         vTaskDelete(NULL);
         return;
    }

    // --- Free the temporary rotation buffer if it was allocated ---
    if (temp_rotated_buffer) {
        free(temp_rotated_buffer);
        temp_rotated_buffer = NULL; // Prevent double free
    }
    // --- END UPLOAD LOGIC ---
    
    if (imageUrl.length() > 0) {
      // Successfully uploaded to Imgur, now send to OpenAI
      Serial.printf("Image uploaded to Imgur: %s\n", imageUrl.c_str());
if (lvgl_lock(-1)) {
    lv_label_set_text(status_label, "Image uploaded to Imgur!");
    lvgl_unlock();
}
// Send the image URL to OpenAI for analysis
String response = getOpenAIResponseWithUrl(imageUrl);
    Serial.printf("API response received, free heap: %d bytes\n", ESP.getFreeHeap());
    
    // Print full response to terminal
    Serial.println("API Response:");
    Serial.println(response);
    
    // Truncate the response if it's too long to avoid display issues
    const int MAX_DISPLAY_LENGTH = 1024;
    String displayResponse = response;
    
    if (displayResponse.length() > MAX_DISPLAY_LENGTH) {
      displayResponse = displayResponse.substring(0, MAX_DISPLAY_LENGTH) + "...\n(Response truncated - see serial output for full text)";
    }
    
    // Check if response is an error message
    if (response.startsWith("Error:") || response.startsWith("API Error:")) {
        // For error messages, just show them directly, but still show QR code
        show_response_panel(response.c_str(), imageUrl);
    } else {
        // For successful responses, use the (potentially truncated) version and show QR code
        show_response_panel(displayResponse.c_str(), imageUrl);
      }
    } else {
      // Failed to upload to Imgur
      Serial.println("Failed to upload image to Imgur");
      if (lvgl_lock(-1)) {
    lv_label_set_text(status_label, "Error: Failed to upload image to Imgur.");
    lvgl_unlock();
}
show_response_panel("Error: Failed to upload image to Imgur. Please check your network connection and try again.", ""); // Pass empty URL
    }
  } else {
    Serial.println("No captured frame available for processing");
    if (lvgl_lock(-1)) {
      lv_label_set_text(status_label, "Error: No image captured");
      lvgl_unlock();
    }
  }
  
  // Reset the processing stage to allow new captures
  processing_stage = 0;
  
  Serial.printf("Processing task completed, free heap: %d bytes\n", ESP.getFreeHeap());
  vTaskDelete(NULL);
}

// Helper function to rotate an RGB565 image buffer 90° counterclockwise
void rotate_rgb565_90ccw(const uint8_t* src, uint8_t* dst, int src_w, int src_h) {
    // src: original buffer (src_w x src_h)
    // dst: rotated buffer (src_h x src_w)
    // Each pixel is 2 bytes (RGB565)
    for (int y = 0; y < src_h; ++y) {
        for (int x = 0; x < src_w; ++x) {
            int src_idx = (y * src_w + x) * 2;
            int dst_x = y;
            int dst_y = src_w - 1 - x;
            int dst_idx = (dst_y * src_h + dst_x) * 2;
            dst[dst_idx] = src[src_idx];
            dst[dst_idx + 1] = src[src_idx + 1];
        }
    }
}

void lvgl_camera_ui_init(lv_obj_t *parent) {
    img_camera = lv_img_create(parent);
    // Set image object size to match display dimensions (LVGL will scale the source)
    lv_obj_set_size(img_camera, 480, 320); // 320x240 for landscape
// Rotate the image widget content itself 270 degrees (90 CCW)
    lv_img_set_angle(img_camera, 2700);
    lv_img_set_pivot(img_camera, 480 / 2, 320 / 2); // Center pivot for rotation

    // Ensure no zoom distortion initially (1.0x zoom)
    // This makes the 320px width of the rotated image match the 320px widget width.
    lv_img_set_zoom(img_camera, 256); // 256 = 1.0x (no zoom)

    // --- FIX: Add explicit offsets to center the vertically oversized image ---
    // Widths match (320px), so X offset is 0.
    lv_img_set_offset_x(img_camera, 0);
    // Rotated image height (480) > widget height (240). Shift image up by half the difference.
    // Offset = -(rotated_height - widget_height) / 2 = -(480 - 240) / 2 = -120
    lv_img_set_offset_y(img_camera, -120);
    // --- End FIX ---

    // Center the widget on the screen (this aligns the 320x240 widget itself)
    lv_obj_align(img_camera, LV_ALIGN_CENTER, 0, 0);

    // Make sure the image is not affected by layout
    lv_obj_add_flag(img_camera, LV_OBJ_FLAG_FLOATING);
    lv_obj_set_scroll_dir(parent, LV_DIR_NONE); // Assuming parent is lv_scr_act()
    lv_obj_set_style_pad_all(img_camera, 0, LV_PART_MAIN); // Remove padding

    // Explicitly make the camera image clickable
    lv_obj_add_flag(img_camera, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(img_camera, LV_OBJ_FLAG_IGNORE_LAYOUT);
    // Add a click event to the image
    lv_obj_add_event_cb(img_camera, img_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(img_camera, img_click_event_cb, LV_EVENT_PRESSED, NULL);  // Also respond to press events
    
    // Add a status label
    status_label = lv_label_create(parent);
    lv_label_set_text(status_label, "Touch IMAGE to capture");
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(status_label, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(status_label, 150, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(status_label, 5, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(status_label, 5, LV_STATE_DEFAULT);

    // --- Additions for Prompt Selection UI ---
    prompt_dropdown = lv_dropdown_create(parent);
    lv_obj_set_width(prompt_dropdown, lv_pct(80)); // Set width to 80% of parent (screen)
    lv_obj_align(prompt_dropdown, LV_ALIGN_BOTTOM_MID, 0, -15); // Move it a bit up from bottom to not block too much of the camera
 
    // Create a single string with options separated by newline
    String dropdown_options = "";
    for (int i = 0; i < num_prompts; i++) {
        dropdown_options += prompt_names[i];
        if (i < num_prompts - 1) {
            dropdown_options += "\n";
        }
    }
    lv_dropdown_set_options(prompt_dropdown, dropdown_options.c_str());

    // Set the default selected option
    lv_dropdown_set_selected(prompt_dropdown, current_prompt_index);

    // Add an event callback function when the selection changes
    lv_obj_add_event_cb(prompt_dropdown, prompt_select_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Add click event handler to prevent event propagation
    lv_obj_add_event_cb(prompt_dropdown, dropdown_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(prompt_dropdown, dropdown_click_event_cb, LV_EVENT_PRESSED, NULL);
    
    // Set proper flags for the dropdown to ensure it captures clicks without bubbling
    lv_obj_clear_flag(prompt_dropdown, LV_OBJ_FLAG_EVENT_BUBBLE); // Explicitly stop bubbling
    lv_obj_add_flag(prompt_dropdown, LV_OBJ_FLAG_CLICKABLE);      // Ensure it's seen as clickable
    // --- End of Additions for Prompt Selection UI ---

    // Bring dropdown and status label to front
    lv_obj_move_foreground(prompt_dropdown);
    lv_obj_move_foreground(status_label);

    // Apply Apple-inspired styles
    lv_obj_set_style_bg_color(parent, lv_color_hex(0xF0F0F0), LV_STATE_DEFAULT); // Light grey background
    lv_obj_set_style_radius(parent, 10, LV_STATE_DEFAULT); // Rounded corners for all elements
    lv_obj_set_style_shadow_width(parent, 10, LV_STATE_DEFAULT); // Subtle shadow
    lv_obj_set_style_shadow_color(parent, lv_color_hex(0xAAAAAA), LV_STATE_DEFAULT); // Shadow color

    // Update status label style
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, LV_STATE_DEFAULT); // Modern font
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT); // White text

    // Update dropdown style
    lv_obj_set_style_bg_color(prompt_dropdown, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT); // White background
    lv_obj_set_style_border_color(prompt_dropdown, lv_color_hex(0xDDDDDD), LV_STATE_DEFAULT); // Light grey border
    lv_obj_set_style_border_width(prompt_dropdown, 1, LV_STATE_DEFAULT); // Thin border
    lv_obj_set_style_radius(prompt_dropdown, 5, LV_STATE_DEFAULT); // Rounded corners
    lv_obj_set_style_text_font(prompt_dropdown, &lv_font_montserrat_14, LV_STATE_DEFAULT); // Consistent font
    if (status_label) lv_obj_move_foreground(status_label);
    if (prompt_dropdown) lv_obj_move_foreground(prompt_dropdown);
}


static void task(void *param) {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_1;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_HVGA;  // Capture at 480x320
  config.pixel_format = PIXFORMAT_RGB565;  // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10; // Quality for potential JPEG capture (not used for API here)
  config.fb_count = 2; // Use 1 frame buffer

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    vTaskDelete(NULL);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // Set automatic controls
  s->set_gain_ctrl(s, 1);     // Auto gain control
  s->set_exposure_ctrl(s, 1); // Disable auto exposure
  s->set_whitebal(s, 1);      // Auto white balance
  s->set_awb_gain(s, 1);      // Auto white balance gain
  s->set_aec2(s, 1);          // Enable AEC second pass for potentially better stability/accuracy
  s->set_hmirror(s, 1);

  // Manual adjustments (adjust these values as needed)
  s->set_brightness(s, 1);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 1);     // -2 to 2
  s->set_sharpness(s, 1);      // -2 to 2
  s->set_denoise(s, 1);        // 0 or 1

  
  // Initialize image descriptor for 480x320
  lv_img_dsc_t img_dsc;
  img_dsc.header.always_zero = 0;
  img_dsc.header.w = 480;  // Source width
  img_dsc.header.h = 320;  // Source height
  img_dsc.data_size = 480 * 320 * 2;  // 2 bytes per pixel for RGB565
  img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR; // Use TRUE_COLOR for RGB565
  img_dsc.data = NULL;
  
  // Store descriptor format for captured images (use same descriptor)
  stored_img_dsc = img_dsc; // Reuse img_dsc structure

  Serial.println("Camera task started (480x320 capture)");

  while (1) {
    if (is_capturing) {
      // Live view mode - get frames continuously
      camera_fb_t *pic = esp_camera_fb_get();
      
      if (pic != NULL) {
        // --- Display the raw frame, LVGL will handle rotation --- 
        // int src_w = pic->width;
        // int src_h = pic->height;
        // int dst_w = src_h;
        // int dst_h = src_w;
        // if (rotated_buf) { // Check if pre-allocation was successful
        //   rotate_rgb565_90ccw(pic->buf, rotated_buf, src_w, src_h);
        //   // Use the rotated frame descriptor
        //   img_dsc.data = rotated_buf;
        //   img_dsc.header.w = dst_w;
        //   img_dsc.header.h = dst_h;
        // } else {
            // Use the original frame buffer directly
            img_dsc.data = pic->buf;
            img_dsc.header.w = pic->width; // Original width (480)
            img_dsc.header.h = pic->height; // Original height (320)
        // }

        if (lvgl_lock(-1)) {
          if (img_camera != NULL && lv_obj_is_valid(img_camera)) {
            lv_img_set_src(img_camera, &img_dsc);
            // No need for manual scaling/offset here as LVGL rotation + zoom handles it
            // float scale_x = (float)lv_obj_get_width(img_camera) / img_dsc.header.w;
            // float scale_y = (float)lv_obj_get_height(img_camera) / img_dsc.header.h;
            // float scale = fmax(scale_x, scale_y); // Use fmax to fill (may crop)
            // lv_coord_t zoom = 256 * scale;
            // lv_img_set_zoom(img_camera, zoom);
            // Center the image
            // lv_coord_t offset_x = (lv_obj_get_width(img_camera) - img_dsc.header.w * scale) / 2;
            // lv_coord_t offset_y = (lv_obj_get_height(img_camera) - img_dsc.header.h * scale) / 2;
            // lv_img_set_offset_x(img_camera, offset_x);
            // lv_img_set_offset_y(img_camera, offset_y);
          }
          lvgl_unlock();
        }
        
        // Return the frame buffer after updating the display
        esp_camera_fb_return(pic);
      }
        } else if (captured_pic == NULL) {
      // Paused state - first time after pausing
      Serial.println("Capturing frame (480x320)...");
      captured_pic = esp_camera_fb_get();

      if (captured_pic != NULL) {
        Serial.printf("Frame captured: %d bytes, dimensions: %dx%d\n",
                    captured_pic->len, captured_pic->width, captured_pic->height);

        // --- START: Remove or comment out this rotation block ---
        /*
        static uint8_t* rotated_captured_buf = NULL;
        static size_t rotated_captured_buf_size = 0;
        int src_w = captured_pic->width;
        int src_h = captured_pic->height;
        int dst_w = src_h;
        int dst_h = src_w;
        size_t needed_size = dst_w * dst_h * 2;
        if (!rotated_captured_buf || rotated_captured_buf_size != needed_size) {
          if (rotated_captured_buf) free(rotated_captured_buf);
          rotated_captured_buf = (uint8_t*)malloc(needed_size);
          rotated_captured_buf_size = needed_size;
        }
        if (rotated_captured_buf) {
          rotate_rgb565_90ccw(captured_pic->buf, rotated_captured_buf, src_w, src_h);
          if (lvgl_lock(-1)) {
            if (img_camera != NULL && lv_obj_is_valid(img_camera)) {
              // Update stored_img_dsc with ROTATED data/dims
              stored_img_dsc.data = rotated_captured_buf;
              stored_img_dsc.header.w = dst_w; // 320
              stored_img_dsc.header.h = dst_h; // 480
              lv_img_set_src(img_camera, &stored_img_dsc);
            }
            lvgl_unlock();
          }
        }
        */
        // --- END: Remove or comment out this rotation block ---

        // --- START: Add this block to display the ORIGINAL frame ---
        if (lvgl_lock(-1)) {
            if (img_camera != NULL && lv_obj_is_valid(img_camera)) {
              // Update stored_img_dsc with ORIGINAL data/dims
              // Note: stored_img_dsc was initialized with cf/data_size, just update data/w/h
              stored_img_dsc.data = captured_pic->buf; // Use original buffer
              stored_img_dsc.header.w = captured_pic->width;  // Original width (480)
              stored_img_dsc.header.h = captured_pic->height; // Original height (320)
              // Let the img_camera widget's existing rotation handle orientation
              lv_img_set_src(img_camera, &stored_img_dsc);
            }
            lvgl_unlock();
        }
        // --- END: Add this block ---

        // Note: Don't return the frame buffer yet, keep it for processing
      } else {
        Serial.println("Failed to capture frame");
        // Handle error: maybe revert to capturing?
        is_capturing = true;
        if (lvgl_lock(-1)) {
            lv_label_set_text(status_label, "Capture failed!");
            lvgl_unlock();
        }
      }
    } else {
      // Paused state, but frame is already captured (captured_pic != NULL)
      // Do nothing, just wait for processing or resumption
    }

    
    vTaskDelay(pdMS_TO_TICKS(5)); // Reduced delay from 30ms to 5ms
  }
  vTaskDelete(NULL);
}

// Function to connect to WiFi with improved reliability
bool connectToWiFi() {
    Serial.println("Connecting to WiFi...");
    WiFi.disconnect(true);  // Disconnect from any previous connections
    delay(500);  // Give time to disconnect properly
    
    WiFi.mode(WIFI_STA);  // Set to station mode explicitly
    
    // Disable power saving for better stability (at cost of higher power consumption)
    esp_wifi_set_ps(WIFI_PS_NONE);
    
    // Set static IP if needed (uncomment and configure if your network is more stable with static IP)
    // WiFi.config(IPAddress(192, 168, 1, 200), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
    
    WiFi.begin(ssid, password);
    
    // Wait for connection with timeout and status indicator
    int attempts = 0;
    const int MAX_CONNECT_ATTEMPTS = 30;  // 15 seconds total (500ms intervals)
    
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_CONNECT_ATTEMPTS) {
        delay(500);
        Serial.print(".");
        attempts++;
        
        // Print status code periodically
        if (attempts % 10 == 0) {
            Serial.printf(" [Status: %d] ", WiFi.status());
        }
    }
    Serial.println();
    
    // Check connection result
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("WiFi connected successfully! IP: %s, RSSI: %d dBm\n", 
                    WiFi.localIP().toString().c_str(), WiFi.RSSI());
        
        // Set TX power to maximum
        esp_wifi_set_max_tx_power(80); // 8dbm (maximum) - may increase power consumption
        
        return true;
    } else {
        Serial.printf("WiFi connection failed with status: %d\n", WiFi.status());
        return false;
    }
}

// Function to check and reconnect WiFi if needed
bool checkWiFiAndReconnect() {
  static unsigned long lastCheckTime = 0;
    static int consecutiveFailures = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastCheckTime >= WIFI_RECHECK_INTERVAL) {
    lastCheckTime = currentTime;

        wl_status_t status = WiFi.status();
        int rssi = WiFi.RSSI();
        
        // Check if we're properly connected
        if (status != WL_CONNECTED) {
            Serial.printf("WiFi disconnected (status: %d), attempting to reconnect...\n", status);
            consecutiveFailures++;
      return reconnectWiFi();
    }

        // Check signal strength
    if (rssi < MIN_RSSI) {
            Serial.printf("Weak WiFi signal (RSSI = %d dBm), checking stability...\n", rssi);
            
            // Test connection with a ping-like test
            if (!testConnection()) {
                Serial.println("Connection test failed, attempting to reconnect...");
                consecutiveFailures++;
      return reconnectWiFi();
            } else {
                Serial.println("Connection test passed despite weak signal, continuing...");
                consecutiveFailures = 0;
    }
        } else {
            // Signal is good
            if (consecutiveFailures > 0) {
                Serial.printf("WiFi signal stable (RSSI = %d dBm), connection restored\n", rssi);
                consecutiveFailures = 0;
  }
        }
    }
    
  return WiFi.status() == WL_CONNECTED;
}

// Function to test connection by attempting a small HTTP request
bool testConnection() {
    HTTPClient http;
    http.setTimeout(3000);  // 3-second timeout for the test
    
    // Try to fetch a small resource (Google's homepage is reliable)
    if (http.begin("http://www.google.com")) {
        int httpCode = http.GET();
        http.end();
        
        // Return true if we got any valid HTTP response
        return httpCode > 0;
    }
    
    return false;
}

// Function to handle WiFi reconnection with improved resilience
bool reconnectWiFi() {
    // Completely disconnect first
    WiFi.disconnect(true);
    delay(500);

  int reconnectAttempts = 0;
  unsigned long delayTime = 1000; // Initial delay of 1 second
    
    // Before attempting reconnection, try a more thorough reset if we've had multiple failures
    static int totalReconnectCount = 0;
    totalReconnectCount++;
    
    if (totalReconnectCount > 5) {
        Serial.println("Multiple reconnection attempts detected, performing WiFi reset...");
        WiFi.mode(WIFI_OFF);
        delay(1000);
        WiFi.mode(WIFI_STA);
        delay(500);
        totalReconnectCount = 0;
    }

  while (reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
    Serial.printf("Attempting to reconnect to WiFi (attempt %d)...\n", reconnectAttempts + 1);
    WiFi.begin(ssid, password);

    int attempts = 0;
        const int MAX_WAIT_ATTEMPTS = 20;
        
        while (WiFi.status() != WL_CONNECTED && attempts < MAX_WAIT_ATTEMPTS) {
      delay(500);
      Serial.print(".");
      attempts++;
            // Print status periodically
            if (attempts % 10 == 0) {
                Serial.printf(" [Status: %d] ", WiFi.status());
            }
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("WiFi reconnected successfully! IP: %s, RSSI: %d dBm\n", 
                        WiFi.localIP().toString().c_str(), WiFi.RSSI());
      return true;
    } else {
            Serial.printf("WiFi reconnection failed (attempt %d, status: %d), retrying in %lu ms...\n", 
                        reconnectAttempts + 1, WiFi.status(), delayTime);
      reconnectAttempts++;
      delay(delayTime);
      delayTime *= 2; // Exponential backoff
    }
  }

  Serial.println("Maximum WiFi reconnection attempts reached. Please check your network configuration.");
  return false;
}

// Function to simply ensure WiFi is connected before important operations
bool ensureSimpleWiFiConnection() {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    // Try to reconnect
    WiFi.disconnect(true);
    delay(200);
    WiFi.begin(ssid, password);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) { // 10s timeout
        delay(200);
    }
    return WiFi.status() == WL_CONNECTED;
}

// Enhanced version of uploadImageToImgur with better WiFi stability checks
String uploadImageToImgur(uint8_t* imageData, size_t imageLen) {
  // First ensure we have a stable connection
  Serial.println("Ensuring stable WiFi connection before upload...");
  bool wifiStable = ensureSimpleWiFiConnection();
  
  if (!wifiStable) {
    Serial.println("Failed to establish stable WiFi connection for upload");
    return "";
  }

  // Print WiFi diagnostic info
  Serial.printf("WiFi connected with RSSI: %d dBm, IP: %s\n", 
                WiFi.RSSI(), WiFi.localIP().toString().c_str());

  // Verify image data is valid
  if (imageData == NULL || imageLen == 0) {
    Serial.println("Error: Invalid image data for Imgur upload");
    return "";
  }

  Serial.printf("Starting Imgur upload: %d bytes\n", imageLen);
  
  String link = ""; // Will store our final URL if successful
  
  // 1. First try: HTTPS with insecure connection (previously proven to work)
  Serial.println("Attempting HTTPS connection with certificate check disabled...");
  WiFiClientSecure *insecureClient = new WiFiClientSecure;
    
  if (insecureClient) {
    // Skip certificate verification
    insecureClient->setInsecure();
    Serial.println("Certificate check disabled for connection");
       
    // Increase timeout
    insecureClient->setTimeout(20000); // 20 seconds timeout
       
    HTTPClient https;
    https.setTimeout(20000);
    
    // Try uploading with better error handling
    bool uploadAttempted = false;
    
    // Try multiple times with increasing timeouts
    for (int attempt = 0; attempt < 3 && link.isEmpty(); attempt++) {
      // Check WiFi before each attempt
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected during upload preparation, reconnecting...");
        if (!reconnectWiFi()) {
          Serial.println("Failed to reconnect WiFi, aborting upload attempt");
          break;
        }
      }
      
      if (https.begin(*insecureClient, "https://api.imgur.com/3/image")) {
        https.addHeader("Authorization", String("Client-ID ") + IMGUR_CLIENT_ID);
        https.addHeader("Content-Type", "application/octet-stream");
        
        // Use progressively longer timeouts for each attempt
        int timeoutMs = 10000 * (attempt + 1);
        https.setTimeout(timeoutMs);
        
        Serial.printf("Sending POST to Imgur (attempt %d, timeout %d ms)...\n", attempt+1, timeoutMs);
        uploadAttempted = true;
        int httpResponseCode = https.POST(imageData, imageLen);
        String payload = https.getString();
        
        Serial.printf("Insecure upload response code: %d\n", httpResponseCode);
        
        if (httpResponseCode > 0) {
          StaticJsonDocument<1024> doc;
          DeserializationError error = deserializeJson(doc, payload);
          
          if (!error && doc["data"]["link"]) {
            link = doc["data"]["link"].as<String>();
            // Also capture the delete hash if present
            if (doc["data"]["deletehash"]) {
              imgurDeleteHash = doc["data"]["deletehash"].as<String>();
              Serial.printf("Insecure upload successful, image URL: %s, Delete hash: %s\n", 
                           link.c_str(), imgurDeleteHash.c_str());
            } else {
              Serial.printf("Insecure upload successful, image URL: %s (No delete hash found)\n", link.c_str());
            }
          } else if (payload.length() > 0) {
            Serial.println("Response doesn't contain image link. Response details:");
            if (payload.length() < 200) {
              Serial.println(payload);
            } else {
              Serial.printf("(Response too long, %d bytes)\n", payload.length());
            }
          }
        } else {
          // Print error details
          switch (httpResponseCode) {
            case -1: Serial.println("ERROR: Connection failed or timed out"); break;
            case -2: Serial.println("ERROR: Server not found, check URL"); break;
            case -3: Serial.println("ERROR: Connection timed out"); break;
            case -4: Serial.println("ERROR: Connection lost"); break;
            case -5: Serial.println("ERROR: No server response received"); break;
            case -6: Serial.println("ERROR: SSL connection failed"); break;
            default: Serial.printf("ERROR: Unknown connection error: %d\n", httpResponseCode);
          }
          
          // WiFi reconnect if connection seems to be the issue
          if (httpResponseCode == -1 || httpResponseCode == -3 || httpResponseCode == -4) {
            Serial.println("Connection issue detected, checking WiFi status...");
            if (WiFi.status() != WL_CONNECTED || !testConnection()) {
              Serial.println("WiFi appears unstable, reconnecting...");
              reconnectWiFi();
            }
          }
        }
        
        https.end();
        
        // Add delay between attempts
        if (link.isEmpty() && attempt < 2) {
          Serial.printf("Upload attempt %d failed, waiting 1 seconds before retry...\n", attempt+1);
          delay(1000);
        }
        } else {
        Serial.println("Failed to initialize HTTPS connection");
      }
    }
    
    delete insecureClient;
    
    // If we couldn't even attempt the upload, it's likely a deeper connection issue
    if (!uploadAttempted) {
      Serial.println("Failed to initialize any upload attempts, WiFi may be blocked or unstable");
      // Perform a more thorough WiFi reset
      WiFi.disconnect(true);
      delay(1000);
      WiFi.mode(WIFI_OFF);
      delay(1000);
      WiFi.mode(WIFI_STA);
      delay(500);
      WiFi.begin(ssid, password);
      // Wait up to 10 seconds for reconnection
      int reconnectWait = 0;
      while (WiFi.status() != WL_CONNECTED && reconnectWait < 20) {
        delay(500);
        Serial.print(".");
        reconnectWait++;
      }
      Serial.println();
    }
  }
  
  // Rest of the function (secure and HTTP attempts) remains unchanged
  // ... existing code ...

  return link;
}

// Function to delete an image from Imgur using the stored delete hash
bool deleteImgurImage() {
  if (imgurDeleteHash.isEmpty()) {
    Serial.println("No delete hash available. Cannot delete image.");
    return false;
  }

  Serial.printf("Attempting to delete Imgur image with hash: %s\n", imgurDeleteHash.c_str());
  
  bool success = false;
  
  // Check WiFi connection first
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, attempting to reconnect...");
    if (!reconnectWiFi()) {
      Serial.println("Failed to reconnect WiFi, cannot delete image");
      return false;
    }
  }
  
  // Try the insecure connection first (as it worked for upload)
  WiFiClientSecure *insecureClient = new WiFiClientSecure;
  
  if (insecureClient) {
    // Skip certificate verification
    insecureClient->setInsecure();
    // insecureClient->setBufferSizes(4096, 4096);
    insecureClient->setTimeout(10000); // 10 seconds timeout
    
    HTTPClient https;
    https.setTimeout(10000);
    
    String deleteUrl = "https://api.imgur.com/3/image/" + imgurDeleteHash;
    
    if (https.begin(*insecureClient, deleteUrl)) {
      https.addHeader("Authorization", String("Client-ID ") + IMGUR_CLIENT_ID);
      
      Serial.println("Sending DELETE request to Imgur...");
      int httpResponseCode = https.sendRequest("DELETE");
      
      Serial.printf("Delete response code: %d\n", httpResponseCode);
      
      if (httpResponseCode > 0) {
        String payload = https.getString();
        
        // Parse response to check success
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error && doc["success"]) {
          bool deleteSuccess = doc["success"].as<bool>();
          if (deleteSuccess) {
            Serial.println("Image deleted successfully");
            success = true;
          } else {
            Serial.println("Image deletion reported failure");
          }
        } else {
          Serial.println("Failed to parse delete response or deletion unsuccessful");
          if (payload.length() < 200) {
            Serial.println(payload);
          } else {
            Serial.printf("(Response too long, %d bytes)\n", payload.length());
          }
        }
      } else {
        // Print error details
        Serial.printf("Delete request failed, error: %d\n", httpResponseCode);
      }
      
      https.end();
    } else {
      Serial.println("Failed to connect to Imgur API for deletion");
    }
    
    delete insecureClient;
  }
  
  // Clear the delete hash if deletion was successful
  if (success) {
    imgurDeleteHash = "";
    return true;
  }
  
  return false;
}

// Helper function to clear the current Imgur image
void clearImgurImage() {
  if (!imgurDeleteHash.isEmpty()) {
    Serial.println("Clearing Imgur image...");
    if (deleteImgurImage()) {
      Serial.println("Successfully cleared Imgur image");
    } else {
      Serial.println("Failed to clear Imgur image, but resetting delete hash anyway");
      imgurDeleteHash = ""; // Reset the hash even if deletion failed
    }
  } else {
    Serial.println("No Imgur image to clear");
  }
}

// Function to handle OpenAI requests with image URL instead of base64 encoding
String getOpenAIResponseWithUrl(const String& imageUrl) {
    Serial.println("Starting OpenAI API request with image URL...");
    
    // Check if we have a valid API key
    if (strcmp(openai_api_key, "YOUR_OPENAI_API_KEY") == 0) {
        Serial.println("Error: OpenAI API key not set");
        return "Error: OpenAI API key not set. Please update the code with your API key.";
    }
    
    // First ensure we have a stable connection before proceeding with the API request
    Serial.println("Ensuring stable WiFi connection before OpenAI request...");
    bool wifiStable = ensureSimpleWiFiConnection();
    
    if (!wifiStable) {
        Serial.println("Failed to establish stable WiFi connection for OpenAI request");
        return "Error: Could not establish a stable WiFi connection. Please check your network and try again.";
    }
    
    String response = "";
    
    try {
        // Create a small JSON document for the request
        Serial.println("Creating OpenAI request with image URL...");
        String selected_prompt_text = prompt_texts[current_prompt_index];
        StaticJsonDocument<2048> doc;  // Only need a small doc now!
        doc["model"] = "gpt-4o-mini";
        doc["max_tokens"] = 300;
        
        JsonArray messages = doc.createNestedArray("messages");

        JsonObject systemMessage = messages.createNestedObject();
        systemMessage["role"] = "system";
        systemMessage["content"] = "You will be provided with an image URL. Your goal is to analyze the image understand user needs and provide helpful responses based on the selected prompt.";

        JsonObject message1 = messages.createNestedObject();
        message1["role"] = "user";
        JsonArray content = message1.createNestedArray("content");

        JsonObject contentText = content.createNestedObject();
        contentText["type"] = "text";
        contentText["text"] = selected_prompt_text;

        JsonObject contentImage = content.createNestedObject();
        contentImage["type"] = "image_url";
        JsonObject imageUrlObj = contentImage.createNestedObject("image_url");
        imageUrlObj["url"] = imageUrl;

        // Serialize to JSON
        String jsonPayload;
        serializeJson(doc, jsonPayload);

        Serial.printf("JSON payload size: %d bytes\n", jsonPayload.length());
        Serial.printf("Image URL: %s\n", imageUrl.c_str());

        // Send HTTP request with increased timeout
        Serial.println("Sending HTTP request to OpenAI...");
        HTTPClient http;
        http.setTimeout(120000); // Increased timeout to 120 seconds

        const int MAX_RETRIES = 3; // Increased retries
        int retryCount = 0;
        int httpResponseCode = -100; // Initialize with an invalid code

        while (retryCount < MAX_RETRIES && httpResponseCode < 0) {
            // Check WiFi before each attempt
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi disconnected before OpenAI request, reconnecting...");
                if (!reconnectWiFi()) {
                    Serial.println("WiFi reconnection failed");
                    retryCount++;
                    delay(1000 * retryCount); // Exponential backoff
                    continue;
                }
            }

            // Test connection before making the actual API call
            if (!testConnection()) {
                Serial.println("Connection test failed before OpenAI request, reconnecting...");
                reconnectWiFi();
                retryCount++;
                delay(1000 * retryCount);
                continue;
            }

            http.begin(openai_endpoint);
            http.addHeader("Content-Type", "application/json");
            http.addHeader("Authorization", "Bearer " + String(openai_api_key));

            Serial.printf("Heap before HTTP POST: %d bytes\n", ESP.getFreeHeap());
            Serial.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
            
            Serial.printf("Sending POST request (attempt %d)...\n", retryCount + 1);
            int postResult = http.POST(jsonPayload);
            httpResponseCode = postResult;
            Serial.printf("HTTP Response code: %d\n", httpResponseCode);

            if (httpResponseCode > 0) {
                break; // Successful response, exit retry loop
            } else {
                Serial.printf("Attempt %d failed with code: %d, retrying...\n", retryCount + 1, httpResponseCode);
                retryCount++;
                
                // More detailed error handling
                switch (httpResponseCode) {
                    case -1: Serial.println("ERROR: Connection failed or timed out"); break;
                    case -2: Serial.println("ERROR: Server not found, check URL"); break;
                    case -3: Serial.println("ERROR: Connection timed out"); break;
                    case -4: Serial.println("ERROR: Connection lost"); break;
                    case -5: Serial.println("ERROR: No server response received"); break;
                    case -6: Serial.println("ERROR: SSL connection failed"); break;
                    default: Serial.printf("ERROR: Unknown connection error: %d\n", httpResponseCode);
                }
                
                // More aggressive WiFi reset on connection issues
                if (httpResponseCode == -1 || httpResponseCode == -3 || httpResponseCode == -4) {
                    Serial.println("Connection issue detected, performing WiFi reset...");
                    WiFi.disconnect(true);
                    delay(1000);
                    WiFi.mode(WIFI_OFF);
                    delay(1000);
                    WiFi.mode(WIFI_STA);
                    delay(500);
                    connectToWiFi();
                }
                
                http.end(); // End HTTP connection before retrying
                delay(2000 * retryCount); // Exponential backoff with longer delays
            }
        }
        
        if (httpResponseCode > 0) {
            Serial.println("Reading response...");
            response = http.getString();
            Serial.printf("Response length: %d\n", response.length());
            
            // Parse the response to extract the actual content
            // Use a smaller document size for the response parsing
            StaticJsonDocument<16384> responseDoc;  // 16KB should be enough for the response
            DeserializationError error = deserializeJson(responseDoc, response);
            
            if (!error) {
                // Extract the content from the response
                if (responseDoc.containsKey("choices") && 
                    responseDoc["choices"][0].containsKey("message") && 
                    responseDoc["choices"][0]["message"].containsKey("content")) {
                    
                    String content = responseDoc["choices"][0]["message"]["content"].as<String>();
                    Serial.println("Successfully extracted content from response");
                    http.end(); // End HTTP connection here for success case
                    
                    return content;
                } else if (responseDoc.containsKey("error") && 
                          responseDoc["error"].containsKey("message")) {
                    String errorMsg = responseDoc["error"]["message"].as<String>();
                    Serial.printf("API Error: %s\n", errorMsg.c_str());
                    http.end(); // End HTTP connection here for API error case
                    
                    return "API Error: " + errorMsg;
                } else {
                    Serial.println("Response format not as expected");
                    http.end(); // End HTTP connection here for format error case
                    
                    return "Error: Unexpected response format from OpenAI API";
                }
            } else {
                Serial.printf("Failed to parse JSON response: %s\n", error.c_str());
                http.end(); // End HTTP connection here for JSON parse error case
                
                return "Error: Failed to parse API response";
            }
        } else {
            // Handle specific error codes
            String errorResponse = "";
            if (httpResponseCode == -1) {
                errorResponse = "Error: Connection failed. Check network connection.";
            } else if (httpResponseCode == -2) {
                errorResponse = "Error: Server not found. Check API endpoint.";
            } else if (httpResponseCode == -3) {
                errorResponse = "Error: Connection timed out. Try again, or try a better WiFi connection.";
            } else if (httpResponseCode == -4) {
                errorResponse = "Error: Connection lost. Check WiFi stability.";
            } else {
                errorResponse = "Error: HTTP request failed with code " + String(httpResponseCode);
            }
            http.end(); // End HTTP connection here for failure case
            Serial.println(errorResponse);
            
            return errorResponse;
        }
        
    } catch (const std::exception& e) {
        Serial.printf("Exception occurred: %s\n", e.what());
        return "Error: Exception during API request processing";
    } catch (...) {
        Serial.println("Unknown exception occurred");
        return "Error: Unknown exception during API request processing";
    }
}

// Legacy function that now uses Imgur upload instead of base64
String getOpenAIResponse(camera_fb_t *fb) {
    if (!fb) return "Error: No image data";
    
    Serial.println("Legacy OpenAI API function called - using Imgur upload instead");
    
    // First, upload the image to Imgur
    String imageUrl;
    
    if (fb->format == PIXFORMAT_JPEG) {
        // If already JPEG, upload directly
        imageUrl = uploadImageToImgur(fb->buf, fb->len);
    } else {
        // Convert RGB565 to JPEG first
        uint8_t *jpg_buf = NULL;
        size_t jpg_size = 0;
        
        Serial.println("Converting RGB565 to JPEG...");
        if (fmt2jpg(fb->buf, fb->len, fb->width, fb->height, 
                  PIXFORMAT_RGB565, 90, &jpg_buf, &jpg_size)) {
            
            Serial.printf("Converted to JPEG: %d bytes\n", jpg_size);
            imageUrl = uploadImageToImgur(jpg_buf, jpg_size);
            free(jpg_buf); // Free the JPEG buffer after uploading
        } else {
            Serial.println("Failed to convert to JPEG");
            return "Error: Failed to convert image to JPEG format.";
        }
    }
    
    if (imageUrl.length() > 0) {
        // Successfully uploaded to Imgur, now send to OpenAI
        Serial.printf("Image uploaded to Imgur: %s\n", imageUrl.c_str());
        
        // Use the new URL-based function
        return getOpenAIResponseWithUrl(imageUrl);
    } else {
        // Failed to upload to Imgur
        return "Error: Failed to upload image to Imgur. Please check your network connection and try again.";
    }
}

void setup() {
  Serial.begin(115200);
  
  // Add a small delay at startup to let hardware stabilize
  delay(100);
  
  // Initialize PSRAM and report available memory
  Serial.printf("Starting with %d bytes of heap and ", ESP.getFreeHeap());
  if (psramInit()) {
    Serial.printf("%d bytes of PSRAM available\n", ESP.getFreePsram());
  } else {
    Serial.println("No PSRAM detected or initialization failed!");
  }
  
  // Increment boot count
  bootCount++;
  Serial.printf("Boot number: %d\n", bootCount);
  
  // --- Pin/RTC Configuration (Early) ---
  // Configure pin early in case we need to read it right away
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  rtc_gpio_pullup_en(BOOT_BUTTON_PIN);
  rtc_gpio_pulldown_dis(BOOT_BUTTON_PIN);
  rtc_gpio_hold_dis(BOOT_BUTTON_PIN); 

  // --- Wake-up Handling --- 
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  bool woke_from_button = (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0);
  
  // Always disable all sources immediately after checking the reason
  // This prevents the current wake-up state (button LOW) from causing issues
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  Serial.println("All wake sources disabled temporarily after wake-up check.");

  if (woke_from_button) {
    // Set the wake-up flag to true immediately
    just_woke_up = true;
    
    Serial.println("Wakeup caused by external signal using RTC_IO (Button Press Detected)");
    
    // Wait a bit first regardless of button state for system stability
    delay(200);
    
    Serial.println("Waiting for button release after wake-up...");
    unsigned long wait_start = millis();
    // Wait until the button goes HIGH (released)
    // Use a longer wait time to ensure button is fully released
    int release_count = 0; // Count consecutive HIGH readings for better debounce
    while (true) {
      if (digitalRead(BOOT_BUTTON_PIN) == HIGH) {
        release_count++;
        if (release_count >= 10) { // Need 10 consecutive HIGH readings
          Serial.println("Button consistently released.");
          break;
        }
      } else {
        release_count = 0; // Reset counter on any LOW reading
      }
      
      delay(20); 
      if (millis() - wait_start > 5000) { // Timeout after 5 seconds
        Serial.println("Timeout waiting for button release post-wake!");
        break; 
      }
    }
    
    Serial.println("Button appears released post-wake. Adding stabilization delay...");
    delay(500); // Longer stabilization delay
    
    // Final check after delay
    int final_state = digitalRead(BOOT_BUTTON_PIN);
    Serial.printf("Button state after stabilization: %d\n", final_state);
    if(final_state == LOW) {
        Serial.println("WARNING: Button still LOW after stabilization! Proceeding cautiously.");
        // If still LOW, wait longer to see if it resolves
        delay(1000);
    }
    Serial.println("Ready to enable wake-up for the *next* press.");

  } else {
    Serial.printf("Wakeup was not caused by deep sleep or other source: %d\n", wakeup_reason);
    if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED) {
      Serial.println("This is likely the initial boot or power-on.");
    } else {
      Serial.printf("Wakeup caused by: %d\n", wakeup_reason);
    }
    // Additional delay for system stability on initial boot
    delay(100);
  }
  
  // --- Re-enable Wake Source for Next Sleep Cycle --- 
  // Delay before re-enabling to make sure system is stable
  delay(200);
  
  // Ensure RTC IO is still configured correctly
  rtc_gpio_pullup_en(BOOT_BUTTON_PIN);
  rtc_gpio_pulldown_dis(BOOT_BUTTON_PIN);
  rtc_gpio_hold_dis(BOOT_BUTTON_PIN); 
  
  // Only enable wake-up source if the button is HIGH (not pressed)
  if (digitalRead(BOOT_BUTTON_PIN) == HIGH) {
    // Enable EXT0 wake-up for the next press (level 0)
    Serial.println("Enabling EXT0 wake-up source for next button press (level 0).");
    esp_sleep_enable_ext0_wakeup(BOOT_BUTTON_PIN, 0);
  } else {
    Serial.println("WARNING: Button is still LOW, waiting before enabling wake-up source...");
    // Wait up to 3 seconds for the button to be released
    unsigned long wait_start = millis();
    while (digitalRead(BOOT_BUTTON_PIN) == LOW && millis() - wait_start < 3000) {
      delay(100);
    }
    
    // Now check again and enable wake-up if safe
    if (digitalRead(BOOT_BUTTON_PIN) == HIGH) {
      Serial.println("Button is now HIGH, enabling wake-up source.");
      esp_sleep_enable_ext0_wakeup(BOOT_BUTTON_PIN, 0);
    } else {
      Serial.println("Button still LOW after waiting! Will enable wake-up but may cause issues.");
      esp_sleep_enable_ext0_wakeup(BOOT_BUTTON_PIN, 0);
    }
  }
  
  // Initialize WiFi
  Serial.println("Connecting to WiFi...");
  if (connectToWiFi()) {
    Serial.println("WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed!");
  }
  
  lvgl_api_mux = xSemaphoreCreateRecursiveMutex();
  Serial.println("Arduino_GFX LVGL_Arduino_v8 example ");
  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);

#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(BLACK);

#ifdef EXAMPLE_PIN_NUM_LCD_BL
  ledcAttach(EXAMPLE_PIN_NUM_LCD_BL, LEDC_FREQ, LEDC_TIMER_10_BIT);
  ledcWrite(EXAMPLE_PIN_NUM_LCD_BL, (1 << LEDC_TIMER_10_BIT) / 100 * 80);
#endif

  // Init touch device
  Wire.begin(EXAMPLE_PIN_NUM_TP_SDA, EXAMPLE_PIN_NUM_TP_SCL);
  bsp_touch_init(&Wire, gfx->getRotation(), gfx->width(), gfx->height());
  lv_init();

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  screenWidth = gfx->width();
  screenHeight = gfx->height();

  bufSize = screenWidth * screenHeight;

  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!disp_draw_buf) {
    // remove MALLOC_CAP_INTERNAL flag try again
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);
  }

  if (!disp_draw_buf) {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  } else {
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, bufSize);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.direct_mode = true;

    lv_disp_drv_register(&disp_drv);

    /* Initialize the (dummy) input device driver */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    /* Initialize the UI */
    lvgl_camera_ui_init(lv_scr_act());
    
    /* Initialize response panel variables */
    response_panel = NULL;
    response_label = NULL;
    close_btn = NULL;
    is_response_visible = false;
  }

  Serial.println("Setup done");
  
  // Create camera task with higher priority
  xTaskCreatePinnedToCore(
    task,
    "lvgl_app_task",
    1024 * 12,  // Increased stack size
    NULL,
    3,  // Higher priority (was 1)
    NULL,
    0);

  // Create process image task with higher priority
  xTaskCreatePinnedToCore(
    process_image_task,
    "process_image_task",
    1024 * 16,  // Increased stack size
    NULL,
    2,  // Higher priority than camera task
    NULL,
    0);

  // Configure capture button pin
  pinMode(CAPTURE_BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  // Check BOOT button state with improved debounce
  static unsigned long lastButtonPressTime = 0;
  static int buttonState = HIGH;         // Current button reading
  static int lastButtonState = HIGH;     // Previous button reading
  static int buttonPressCount = 0;       // Counter for consistent readings
  
  // Add debounce variables for capture button
  static unsigned long lastCaptureButtonPressTime = 0;
  static int captureButtonState = HIGH;
  static int lastCaptureButtonState = HIGH;
  static int captureButtonPressCount = 0;
  
  // If we just woke up from sleep, ignore button presses for a moment
  if (just_woke_up) {
    static unsigned long wakeupTime = 0;
    if (wakeupTime == 0) {
      wakeupTime = millis();
    }
    
    if (millis() - wakeupTime < 2000) { // Ignore button for 2 seconds after wake-up
      // Just check LVGL and return early
      if (lvgl_lock(10)) {
        lv_timer_handler();
        lvgl_unlock();
      }
      vTaskDelay(pdMS_TO_TICKS(5));
      return;
    } else {
      // Reset the flag and timer after the delay
      just_woke_up = false;
      wakeupTime = 0;
      Serial.println("Wake-up cooldown period ended, now accepting button presses");
    }
  }
  
  unsigned long currentMillis = millis();
  int reading = digitalRead(BOOT_BUTTON_PIN);
  
  // If reading changed, reset the counter
  if (reading != lastButtonState) {
    buttonPressCount = 0;
    lastButtonState = reading;
  } else if (buttonPressCount < 5) {
    // Increment counter if reading is stable
    buttonPressCount++;
  }
  
  // If reading has been stable for 5 cycles, update the actual button state
  if (buttonPressCount >= 5) {
    if (buttonState != reading) {
      buttonState = reading;
      
      // If button has been pressed (LOW) - This triggers sleep entry
      if (buttonState == LOW) {
        // Ensure sufficient time has passed since last confirmed press state change
        if (currentMillis - lastButtonPressTime > 1000) { // Increased debounce for sleep entry
          lastButtonPressTime = currentMillis;
          Serial.println("DEBOUNCED BOOT button press detected. Preparing for deep sleep...");
          
          // --- PRE-SLEEP CONFIGURATION --- 
          // Clear all existing wake-up sources first
          esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
          
          // Turn off display backlight if possible
#ifdef EXAMPLE_PIN_NUM_LCD_BL
          ledcWrite(EXAMPLE_PIN_NUM_LCD_BL, 0); 
          Serial.println("Display backlight turned off.");
#endif
          
          // IMPORTANT: Wait for button release before enabling wake-up source
          Serial.println("Waiting for button release before enabling wake-up...");
          unsigned long wait_start = millis();
          while (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            delay(50);
            if (millis() - wait_start > 5000) { // Timeout after 5 seconds
              Serial.println("Timeout waiting for button release!");
              break;
            }
          }
          
          // Wait additional time to prevent bouncing
          delay(300);
          
          // Configure RTC IO properties for the BOOT button pin for wake-up
          rtc_gpio_pullup_en(BOOT_BUTTON_PIN);
          rtc_gpio_pulldown_dis(BOOT_BUTTON_PIN);
          rtc_gpio_hold_dis(BOOT_BUTTON_PIN);
          
          // Double-check that button is really released
          if (digitalRead(BOOT_BUTTON_PIN) == HIGH) {
            Serial.println("BOOT button confirmed released. Now enabling wake-up source.");
            // Only now enable wake-up on the next button press (level 0)
            esp_sleep_enable_ext0_wakeup(BOOT_BUTTON_PIN, 0);
            Serial.println("EXT0 wake-up source set for next press (level 0).");
            
            // Add a delay to make sure wake-up source stabilizes
            delay(500);  // Increased from 100ms to 500ms
            
            // Set the just_woke_up flag to false before sleeping
            just_woke_up = false;
            
            // --- ENTER SLEEP --- 
            Serial.println("Going to sleep now. Wake on next press.");
            delay(50); // Short delay for serial message
            esp_deep_sleep_start();
            
            // Code below esp_deep_sleep_start() will not execute until wake-up
          } else {
            Serial.println("BOOT button still pressed or bouncing. Aborting sleep.");
            // Re-enable wake-up source for normal operation
            esp_sleep_enable_ext0_wakeup(BOOT_BUTTON_PIN, 0);
          }
        } 
      } else { 
        // Button state changed to HIGH (released), update last press time if needed 
        // to ensure the debounce timer works correctly for the *next* press.
        // We don't want rapid press/release/press to bypass debounce.
        lastButtonPressTime = currentMillis; 
      }
    }
  }
  
  // Handle capture button with debounce
  int captureReading = digitalRead(CAPTURE_BUTTON_PIN);
  
  // If reading changed, reset the counter
  if (captureReading != lastCaptureButtonState) {
    captureButtonPressCount = 0;
    lastCaptureButtonState = captureReading;
  } else if (captureButtonPressCount < 3) {
    // Increment counter if reading is stable
    captureButtonPressCount++;
  }
  
  // If reading has been stable for 5 cycles, update the actual button state
  if (captureButtonPressCount >= 3) {
    if (captureButtonState != captureReading) {
      captureButtonState = captureReading;
      
      // If button has been pressed (LOW)
      if (captureButtonState == LOW) {
        // Ensure sufficient time has passed since last press
        if (currentMillis - lastCaptureButtonPressTime > 500) { // 500ms debounce
          lastCaptureButtonPressTime = currentMillis;
          
          // Only trigger if we're not already processing and response isn't visible
          if (processing_stage == 0 && !is_response_visible) {
            // Toggle capture state just like the touch interface does
            is_capturing = !is_capturing;
            Serial.printf("Capture button pressed, toggled capture state: %s\n", 
                        is_capturing ? "capturing" : "paused");
            
            if (!is_capturing) {
              // Just paused - update status and start capture process
              if (lvgl_lock(-1)) {
                lv_label_set_text(status_label, "Capturing image...");
                lvgl_unlock();
              }
              // Set the processing stage to waiting for frame
              processing_stage = 1;
            } else {
              // Resuming live feed
              if (lvgl_lock(-1)) {
                lv_label_set_text(status_label, "Touch IMAGE to capture");
                lvgl_unlock();
              }
              // Free the captured frame if it exists
              if (captured_pic != NULL) {
                esp_camera_fb_return(captured_pic);
                captured_pic = NULL;
              }
              // Reset processing stage
              processing_stage = 0;
            }
          }
        }
      }
    }
  }
  
  // Handle LVGL tasks
  if (lvgl_lock(10)) {  // Use a timeout to avoid blocking indefinitely
    lv_timer_handler(); /* let the GUI do its work */
    lvgl_unlock();
  }
  
  // Check WiFi connection periodically with enhanced watchdog functionality
  
  if (currentMillis - lastWifiCheckTime >= WIFI_RECHECK_INTERVAL) {
    lastWifiCheckTime = currentMillis;
    
    // Only check if no request is in progress to avoid interference
    if (!wifiRequestInProgress && !is_response_visible) {
      int wifiStatus = WiFi.status();
      int rssi = WiFi.RSSI();
      
      // Log WiFi status occasionally
      Serial.printf("WiFi watchdog: Status=%d, RSSI=%d dBm\n", wifiStatus, rssi);
      
      // If disconnected or very weak signal, attempt reconnection
      if (wifiStatus != WL_CONNECTED || rssi < MIN_RSSI - 10) { // Even stricter RSSI requirement for watchdog
        Serial.println("WiFi watchdog: Connection issue detected");
        
        // Don't try to fix if processing is happening, just log the issue
        if (processing_stage > 0) {
          Serial.println("Skipping reconnection since processing is in progress");
        } else {
          reconnectWiFi();
        }
      }
    }
  }
  
  // Small delay to prevent CPU hogging
  vTaskDelay(pdMS_TO_TICKS(5));
}
