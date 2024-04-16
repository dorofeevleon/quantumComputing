
#define PIN_LED1   D0// 0 //  D0
#define PIN_LED2    D1 //  D1
#define PIN_red     D5 //  D5
#define PIN_green   D8 //  D8
//D4 -> Achtung bei Nutzung da kritisch für Programmierbetrieb
#define PIN_blue    D6 //  D6
#define PIN_WS2812B 7 //D7

#define myBaudrate 38400

#define LED_COUNT 113 // number of connected WS2812B

// EEPRM storage definitions
struct Lampenparameter {
    uint8_t LED1;//on or off?
    uint8_t LED2;//on or off?
    uint8_t RGBStatus; //on or off?
    uint32_t RGB_rgb;
    uint8_t StripStatus; //on or off?
    uint32_t Strip_rgb;
    int Strip_F; //Effekt
    uint8_t Brightness;
    byte changed;
    long stamp;
};

struct Lampenparameter myLight;

String myFileName;
