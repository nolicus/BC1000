#include "RTClib.h"
#include <stdio.h>
#include "Wire.h"

#define FAULT_INDICATOR_LED           14
#define SET_RTC_MODE_PIN              11
#define DEFAULT_SERIAL_BAUD           115200

#define DEFAULT_CHAR_BUFF_SIZE_SMALL  32

RTC_DS3231 rtc;

const bool SELF_TEST_STATUS = true; 

enum eDoorPosition_t {
  UP, 
  DOWN,
  end
};

typedef struct  {
  uint16_t mDay;
  uint16_t mSunrise;
  uint16_t mSunset;
} xSolarEvent_t;

/*
  This table consists of 3 values, DAY, SUNRISE, SUNSET.

  DAY - The table consists of values for every 5 days until the end of a calendar year
  SUNRISE - This is the time of the sunrise in military time HHMM.
  SUNSET - This is the time of the sunset in military time HHMM.
*/
xSolarEvent_t Sun_LUT[74];
const uint16_t Sun_LUT_LENGTH = sizeof(Sun_LUT) / sizeof(xSolarEvent_t);

void setup() 
{
  // Start up serial lines
  Serial.begin(DEFAULT_SERIAL_BAUD);
  delay(5000);

  Wire1.begin();
  
  // Fill in the sunrise and sunset table (Nautical Twilight)
  Sun_LUT[0] = (xSolarEvent_t) {0, 646, 1815};            // 2023 Jan 1
  Sun_LUT[1] = (xSolarEvent_t) {5, 646, 1819};            // 2023 Jan 5
  Sun_LUT[2] = (xSolarEvent_t) {10, 646, 1823};           // 2023 Jan 10
  Sun_LUT[3] = (xSolarEvent_t) {15, 645, 1828};           // 2023 Jan 15
  Sun_LUT[4] = (xSolarEvent_t) {20, 643, 1833};           // 2023 Jan 20 
  Sun_LUT[5] = (xSolarEvent_t) {25, 641, 1839};           // 2023 Jan 25
  Sun_LUT[6] = (xSolarEvent_t) {30, 637, 1844};           // 2023 Jan 30
  Sun_LUT[7] = (xSolarEvent_t) {35, 633, 1850};           // 2023 Feb 4
  Sun_LUT[8] = (xSolarEvent_t) {40, 628, 1855};           // 2023 Feb 9
  Sun_LUT[9] = (xSolarEvent_t) {45, 622, 1901};           // 2023 Feb 14
  Sun_LUT[10] = (xSolarEvent_t) {50, 616, 1906};           // 2023 Feb 19 
  Sun_LUT[11] = (xSolarEvent_t) {55, 609, 1912};           // 2023 Feb 24
  Sun_LUT[12] = (xSolarEvent_t) {60, 602, 1917};           // 2023 Mar 1
  Sun_LUT[13] = (xSolarEvent_t) {65, 554, 1923};           // 2023 Mar 6
  Sun_LUT[14] = (xSolarEvent_t) {70, 546, 1928};           // 2023 Mar 11
  Sun_LUT[15] = (xSolarEvent_t) {75, 538, 1934};           // 2023 Mar 16
  Sun_LUT[16] = (xSolarEvent_t) {80, 530, 1940};           // 2023 Mar 21
  Sun_LUT[17] = (xSolarEvent_t) {85, 521, 1945};           // 2023 Mar 26
  Sun_LUT[18] = (xSolarEvent_t) {90, 512, 1951};           // 2023 Mar 31
  Sun_LUT[19] = (xSolarEvent_t) {95, 503, 1957};           // 2023 Apr 5
  Sun_LUT[20] = (xSolarEvent_t) {100, 455, 2003};          // 2023 Apr 10
  Sun_LUT[21] = (xSolarEvent_t) {105, 446, 2009};          // 2023 Apr 15
  Sun_LUT[22] = (xSolarEvent_t) {110, 438, 2015};          // 2023 Apr 20
  Sun_LUT[23] = (xSolarEvent_t) {115, 429, 2022};          // 2023 Apr 25
  Sun_LUT[24] = (xSolarEvent_t) {120, 421, 2028};          // 2023 Apr 30
  Sun_LUT[25] = (xSolarEvent_t) {125, 414, 2035};          // 2023 May 5
  Sun_LUT[26] = (xSolarEvent_t) {130, 407, 2041};          // 2023 May 10 
  Sun_LUT[27] = (xSolarEvent_t) {135, 400, 2047};          // 2023 May 15
  Sun_LUT[28] = (xSolarEvent_t) {140, 355, 2053};          // 2023 May 20
  Sun_LUT[29] = (xSolarEvent_t) {145, 350, 2059};          // 2023 May 25
  Sun_LUT[30] = (xSolarEvent_t) {150, 345, 2104};          // 2023 May 30
  Sun_LUT[31] = (xSolarEvent_t) {155, 342, 2109};          // 2023 Jun 4
  Sun_LUT[32] = (xSolarEvent_t) {160, 340, 2113};          // 2023 Jun 9
  Sun_LUT[33] = (xSolarEvent_t) {165, 339, 2116};          // 2023 Jun 14
  Sun_LUT[34] = (xSolarEvent_t) {170, 339, 2118};          // 2023 Jun 19
  Sun_LUT[35] = (xSolarEvent_t) {175, 340, 2119};          // 2023 Jun 24
  Sun_LUT[36] = (xSolarEvent_t) {180, 343, 2118};          // 2023 Jun 29
  Sun_LUT[37] = (xSolarEvent_t) {185, 346, 2117};          // 2023 Jul 4
  Sun_LUT[38] = (xSolarEvent_t) {190, 350, 2114};          // 2023 Jul 9
  Sun_LUT[39] = (xSolarEvent_t) {195, 354, 2111};          // 2023 Jul 14
  Sun_LUT[40] = (xSolarEvent_t) {200, 400, 2107};          // 2023 Jul 19
  Sun_LUT[41] = (xSolarEvent_t) {205, 405, 2101};          // 2023 Jul 24
  Sun_LUT[42] = (xSolarEvent_t) {210, 411, 2055};          // 2023 Jul 29
  Sun_LUT[43] = (xSolarEvent_t) {215, 417, 2048};          // 2023 Aug 3
  Sun_LUT[44] = (xSolarEvent_t) {220, 423, 2041};          // 2023 Aug 8
  Sun_LUT[45] = (xSolarEvent_t) {225, 430, 2033};          // 2023 Aug 13
  Sun_LUT[46] = (xSolarEvent_t) {230, 436, 2025};          // 2023 Aug 18
  Sun_LUT[47] = (xSolarEvent_t) {235, 442, 2017};          // 2023 Aug 23
  Sun_LUT[48] = (xSolarEvent_t) {240, 448, 2008};          // 2023 Aug 28
  Sun_LUT[49] = (xSolarEvent_t) {245, 453, 1959};          // 2023 Sep 2
  Sun_LUT[50] = (xSolarEvent_t) {250, 459, 1950};          // 2023 Sep 7
  Sun_LUT[51] = (xSolarEvent_t) {255, 504, 1941};          // 2023 Sep 12
  Sun_LUT[52] = (xSolarEvent_t) {260, 510, 1932};          // 2023 Sep 17
  Sun_LUT[53] = (xSolarEvent_t) {265, 515, 1924};          // 2023 Sep 22
  Sun_LUT[54] = (xSolarEvent_t) {270, 520, 1915};          // 2023 Sep 27
  Sun_LUT[55] = (xSolarEvent_t) {275, 525, 1907};          // 2023 Oct 2
  Sun_LUT[56] = (xSolarEvent_t) {280, 531, 1858};          // 2023 Oct 7
  Sun_LUT[57] = (xSolarEvent_t) {285, 536, 1851};          // 2023 Oct 12
  Sun_LUT[58] = (xSolarEvent_t) {290, 541, 1843};          // 2023 Oct 17
  Sun_LUT[59] = (xSolarEvent_t) {295, 546, 1836};          // 2023 Oct 22
  Sun_LUT[60] = (xSolarEvent_t) {300, 551, 1830};          // 2023 Oct 27
  Sun_LUT[61] = (xSolarEvent_t) {305, 557, 1824};          // 2023 Nov 1
  Sun_LUT[62] = (xSolarEvent_t) {310, 602, 1819};          // 2023 Nov 6
  Sun_LUT[63] = (xSolarEvent_t) {315, 607, 1815};          // 2023 Nov 11
  Sun_LUT[64] = (xSolarEvent_t) {320, 612, 1811};          // 2023 Nov 16
  Sun_LUT[65] = (xSolarEvent_t) {325, 618, 1808};          // 2023 Nov 21
  Sun_LUT[66] = (xSolarEvent_t) {330, 623, 1806};          // 2023 Nov 26
  Sun_LUT[67] = (xSolarEvent_t) {335, 627, 1805};          // 2023 Dec 1
  Sun_LUT[68] = (xSolarEvent_t) {340, 632, 1804};          // 2023 Dec 6
  Sun_LUT[69] = (xSolarEvent_t) {345, 636, 1805};          // 2023 Dec 11
  Sun_LUT[70] = (xSolarEvent_t) {350, 639, 1806};          // 2023 Dec 16
  Sun_LUT[71] = (xSolarEvent_t) {355, 642, 1808};          // 2023 Dec 21
  Sun_LUT[72] = (xSolarEvent_t) {360, 644, 1811};          // 2023 Dec 26
  Sun_LUT[73] = (xSolarEvent_t) {367, 645, 1812};           // **overflow protection**

  // put your setup code here, to run once:
  pinMode(FAULT_INDICATOR_LED, OUTPUT);
  pinMode(SET_RTC_MODE_PIN, INPUT_PULLUP);
  digitalWrite(FAULT_INDICATOR_LED, HIGH);

  // Start up RTC
  init_RTC();

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

  Test_Serial();              // COMPLETE - RED
  Test_StatusLED();           // INCOMPLETE 
  Test_I2C();                 // INCOMPLETE
  Test_RTC();                 // COMPLETE - RED
  Test_TempHumid();           // INCOMPLETE
  Test_MotorController();     // INCOMPLETE
}

void TEST_PASSED(String testName)
{
  char buffer[DEFAULT_CHAR_BUFF_SIZE_SMALL] = "";
  sprintf(buffer, "TEST: %s >> PASSED\r\n", testName.c_str());  
  Serial.printf(buffer);
}

void TEST_FAILED(String testName)
{
  char buffer[DEFAULT_CHAR_BUFF_SIZE_SMALL] = "";
  sprintf(buffer, "TEST: %s >> FAILED\r\n", testName.c_str());  
  Serial.printf(buffer);
}

void Test_Serial()
{
  Serial.print("Testing Serial Port ...\r\n");

  // if(numBytesToWrite)
  // {
    TEST_PASSED("Test_Serial");
  // }
  // else 
  // {
  //   TEST_FAILED("Test_Serial");
  // }
}

void Test_StatusLED() 
{
  for(uint8_t i = 0; i < 10; i++)
  {
    digitalWrite(FAULT_INDICATOR_LED, LOW);
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

  printTime();
   
  // 1. Check to see if the RTC exists 
}

void Test_TempHumid()
{
  // Check the temp and humidity sensor to make sure it is connected and operating
}

void Test_MotorController()
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
  Serial.print("STARTING INIT_RTC...\r\n");

  if(!rtc.begin(&Wire1))
  {
    Serial.print("Couldn't find RTC\r\n");
    Serial.flush();
  }
  else
  {
    Serial.print("RTC Found\r\n");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));      // Use this line if need to update timing.
  }

  if(rtc.lostPower())
  {
    Serial.print("RTC lost power, let's set the time!\r\n");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    
  }
  else
  {
    Serial.print("RTC hasn't lost power\r\n");
  }

  Serial.print("INIT_RTC COMPLETE ...\r\n\r\n");
}

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

/*
  Get the current time from the RTC, and print it out
*/
void printTime()
{
    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");

    // calculate a date which is 7 days, 12 hours, 30 minutes, 6 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));

    Serial.print(" now + 7d + 12h + 30m + 6s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();

    Serial.print("Temperature: ");
    Serial.print(rtc.getTemperature());
    Serial.println(" C");

    Serial.println();
    delay(3000);
}

#define JANUARY_DAY_COUNT     31
#define FEBRUARY_DAY_COUNT    28
#define MARCH_DAY_COUNT       31
#define APRIL_DAY_COUNT       30
#define MAY_DAY_COUNT         31
#define JUNE_DAY_COUNT        30
#define JULY_DAY_COUNT        31
#define AUGUST_DAY_COUNT      31
#define SEPTEMBER_DAY_COUNT   30
#define OCTOBER_DAY_COUNT     31
#define NOVEMBER_DAY_COUNT    30
#define DECEMBER_DAY_COUNT    31

#define LEAP_YEAR_BASE        2020

/*
  returns the day of the year for comparison with the LUT
*/
uint16_t GetDaysToDate()
{
  uint16_t runningDayTotal = 0;
  uint16_t month = 0; 
  int year = 0; 
    
  DateTime now = rtc.now();
  month = now.month(); 
  year = now.year(); 

  if((year - LEAP_YEAR_BASE) % 4)
  {
    runningDayTotal = monthToDays(month) + 1;           
  }
  else
  {
    runningDayTotal = monthToDays(month);        
  }
  
  runningDayTotal += now.day();  
  
  Serial.print(runningDayTotal, DEC);

  return runningDayTotal; 
}

/*
  returns 4 digits representing HH:MM. 
*/
uint16_t GetHourMin()
{
  uint16_t hour = 0; 
  uint16_t min = 0; 

  DateTime now = rtc.now();
  hour = now.hour(); 
  min = now.minute();  

  return min | (hour << 8);
}

uint16_t monthToDays(uint16_t month)
{
  uint16_t totalDays = 0; 
  
  // NOTE: This switch statement is intended to fall through, adding all
  // subsequent months. 
  switch(month)
  {
    case 12: 
      totalDays += NOVEMBER_DAY_COUNT;
    case 11: 
      totalDays += OCTOBER_DAY_COUNT;
    case 10:
      totalDays += SEPTEMBER_DAY_COUNT;
    case 9:                      
      totalDays += AUGUST_DAY_COUNT;
    case 8:
      totalDays += JULY_DAY_COUNT;
    case 7:
      totalDays += JUNE_DAY_COUNT;
    case 6:
      totalDays += MAY_DAY_COUNT;
    case 5:
      totalDays += APRIL_DAY_COUNT;
    case 4:
      totalDays += MARCH_DAY_COUNT;
    case 3:
      totalDays += FEBRUARY_DAY_COUNT;
    case 2:
      totalDays += JANUARY_DAY_COUNT;
      break;                                    
  }  

  return totalDays; 
}

/*****************************************
  TEMP AND HUMID 
******************************************/

void init_TandH()
{
  
}

uint32_t getTemp()
{
  uint32_t result = 0; 

  return result; 
}

uint32_t getHumid()
{
  uint32_t result = 0; 

  return result; 
}

/*****************************************
  MOTOR CONTROLLER
******************************************/

void init_Motor()
{

}