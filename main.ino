#include <WiFi.h>
#include <ArduinoJson.h>
#include <Pico_Unicorn_GFX.h>

//char ssid[] = "YOUR_NETWORK_SSID";       // your network SSID (name)
//char password[] = "YOUR_NETWORK_PASSWORD";  // your network key
#define timezone "Europe/London" // your timezone in IANA time zone standard
#define metofficeID "3772" // your selected metoffice weather forecast location (e.g. 3772 for Heathrow)
#define apiKey "YOUR_METOFFICE_API_KEY" // your api key
WiFiClient client;
WiFiClientSecure clientSecure;

int hrs = 0, mins = 0, secs = 0, lastTimeCall = 0;

int temp = 0, rainProb = 0, weatherType = 0, windSpeed = 0, humidity = 0, weatherType3Hr = 0, weatherType6Hr = 0, lastWeatherCall = 0;
const char* windDir;

int cycleVal = 0;

#define ora {156,70,15}
#define blu {0,0,156}
#define wht {156,156,156}
#define gry {109,109,86}
#define off {0,0,0}
#define ylw {125,125,0}
#define pur {78,0,94}
#define red {156,0,0}
#define grn {0,110,0}

int weatherArray[][35][3] = {
  {
    off, off, off, off, off,
    off, gry, gry, off, off,
    off, off, gry, gry, off,
    off, off, gry, gry, off,
    off, off, gry, gry, off,
    off, gry, gry, off, off,
    off, off, off, off, off
  },
  {
    off, off, off, off, off,
    off, ora, ora, ora, off,
    ora, ora, ora, ora, ora,
    ora, ora, ora, ora, ora,
    ora, ora, ora, ora, ora,
    off, ora, ora, ora, off,
    off, off, off, off, off
  },
  {
    off, off, off, off, off,
    off, off, off, off, off,
    off, gry, gry, ora, ora,
    gry, gry, gry, gry, ora,
    gry, gry, gry, gry, gry,
    off, off, off, off, off,
    off, off, off, off, off
  },
  {
    off, off, off, off, off,
    gry, gry, gry, off, off,
    off, off, off, off, off,
    off, off, gry, gry, gry,
    off, off, off, off, off,
    gry, off, off, gry, gry,
    off, off, off, off, off
  },
  {
    off, off, off, off, off,
    gry, gry, gry, gry, gry,
    off, off, off, off, off,
    gry, gry, gry, gry, gry,
    off, off, off, off, off,
    gry, gry, gry, gry, gry,
    off, off, off, off, off
  },
  {
    off, off, off, off, off,
    off, off, off, off, off,
    off, off, gry, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    off, off, off, off, off,
    off, off, off, off, off
  },
  {
    off, off, off, off, off,
    off, off, gry, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    blu, off, off, blu, off,
    off, off, off, off, off,
    off, off, blu, off, off
  },
  {
    off, off, off, off, off,
    off, gry, off, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    blu, blu, off, blu, blu,
    off, blu, off, off, blu,
    blu, off, blu, blu, off
  },
  {
    off, off, off, off, off,
    off, gry, gry, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    blu, off, off, blu, wht,
    off, wht, off, off, off,
    off, off, blu, off, wht
  },
  {
    off, off, off, off, off,
    off, gry, gry, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    wht, off, off, wht, off,
    off, off, off, off, off,
    off, off, wht, off, wht
  },
  {
    off, off, off, off, off,
    off, gry, gry, gry, off,
    gry, gry, gry, gry, gry,
    gry, gry, gry, gry, gry,
    wht, off, off, wht, wht,
    off, wht, off, wht, off,
    wht, off, wht, off, wht
  },
  {
    off, off, off, off, off,
    off, gry, gry, off, off,
    gry, gry, gry, gry, off,
    gry, gry, gry, gry, gry,
    off, ylw, off, off, off,
    off, ylw, ylw, off, off,
    off, off, ylw, off, off
  },
  {
    off, off, off, off, off,
    off, pur, pur, pur, off,
    pur, pur, pur, pur, pur,
    pur, pur, pur, pur, pur,
    off, off, ora, off, off,
    off, off, ora, off, off,
    off, off, ora, ora, off
  },
  {
    off, off, off, off, off,
    off, off, gry, gry, off,
    off, off, off, gry, off,
    gry, gry, gry, gry, off,
    off, off, off, off, off,
    gry, gry, gry, gry, off,
    off, off, off, off, off
  }
};

int digitArray[][21][3] = {
  {
    wht, wht, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht
  },
  {
    wht, wht, off,
    off, wht, off,
    off, wht, off,
    off, wht, off,
    off, wht, off,
    off, wht, off,
    wht, wht, wht
  },
  {
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    wht, wht, wht,
    wht, off, off,
    wht, off, off,
    wht, wht, wht
  },
  {
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    wht, wht, wht
  },
  {
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    off, off, wht
  },
  {
    wht, wht, wht,
    wht, off, off,
    wht, off, off,
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    wht, wht, wht
  },
  {
    wht, wht, wht,
    wht, off, off,
    wht, off, off,
    wht, wht, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht
  },
  {
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    off, off, wht,
    off, off, wht,
    off, off, wht,
    off, off, wht
  },
  {
    wht, wht, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht
  },
  {
    wht, wht, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht,
    off, off, wht,
    off, off, wht,
    wht, wht, wht
  },
  {
    wht, wht, off,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht
  },
  {
    wht, wht, wht,
    wht, off, off,
    wht, off, off,
    wht, wht, wht,
    wht, off, off,
    wht, off, off,
    wht, wht, wht
  },
  {
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, off, wht,
    wht, wht, wht,
    wht, off, wht
  }
};

float brightnessModifier = 1.0;

PicoUnicorn unicorn = PicoUnicorn(); // Initialise unicorn display object

////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() { // Core 0 setup
  // Initialise Pimoroni Unicorn and clear the display
  unicorn.init();
  unicorn.clear();

  displayFullscreen(0);

  while (!rp2040.fifo.available()) {
    ;
  }

  while (rp2040.fifo.available()) {
    rp2040.fifo.pop();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void setup1() { // Core 1 setup
  Serial.begin(115200);

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

    if (millis() - startMillis >= maxDelay) {
      displayRetry();
    }

  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  getCurrentTime();
  getWeatherForecast();

  rp2040.fifo.push(255);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() { // Core 0 loop
  if (rp2040.fifo.available() && rp2040.fifo.pop() == 1) {
    while (rp2040.fifo.pop() != 2) {
      rp2040.fifo.pop();
      delay(200);
    }
  }

  if (unicorn.is_pressed(unicorn.A) && unicorn.is_pressed(unicorn.B)) {
    settings();
  }

  int startFcnTime;

  switch (cycleVal) {
    case 0: {
        startFcnTime = displayTime();
        break;
      }
    case 1: {
        startFcnTime = displayTemperature();
        break;
      }
    case 2: {
        startFcnTime = displayHumidity();
        break;
      }
    case 3: {
        startFcnTime = displayRainChance();
        break;
      }
    case 4: {
        startFcnTime = displayWindSpeed();
        break;
      }
    case 5: {
        startFcnTime = displayWindDir();
        break;
      }
    case 6: {
        startFcnTime = display3HrForecast();
        break;
      }
    default: {
        startFcnTime = displayTime();
        break;
      }
  }

  while ((millis() - startFcnTime) <= 5000) {
    ;
  }

  cycleVal = (cycleVal + 1) % 7;

}

////////////////////////////////////////////////////////////////////////////////////////////////////

void loop1() { // Core 1 loop
  if (millis() - lastWeatherCall >= 1800000) {
    rp2040.fifo.push(1);
    getCurrentTime();
    getWeatherForecast();
    rp2040.fifo.push(2);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////DISPLAY HIGH-LEVEL/////////////////////////////////////////////

int displayTime() {
  int startFcnTime = millis();
  unicorn.clear();

  int colon[7][3] = {
    off,
    off,
    wht,
    off,
    wht,
    off,
    off
  };

  int localSecs = secs + (millis() - lastTimeCall) / 1000;
  int minsOverflow = (localSecs - (localSecs % 60) ) / 60;
  int localMins = (mins + minsOverflow) % 60;
  int hrsOverflow = ((mins + minsOverflow) - localMins) / 60;
  int localHrs = (hrs + hrsOverflow) % 24;

  int hrsTens = (localHrs - (localHrs % 10)) / 10;
  int hrsUnits = localHrs % 10;
  int minsTens = (localMins - (localMins % 10)) / 10;
  int minsUnits = localMins % 10;

  displayDigit(0, hrsTens);

  displayDigit(4, hrsUnits);

  displayDigit(9, minsTens);

  displayDigit(13, minsUnits);

  for (int i = 0; i <= 6; i++) {
    unicorn.set_pixel(8, i, round(brightnessModifier * colon[i][0]), round(brightnessModifier * colon[i][1]), round(brightnessModifier * colon[i][2]));
  }
  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int displayWindDir() {
  int startFcnTime = millis();
  unicorn.clear();

  displayWeather(0, 13);

  switch (windDir[0]) {
    case 'N':
      displayDigit(5, 10);
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
      displayDigit(9, 10);
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
      displayDigit(13, 10);
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
  unicorn.clear();

  displayWeather(0, 13);

  int windSpeedUnits = windSpeed % 10;
  int windSpeedTens = (windSpeed - windSpeedUnits) / 10;

  if (windSpeedTens != 0) {
    displayDigit(6, windSpeedTens);
  }

  displayDigit(10, windSpeedUnits);

  delay(2500);

  displayMPH();

  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int displayTemperature() {
  int startFcnTime = millis();
  int weatherMap[31] = {0, 1, 2, 2, 12, 3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11};
  unicorn.clear();
  displayWeather(0, weatherMap[weatherType]);

  int tempUnits = temp % 10;
  int tempTens = (temp - tempUnits) / 10;

  if (tempTens != 0) {
    displayDigit(6, tempTens);
  }

  displayDigit(10, tempUnits);

  displaySymbol(14, 0);

  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int displayRainChance() {
  int startFcnTime = millis();
  unicorn.clear();
  displayWeather(0, 12);
  if (rainProb != 100) {
    int rainProbUnits = rainProb % 10;
    int rainProbTens = (rainProb - rainProbUnits) / 10;

    if (rainProbTens != 0) {
      displayDigit(6, rainProbTens);
    }

    displayDigit(10, rainProbUnits);
  } else {
    displayHundred();
  }

  displaySymbol(14, 1);
  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int displayHumidity() {
  int startFcnTime = millis();
  int weatherMap[31] = {0, 1, 2, 2, 12, 3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11};
  unicorn.clear();
  displayWeather(0, weatherMap[weatherType]);

  if (humidity != 100) {
    int humidityUnits = humidity % 10;
    int humidityTens = (humidity - humidityUnits) / 10;

    if (humidityTens != 0) {
      displayDigit(6, humidityTens);
    }

    displayDigit(10, humidityUnits);
  } else {
    displayHundred();
  }

  displaySymbol(14, 1);
  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int display3HrForecast() {
  int startFcnTime = millis();
  int weatherMap[31] = {0, 1, 2, 2, 12, 3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11};
  unicorn.clear();

  displayWeather(0, weatherMap[weatherType]);
  displayWeather(5, weatherMap[weatherType3Hr]);
  displayWeather(10, weatherMap[weatherType6Hr]);

  return startFcnTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void settings() {
  unicorn.clear();

  displayFullscreen(1);
  delay(500);

  while (unicorn.is_pressed(unicorn.A) || unicorn.is_pressed(unicorn.B) || unicorn.is_pressed(unicorn.X) || unicorn.is_pressed(unicorn.Y)) {
    ;
  }
  
  delay(1000);

  unicorn.clear();

  while (!unicorn.is_pressed(unicorn.B) && !unicorn.is_pressed(unicorn.A)) {
    int brightnessUnits = floor(brightnessModifier);
    int brightnessTenths = floor((10 * brightnessModifier) - (10 * brightnessUnits));
    int brightnessHundreths = floor((100 * brightnessModifier) - (100 * brightnessUnits + 10 * brightnessTenths));

    unicorn.set_pixel(15, 1, 0, round(brightnessModifier * 110), 0);
    unicorn.set_pixel(15, 2, 0, round(brightnessModifier * 110), 0);
    unicorn.set_pixel(15, 4, round(brightnessModifier * 156), 0, 0);
    unicorn.set_pixel(15, 5, round(brightnessModifier * 156), 0, 0);
    displayDigit(0, brightnessUnits);
    unicorn.set_pixel(4, 5, round(brightnessModifier * 156), round(brightnessModifier * 156), round(brightnessModifier * 156));
    displayDigit(6, brightnessTenths);
    displayDigit(10, brightnessHundreths);

    if (unicorn.is_pressed(unicorn.X)) {
      brightnessModifier = brightnessModifier + 0.01;
    } else if (unicorn.is_pressed(unicorn.Y)) {
      brightnessModifier = brightnessModifier - 0.01;
    }

    while (unicorn.is_pressed(unicorn.X) || unicorn.is_pressed(unicorn.Y)) {
      ;
    }

    brightnessModifier = max(brightnessModifier, 0.25);
    brightnessModifier = min(brightnessModifier, 1.60);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////DISPLAY BASICS/////////////////////////////////////////////////

void displayWeather(int offset, int weatherIndex) {
  for (int i = 0; i <= 6; i++) {
    for (int ii = 0; ii <= 4; ii++) {
      unicorn.set_pixel(ii + offset, i, round(brightnessModifier * weatherArray[weatherIndex][5 * i + ii][0]), round(brightnessModifier * weatherArray[weatherIndex][5 * i + ii][1]), round(brightnessModifier * weatherArray[weatherIndex][5 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displaySymbol(int offset, int digitIndex) {
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

  for (int i = 0; i <= 6; i++) {
    for (int ii = 0; ii <= 1; ii++) {
      unicorn.set_pixel(ii + offset, i, round(brightnessModifier * symbolArray[digitIndex][2 * i + ii][0]), round(brightnessModifier * symbolArray[digitIndex][2 * i + ii][1]), round(brightnessModifier * symbolArray[digitIndex][2 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displayDigit(int offset, int digitIndex) {
  for (int i = 0; i <= 6; i++) {
    for (int ii = 0; ii <= 2; ii++) {
      unicorn.set_pixel(ii + offset, i, round(brightnessModifier * digitArray[digitIndex][3 * i + ii][0]), round(brightnessModifier * digitArray[digitIndex][3 * i + ii][1]), round(brightnessModifier * digitArray[digitIndex][3 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displaySettings(int offset, int settingsIndex) {
  int settingsArray[2][35][3] = {
    {
      off, off, off, off, off,
      off, wht, off, wht, off,
      off, wht, off, wht, off,
      off, off, wht, off, off,
      off, off, wht, off, off,
      off, off, wht, off, off,
      off, off, wht, off, off
    },
    {
      off, off, off, off, off,
      wht, off, off, off, wht,
      off, off, wht, off, off,
      off, wht, wht, wht, off,
      off, off, wht, off, off,
      wht, off, off, off, wht,
      off, off, off, off, off
    }
  };

  for (int i = 0; i <= 6; i++) {
    for (int ii = 0; ii <= 4; ii++) {
      unicorn.set_pixel(ii + offset, i, round(brightnessModifier * settingsArray[settingsIndex][5 * i + ii][0]), round(brightnessModifier * settingsArray[settingsIndex][5 * i + ii][1]), round(brightnessModifier * settingsArray[settingsIndex][5 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displayRetry() {
  int cross[25][3] = {
    red, off, off, off, red,
    off, red, off, red, off,
    off, off, red, off, off,
    off, red, off, red, off,
    red, off, off, off, red
  };

  int wifi[77][3] = {
    off, off, off, off, off, off, off, off, off, off, off,
    red, off, red, off, red, off, red, red, off, red, off,
    red, off, red, off, off, off, red, off, off, off, off,
    red, red, red, off, red, off, red, red, off, red, off,
    red, off, red, off, red, off, red, off, off, red, off,
    off, off, off, off, off, off, off, off, off, off, off,
    off, off, off, off, off, off, off, off, off, off, off
  };
  unicorn.clear();

  while (true) {
    for (int i = 0; i <= 4; i++) {
      for (int ii = 0; ii <= 4; ii++) {
        unicorn.set_pixel(ii, i + 1, cross[5 * i + ii][0], cross[5 * i + ii][1], cross[5 * i + ii][2]);
      }
    }

    for (int i = 0; i <= 4; i++) {
      for (int ii = 0; ii <= 10; ii++) {
        unicorn.set_pixel(ii + 6, i, wifi[11 * i + ii][0], wifi[11 * i + ii][1], wifi[11 * i + ii][2]);
      }
    }

    delay(500);
    unicorn.clear();

    for (int i = 0; i <= 4; i++) {
      for (int ii = 0; ii <= 10; ii++) {
        unicorn.set_pixel(ii + 6, i, wifi[11 * i + ii][0], wifi[11 * i + ii][1], wifi[11 * i + ii][2]);
      }
    }

    delay(500);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displayHundred() {
  int hundred[49][3] = {
    wht, wht, off, off, off, off, off,
    off, wht, off, wht, wht, wht, off,
    wht, wht, wht, wht, off, wht, off,
    off, off, off, wht, wht, wht, wht,
    off, off, off, off, wht, wht, wht,
    off, off, off, off, wht, off, wht,
    off, off, off, off, wht, wht, wht
  };

  for (int i = 0; i <= 6; i++) {
    for (int ii = 0; ii <= 6; ii++) {
      unicorn.set_pixel(ii + 6, i, round(brightnessModifier * hundred[7 * i + ii][0]), round(brightnessModifier * hundred[7 * i + ii][1]), round(brightnessModifier * hundred[7 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displayMPH() {
  int mph[77][3] = {
    wht, off, wht, off, off, off, off, off, off, off, off,
    wht, wht, wht, off, wht, wht, wht, off, off, off, off,
    wht, off, wht, off, wht, off, wht, off, wht, off, wht,
    wht, off, wht, off, wht, wht, wht, off, wht, off, wht,
    off, off, off, off, wht, off, off, off, wht, wht, wht,
    off, off, off, off, wht, off, off, off, wht, off, wht,
    off, off, off, off, off, off, off, off, wht, off, wht
  };

  for (int i = 0; i <= 6; i++) {
    for (int ii = 0; ii <= 10; ii++) {
      unicorn.set_pixel(ii + 5, i, round(brightnessModifier * mph[11 * i + ii][0]), round(brightnessModifier * mph[11 * i + ii][1]), round(brightnessModifier * mph[11 * i + ii][2]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void displayFullscreen(int index) {
  int fullscreenArray[2][112][3] = {
    {
      pur, off, pur, ora, ora, ora, blu, off, off, grn, off, off, red, red, red, off,
      pur, off, pur, ora, off, off, blu, off, off, grn, off, off, red, off, red, off,
      pur, off, pur, ora, off, off, blu, off, off, grn, off, off, red, off, red, off,
      pur, pur, pur, ora, ora, ora, blu, off, off, grn, off, off, red, off, red, off,
      pur, off, pur, ora, off, off, blu, off, off, grn, off, off, red, off, red, off,
      pur, off, pur, ora, off, off, blu, off, off, grn, off, off, red, off, red, off,
      pur, off, pur, ora, ora, ora, blu, blu, blu, grn, grn, grn, red, red, red, off
    },
    {
      grn, grn, off, off, grn, grn, off, off, off, off, grn, grn, grn, off, grn, off,
      grn, off, grn, off, grn, off, grn, off, grn, off, grn, off, off, off, grn, off,
      grn, off, grn, off, grn, grn, off, off, off, off, grn, off, off, off, grn, grn,
      grn, grn, off, off, grn, grn, off, off, grn, off, grn, off, grn, off, grn, off,
      grn, off, grn, off, grn, off, grn, off, grn, off, grn, off, grn, off, grn, off,
      grn, off, grn, off, grn, off, grn, off, grn, off, grn, off, grn, off, grn, off,
      grn, grn, off, off, grn, off, grn, off, grn, off, grn, grn, grn, off, grn, grn
    }
  };

  for (int i = 0; i <= 6; i++) {
    for (int ii = 0; ii <= 15; ii++) {
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

  if (!error) {
    hrs = doc["hour"];
    mins = doc["minute"];
    secs = doc["seconds"];

    lastTimeCall = millis();
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
  int localSecs = secs + (millis() - lastTimeCall) / 1000;
  int minsOverflow = (localSecs - (localSecs % 60) ) / 60;
  int localMins = (mins + minsOverflow) % 60;
  int hrsOverflow = ((mins + minsOverflow) - localMins) / 60;
  int localHrs = (hrs + hrsOverflow) % 24;

  int minsSinceUpdate = (localHrs * 60 + localMins) % 180;
  int index;
  int threeHourIndex;
  int sixHourIndex;

  if (minsSinceUpdate <= 45 || minsSinceUpdate >= 90) {
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

  StaticJsonDocument<8192> doc;
  StaticJsonDocument<512> filter;

  filter["SiteRep"]["DV"]["Location"]["Period"][0]["Rep"] = true;

  DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));

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

    lastWeatherCall = millis();
    return;

  } else {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
}
