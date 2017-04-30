//************************************************************************
//
//  SSA DESK LAMP
//  Silicon STEM Academy
//  Written by the Students of
//  Programming and Engineering Practicum Courses
//  Winter Session 2017
//  Instructors
//     Kyle Yamashita  (Programming Practicum)
//     Steven Kibler (Engineering Practicum)
//
//  Revision 1.0
//  Date Mar 07th, 2017
//
//  NOTE: Read the instructions in the README.md file 
//  for setup procedure for your specific lamp.
//
//*************************************************************************

// Required Libraries to use
// The following libraries are installed along with the ESP8266 Package
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

//The following libraries need to be installed.  
//Follow the instructions in the installation guide.
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <Adafruit_LIS3DH.h>

//Uncomment below if you want debug information printed to the console
//#define DEBUG 

//macros
#define NUMPIXELS  60
#define PIXPIN     12
#define NUMPIXELS_IN_ONE_LOOP   38

//Instantiate the accelerometer
Adafruit_LIS3DH xyz = Adafruit_LIS3DH();

//Instantiate the NeoPixels
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIXPIN, NEO_GRB + NEO_KHZ800);

//Instantiate the web server and dns server
ESP8266WebServer server(80);
MDNSResponder mdns;

//Wifi Requirements
WiFiClient client;
DynamicJsonBuffer jsonBuffer;

IPAddress ip;
uint8_t ip_byte_array[4];

// Replace with your home network credentials
const char* ssid = "enter_your_router_name_here";
const char* password =  "enter_your_wifi_password_here";


// Lamp constants
uint8_t state = 7;
#define OFF        0
#define ALL_WHITE  1
#define ALL_BLUE   2
#define ALL_GREEN  3
#define ALL_RED    4
#define BOUNCE     5
#define WAKE_UP    6
#define RAINBOW    7
#define STATIC_RAINBOW  8
#define NIGHT_NIGHT  9
#define CUSTOM_COLOR 10

uint16_t brightness = 0;

uint32_t last_change_time = 0;
uint16_t change_delay = 10;

uint32_t last_analog_time = 0;
uint16_t analog_delay = 50;

uint8_t userRed = 128;
uint8_t userGreen = 128;
uint8_t userBlue = 128;
uint8_t userBrightness = 128;

boolean webControl = false;
boolean apiCalled = false;

String weatherData = "";
String webPage = "";

void getParameters(void) {
  userBrightness = server.arg("B").toInt();
  userRed = server.arg("R").toInt();
  userGreen = server.arg("G").toInt();
  userBlue = server.arg("L").toInt();
  server.send(200, "text/html", server.arg("T"));
}


void setup() {

  strip.begin();
  Serial.begin(115200);

#ifdef DEBUG
  Serial.println("System Started");
#endif

  if (!xyz.begin())
  {
#ifdef DEBUG
    Serial.println("System Failed to start!");
#endif
    while (1);
  }

#ifdef DEBUG
  Serial.println("MMA Found!");
#endif

  xyz.setRange(LIS3DH_RANGE_2_G);

#ifdef DEBUG
  Serial.print("Range = "); Serial.print(2 << xyz.getRange());
  Serial.println("G");
#endif

  // WiFi and lamp initial setup
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.println("Establishing WiFi connection");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  ip = WiFi.localIP();
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(ip);

  set_webpage();
 
  // ---------------------------------------------------
  // Complete Arduino side of server sockets here:
  // ---------------------------------------------------
  // server.on("/LINK_NAME_HERE", ARDUINO_FUNCTION [](){
  //   server.send(200, "text/html", webPage);
  // });
  // ---------------------------------------------------
  server.on("/", []() {
    server.send(200, "text/html", webPage);
  });
  server.on("/Off", []() {
    server.send(200, "text/html", webPage);
    state = OFF;
  });
  server.on("/All_White", []() {
    server.send(200, "text/html", webPage);
    state = ALL_WHITE;
  });
  server.on("/All_Red", []() {
    server.send(200, "text/html", webPage);
    state = ALL_RED;
  });
  server.on("/All_Green", []() {
    server.send(200, "text/html", webPage);
    state = ALL_GREEN;
  });
  server.on("/All_Blue", []() {
    server.send(200, "text/html", webPage);
    state = ALL_BLUE;
  });
  server.on("/Bounce", []() {
    server.send(200, "text/html", webPage);
    state = BOUNCE;
  });
  server.on("/Wake_Up", []() {
    server.send(200, "text/html", webPage);
    state = WAKE_UP;
  });
  server.on("/Rainbow", []() {
    server.send(200, "text/html", webPage);
    state = RAINBOW;
  });
  server.on("/Static_Rainbow", []() {
    server.send(200, "text/html", webPage);
    state = STATIC_RAINBOW;
  });
  server.on("/Night_Night", []() {
    server.send(200, "text/html", webPage);
    state = NIGHT_NIGHT;
  });
  server.on("/Toggle", []() {
    server.send(200, "text/html", webPage);
    webControl = !webControl;
    userBrightness = 128;
  });
  server.on("/Custom", []() {
    server.send(200, "text/html", webPage);
    state = CUSTOM_COLOR;
  });
  server.on("/Parameters.html", getParameters);
  server.begin();
  Serial.println("HTTP server started");


  // API setup -- Follow steps and remove comments (select lines and press: "ctrl" + "/" )
//  if (client.connect("api.openweathermap.org", 80)) {
//    Serial.println("connection established");
//
//    /* From openweathermap.org, sign up for a free account to get a key
//     * Copy the key and on the following line insert this after GET:
//     * /data/2.5/weather?zip=80222,us&APPID=[YOUR KEY HERE]
//     * Remember to update with your own ZIP code!
//    */
//    
//    client.println("GET /data/2.5/weather?zip=80222,us&APPID=23401918dd84c270e4aacba1cc73b821");
//    client.println();
//    client.println("connection: close");
//    client.println();
//
//  } else {
//    Serial.println("connection failed");
//  }
}

//************************************************************************
//  GET STATE
//  
//  This function reads the current state of the accelerometer and 
//  makes a decision about which state it will be in.  If you make your
//  own state functions, you'll need to replace one of the CAPITAL states
//  with the name of one of your own states.
//************************************************************************
uint8_t get_state(void)
{
  xyz.read();

  if (xyz.z > 8000)
  {
    if (xyz.y > 0)
    {
      change_delay = 10;
      state = BOUNCE;
    }
    else
    {
      change_delay = 10;
      state = WAKE_UP;
    }
  }
  else if (xyz.z > 0)
  {
    if (xyz.y > 0)
    {
      change_delay = 10;
      state = ALL_BLUE;
    }
    else
    {
      change_delay = 10;
      state = ALL_GREEN;
    }
  }
  else if (xyz.z > -8000)
  {
    if (xyz.y > 0)
    {
      change_delay = 10;
      state = STATIC_RAINBOW;
    }
    else
    {
      change_delay = 10;
      state = NIGHT_NIGHT;
    }
  }
  else
  {
    if (xyz.y > 0)
    {
      change_delay = 10;
      state = ALL_WHITE;
    }
    else
    {
      change_delay = 10;
      state = RAINBOW;
    }
  }

#ifdef DEBUG
  Serial.print("X:  "); Serial.print(xyz.x);
  Serial.print("\tY:  "); Serial.print(xyz.y);
  Serial.print("\tZ:  "); Serial.print(xyz.z);
  Serial.print("\tState:  "); Serial.print(state);
  Serial.println();
#endif

}



void all_off(void)
{
  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, 0);
  }
  strip.show();
}

void all_blue(void)
{
  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, strip.Color(0, 0, userBrightness));
  }
  strip.show();
}

void all_red(void)
{
  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, strip.Color(userBrightness, 0, 0));
  }
  strip.show();
}

void all_green(void)
{
  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, strip.Color(0, userBrightness, 0));
  }
  strip.show();
}

void all_white(void)
{
  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, strip.Color(userBrightness, userBrightness, userBrightness));
  }
  strip.show();
}

void night_night(void)
{
  uint8_t red = 255;
  uint8_t green = 0;
  uint8_t blue = 0;
  static uint32_t last_bright_change_time = 0;
  static uint32_t first_time_in_night_night = 0;

  if ( ( millis() - last_bright_change_time ) > ( 64000 ) ) //this is approximaly 9 times the change delay.  So if we're coming back to this state after being gone for awhile, do this...
  {
    red = 255;
    green = 0;
    blue = 0;
    first_time_in_night_night = millis();
  }

  if ( ( millis() - last_bright_change_time) > 7100)
  {
    if ( ( millis() - first_time_in_night_night ) < 28800000)
    {
      if (--red < 10) red = 10;
      blue = 0;
      green = 0;

    }
    else
    {
      if (++red > 254) red = 254;
      blue = 0;
      green = 0;
    }
  }

  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, strip.Color(red, green, blue));
  }
  strip.show();

}

void wake_up(void)
{

  float bright;
  uint8_t red;
  uint8_t green;
  uint8_t blue;

  if (++brightness >= 1000) brightness = 1000;
  bright = brightness / 1000.0;
  red = (uint8_t)(255.0 * bright);
  green = (uint8_t)(100.0 * bright);
  blue = (uint8_t)(100.0 * bright);

  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, strip.Color(red, green, blue));
  }
  strip.show();
}

void rainbow(void)
{
  static uint8_t red = 255;
  static uint8_t green = 0;
  static uint8_t blue = 0;
  static uint8_t color_state = 0;

  switch (color_state)
  {
    case 0:
      if (--red == 0)
      {
        color_state = 1;
      }
      ++blue;
      green = 0;
      break;
    case 1:
      if (--blue == 0)
      {
        color_state = 2;
      }
      ++green;
      red = 0;
      break;
    case 2:
      if (--green == 0)
      {
        color_state = 0;
      }
      ++red;
      blue = 0;
      break;
    default:
      color_state = 0;
      red = 255;
      green = 0;
      blue = 0;
      break;
  }

  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, strip.Color(red, green, blue));
  }
  strip.show();

}

void bounce(void)
{
  static uint16_t pos = 0;

  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, strip.Color(0, 0, (uint8_t)(userBrightness / 2.5)));
  }
  strip.setPixelColor(pos, strip.Color(userBrightness, 0, 0));
  strip.show();

  if (++pos >= NUMPIXELS) pos = 0;


}

void static_rainbow(void)
{
  int16_t red = 255;
  int16_t green = 0;
  int16_t blue = 0;
  uint8_t color_state = 0;
  uint8_t step_size = (uint8_t)(255.0 / (NUMPIXELS_IN_ONE_LOOP / 3.0));

  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    switch (color_state)
    {
      case 0:
        blue += step_size;
        if (blue > 255)
        {
          blue = 255;
        }
        green = 0;
        red -= step_size;
        if (--red <= 0)
        {
          red = 0;
          color_state = 1;
        }
        break;
      case 1:
        green += step_size;
        if (green > 255)
        {
          green = 255;
        }
        red = 0;
        blue -= step_size;
        if (blue <= 0)
        {
          blue = 0;
          color_state = 2;
        }
        break;
      case 2:
        red += step_size;
        if (red > 255)
        {
          red = 255;
        }
        blue = 0;
        green -= step_size;
        if (green <= 0)
        {
          green = 0;
          color_state = 0;
        }
        break;
      default:
        color_state = 0;
        red = 255;
        green = 0;
        blue = 0;
        break;
    }
    strip.setPixelColor(ii, strip.Color(red, green, blue));
  }
  strip.show();
}

void custom_color(void)
{
  for (int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, strip.Color(userRed, userGreen, userBlue));
  }
  strip.show();
}

void callWeather(void) {

  if (!apiCalled) {

    // API data extraction
    while (client.available()) {
      char c = client.read();
      weatherData += c;
    }
    Serial.println("API data received");

    // API close connection
    Serial.println("disconnecting.");
    client.stop();

    // Weather data parse
    int buffer_len = weatherData.length() + 1;
    char output[buffer_len];
    weatherData.toCharArray(output, buffer_len);

    // Deserialize the JSON string
    JsonObject& root = jsonBuffer.parseObject(output);

    if (!root.success())
    {
      Serial.println("parseObject() failed");
    }

    // Retrieve the values
    // Declare these variables above setup if you want to use them in a function!
    double      lon         = root["coord"]["lon"];
    double      lat         = root["coord"]["lat"];
    int         id          = root["weather"][0]["id"];
    String      main        = root["weather"][0]["main"];
    String      description = root["weather"][0]["description"];
    String      icon        = root["weather"][0]["icon"];
    String      base        = root["base"];
    double      temp        = root["main"]["temp"];
    double      pressure    = root["main"]["pressure"];
    int         humidity    = root["main"]["humidity"];
    double      temp_min    = root["main"]["temp_min"];
    double      temp_max    = root["main"]["temp_max"];
    double      sea_level   = root["main"]["sea_level"];
    double      grnd_level  = root["main"]["grnd_level"];
    double      wind_speed  = root["wind"]["speed"];
    double      deg         = root["wind"]["deg"];
    int         clouds      = root["clouds"]["all"];

    Serial.print("longitude: ");  Serial.println(lon);
    Serial.print("latitude: ");  Serial.println(lat);
    Serial.print("id: ");  Serial.println(id);
    Serial.print("main description: ");  Serial.println(main);
    Serial.print("description: ");  Serial.println(description);
    Serial.print("icon: ");  Serial.println(icon);
    Serial.print("base: ");  Serial.println(base);
    Serial.print("temperature: ");  Serial.println(temp);
    Serial.print("pressure: ");  Serial.println(pressure);
    Serial.print("humidity: ");  Serial.println(humidity);
    Serial.print("min temp: ");  Serial.println(temp_min);
    Serial.print("max temp: ");  Serial.println(temp_max);
    Serial.print("sea level: ");  Serial.println(sea_level);
    Serial.print("ground level: ");  Serial.println(grnd_level);
    Serial.print("wind speed: ");  Serial.println(wind_speed);
    Serial.print("wind direction: ");  Serial.println(deg);
    Serial.print("cloud status: ");  Serial.println(clouds);

    apiCalled = true;
  }
}

void loop() {

  server.handleClient();

  // Use this function to access the weather API above
  // callWeather();

  if ((millis() - last_analog_time) > analog_delay)
  {
    last_analog_time = millis();
    if (!webControl) {
      get_state();
    }
  }

  if ((millis() - last_change_time) > change_delay)
  {
    last_change_time = millis();

    switch (state)
    {
      case OFF:
        all_off();
        break;
      case ALL_WHITE:
        all_white();
        break;
      case ALL_BLUE:
        all_blue();
        break;
      case ALL_GREEN:
        all_green();
        break;
      case ALL_RED:
        all_red();
        break;
      case BOUNCE:
        bounce();
        break;
      case WAKE_UP:
        wake_up();
        break;
      case RAINBOW:
        rainbow();
        break;
      case STATIC_RAINBOW:
        static_rainbow();
        break;
      case NIGHT_NIGHT:
        night_night();
        break;
      case CUSTOM_COLOR:
        custom_color();
        break;
      default:
        break;
    }
  }//end last_change_time
}//end loop


void your_function_name(void)
{
  static uint8_t red_value = 0;
  static uint8_t green_value = 0;
  static uint8_t blue_value = 0;
  
  //type your code here

  for(int ii = 0; ii < NUMPIXELS; ++ii)
  {
    strip.setPixelColor(ii, strip.Color(red_value, green_value, blue_value));
  }
}


void set_webpage(void)
{

 // ---------------------------------------------------------------
  // Web page inserted here as a single String
  // Enclose web page like the line below (remove comment)
  // "<INSERT YOUR HTML HERE>\n"
  // ---------------------------------------------------------------


  webPage +=  "<!DOCTYPE html>\n"
              "<html>\n"
              "<head>\n"
              "<meta charset='ISO-8859-1'>\n"
              "<title>Lamp Web Server</title>\n"
              "<style>\n"
              // -----------------------
              // Create CSS styles here:
              // -----------------------
              "  h1 {\n"
              "    font-size:48px;\n"
              "  }\n"
              "  h2 {\n"
              "    font-size:20px;\n"
              "  }\n"
              // -----------------------
              // Button CSS from online:
              // -----------------------              
              "  button::before {  \n"
              "    -webkit-border-radius: 3px;  \n"
              "    -moz-border-radius: 3px;  \n"
              "    -webkit-box-shadow: #959595 0 2px 5px;  \n"
              "    -moz-box-shadow: #959595 0 2px 5px;  \n"
              "    border-radius: 3px;  \n"
              "    box-shadow: #959595 0 2px 5px;  \n"
              "    content: "";  \n"
              "    display: block;  \n"
              "    height: 100%;  \n"
              "    left: 0;  \n"
              "    padding: 2px 0 0;  \n"
              "    position: absolute;  \n"
              "    top: 0;  \n"
              "    width: 100%; \n"
              "  }  \n"
              "  button:active::before { \n"
              "    padding: 1px 0 0; \n"
              "  }  \n"
              "  button {  \n"
              "    -moz-box-shadow: inset 0 0 0 1px #63ad0d;  \n"
              "    -webkit-box-shadow: inset 0 0 0 1px #63ad0d;  \n"
              "    -moz-border-radius: 3px;  \n"
              "    -webkit-border-radius: 3px;  \n"
              "    background: #eee;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#eee), to(#e2e2e2));  \n"
              "    background: -moz-linear-gradient(#eee, #e2e2e2);  \n"
              "    background: linear-gradient(#eee, #e2e2e2);  \n"
              "    border: solid 1px #d0d0d0;  \n"
              "    border-bottom: solid 3px #b2b1b1;  \n"
              "    border-radius: 3px;  \n"
              "    box-shadow: inset 0 0 0 1px #f5f5f5;  \n"
              "    color: #555;  \n"
              "    display: inline-block;  \n"
              "    font: bold 12px Arial, Helvetica, Clean, sans-serif;  \n"
              "    margin: 0 25px 25px 0;  \n"
              "    padding: 10px 20px;  \n"
              "    position: relative;  \n"
              "    text-align: center;  \n"
              "    text-decoration: none;  \n"
              "    text-shadow: 0 1px 0 #fafafa; \n"
              "  }  \n"
              "  button:hover {  \n"
              "    background: #e4e4e4;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#e4e4e4), to(#ededed));  \n"
              "    background: -moz-linear-gradient(#e4e4e4, #ededed);  \n"
              "    background: linear-gradient(#e4e4e4, #ededed);  \n"
              "    border: solid 1px #c2c2c2;  \n"
              "    border-bottom: solid 3px #b2b1b1;  \n"
              "    box-shadow: inset 0 0 0 1px #efefef; \n"
              "  }  \n"
              "  button:active {  \n"
              "    background: #dfdfdf;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#dfdfdf), to(#e3e3e3));  \n"
              "    background: -moz-linear-gradient(#dfdfdf, #e3e3e3);  \n"
              "    background: linear-gradient(#dfdfdf, #e3e3e3);  \n"
              "    border: solid 1px #959595;  \n"
              "    box-shadow: inset 0 10px 15px 0 #c4c4c4;  \n"
              "    top:2px;  \n"
              "  }  \n"
              "  button.pink {  \n"
              "    background: #f997b0;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#f997b0), to(#f56778));  \n"
              "    background: -moz-linear-gradient(#f997b0, #f56778);  \n"
              "    background: linear-gradient(#f997b0, #f56778);  \n"
              "    border: solid 1px #ee8090;  \n"
              "    border-bottom: solid 3px #cb5462;  \n"
              "    box-shadow: inset 0 0 0 1px #fbc1d0;  \n"
              "    color: #913944;  \n"
              "    text-shadow: 0 1px 0 #f9a0ad; \n"
              "  }  \n"
              "  button.pink:hover {  \n"
              "    background: #f57184;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#f57184), to(#f78297));  \n"
              "    background: -moz-linear-gradient(#f57184, #f78297);  \n"
              "    background: linear-gradient(#f57184, #f78297);  \n"
              "    border: solid 1px #e26272;  \n"
              "    border-bottom: solid 3px #cb5462;  \n"
              "    box-shadow: inset 0 0 0 1px #f9aab5; \n"
              "  }  \n"
              "  button.pink:active {  \n"
              "    background: #f06a7c;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#f06a7c), to(#f56c7e));  \n"
              "    background: -moz-linear-gradient(#f06a7c, #f56c7e);  \n"
              "    background: linear-gradient(#f06a7c, #f56c7e);  \n"
              "    border: solid 1px #a14753;  \n"
              "    box-shadow: inset 0 10px 15px 0 #d45d6d; \n"
              "  }  \n"
              "  button.blue {  \n"
              "    background: #abe4f8;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#abe4f8), to(#74d0f4));  \n"
              "    background: -moz-linear-gradient(#abe4f8, #74d0f4);  \n"
              "    background: linear-gradient(#abe4f8, #74d0f4);  \n"
              "    border: solid 1px #8cc5d9;  \n"
              "    border-bottom: solid 3px #589cb6;  \n"
              "    box-shadow: inset 0 0 0 1px #cdeffb;  \n"
              "    color: #42788e;  \n"
              "    text-shadow: 0 1px 0 #b6e6f9; \n"
              "  }  \n"
              "  button.blue:hover {  \n"
              "    background: #80d4f5;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#80d4f5), to(#92dbf6));  \n"
              "    background: -moz-linear-gradient(#80d4f5, #92dbf6);  \n"
              "    background: linear-gradient(#80d4f5, #92dbf6);  \n"
              "    border: solid 1px #79acbe;  \n"
              "    border-bottom: solid 3px #589cb6;  \n"
              "    box-shadow: inset 0 0 0 1px #b2e6f8; \n"
              "  }  \n"
              "  button.blue:active {  \n"
              "    background: #89d2ee;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#89d2ee), to(#84cae6));  \n"
              "    background: -moz-linear-gradient(#89d2ee, #84cae6);  \n"
              "    background: linear-gradient(#89d2ee, #84cae6);  \n"
              "    border: solid 1px #5c8d9f;  \n"
              "    box-shadow: inset 0 10px 15px 0 #79b9d2; \n"
              "  }  \n"
              "  button.green {  \n"
              "    background: #cae285;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#cae285), to(#a3cd5a));  \n"
              "    background: -moz-linear-gradient(#cae285, #a3cd5a);  \n"
              "    background: linear-gradient(#cae285, #a3cd5a);  \n"
              "    border: solid 1px #aad063;  \n"
              "    border-bottom: solid 3px #799545;  \n"
              "    box-shadow: inset 0 0 0 1px #e0eeb6;  \n"
              "    color: #5d7731;  \n"
              "    text-shadow: 0 1px 0 #d0e5a4; \n"
              "  }  \n"
              "  button.green:hover {  \n"
              "    background: #abd164;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#abd164), to(#b9d972));  \n"
              "    background: -moz-linear-gradient(#abd164, #b9d972);  \n"
              "    background: linear-gradient(#abd164, #b9d972);  \n"
              "    border: solid 1px #98b85b;  \n"
              "    border-bottom: solid 3px #799545;  \n"
              "    box-shadow: inset 0 0 0 1px #cce3a1; \n"
              "  }  \n"
              "  button.green:active {  \n"
              "    background: #a4cb5d;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#a4cb5d), to(#9ec45a));  \n"
              "    background: -moz-linear-gradient(#a4cb5d, #9ec45a);  \n"
              "    background: linear-gradient(#a4cb5d, #9ec45a);  \n"
              "    border: solid 1px #6e883f;  \n"
              "    box-shadow: inset 0 10px 15px 0 #90b352; \n"
              "  }  \n"
              "  button.black {  \n"
              "    background: #656565;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#656565), to(#444));  \n"
              "    background: -moz-linear-gradient(#656565, #444);  \n"
              "    background: linear-gradient(#656565, #444);  \n"
              "    border: solid 1px #535353;  \n"
              "    border-bottom: solid 3px #414141;  \n"
              "    box-shadow: inset 0 0 0 1px #939393;  \n"
              "    color: #fff;  \n"
              "    text-shadow: 0 1px 0 #2f2f2f; \n"
              "  }  \n"
              "  button.black:hover {  \n"
              "    background: #4c4c4c;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#4c4c4c), to(#565656));  \n"
              "    background: -moz-linear-gradient(#4c4c4c, #565656);  \n"
              "    background: linear-gradient(#4c4c4c, #565656);  \n"
              "    border: solid 1px #464646;  \n"
              "    border-bottom: solid 3px #414141;  \n"
              "    box-shadow: inset 0 0 0 1px #818181; \n"
              "  }  \n"
              "  button.black:active {  \n"
              "    background: #474747;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#474747), to(#444));  \n"
              "    background: -moz-linear-gradient(#474747, #444);  \n"
              "    background: linear-gradient(#474747, #444);  \n"
              "    border: solid 1px #2f2f2f;  \n"
              "    box-shadow: inset 0 10px 15px 0 #3e3e3e; \n"
              "  }  \n"
              "  button.dark_grey {  \n"
              "    background: #d1d1d1;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#d1d1d1), to(#afafaf));  \n"
              "    background: -moz-linear-gradient(#d1d1d1, #afafaf);  \n"
              "    background: linear-gradient(#d1d1d1, #afafaf);  \n"
              "    border: solid 1px #b4b4b4;  \n"
              "    border-bottom: solid 3px #878787;  \n"
              "    box-shadow: inset 0 0 0 1px #e3e3e3;  \n"
              "    color: #555;  \n"
              "    text-shadow: 0 1px 0 #d6d6d6; \n"
              "  }  \n"
              "  button.dark_grey:hover {  \n"
              "    background: #b7b7b7;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#b7b7b7), to(#c2c2c2));  \n"
              "    background: -moz-linear-gradient(#b7b7b7, #c2c2c2);  \n"
              "    background: linear-gradient(#b7b7b7, #c2c2c2);  \n"
              "    border: solid 1px #a2a2a2;  \n"
              "    border-bottom: solid 3px #878787;  \n"
              "    box-shadow: inset 0 0 0 1px #d4d4d4; \n"
              "  }  \n"
              "  button.dark_grey:active {  \n"
              "    background: #afafaf;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#afafaf), to(#a9a9a9));  \n"
              "    background: -moz-linear-gradient(#afafaf, #a9a9a9);  \n"
              "    background: linear-gradient(#afafaf, #a9a9a9);  \n"
              "    border: solid 1px #757575;  \n"
              "    box-shadow: inset 0 10px 15px 0 #9a9a9a; \n"
              "  }  \n"
              "  button.orange {  \n"
              "    background: #feda71;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#feda71), to(#febe4d));  \n"
              "    background: -moz-linear-gradient(#feda71, #febe4d);  \n"
              "    background: linear-gradient(#feda71, #febe4d);  \n"
              "    border: solid 1px #eab551;  \n"
              "    border-bottom: solid 3px #b98a37;  \n"
              "    box-shadow: inset 0 0 0 1px #fee9aa;  \n"
              "    color: #996633;  \n"
              "    text-shadow: 0 1px 0 #fedd9b; \n"
              "  }  \n"
              "  button.orange:hover {  \n"
              "    background: #fec455;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#fec455), to(#fecd61));  \n"
              "    background: -moz-linear-gradient(#fec455, #fecd61);  \n"
              "    background: linear-gradient(#fec455, #fecd61);  \n"
              "    border: solid 1px #e6a93d;  \n"
              "    border-bottom: solid 3px #b98a37;  \n"
              "    box-shadow: inset 0 0 0 1px #fedb98; \n"
              "  }  \n"
              "  button.orange:active {  \n"
              "    background: #f9bd4f;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#f9bd4f), to(#f0b64d));  \n"
              "    background: -moz-linear-gradient(#f9bd4f, #f0b64d);  \n"
              "    background: linear-gradient(#f9bd4f, #f0b64d);  \n"
              "    border: solid 1px #a77f35;  \n"
              "    box-shadow: inset 0 10px 15px 0 #dba646; \n"
              "  }  \n"
              "  button.blue_alt {  \n"
              "    background: #9fd6fa;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#9fd6fa), to(#6bb9f7));  \n"
              "    background: -moz-linear-gradient(#9fd6fa, #6bb9f7);  \n"
              "    background: linear-gradient(#9fd6fa, #6bb9f7);  \n"
              "    border: solid 1px #72bdf4;  \n"
              "    border-bottom: solid 3px #4a9de1;  \n"
              "    box-shadow: inset 0 0 0 1px #bfe4fc;  \n"
              "    color: #fff;  \n"
              "    text-shadow: 0 1px 0 #4598f3; \n"
              "  }  \n"
              "  button.blue_alt:hover {  \n"
              "    background: #6bb9f7;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#6bb9f7), to(#9fd6fa));  \n"
              "    background: -moz-linear-gradient(#6bb9f7, #9fd6fa);  \n"
              "    background: linear-gradient(#6bb9f7, #9fd6fa);  \n"
              "    border: solid 1px #72bdf4;  \n"
              "    border-bottom: solid 3px #4a9de1;  \n"
              "    box-shadow: inset 0 0 0 1px #bfe4fc; \n"
              "  }  \n"
              "  button.blue_alt:active {  \n"
              "    background: #6bb9f7;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#6bb9f7), to(#9fd6fa));  \n"
              "    background: -moz-linear-gradient(#6bb9f7, #9fd6fa);  \n"
              "    background: linear-gradient(#6bb9f7, #9fd6fa);  \n"
              "    border: solid 1px #72bdf4;  \n"
              "    box-shadow: inset 0 10px 15px 0 #50aaf3; \n"
              "  }  \n"
              "  button.green_alt {  \n"
              "    background: #c4e125;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#c4e125), to(#88a819));  \n"
              "    background: -moz-linear-gradient(#c4e125, #88a819);  \n"
              "    background: linear-gradient(#c4e125, #88a819);  \n"
              "    border: solid 1px #829c15;  \n"
              "    border-bottom: solid 3px #819d15;  \n"
              "    box-shadow: inset 0 0 0 1px #c6da7b;  \n"
              "    color: #fff;  \n"
              "    text-shadow: 0 1px 0 #819d15; \n"
              "  }  \n"
              "  button.green_alt:hover {  \n"
              "    background: #88a819;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#88a819), to(#c4e125));  \n"
              "    background: -moz-linear-gradient(#88a819, #c4e125);  \n"
              "    background: linear-gradient(#88a819, #c4e125);  \n"
              "    border: solid 1px #829c15;  \n"
              "    border-bottom: solid 3px #819d15;  \n"
              "    box-shadow: inset 0 0 0 1px #c6da7b; \n"
              "  }  \n"
              "  button.green_alt:active {  \n"
              "    background: #88a819;  \n"
              "    background: -webkit-gradient(linear, 0 0, 0 bottom, from(#88a819), to(#c4e125));  \n"
              "    background: -moz-linear-gradient(#88a819, #c4e125);  \n"
              "    background: linear-gradient(#88a819, #c4e125);  \n"
              "    border: solid 1px #829c15;  \n"
              "    box-shadow: inset 0 10px 15px 0 #819d15; \n"
              "  }  \n"
              // -----------------------
              // Slider CSS from online:
              // -----------------------
              "  input[type=range] { \n"
              "    -webkit-appearance: none; \n"
              "    margin: 18px 0; \n"
              "    width: 100%; \n"
              "  } \n"
              "  input[type=range]:focus { \n"
              "    outline: none; \n"
              "  } \n"
              "  input[type=range]::-webkit-slider-runnable-track { \n"
              "    width: 100%; \n"
              "    height: 8.4px; \n"
              "    cursor: pointer; \n"
              "    animate: 0.2s; \n"
              "    box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d; \n"
              "    border-radius: 1.3px; \n"
              "    border: 0.2px solid #010101; \n"
              "  } \n"
              "  input[type=range][id=brightness]::-webkit-slider-runnable-track { \n"
              "    background: linear-gradient(#ffffff, #000000); \n"
              "  } \n"
              "  input[type=range][id=redSlider]::-webkit-slider-runnable-track { \n"
              "    background: linear-gradient(#ff0000, #000000); \n"
              "  } \n"
              "  input[type=range][id=greenSlider]::-webkit-slider-runnable-track { \n"
              "    background: linear-gradient(#00ff00, #000000); \n"
              "  } \n"
              "  input[type=range][id=blueSlider]::-webkit-slider-runnable-track { \n"
              "    background: linear-gradient(#0000ff, #000000); \n"
              "  } \n"
              "  input[type=range]::-webkit-slider-thumb { \n"
              "    box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d; \n"
              "    border: 1px solid #000000; \n"
              "    height: 36px; \n"
              "    width: 16px; \n"
              "    border-radius: 3px; \n"
              "    background: #ffffff; \n"
              "    cursor: pointer; \n"
              "    -webkit-appearance: none; \n"
              "    margin-top: -14px; \n"
              "  } \n"
              "  input[type=range][id=brightness]:focus::-webkit-slider-runnable-track { \n"
              "    background: linear-gradient(#cccccc, #444444); \n"
              "  } \n"
              "  input[type=range][id=redSlider]:focus::-webkit-slider-runnable-track { \n"
              "    background: linear-gradient(#cc0000, #440000); \n"
              "  } \n"
              "  input[type=range][id=greenSlider]:focus::-webkit-slider-runnable-track { \n"
              "    background: linear-gradient(#00cc00, #004400); \n"
              "  } \n"
              "  input[type=range][id=blueSlider]:focus::-webkit-slider-runnable-track { \n"
              "    background: linear-gradient(#0000cc, #000044); \n"
              "  } \n"
              "  input[type=range]::-moz-range-track { \n"
              "    width: 100%; \n"
              "    height: 8.4px; \n"
              "    cursor: pointer; \n"
              "    animate: 0.2s; \n"
              "    box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d; \n"
              "    background: #3071a9; \n"
              "    border-radius: 1.3px; \n"
              "    border: 0.2px solid #010101; \n"
              "  } \n"
              "  input[type=range]::-moz-range-thumb { \n"
              "    box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d; \n"
              "    border: 1px solid #000000; \n"
              "    height: 36px; \n"
              "    width: 16px; \n"
              "    border-radius: 3px; \n"
              "    background: #ffffff; \n"
              "    cursor: pointer; \n"
              "  } \n"
              "  input[type=range]::-ms-track { \n"
              "    width: 100%; \n"
              "    height: 8.4px; \n"
              "    cursor: pointer; \n"
              "    animate: 0.2s; \n"
              "    background: transparent; \n"
              "    border-color: transparent; \n"
              "    border-width: 16px 0; \n"
              "    color: transparent; \n"
              "  } \n"
              "  input[type=range]::-ms-fill-lower { \n"
              "    background: #2a6495; \n"
              "    border: 0.2px solid #010101; \n"
              "    border-radius: 2.6px; \n"
              "    box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d; \n"
              "  } \n"
              "  input[type=range]::-ms-fill-upper { \n"
              "    background: #3071a9; \n"
              "    border: 0.2px solid #010101; \n"
              "    border-radius: 2.6px; \n"
              "    box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d; \n"
              "  } \n"
              "  input[type=range]::-ms-thumb { \n"
              "    box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d; \n"
              "    border: 1px solid #000000; \n"
              "    height: 36px; \n"
              "    width: 16px; \n"
              "    border-radius: 3px; \n"
              "    background: #ffffff; \n"
              "    cursor: pointer; \n"
              "  } \n"
              "  input[type=range]:focus::-ms-fill-lower { \n"
              "    background: #3071a9; \n"
              "  } \n"
              "  input[type=range]:focus::-ms-fill-upper { \n"
              "    background: #367ebd; \n"
              "  } \n"
              "</style>\n"
              "<script>\n"
              // ---------------------------
              // Write Javascript code here:
              // ---------------------------
              "  var brightness = 128;\n"
              "  var red = 128;\n"
              "  var green = 128;\n"
              "  var blue = 128;\n"
              "  \n"
              "  function updateRGB() {\n"
              "    red = document.getElementById('redSlider').value;\n"
              "    green = document.getElementById('greenSlider').value;\n"
              "    blue = document.getElementById('blueSlider').value;\n"
              "    brightness = document.getElementById('brightness').value;\n"
              "  }\n"
              "  function updateBrightness() {\n"
              "    updateRGB();\n"
              "    sendGetRequest('B=' + brightness + '&R=' + red + '&G=' + green + '&L=' + blue);\n"
              "  }\n"
              "  function startSending() {\n"
              "    document.getElementById('start').disabled = true;\n"
              "    document.getElementById('stop').disabled = false;\n"
              "    updateRGB();\n"
              "    refreshIntervalId = setInterval(function() {\n"
              "      sendGetRequest('B=' + brightness + '&R=' + red + '&G=' + green + '&L=' + blue)\n"
              "    }, 200);\n"
              "  }\n"
              "  function stop() {\n"
              "    clearInterval(refreshIntervalId);\n"
              "    updateRGB();\n"
              "    sendGetRequest('B=' + brightness + '&R=' + red + '&G=' + green + '&L=' + blue);\n"
              "    document.getElementById('start').disabled = false;\n"
              "    document.getElementById('stop').disabled = true;\n"
              "  }\n"
              "  function sendGetRequest(data) {\n"
              "    var request = new XMLHttpRequest();\n"
              "    var msg = '";
              webPage += String(ip[0]); 
              webPage += ".";
              webPage += String(ip[1]);
              webPage += ".";
              webPage += String(ip[2]);
              webPage += ".";
              webPage += String(ip[3]);
              webPage += "/Parameters.html?' + data + '&T=' + (new Date()).getTime();\n"
              "    request.open('GET', msg, true);\n"
              "    document.getElementById('msg').innerHTML = 'Message: ' + msg;\n"
              "    request.send();\n"
              "  }\n"
              "</script>\n"
              "</head>\n"
              // ----------------------------------------
              // Create HTML side of server sockets here:
              // ----------------------------------------
              // <a href='LINK_NAME_HERE'>
              // ----------------------------------------
              "<body>\n"
              "  <h1>Lamp Web Server</h1>\n"
              "    <a href='Toggle'><button class='green_alt'>TOGGLE WEB CONTROL</button></a>\n"              
              "  <p class = 'basic'><h2>Basic Color Schemes:</h2>\n"
              "    <a href='All_White'><button class='grey'>WHITE</button></a>&nbsp;\n"
              "    <a href='All_Red'><button class='pink'>RED</button></a>&nbsp;\n"
              "    <a href='All_Green'><button class='green'>GREEN</button></a>&nbsp;\n"
              "    <a href='All_Blue'><button class='blue'>BLUE</button></a>&nbsp;\n"
              "    <a href='Off'><button class='black'>OFF</button></a>\n"
              "  </p>\n"
              "  <p class='multicolor'><h2>Multicolor Schemes:</h2>\n"
              "    <a href='Bounce'><button class='orange'>BOUNCE</button></a>&nbsp;\n"
              "    <a href='Wake_Up'><button class='blue_alt'>WAKE UP</button></a>&nbsp;\n"
              "    <a href='Rainbow'><button class='green_alt'>RAINBOW</button></a>&nbsp;\n"
              "    <a href='Static_Rainbow'><button class='dark_grey'>STATIC RB</button></a>\n"
              "    <a href='Night_Night'><button class='black'>NIGHT</button></a>\n"
              "  </p>\n"
              "  <p class='sliders'><h2 id='level'>Brightness:</h2>\n"
              "    <input type='range' id='brightness' value=128 min=0 max=255 step=1 onclick='updateBrightness();'>\n"
              "    <h2 id='custom'>Custom Color Sliders:</h2>\n"
              "    <a href='Custom'><button class='dark_grey'>TOGGLE</button></a>&nbsp;\n"              
              "    <input type='range' id='redSlider' value=128 min=0 max=255 step=1 onclick='updateRGB();'>\n"
              "    <input type='range' id='greenSlider' value=128 min=0 max=255 step=1 onclick='updateRGB();'>\n"
              "    <input type='range' id='blueSlider' value=128 min=0 max=255 step=1 onclick='updateRGB();'>\n"
              "    <button id='start' class='green' onclick='startSending();'>START</button>\n"
              "    <button id='stop' class='pink' onclick='stop();'>STOP</button>\n"
              "    <div id='msg'>Message:</div>\n"
              "  </p>\n"
              "  \n"
              "</body>\n"
              "</html>" ;

              Serial.print(webPage);
}



