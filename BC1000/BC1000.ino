#include "RTClib.h"

#define FAULT_INDICATOR_LED     14
#define SET_RTC_MODE_PIN        11
#define DEFAULT_SERIAL_BAUD     115200

const bool SELF_TEST_STATUS = true; 

enum eDoorPosition_t {
  UP, 
  DOWN,
  end
};

struct xSolarEvent_t {
  uint16_t mDay;
  uint16_t mSunrise;
  uint16_t mSunset;
};

/*
  This table consists of 3 values, DAY, SUNRISE, SUNSET.

  DAY - The table consists of values for every 5 days until the end of a calendar year
  SUNRISE - This is the time of the sunrise in military time HHMM.
  SUNSET - This is the time of the sunset in military time HHMM.
*/
xSolarEvent_t Sun_LUT[] = {
  [0, 0646, 1815],            // 2023 Jan 1
  [5, 0646, 1819],            // 2023 Jan 5
  [10, 0646, 1823],           // 2023 Jan 10
  [15, 0645, 1828],           // 2023 Jan 15
  [20, 0643, 1833],           // 2023 Jan 20 
  [25, 0641, 1839],           // 2023 Jan 25
  [30, 0637, 1844],           // 2023 Jan 30
  [35, 0633, 1850],           // 2023 Feb 4
  [40, 0628, 1855],           // 2023 Feb 9
  [45, 0622, 1901],           // 2023 Feb 14
  [50, 0616, 1906],           // 2023 Feb 19 
  [55, 0609, 1912],           // 2023 Feb 24
  [60, 0602, 1917],           // 2023 Mar 1
  [65, 0554, 1923],           // 2023 Mar 6
  [70, 0546, 1928],           // 2023 Mar 11
  [75, 0538, 1934],           // 2023 Mar 16
  [80, 0530, 1940],           // 2023 Mar 21
  [85, 0521, 1945],           // 2023 Mar 26
  [90, 0512, 1951],           // 2023 Mar 31
  [95, 0503, 1957],           // 2023 Apr 5
  [100, 0455, 2003],          // 2023 Apr 10
  [105, 0446, 2009],          // 2023 Apr 15
  [110, 0438, 2015],          // 2023 Apr 20
  [115, 0429, 2022],          // 2023 Apr 25
  [120, 0421, 2028],          // 2023 Apr 30
  [125, 0414, 2035],          // 2023 May 5
  [130, 0407, 2041],          // 2023 May 10 
  [135, 0400, 2047],          // 2023 May 15
  [140, 0355, 2053],          // 2023 May 20
  [145, 0350, 2059],          // 2023 May 25
  [150, 0345, 2104],          // 2023 May 30
  [155, 0342, 2109],          // 2023 Jun 4
  [160, 0340, 2113],          // 2023 Jun 9
  [165, 0339, 2116],          // 2023 Jun 14
  [170, 0339, 2118],          // 2023 Jun 19
  [175, 0340, 2119],          // 2023 Jun 24
  [180, 0343, 2118],          // 2023 Jun 29
  [185, 0346, 2117],          // 2023 Jul 4
  [190, 0350, 2114],          // 2023 Jul 9
  [195, 0354, 2111],          // 2023 Jul 14
  [200, 0400, 2107],          // 2023 Jul 19
  [205, 0405, 2101],          // 2023 Jul 24
  [210, 0411, 2055],          // 2023 Jul 29
  [215, 0417, 2048],          // 2023 Aug 3
  [220, 0423, 2041],          // 2023 Aug 8
  [225, 0430, 2033],          // 2023 Aug 13
  [230, 0436, 2025],          // 2023 Aug 18
  [235, 0442, 2017],          // 2023 Aug 23
  [240, 0448, 2008],          // 2023 Aug 28
  [245, 0453, 1959],          // 2023 Sep 2
  [250, 0459, 1950],          // 2023 Sep 7
  [255, 0504, 1941],          // 2023 Sep 12
  [260, 0510, 1932],          // 2023 Sep 17
  [265, 0515, 1924],          // 2023 Sep 22
  [270, 0520, 1915],          // 2023 Sep 27
  [275, 0525, 1907],          // 2023 Oct 2
  [280, 0531, 1858],          // 2023 Oct 7
  [285, 0536, 1851],          // 2023 Oct 12
  [290, 0541, 1843],          // 2023 Oct 17
  [295, 0546, 1836],          // 2023 Oct 22
  [300, 0551, 1830],          // 2023 Oct 27
  [305, 0557, 1824],          // 2023 Nov 1
  [310, 0602, 1819],          // 2023 Nov 6
  [315, 0607, 1815],          // 2023 Nov 11
  [320, 0612, 1811],          // 2023 Nov 16
  [325, 0618, 1808],          // 2023 Nov 21
  [330, 0623, 1806],          // 2023 Nov 26
  [335, 0627, 1805],          // 2023 Dec 1
  [340, 0632, 1804],          // 2023 Dec 6
  [345, 0636, 1805],          // 2023 Dec 11
  [350, 0639, 1806],          // 2023 Dec 16
  [355, 0642, 1808],          // 2023 Dec 21
  [360, 0644, 1811],          // 2023 Dec 26
  [367, 0645, 1812]           // **overflow protection**
};

const uint16_t Sun_LUT_LENGTH = sizeof(Sun_LUT) / sizeof(xSolarEvent_t);

void setup() 
{
  // put your setup code here, to run once:
  setPinMode(FAULT_INDICATOR_LED, OUTPUT);
  setPinMode(SET_RTC_MODE_PIN, INPUT_PULLUP);
  digitalWrite(FAULT_INDICATOR_LED, HIGH);

  // Start up serial lines
  Serial.begin(DEFAULT_SERIAL_BAUD);

  // Self Test
  StartUpTest();
}

void loop() 
{
  // put your main code here, to run repeatedly:

}

void AssertFault()
{
  digitalWrite(FAULT_INDICATOR_LED, LOW);
}

void SetDoor(eDoorPosition_t pos)
{
  // sets door in the position
  switch(pos)
  {
    case eDoorPosition_t::UP:
      // whatever it takes to get the door up
      break;
    case eDoorPosition_t::DOWN:
      // whatever it takes to get the door down
      break;
    default:
      // some kind of error state that we should never get to. 
      break;
  }
}

/*****************************************
  TEST FUNCTIONS
******************************************/

void StartUpTest()
{
  // run systems test, verify that all systems are connected and fully operational!

  Test_Serial(); 
  Test_StatusLED();
  Test_I2C();  
  Test_RTC(); 
  Test_TempHumid(); 
  Test_MotorController();
}

void TEST_PASSED(String testName)
{
  Serial.printf("TEST: %s >> PASSED\r\n");
}

void TEST_FAILED(String testName)
{
  Serial.printf("TEST: %s >> FAILED\r\n");
}

void Test_Serial()
{
  Serial.println("Testing Serial Port ...");

  if(numBytesToWrite)
  {
    TEST_PASSED("Test_Serial");
  }
  else 
  {
    TEST_FAILED("Test_Serial");
  }
}

void Test_StatusLED() 
{
  for(uint8_t i = 0; i < 10; i++)
  {
    digitalWrite(FAULT_INDICATOR_PIN, LOW);
    delay(250);
    digitalWrite(FAULT_INDICATOR_LED, HIGH);
    delay(250);
  }

  TEST_PASSED("Test_StatusLED");
}

void Test_I2C()
{
  // scan all the I2C address and verify all peripherals are connected and working. 

}

void Test_RTC()
{
  // Check the real time clock to make sure it is connected and operating

  // 1. Check to see if the RTC exists 
}

void Test_TempHumid()
{
  // Check the temp and humidity sensor to make sure it is connected and operating
}

void Test_MotorContorller()
{
  // Check the motor controller to make sure it is connected and working
}

/*****************************************
  SYSTEM FUNCTIONS
******************************************/

uint16_t getCurrDayIndex(uint16_t &day)
{
  uint16_t result = 0xFFFF;

  for(uint16_t i = 0; i < Sun_LUT_LENGTH; i++)
  {
    if(Sun_LUT[i].mDay <= day && Sun_LUT[i+1].mDay > day)
    {
      result = i;
      break;
    }
  }

  return result; 
}

uint16_t getSunrise(uint16_t &day)
{
  uint16_t result = 0; 
  uint16_t currDay = getCurrDayIndex(day);

  if(currDay != 0xFFFF)
  {
    result = Sun_LUT[currDay].mSunrise; 
  }
  else
  {
    // Report Error
  }

  return result; 
}

uint16_t getSunset(uint16_t &day)
{
  uint16_t result = 0; 
  uint16_t currDay = getCurrDayIndex(day);

  if(currDay != 0xFFFF)
  {
    result = Sun_LUT[currDay].mSunset; 
  }
  else
  {
    // Report Error
  }

  return result; 
}

/*****************************************
  REAL TIME CLOCK
******************************************/

void init_RTC()
{
  if(digitalRead(SET_RTC_MODE_PIN) == LOW)
  {
    int inputChar = '': 
    // Have a set pin pull down that can put the 
    // unit into "set date and time" mode
    Serial.printf("SET DATE AND TIME? (Y/N)");
  
    for(uint8_t i = 0; i < 200; i++)
    {
      if(Serial.available() > 0)
      {
        inputChar = Serial.read();
        break;
      }
      delay(300);
    }

    if(inputChar == 'Y')
    {
      Serial.println("Received character 'y', proceeding with setting date and time\r\n");

      Serial.println("Set Year:");
      while
      {}
      Serial.println("Set Month:");
      Serial.println("Set Day:");

      Serial.println("Set Hour:");
      Serial.println("Set Minute:");

    }
    else 
    {
      Serial.println("Did not recieve a 'y' character or system timed out before character was read\r\n");
    }
  }
}

/*
  returns the day of the year for comparison with the LUT
*/
uint16_t getDay()
{

}

/*
  returns 4 digits representing HH:MM. 
*/
uint16_t getHourMin()
{

}

/*****************************************
  TEMP AND HUMID 
******************************************/

void init_TandH()
{
  
}

uint32_t getTemp()
{

}

uint32_t getHumid()
{

}

/*****************************************
  MOTOR CONTROLLER
******************************************/

void init_Motor()
{

}