#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include "UI_images.h"
#include <HardwareSerial.h>
#define TFT_BL 2
#define GFX_BL DF_GFX_BL 

HardwareSerial ReceivedLayar(1);

lv_obj_t *container;
lv_obj_t *preasureTitle;
lv_obj_t *voltTitle;
lv_obj_t *fireTemperatureTitle;
lv_obj_t *wattmeterTitle;
lv_obj_t *temperatureTitle;
lv_obj_t *preasureFooter;
lv_obj_t *voltFooter;
lv_obj_t *wattmeterFooter;
lv_obj_t *judul;
lv_obj_t * bar1;
lv_obj_t * bar2;
lv_obj_t * fireTemperature; // BAR 3 DIHAPUS KARENA TAK TERPAKAI DAN DIGANTI SUHU API
lv_obj_t * bar4;
lv_obj_t * indikator1;
lv_obj_t * indikator2;
lv_obj_t * indikator3;
lv_obj_t * indikator4;
lv_obj_t * temperature;
float GLBsensor_tekanan;
float GLBsensor_suhu;
float GLBsensor_wattmeter_v = 120;
float GLBsensor_suhu_api = 270;
int GLBsensor_wattmeter = 20;
String receivedData = "";
String sGLBsensor_tekanan = "";
String sGLBsensor_suhu = "";
String sGLBsensor_wattmeter_v = "";

#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED /* CS */, GFX_NOT_DEFINED /* SCK */, GFX_NOT_DEFINED /* SDA */,
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */
);
// option 1:
// ILI6485 LCD 480x272
Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(
  bus,
  480 /* width */, 0 /* hsync_polarity */, 8 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 43 /* hsync_back_porch */,
  272 /* height */, 0 /* vsync_polarity */, 8 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 12 /* vsync_back_porch */,
  1 /* pclk_active_neg */, 9000000 /* prefer_speed */, true /* auto_flush */);
#endif /* !defined(DISPLAY_DEV_KIT) */
#include "touch.h"

/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;







void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  if (touch_has_signal())
  {
    if (touch_touched())
    {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
    }
    else if (touch_released())
    {
      data->state = LV_INDEV_STATE_REL;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

//////////////////////////////////////////////////////////////FUNGSI-FUNGSI UTAMA/////////////////////////


void PKbitMap(int width, int totalPixel, int offsetX, int offsetY, const uint16_t input[]) {
    int totalPixels = totalPixel; // Hitung jumlah total piksel dalam array input
    int height = (totalPixels + width - 1) / width; // Hitung tinggi grid berdasarkan jumlah total piksel dan lebar

    for (int i = 0; i < totalPixels; i++) {
        int x = offsetX + (i % width); // Hitung posisi X dengan offset
        int y = offsetY + (i / width); // Hitung posisi Y dengan offset
        gfx->drawPixel(x, y, input[i % totalPixels]); // Gambar piksel dengan warna dari input
    }
}








void drawImage(int x, int y) {
    // Replace with your own drawing function or image
    gfx->fillRect(x, y, 50, 50, RED); // Draw a red rectangle as a placeholder
}















































void setup()
{
  Serial.begin(115200);
  ReceivedLayar.begin(115200, SERIAL_8N1, 18, 17);
  Serial.println("Penerima siap...");
  gfx->begin();
#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif
  delay(3000);// BARIS INI PENTING GASAN DELAY TOUCHSCREEN SUPAYA BISA KE BACA minimal 2000 milidetik
  lv_init();
  delay(10);
  touch_init();
  screenWidth = gfx->width();
  screenHeight = gfx->height();
#ifdef ESP32
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * screenHeight/4 , MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#else
  disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * screenWidth * screenHeight/4);
#endif
  if (!disp_draw_buf)
  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  }
  else
  {
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * screenHeight/4);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* Initialize the (dummy) input device driver */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    
    Serial.println("Setup done");


////////////////////////////////////////////////////////////////////////////MENCETAK LVGL///////////////////////////////////////////////

  // Buat container yang lebih tinggi dari layar
  container = lv_obj_create(lv_scr_act());
  lv_obj_set_size(container, 480, 600); // Container lebih tinggi dari layar
  lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_AUTO); // Aktifkan scrollbar

  //buat text judul
  judul = lv_label_create(container);
  lv_label_set_text(judul, "Bioner-s Monitoring System");
  lv_obj_set_style_text_color(judul, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(judul, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_align(judul, LV_ALIGN_TOP_MID, 0, 20);

  //buat text preasureTitle
  preasureTitle = lv_label_create(container);
  lv_label_set_text(preasureTitle, "Steam\nPreasure");
  lv_obj_set_style_text_color(preasureTitle, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(preasureTitle, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(preasureTitle, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(preasureTitle, LV_ALIGN_TOP_MID, -144, 60);

//buat text voltTitle                                   //waterTitle diganti voltage karena tak diperlukan
  voltTitle = lv_label_create(container);
  lv_label_set_text(voltTitle, "Voltage");
  lv_obj_set_style_text_color(voltTitle, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(voltTitle, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(voltTitle, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(voltTitle, LV_ALIGN_TOP_MID, -48, 60);

//buat text fireTemperatureTitle
  fireTemperatureTitle = lv_label_create(container);
  lv_label_set_text(fireTemperatureTitle, "Fire\nTemp.");
  lv_obj_set_style_text_color(fireTemperatureTitle, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(fireTemperatureTitle, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(fireTemperatureTitle, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(fireTemperatureTitle, LV_ALIGN_TOP_MID, 48, 60);

// //buat text wattmeterTitle
//   wattmeterTitle = lv_label_create(container);
//   lv_label_set_text(wattmeterTitle, "Watt");
//   lv_obj_set_style_text_color(wattmeterTitle, lv_color_hex(0x000000), LV_PART_MAIN);
//   lv_obj_set_style_text_font(wattmeterTitle, &lv_font_montserrat_14, LV_PART_MAIN);
//   lv_obj_set_style_text_align(wattmeterTitle, LV_TEXT_ALIGN_CENTER, 0);
//   lv_obj_align(wattmeterTitle, LV_ALIGN_TOP_MID, 0, 60);          //WATTMETER tidak digunakan karena arus tak bisa diukur

//buat text temperatureTitle
  temperatureTitle = lv_label_create(container);
  lv_label_set_text(temperatureTitle, "Water\nTemp.");
  lv_obj_set_style_text_color(temperatureTitle, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(temperatureTitle, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(temperatureTitle, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(temperatureTitle, LV_ALIGN_TOP_MID, 144, 60);

  //buat text preasureFooter
  preasureFooter = lv_label_create(container);
  lv_label_set_text(preasureFooter, "Bar");
  lv_obj_set_style_text_color(preasureFooter, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(preasureFooter, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(preasureFooter, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(preasureFooter, LV_ALIGN_TOP_MID, -144, 220);

//buat text voltFooter                                  //waterfooter diganti voltFooter
  voltFooter = lv_label_create(container);
  lv_label_set_text(voltFooter, "V");
  lv_obj_set_style_text_color(voltFooter, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(voltFooter, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_text_align(voltFooter, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(voltFooter, LV_ALIGN_TOP_MID, -48, 220);

// //buat text wattmeterFooter
//   wattmeterFooter = lv_label_create(container);
//   lv_label_set_text(wattmeterFooter, "Watt");
//   lv_obj_set_style_text_color(wattmeterFooter, lv_color_hex(0x000000), LV_PART_MAIN);
//   lv_obj_set_style_text_font(wattmeterFooter, &lv_font_montserrat_14, LV_PART_MAIN);
//   lv_obj_set_style_text_align(wattmeterFooter, LV_TEXT_ALIGN_CENTER, 0);
//   lv_obj_align(wattmeterFooter, LV_ALIGN_TOP_MID, 0, 220);             ////WATTMETER tidak digunakan karena arus tak bisa diukur


    // Buat bar tekanan
    bar1 = lv_bar_create(container);
    lv_obj_set_size(bar1, 10, 100);
    lv_obj_align(bar1, LV_ALIGN_CENTER, -144, -130);
    lv_bar_set_range(bar1, 0, 12);
    lv_bar_set_value(bar1, 0, LV_ANIM_OFF);

    // Buat bar voltage                           //water level diganti volatage karena tak dipakai
    bar2 = lv_bar_create(container);
    lv_obj_set_size(bar2, 10, 100);
    lv_obj_align(bar2, LV_ALIGN_CENTER, -48, -130);
    lv_bar_set_range(bar2, 0, 500);
    lv_bar_set_value(bar2, 0, LV_ANIM_OFF);
                                          ///BAR KE 3 DIHAPUS KARENA BAR KETIGA TAK DIPAKAI DAN DIGANTI MENJADI FIRETEMPERATURE
    // Buat bar watt ~~~~~~WATTMETER tidak digunakan karena arus tak bisa diukur
    // bar4 = lv_bar_create(container);
    // lv_obj_set_size(bar4, 10, 100);
    // lv_obj_align(bar4, LV_ALIGN_CENTER, 0, -130);
    // lv_bar_set_range(bar4, 0, 2500);
    // lv_bar_set_value(bar4, 0, LV_ANIM_OFF);
   

    //buat text bar
  indikator1 = lv_label_create(container);
  lv_obj_set_style_text_color(indikator1, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(indikator1, &lv_font_montserrat_12, LV_PART_MAIN);

  //buat text bar
  indikator2 = lv_label_create(container);
  lv_obj_set_style_text_color(indikator2, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(indikator2, &lv_font_montserrat_12, LV_PART_MAIN);

  //buat text bar
  fireTemperature = lv_label_create(container);
  lv_obj_set_style_text_color(fireTemperature, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(fireTemperature, &lv_font_montserrat_16, LV_PART_MAIN);
  lv_obj_align(fireTemperature, LV_ALIGN_CENTER, 48, -130);

  // //buat text bar ~WATTMETER tidak digunakan karena arus tak bisa diukur
  // indikator4 = lv_label_create(container);
  // lv_obj_set_style_text_color(indikator4, lv_color_hex(0x000000), LV_PART_MAIN);
  // lv_obj_set_style_text_font(indikator4, &lv_font_montserrat_12, LV_PART_MAIN);

      //buat text bar
  temperature = lv_label_create(container);
  lv_obj_set_style_text_color(temperature, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(temperature, &lv_font_montserrat_16, LV_PART_MAIN);
  lv_obj_align(temperature, LV_ALIGN_CENTER, 144, -130);

  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////   

  }
}









void loop()
{
 // Serial.println(touch_last_x);
 // Serial.println(touch_last_y);

if (ReceivedLayar.available()) {
    while (ReceivedLayar.available()) {
        char c = ReceivedLayar.read();
        if (c == '\n') { // Memeriksa akhir baris
            Serial.println(receivedData); // Cetak data yang diterima
            // Proses data sebelum buffer dikosongkan
            String dummy = receivedData.substring(0, 4);
            String dummy2 = receivedData.substring(5, 10);
            String dummy3 = receivedData.substring(12, 17);
            String dummy4 = receivedData.substring(19, 24);
            GLBsensor_tekanan = dummy.toFloat();
            GLBsensor_suhu = dummy2.toFloat();
            GLBsensor_wattmeter_v = dummy3.toFloat();
            GLBsensor_suhu_api = dummy4.toFloat();
            Serial.println("dummy: " + dummy);
            Serial.println("dummy2: " + dummy2);
            Serial.println("dummy3: " + dummy3);
            Serial.println("dummy4: " + dummy4);

            receivedData = ""; // Kosongkan buffer untuk data berikutnya
        } else {
            receivedData += c; // Tambahkan karakter ke buffer
        }
    }
    }

//~~~~~~~~~~~~~~~~~~~BAR AND TEKS TES RANDOM DATA ~~~~~~~~~~~~~~
// GLBsensor_tekanan = 8.00 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (8.79 - 8.00)));
// GLBsensor_wattmeter_v = 120 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (124 - 120)));
// GLBsensor_wattmeter = 50 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (52 - 50)));
// GLBsensor_suhu_api = 270.00 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (270.75 - 270.00)));
// GLBsensor_suhu = 134.00 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (134.75 - 134.00)));

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    
//SENSOR TEKANAN
    // float sensor_tekanan = rand() % 12 + 1 ; 
    // float sensor_tekanan = 9.67; 
   // Serial.println(GLBsensor_tekanan);
    lv_bar_set_value(bar1, GLBsensor_tekanan, LV_ANIM_OFF); 
    if(lv_bar_get_value(bar1) >= 0 && lv_bar_get_value(bar1) < 4 ){
    lv_obj_set_style_bg_color(bar1, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
    }else if(lv_bar_get_value(bar1) >= 4 && lv_bar_get_value(bar1) < 8 ){
    lv_obj_set_style_bg_color(bar1, lv_color_hex(0xFFFF00), LV_PART_INDICATOR); 
    }
    else {
    lv_obj_set_style_bg_color(bar1, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
    }
    float a;
    char b[50];
    a = 0.00 + GLBsensor_tekanan;//DISINI NILAI 22 DI GANTI KE NILAI DARI SENSOR
    sprintf(b, "%0.3f", a);
    lv_label_set_text(indikator1, b);
    // Dapatkan posisi bar
    lv_coord_t bar1_x = lv_obj_get_x(bar1);
    lv_coord_t bar1_y = lv_obj_get_y(bar1);
    lv_coord_t bar1_h = lv_obj_get_height(bar1);
    // Hitung posisi label untuk menempatkannya di tengah bar
    lv_coord_t label1_x = bar1_x - lv_obj_get_width(bar1);
    lv_coord_t label1_y = bar1_y + bar1_h - (bar1_h * GLBsensor_tekanan / lv_bar_get_max_value(bar1)) - lv_obj_get_height(indikator1) / 2;
    lv_obj_set_pos( indikator1, label1_x, label1_y);


//VOLTAGE
 //float sensor_ultrasonic = rand() % 100 + 1 ; 
    // float sensor_ultrasonic = 27.67; 
    //Serial.println(GLBsensor_wattmeter_v);
    lv_bar_set_value(bar2, GLBsensor_wattmeter_v, LV_ANIM_OFF); 
    if(lv_bar_get_value(bar2) >= 0 && lv_bar_get_value(bar2) < 220 ){
    lv_obj_set_style_bg_color(bar2, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
    }else if(lv_bar_get_value(bar2) >= 220 && lv_bar_get_value(bar2) < 300 ){
    lv_obj_set_style_bg_color(bar2, lv_color_hex(0xFFFF00), LV_PART_INDICATOR); 
    }
    else {
    lv_obj_set_style_bg_color(bar2, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
    }
    int c;
    char d[50];
    c = 0 + GLBsensor_wattmeter_v;//DISINI NILAI 22 DI GANTI KE NILAI DARI SENSOR
    sprintf(d, "%d", c);
    lv_label_set_text(indikator2, d);
    // Dapatkan posisi bar
    lv_coord_t bar2_x = lv_obj_get_x(bar2);
    lv_coord_t bar2_y = lv_obj_get_y(bar2);
    lv_coord_t bar2_h = lv_obj_get_height(bar2);
    // Hitung posisi label untuk menempatkannya di tengah bar
    lv_coord_t label2_x = bar2_x - 2;
    lv_coord_t label2_y = bar2_y + bar2_h - (bar2_h * GLBsensor_wattmeter_v / lv_bar_get_max_value(bar2)) - lv_obj_get_height(indikator2) / 2;
    lv_obj_set_pos( indikator2, label2_x, label2_y);

//SENSOR SUHU API
    float k;
    char l[50];
    char Fireper[5] = " °C";
    char resFireTemp[60];
    k = 0 + GLBsensor_suhu_api;//DISINI NILAI 22 DI GANTI KE NILAI DARI SENSOR
    sprintf(l, "%0.2f", k);
    sprintf(resFireTemp, "%s%s", l, Fireper);;
    lv_label_set_text(fireTemperature, resFireTemp);


//WATTMETER   ~~~~~WATTMETER tidak digunakan karena arus tak bisa diukur
    //float sensor_wattmeter = rand() % 100 + 1 ; 
    //int GLBsensor_wattmeter = 2000; 
   // Serial.println(GLBsensor_wattmeter);
    // lv_bar_set_value(bar4, GLBsensor_wattmeter, LV_ANIM_OFF); 
    // if(lv_bar_get_value(bar4) >= 0 && lv_bar_get_value(bar4) <= 450 ){
    // lv_obj_set_style_bg_color(bar4, lv_color_hex(0xFF7D29), LV_PART_INDICATOR);
    // }else if(lv_bar_get_value(bar4) > 450 && lv_bar_get_value(bar4) <= 900 ){
    // lv_obj_set_style_bg_color(bar4, lv_color_hex(0x55AD9B), LV_PART_INDICATOR); 
    // }else if(lv_bar_get_value(bar4) > 900 && lv_bar_get_value(bar4) <= 1300 ){
    // lv_obj_set_style_bg_color(bar4, lv_color_hex(0x5AB2FF), LV_PART_INDICATOR);
    // }else {
    // lv_obj_set_style_bg_color(bar4, lv_color_hex(0xF0EBE3), LV_PART_INDICATOR);
    // }
    // int g;
    // char h[50];
    // g = 0 + GLBsensor_wattmeter;//DISINI NILAI 22 DI GANTI KE NILAI DARI SENSOR
    // sprintf(h, "%d", g);
    // lv_label_set_text(indikator4, h);
    // // Dapatkan posisi bar
    // lv_coord_t bar4_x = lv_obj_get_x(bar4);
    // lv_coord_t bar4_y = lv_obj_get_y(bar4);
    // lv_coord_t bar4_h = lv_obj_get_height(bar4);
    // // Hitung posisi label untuk menempatkannya di tengah bar
    // lv_coord_t label4_x = bar4_x - 10;
    // lv_coord_t label4_y = bar4_y + bar4_h - (bar4_h * GLBsensor_wattmeter / lv_bar_get_max_value(bar4)) - lv_obj_get_height(indikator4) / 2;
    // lv_obj_set_pos( indikator4, label4_x, label4_y);

//  float sensor_suhu = 221.31;
    float i;
    char j[50];
    char per[5] = " °C";
    char resTemp[60];
    i = 0 + GLBsensor_suhu;//DISINI NILAI 22 DI GANTI KE NILAI DARI SENSOR
    sprintf(j, "%0.2f", i);
    sprintf(resTemp, "%s%s", j, per);;
    lv_label_set_text(temperature, resTemp);

  lv_timer_handler(); /* let the GUI do its work */
  delay(5); 
   


} 



