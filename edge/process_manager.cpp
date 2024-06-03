#include "process_manager.h"
#include "opcode.h"
#include "byte_op.h"
#include "setting.h"
#include <cstring>
#include <iostream>
#include <ctime>
using namespace std;

ProcessManager::ProcessManager()
{
  this->num = 0;
}

void ProcessManager::init()
{
}

// TODO: You should implement this function if you want to change the result of the aggregation
uint8_t *ProcessManager::processData(DataSet *ds, int *dlen)
{
  uint8_t *ret, *p;  // Pointer for the return value & Pointer for the current position in the buffer
  int num, len;  // Number of houses & length of the data
  HouseData *house;  // Pointer to the house data
  Info *info;  // Pointer to the info data
  TemperatureData *tdata;  // Pointer to the temperature data
  HumidityData *hdata;  // Pointer to the humidity data
  PowerData *pdata;  // Pointer to the power data
  char buf[BUFLEN];  // Buffer for the data
  ret = (uint8_t *)malloc(BUFLEN);  // Allocate memory for the return value
  int tmp, min_humid, min_temp, min_power, month;  // Temporary variable & minimum values
  time_t ts;  // Timestamp
  struct tm *tm;  // Time structure

  tdata = ds->getTemperatureData();  // Get the temperature data
  hdata = ds->getHumidityData();  // Get the humidity data
  num = ds->getNumHouseData();  // Get the number of houses

  // Example) I will give the minimum daily temperature (1 byte), the minimum daily humidity (1 byte), 
  // the minimum power data (2 bytes), the month value (1 byte) to the network manager
  
  // Example) getting the minimum daily temperature
  min_temp = (int) tdata->getMin();  // Get the minimum temperature

  // Example) getting the minimum daily humidity
  min_humid = (int) hdata->getMin();  // Get the minimum humidity

  // Example) getting the minimum power value
  min_power = 10000;  // Initialize the minimum power value
  for (int i=0; i<num; i++)  // Loop over all houses
  {
    house = ds->getHouseData(i);  // Get the house data for the current house
    pdata = house->getPowerData();  // Get the power data for the current house
    tmp = (int)pdata->getValue();  // Get the power value

    if (tmp < min_power)  // If the current power value is less than the minimum
      min_power = tmp;  // Update the minimum power value
  }

  // Example) getting the month value from the timestamp
  ts = ds->getTimestamp();  // Get the timestamp
  tm = localtime(&ts);  // Convert the timestamp to a time structure
  month = tm->tm_mon + 1;  // Get the month value

  // Example) initializing the memory to send to the network manager
  memset(ret, 0, BUFLEN);  // Clear the memory
  *dlen = 0;  // Initialize the data length
  p = ret;  // Set the current position to the start of the buffer

  // Example) saving the values in the memory
  VAR_TO_MEM_1BYTE_BIG_ENDIAN(min_temp, p);  // Save the minimum temperature
  *dlen += 1;  // Increase the data length
  VAR_TO_MEM_1BYTE_BIG_ENDIAN(min_humid, p);  // Save the minimum humidity
  *dlen += 1;  // Increase the data length
  VAR_TO_MEM_2BYTES_BIG_ENDIAN(min_power, p);  // Save the minimum power value
  *dlen += 2;  // Increase the data length
  VAR_TO_MEM_1BYTE_BIG_ENDIAN(month, p);  // Save the month value
  *dlen += 1;  // Increase the data length

  return ret;  // Return the buffer
}
