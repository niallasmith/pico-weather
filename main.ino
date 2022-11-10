#include <WiFi.h>
#include <ArduinoJson.h>
#include <Pico_Unicorn_GFX.h>

char ssid[] = "YOUR_NETWORK_SSID";       // your network SSID (name)
char password[] = "YOUR_NETWORK_PASSWORD";  // your network key
#define timezone "Europe/London" // your timezone in IANA time zone standard
#define metofficeID "3772" // your selected metoffice weather forecast location, e.g. 3772 for Heathrow 
#define apiKey "YOUR_METOFFICE_API_KEY" // your api key
WiFiClient client;
WiFiClientSecure clientSecure;

// Initialise global variables
int hrs = 0, mins = 0, secs = 0, lastTimeCall = 0;
int temp = 0, rainProb = 0, weatherType = 0, windSpeed = 0, humidity = 0, weatherType3Hr = 0, weatherType6Hr = 0, lastWeatherCall = 0;
const char* windDir;

int cycleIndex = 0; // Value for keeping track of which item to display


// Define colour values for images and display
#define ora {156,70,15}
#define blu {0,0,156}
#define wht {156,156,156}
#define gry {109,109,86}
#define off {0,0,0}
#define ylw {125,125,0}
#define pur {78,0,94}
#define red {156,0,0}
#define grn {0,110,0}


// Weather 5x7 image array
// Larger image arrays must be global as loading into memory would crash core 1
int weatherArray[][35][3] = {
  {
    off, off, off, off, off, // clear night
    off, gry, gry, off, off,
    off, off, gry, gry, off,
    off, off, gry, gry, off,
    off, off, gry, gry, off,
    off, gry, gry, off, off,
    off, off, off, off, off
  },
  {
    off, off, off, off, off, // full sun
    off, ora, ora, ora, off,
    ora, ora, ora, ora, ora,
    ora, ora, ora, ora, ora,
    ora, ora, ora, ora, ora,
    off, ora, ora, ora, off,
    off, off, off, off, off
  },
  {
    off, off, off, off, off, // sunny with cloud
    off, off, off, off, off,
    off, gry, gry, ora, ora,
    gry, gry, gry, gry, ora,
    gry, gry, gry, gry, gry,
    off, off, off, off, off,
    off, off, off, off, off
  },
  {
    off, off, off, off, off, // mist
    gry, gry, gry, off, off,
    off, off, off, off, off,
    off, off, gry, gry, gry,
    off, off, off, off, off,
    gry, off, off, gry, gry,
    off, off, off, off, off
  },
  {
    off, off, off, off, off, // fog
    gry, gry, gry, gry, gry,
    off, off, off, off, off,
    gry, gry, gry, gry, gry,
    off, off, off, off, off,
    gry, gry, gry, gry, gry,
    off, off, off, off, off
  },
  {
    off, off, off, off, off, // cloudy/overcast
    off, off, off, off, off,
    off, off, gry, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    off, off, off, off, off,
    off, off, off, off, off
  },
  {
    off, off, off, off, off, // light rain
    off, off, gry, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    blu, off, off, blu, off,
    off, off, off, off, off,
    off, off, blu, off, off
  },
  {
    off, off, off, off, off, // heavy rain
    off, gry, off, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    blu, blu, off, blu, blu,
    off, blu, off, off, blu,
    blu, off, blu, blu, off
  },
  {
    off, off, off, off, off, // light snow
    off, gry, gry, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    blu, off, off, blu, wht,
    off, wht, off, off, off,
    off, off, blu, off, wht
  },
  {
    off, off, off, off, off, // heavy snow
    off, gry, gry, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    wht, off, off, wht, off,
    off, off, off, off, off,
    off, off, wht, off, wht
  },
  {
    off, off, off, off, off, // heavy snow
    off, gry, gry, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    wht, off, off, wht, wht,
    off, wht, off, wht, off,
    wht, off, wht, off, wht
  },
  {
    off, off, off, off, off, // lightning
    off, gry, gry, off, off,
    gry, gry, gry, gry, off,
    gry, gry, gry, gry, gry,
    off, ylw, off, off, off,
    off, ylw, ylw, off, off,
    off, off, ylw, off, off
  },
  {
    off, off, off, off, off, // umbrella
    off, pur, pur, pur, off,
    pur, pur, pur, pur, pur,
    pur, pur, pur, pur, pur,
    off, off, ora, off, off,
    off, off, ora, off, off,
    off, off, ora, ora, off
  },
  {
    off, off, off, off, off, // wind
    off, off, gry, gry, off,
    off, off, off, gry, off,
    gry, gry, gry, gry, off,
    off, off, off, off, off,
    gry, gry, gry, gry, off,
    off, off, off, off, off
  }
};

// Digit 3x7 image array
int digitArray[][21][3] = {
  {
    wht, wht, wht, // 0
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht
  },
  {
    wht, wht, off, // 1
    off, wht, off,
    off, wht, off,
    off, wht, off,
    off, wht, off,
    off, wht, off,
    wht, wht, wht
  },
  {
    wht, wht, wht, // 2
    off, off, wht,
    off, off, wht,
    wht, wht, wht,
    wht, off, off,
    wht, off, off,
    wht, wht, wht
  },
  {
    wht, wht, wht, // 3
    off, off, wht,
    off, off, wht,
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    wht, wht, wht
  },
  {
    wht, off, wht, // 4
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    off, off, wht
  },
  {
    wht, wht, wht, // 5
    wht, off, off,
    wht, off, off,
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    wht, wht, wht
  },
  {
    wht, wht, wht, // 6
    wht, off, off,
    wht, off, off,
    wht, wht, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht
  },
  {
    wht, wht, wht, // 7
    off, off, wht,
    off, off, wht,
    off, off, wht,
    off, off, wht,
    off, off, wht,
    off, off, wht
  },
  {
    wht, wht, wht, // 8
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht
  },
  {
    wht, wht, wht, // 9
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    wht, wht, wht
  },
  {
    wht, wht, off, // N
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht
  },
  {
    wht, wht, wht, // E
    wht, off, off,
    wht, off, off,
    wht, wht, wht,
    wht, off, off,
    wht, off, off,
    wht, wht, wht
  },
  {
    wht, off, wht, // W
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht,
    wht, off, wht
  }
};

float brightnessModifier = 1.0; // Value for modifying global brightness of display

PicoUnicorn unicorn = PicoUnicorn(); // Initialise unicorn display object

////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() { // Core 0 setup
  // Initialise Pimoroni Unicorn and clear the display
  unicorn.init();
  unicorn.clear();

  displayFullscreen(0); // display hello message

  // wait until core 1 setup has completed
  while (!rp2040.fifo.available()) {
    ;
  }

  // clear out fifo queue
  while (rp2040.fifo.available()) {
    rp2040.fifo.pop();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void setup1() { // Core 1 setup
  Serial.begin(115200); // begin serial connection

  // wait until serial has been established
  //  while (!Serial) {
  //    ;
  //  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int maxDelay = 10000;
  int startMillis = millis();
  int failFlag;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

    // if WiFi is not connected within certain time, call displayRetry function (which will never return)
    if (millis() - startMillis >= maxDelay) {
      displayRetry();
    }
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  // intital HTTP request and deserialise
  getCurrentTime();
  getWeatherForecast();

  rp2040.fifo.push(255); // push value onto to fifo queue to indicate core 1 setup complete
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() { // Core 0 loop

  // check if both A and B buttons pressed, enter settings menu if so
  if (unicorn.is_pressed(unicorn.A) && unicorn.is_pressed(unicorn.B)) {
    settings();
  }

  int startFcnTime; // intialise variable for storing time at function call, used for timing

  switch (cycleIndex) {
    case 0: {
        startFcnTime = displayTime(); // display time HH:MM
        break;
      }
    case 1: {
        startFcnTime = displayTemperature(); // display temperature and weather
        break;
      }
    case 2: {
        startFcnTime = displayHumidity(); // display humidity and weather
        break;
      }
    case 3: {
        startFcnTime = displayRainChance(); // display probability of rain
        break;
      }
    case 4: {
        startFcnTime = displayWindSpeed(); // display wind speed
        break;
      }
    case 5: {
        startFcnTime = displayWindDir(); // display wind direction on 16 point compass
        break;
      }
    case 6: {
        startFcnTime = display3HrForecast(); // display current forecast, 3hr forecast and 6hr forecast
        break;
      }
  }

  // delay until time since function call has exceed 5 seconds
  while ((millis() - startFcnTime) <= 5000) {
    ;
  }

  // increment cycle index with overflow at 7 (i.e. only values 0-6)
  cycleVal = (cycleIndex + 1) % 7;

}

////////////////////////////////////////////////////////////////////////////////////////////////////

void loop1() { // Core 1 loop

  // if time since last weather update was over 30 minutes, refresh data
  if (millis() - lastWeatherCall >= 1800000) {
    getCurrentTime();
    getWeatherForecast();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////DISPLAY HIGH-LEVEL/////////////////////////////////////////////

int displayTime() {
  int startFcnTime = millis();
  unicorn.clear(); // clear display

  // calculate current time based on most recent time API update, and time elapsed since then
  int localSecs = secs + (millis() - lastTimeCall) / 1000;
  int minsOverflow = (localSecs - (localSecs % 60) ) / 60; // some trickery involved with managing overflows when seconds > 60 etc.
  int localMins = (mins + minsOverflow) % 60;
  int hrsOverflow = ((mins + minsOverflow) - localMins) / 60;
  int localHrs = (hrs + hrsOverflow) % 24;

  // convert decimal numbers into split digits
  int hrsTens = (localHrs - (localHrs % 10)) / 10;
  int hrsUnits = localHrs % 10;
  int minsTens = (localMins - (localMins % 10)) / 10;
  int minsUnits = localMins % 10;

  displayDigit(0, hrsTens); // display hours digit 1
  displayDigit(4, hrsUnits); // display hours digit 2
  displayDigit(9, minsTens); // display minutes digit 1
  displayDigit(13, minsUnits); // display minutes digit 2

  // display colon
  unicorn.set_pixel(8, 2, round(brightnessModifier * 156), round(brightnessModifier * 156), round(brightnessModifier * 156));
  unicorn.set_pixel(8, 4, round(brightnessModifier * 156), round(brightnessModifier * 156), round(brightnessModifier * 156));

  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int displayWindDir() {
  int startFcnTime = millis();
  unicorn.clear(); // clear display

  displayWeather(0, 13); // display wind symbol

  // convert char values into displayed text
  switch (windDir[0]) {
    case 'N':
      displayDigit(5, 10); // display N if 'N', etc. in correct location
      break;
    case 'E':
      displayDigit(5, 11);
      break;
    case 'S':
      displayDigit(5, 5);
      break;
    case 'W':
      displayDigit(5, 12);
      break;
  }

  switch (windDir[1]) {
    case 'N':
      displayDigit(9, 10); // display N if 'N', etc. in correct location
      break;
    case 'E':
      displayDigit(9, 11);
      break;
    case 'S':
      displayDigit(9, 5);
      break;
    case 'W':
      displayDigit(9, 12);
      break;
  }

  switch (windDir[2]) {
    case 'N':
      displayDigit(13, 10); // display N if 'N', etc. in correct location
      break;
    case 'E':
      displayDigit(13, 11);
      break;
    case 'S':
      displayDigit(13, 5);
      break;
    case 'W':
      displayDigit(13, 12);
      break;
  }

  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int displayWindSpeed() {
  int startFcnTime = millis();
  unicorn.clear(); // clear display

  displayWeather(0, 13); // display wind symbol

  // convert decimal wind speed into digits
  int windSpeedUnits = windSpeed % 10;
  int windSpeedTens = (windSpeed - windSpeedUnits) / 10;

  // if 1st digit non zero, display. prevents case of, e.g. wind speed of 8 mph reading 08 mph, which looks bad
  if (windSpeedTens != 0) {
    displayDigit(6, windSpeedTens); // display wind speed digit 1
  }


  displayDigit(10, windSpeedUnits); // display wind speed digit 2

  delay(2500); // delay for 2.5 sec

  displayMPH(); // display MPH symbol

  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int displayTemperature() {
  int startFcnTime = millis();
  int weatherMap[31] = {0, 1, 2, 2, 12, 3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11}; // array for mapping metoffice weather codes to corresponding weather images
  unicorn.clear(); // clear display
  displayWeather(0, weatherMap[weatherType]); // display relevant weather image

  // convert decimal temperature into digits
  int tempUnits = temp % 10;
  int tempTens = (temp - tempUnits) / 10;

  // if 1st digit non zero, display. prevents case of, e.g. temperature of 8 degC reading 08 degC, which looks bad
  if (tempTens != 0) {
    displayDigit(6, tempTens); // display temperature digit 1
  }

  displayDigit(10, tempUnits); // display temperature digit 2

  displaySymbol(14, 0); // display degrees C symbol

  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int displayRainChance() {
  int startFcnTime = millis();
  unicorn.clear(); // clear display
  displayWeather(0, 12); // display umbrella

  if (rainProb != 100) {
    // convert rain probability decimal into digits
    int rainProbUnits = rainProb % 10;
    int rainProbTens = (rainProb - rainProbUnits) / 10;

    // if 1st digit non zero, display. prevents case of, e.g. rain probability of 8 % reading 08 %, which looks bad
    if (rainProbTens != 0) {
      displayDigit(6, rainProbTens); // display rain probability digit 1
    }

    displayDigit(10, rainProbUnits);// display rain probability digit 1
  } else {
    displayHundred(); // if rain probability is equal to 100, display special 100 text
  }

  displaySymbol(14, 1); // display percentage sign symbol
  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int displayHumidity() {
  int startFcnTime = millis();
  int weatherMap[31] = {0, 1, 2, 2, 12, 3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11}; // array for mapping metoffice weather codes to corresponding weather images
  unicorn.clear(); // clear display
  displayWeather(0, weatherMap[weatherType]); // display relevant weather image

  if (humidity != 100) {
    // convert humidity decimal into digits
    int humidityUnits = humidity % 10;
    int humidityTens = (humidity - humidityUnits) / 10;

    // if 1st digit non zero, display. prevents case of, e.g. humidity of 8 % reading 08 %, which looks bad
    if (humidityTens != 0) {
      displayDigit(6, humidityTens); // display humidity digit 1
    }

    displayDigit(10, humidityUnits); // display humidity digit 2
  } else {
    displayHundred(); // if humidity is equal to 100, display special 100 text
  }

  displaySymbol(14, 1); // display percentage sign symbol
  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int display3HrForecast() {
  int startFcnTime = millis();
  int weatherMap[31] = {0, 1, 2, 2, 12, 3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11}; // array for mapping metoffice weather codes to corresponding weather images
  unicorn.clear(); // clear display

  displayWeather(0, weatherMap[weatherType]); // display current weather image
  displayWeather(5, weatherMap[weatherType3Hr]); // display 3hr forecast weather image
  displayWeather(10, weatherMap[weatherType6Hr]); // display 6hr forecast weather image

  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void settings() {
  unicorn.clear(); // clear display

  displayFullscreen(1); // display settings splash screen
  delay(500);

  // wait until no buttons are pressed
  while (unicorn.is_pressed(unicorn.A) || unicorn.is_pressed(unicorn.B) || unicorn.is_pressed(unicorn.X) || unicorn.is_pressed(unicorn.Y)) {
    ;
  }

  delay(1000); // slight delay so user inputs are not registered

  unicorn.clear(); // clear display

  // exit while loop when both A and B buttons are pressed
  while (!unicorn.is_pressed(unicorn.B) && !unicorn.is_pressed(unicorn.A)) {
    // convert float brightness modifier into digits
    int brightnessUnits = floor(brightnessModifier);
    int brightnessTenths = floor((10 * brightnessModifier) - (10 * brightnessUnits));
    int brightnessHundreths = floor((100 * brightnessModifier) - (100 * brightnessUnits + 10 * brightnessTenths));

    // display green next to X (increment) button
    unicorn.set_pixel(15, 1, 0, round(brightnessModifier * 110), 0);
    unicorn.set_pixel(15, 2, 0, round(brightnessModifier * 110), 0);

    // display red next to Y (decrement) button
    unicorn.set_pixel(15, 4, round(brightnessModifier * 156), 0, 0);
    unicorn.set_pixel(15, 5, round(brightnessModifier * 156), 0, 0);


    displayDigit(0, brightnessUnits); // display brightness modifier 1st digit
    unicorn.set_pixel(4, 5, round(brightnessModifier * 156), round(brightnessModifier * 156), round(brightnessModifier * 156)); // display point
    displayDigit(6, brightnessTenths); // display brightness modifier 2nd digit
    displayDigit(10, brightnessHundreths); // display brightness modifier 3rd digit

    // if any of the buttons pressed, increase or decrease brightness modifier value correspondingly
    if (unicorn.is_pressed(unicorn.X)) {
      brightnessModifier = brightnessModifier + 0.05;
    } else if (unicorn.is_pressed(unicorn.Y)) {
      brightnessModifier = brightnessModifier - 0.05;
    }

    // wait until user has released button
    while (unicorn.is_pressed(unicorn.X) || unicorn.is_pressed(unicorn.Y)) {
      ;
    }

    // regardless of what the user has inputted, do not let brightness modifier values exceed 0.25 below and 1.60 above
    brightnessModifier = max(brightnessModifier, 0.25);
    brightnessModifier = min(brightnessModifier, 1.60);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////DISPLAY BASICS/////////////////////////////////////////////////

void displayWeather(int offset, int weatherIndex) {
  for (int i = 0; i <= 6; i++) { // loop Y pixels
    for (int ii = 0; ii <= 4; ii++) { // loop X pixels
      // set X,Y pixel to colour from image array
      unicorn.set_pixel(ii + offset, i, round(brightnessModifier * weatherArray[weatherIndex][5 * i + ii][0]), round(brightnessModifier * weatherArray[weatherIndex][5 * i + ii][1]), round(brightnessModifier * weatherArray[weatherIndex][5 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displaySymbol(int offset, int digitIndex) {
  // setup array for degree C and percentage sign symbols
  int symbolArray[2][14][3] = {
    {
      off, off,
      wht, off,
      off, off,
      wht, wht,
      wht, off,
      wht, wht,
      off, off,
    },
    {
      off, off,
      wht, off,
      off, off,
      off, wht,
      wht, off,
      off, off,
      off, wht,
    }
  };

  for (int i = 0; i <= 6; i++) { // loop Y pixels
    for (int ii = 0; ii <= 1; ii++) { // loop X pixels
      // set X,Y pixel to colour from image array
      unicorn.set_pixel(ii + offset, i, round(brightnessModifier * symbolArray[digitIndex][2 * i + ii][0]), round(brightnessModifier * symbolArray[digitIndex][2 * i + ii][1]), round(brightnessModifier * symbolArray[digitIndex][2 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displayDigit(int offset, int digitIndex) {
  for (int i = 0; i <= 6; i++) { // loop Y pixels
    for (int ii = 0; ii <= 2; ii++) { // loop X pixels
      // set X,Y pixel to colour from image array
      unicorn.set_pixel(ii + offset, i, round(brightnessModifier * digitArray[digitIndex][3 * i + ii][0]), round(brightnessModifier * digitArray[digitIndex][3 * i + ii][1]), round(brightnessModifier * digitArray[digitIndex][3 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displayRetry() {
  // setup cross image array
  int cross[25][3] = {
    red, off, off, off, red,
    off, red, off, red, off,
    off, off, red, off, off,
    off, red, off, red, off,
    red, off, off, off, red
  };

  // setup 'WiFi' text image array
  int wifi[77][3] = {
    off, off, off, off, off, off, off, off, off, off, off,
    red, off, red, off, red, off, red, red, off, red, off,
    red, off, red, off, off, off, red, off, off, off, off,
    red, red, red, off, red, off, red, red, off, red, off,
    red, off, red, off, red, off, red, off, off, red, off,
    off, off, off, off, off, off, off, off, off, off, off,
    off, off, off, off, off, off, off, off, off, off, off
  };
  unicorn.clear(); // clear display

  while (true) { // infinite loop

    // display cross
    for (int i = 0; i <= 4; i++) { // loop Y pixels
      for (int ii = 0; ii <= 4; ii++) { // loop X pixels
        // set X,Y pixel to colour from image array
        unicorn.set_pixel(ii, i + 1, cross[5 * i + ii][0], cross[5 * i + ii][1], cross[5 * i + ii][2]);
      }
    }

    // display WiFi text
    for (int i = 0; i <= 4; i++) { // loop Y pixels
      for (int ii = 0; ii <= 10; ii++) { // loop X pixels
        // set X,Y pixel to colour from image array
        unicorn.set_pixel(ii + 6, i, wifi[11 * i + ii][0], wifi[11 * i + ii][1], wifi[11 * i + ii][2]);
      }
    }

    delay(500);
    unicorn.clear(); // clear display

    // display WiFi text
    for (int i = 0; i <= 4; i++) { // loop Y pixels
      for (int ii = 0; ii <= 10; ii++) { // loop X pixels
        // set X,Y pixel to colour from image array
        unicorn.set_pixel(ii + 6, i, wifi[11 * i + ii][0], wifi[11 * i + ii][1], wifi[11 * i + ii][2]);
      }
    }

    delay(500);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displayHundred() {
  // setup special 100 image array
  int hundred[49][3] = {
    wht, wht, off, off, off, off, off,
    off, wht, off, wht, wht, wht, off,
    wht, wht, wht, wht, off, wht, off,
    off, off, off, wht, wht, wht, wht,
    off, off, off, off, wht, wht, wht,
    off, off, off, off, wht, off, wht,
    off, off, off, off, wht, wht, wht
  };

  for (int i = 0; i <= 6; i++) { // loop Y pixels
    for (int ii = 0; ii <= 6; ii++) { // loop X pixels
      // set X,Y pixel to colour from image array
      unicorn.set_pixel(ii + 6, i, round(brightnessModifier * hundred[7 * i + ii][0]), round(brightnessModifier * hundred[7 * i + ii][1]), round(brightnessModifier * hundred[7 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displayMPH() {
  // setup 'MPH' text image array 
  int mph[77][3] = {
    wht, off, wht, off, off, off, off, off, off, off, off,
    wht, wht, wht, off, wht, wht, wht, off, off, off, off,
    wht, off, wht, off, wht, off, wht, off, wht, off, wht,
    wht, off, wht, off, wht, wht, wht, off, wht, off, wht,
    off, off, off, off, wht, off, off, off, wht, wht, wht,
    off, off, off, off, wht, off, off, off, wht, off, wht,
    off, off, off, off, off, off, off, off, wht, off, wht
  };

  for (int i = 0; i <= 6; i++) { // loop Y pixels
    for (int ii = 0; ii <= 10; ii++) { // loop X pixels
      // set X,Y pixel to colour from image array
      unicorn.set_pixel(ii + 5, i, round(brightnessModifier * mph[11 * i + ii][0]), round(brightnessModifier * mph[11 * i + ii][1]), round(brightnessModifier * mph[11 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displayFullscreen(int index) {
  // setup fullscreen image arrays for hello and brightness
  int fullscreenArray[2][112][3] = {
    {
      pur, off, pur, ora, ora, ora, blu, off, off, grn, off, off, red, red, red, off, // hello (multicoloured)
      pur, off, pur, ora, off, off, blu, off, off, grn, off, off, red, off, red, off,
      pur, off, pur, ora, off, off, blu, off, off, grn, off, off, red, off, red, off,
      pur, pur, pur, ora, ora, ora, blu, off, off, grn, off, off, red, off, red, off,
      pur, off, pur, ora, off, off, blu, off, off, grn, off, off, red, off, red, off,
      pur, off, pur, ora, off, off, blu, off, off, grn, off, off, red, off, red, off,
      pur, off, pur, ora, ora, ora, blu, blu, blu, grn, grn, grn, red, red, red, off
    },
    {
      grn, grn, off, off, grn, grn, off, off, off, off, grn, grn, grn, off, grn, off, // bright(ness)
      grn, off, grn, off, grn, off, grn, off, grn, off, grn, off, off, off, grn, off,
      grn, off, grn, off, grn, grn, off, off, off, off, grn, off, off, off, grn, grn,
      grn, grn, off, off, grn, grn, off, off, grn, off, grn, off, grn, off, grn, off,
      grn, off, grn, off, grn, off, grn, off, grn, off, grn, off, grn, off, grn, off,
      grn, off, grn, off, grn, off, grn, off, grn, off, grn, off, grn, off, grn, off,
      grn, grn, off, off, grn, off, grn, off, grn, off, grn, grn, grn, off, grn, grn
    }
  };

  for (int i = 0; i <= 6; i++) { // loop Y pixels
    for (int ii = 0; ii <= 15; ii++) { // loop X pixels
      // set X,Y pixel to colour from image array
      unicorn.set_pixel(ii, i, round(brightnessModifier * fullscreenArray[index][16 * i + ii][0]), round(brightnessModifier * fullscreenArray[index][16 * i + ii][1]), round(brightnessModifier * fullscreenArray[index][16 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////HTTP REQUESTS///////////////////////////////////////////////////

void getCurrentTime() {
  clientSecure.setFingerprint("01 93 97 F3 AA AA 55 AE A0 10 EA ED 06 0E B3 18 E1 50 2C 50");
  clientSecure.setTimeout(10000); // Set a long timeout in case the server is slow to respond or whatever

  // Opening connection to server
  if (!clientSecure.connect("timeapi.io", 443))
  {
    Serial.println(F("Could not connect to server on port 443"));
    return;
  }

  yield();

  // Send HTTP request
  clientSecure.print(F("GET /api/Time/current/zone?timeZone="));
  clientSecure.print(timezone);
  clientSecure.println(F(" HTTP/1.1"));
  clientSecure.println(F("Host: timeapi.io"));
  clientSecure.println(F("Cache-Control: no-cache"));

  if (clientSecure.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  clientSecure.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!clientSecure.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  while (clientSecure.available() && clientSecure.peek() != '{')
  {
    char c = 0;
    clientSecure.readBytes(&c, 1);
  }

  StaticJsonDocument<384> doc;

  DeserializationError error = deserializeJson(doc, clientSecure);

// if no deserialisation error, store data in global variables
  if (!error) {
    hrs = doc["hour"];
    mins = doc["minute"];
    secs = doc["seconds"];

    lastTimeCall = millis(); // update last time API call time
    return;
  } else {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////

void getWeatherForecast() {
  client.setTimeout(10000); // Set a long timeout in case the server is slow to respond or whatever

  // Opening connection to server
  if (!client.connect("datapoint.metoffice.gov.uk", 80))
  {
    Serial.println(F("Could not connect to server on port 80"));
    return;
  }

  yield();

  // Send HTTP request
  client.print(F("GET /public/data/val/wxfcs/all/json/"));
  client.print(metofficeID);
  //  client.print(F("?res=3hourly&key="));
  //  client.print(apiKey);
  client.print(F("?res=3hourly&key=164ee130-8606-46cf-912b-e41e86c597b4"));
  client.println(F(" HTTP/1.1"));
  client.println(F("Host: datapoint.metoffice.gov.uk"));
  client.println(F("Cache-Control: max-age=0"));


  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    return;
  }

  while (!client.available()) {
    ;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  while (client.available() && client.peek() != '{')
  {
    char c = 0;
    client.readBytes(&c, 1);
  }

  weatherDeserialise();

}

////////////////////////////////////////////////////////////////////////////////////////////////////

void weatherDeserialise() {
  // calculate current time based on most recent time API update, and time elapsed since then
  int localSecs = secs + (millis() - lastTimeCall) / 1000;
  int minsOverflow = (localSecs - (localSecs % 60) ) / 60;
  int localMins = (mins + minsOverflow) % 60;
  int hrsOverflow = ((mins + minsOverflow) - localMins) / 60;
  int localHrs = (hrs + hrsOverflow) % 24;

  int minsSinceUpdate = (localHrs * 60 + localMins) % 180; // calculate minutes since last metoffice update (forecast is at 3am, 6am, etc.)
  
  int index = 0, threeHourIndex = 0, sixHourIndex = 0; // index variables used for correct forecast

  if (minsSinceUpdate <= 45 || minsSinceUpdate >= 90) { // conditional logic finding best index based on time since last update
    index = 2;
  } else {
    index = 1;
  }


  Serial.print(localHrs);
  Serial.print(":");
  Serial.print(localMins);
  Serial.print(" ");
  Serial.print(minsSinceUpdate);
  Serial.print(" ");
  Serial.println(index);

// making sure last forecast time can be 9pm due to JSON formatting (forecast for the next day would require parsing next metoffice 'Period' which likely would exceed allocated memory size)
  if (localHrs >= 19 && minsSinceUpdate >= 90) {
    threeHourIndex = index;
    sixHourIndex = index;
  } else if (localHrs >= 16 && minsSinceUpdate >= 90) {
    threeHourIndex = index + 1;
    sixHourIndex = index + 1;
  } else {
    threeHourIndex = index + 1;
    sixHourIndex = index + 2;
  }

// setup JSON documents for deserialsing
  StaticJsonDocument<8192> doc;
  StaticJsonDocument<512> filter;

  filter["SiteRep"]["DV"]["Location"]["Period"][0]["Rep"] = true; // filter out everything but todays 3Hr forecast, due to memory limits

  DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));

// if deserialistion successful, parse data and assign to global variables. also display to serial monitor.
  if (!error) {

    JsonObject SiteRep_DV = doc["SiteRep"]["DV"];
    JsonObject SiteRep_DV_Location = SiteRep_DV["Location"];
    JsonArray SiteRep_DV_Location_Period = SiteRep_DV_Location["Period"];
    JsonObject SiteRep_DV_Location_Period_0 = SiteRep_DV_Location_Period[0];
    JsonArray SiteRep_DV_Location_Period_0_Rep_item = SiteRep_DV_Location_Period_0["Rep"];
    JsonObject SiteRep_DV_Location_Period_0_Rep_item_0 = SiteRep_DV_Location_Period_0_Rep_item[index];

    const char* SiteRep_DV_Location_Period_0_Rep_item_D = SiteRep_DV_Location_Period_0_Rep_item_0["D"];
    const char* SiteRep_DV_Location_Period_0_Rep_item_F = SiteRep_DV_Location_Period_0_Rep_item_0["F"];
    const char* SiteRep_DV_Location_Period_0_Rep_item_G = SiteRep_DV_Location_Period_0_Rep_item_0["G"];
    const char* SiteRep_DV_Location_Period_0_Rep_item_H = SiteRep_DV_Location_Period_0_Rep_item_0["H"];
    const char* SiteRep_DV_Location_Period_0_Rep_item_Pp = SiteRep_DV_Location_Period_0_Rep_item_0["Pp"];
    const char* SiteRep_DV_Location_Period_0_Rep_item_S = SiteRep_DV_Location_Period_0_Rep_item_0["S"];
    const char* SiteRep_DV_Location_Period_0_Rep_item_T = SiteRep_DV_Location_Period_0_Rep_item_0["T"];
    const char* SiteRep_DV_Location_Period_0_Rep_item_V = SiteRep_DV_Location_Period_0_Rep_item_0["V"];
    const char* SiteRep_DV_Location_Period_0_Rep_item_W = SiteRep_DV_Location_Period_0_Rep_item_0["W"];
    const char* SiteRep_DV_Location_Period_0_Rep_item_U = SiteRep_DV_Location_Period_0_Rep_item_0["U"];
    const char* SiteRep_DV_Location_Period_0_Rep_item_ = SiteRep_DV_Location_Period_0_Rep_item_0["$"];

    windDir = SiteRep_DV_Location_Period_0_Rep_item_D;
    temp = atoi(SiteRep_DV_Location_Period_0_Rep_item_T);
    rainProb = atoi(SiteRep_DV_Location_Period_0_Rep_item_Pp);
    weatherType = atoi(SiteRep_DV_Location_Period_0_Rep_item_W);
    windSpeed = atoi(SiteRep_DV_Location_Period_0_Rep_item_S);
    humidity = atoi(SiteRep_DV_Location_Period_0_Rep_item_H);

    JsonObject SiteRep_DV_Location_Period_0_Rep_item_1 = SiteRep_DV_Location_Period_0_Rep_item[threeHourIndex];
    const char* SiteRep_DV_Location_Period_0_Rep_item_1_W = SiteRep_DV_Location_Period_0_Rep_item_1["W"];
    weatherType3Hr = atoi(SiteRep_DV_Location_Period_0_Rep_item_1_W);

    JsonObject SiteRep_DV_Location_Period_0_Rep_item_2 = SiteRep_DV_Location_Period_0_Rep_item[sixHourIndex];
    const char* SiteRep_DV_Location_Period_0_Rep_item_2_W = SiteRep_DV_Location_Period_0_Rep_item_2["W"];
    weatherType6Hr = atoi(SiteRep_DV_Location_Period_0_Rep_item_2_W);


    Serial.print(F("Pp: "));
    Serial.print(rainProb);
    Serial.print(F(" T: "));
    Serial.print(temp);
    Serial.print(F(" W: "));
    Serial.print(weatherType);
    Serial.print(F(" S: "));
    Serial.print(windSpeed);
    Serial.print(F(" D: "));
    Serial.print(windDir);
    Serial.print(F(" S 3hr: "));
    Serial.print(weatherType3Hr);
    Serial.print(F(" S 6hr: "));
    Serial.println(weatherType6Hr);

    lastWeatherCall = millis(); // update last weather forecast call time
    return;

  } else {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
}
