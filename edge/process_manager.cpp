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
  uint8_t *ret, *p;
  int num, len;
  HouseData *house;
  Info *info;
  TemperatureData *tdata;
  HumidityData *hdata;
  PowerData *pdata;
  char buf[BUFLEN];
  ret = (uint8_t *)malloc(BUFLEN);
  time_t ts;
  struct tm *tm;

  tdata = ds->getTemperatureData();
  hdata = ds->getHumidityData();
  num = ds->getNumHouseData();

  // 1) the minimum daily temperature (1 byte), the minimum daily humidity (1 byte), the minimum power data (2 bytes)
//   int tmp, min_humid, min_temp, min_power, month;
//   min_temp = (int) tdata->getMin();

//   min_humid = (int) hdata->getMin();

//   min_power = 10000;
//   for (int i=0; i<num; i++)
//   {
//     house = ds->getHouseData(i);
//     pdata = house->getPowerData();
//     tmp = (int)pdata->getValue();

//     if (tmp < min_power)
//       min_power = tmp;
//   }

//   memset(ret, 0, BUFLEN);
//   *dlen = 0;
//   p = ret;

//   VAR_TO_MEM_1BYTE_BIG_ENDIAN(min_temp, p);
//   *dlen += 1;
//   VAR_TO_MEM_1BYTE_BIG_ENDIAN(min_humid, p);
//   *dlen += 1;
//   VAR_TO_MEM_2BYTES_BIG_ENDIAN(min_power, p);
//   *dlen += 2;

  // 2) the average daily temperature (1 byte), the average daily humidity (1 byte), the average power data (2 bytes)
  int tmp, avg_humid, avg_temp, avg_power, month;
  avg_temp = (int) tdata->getValue();

  avg_humid = (int) hdata->getValue();

//   avg_power = (int) pdata->getValue();
  avg_power = 0;
  for (int i=0; i<num; i++)
  {
    house = ds->getHouseData(i);
    pdata = house->getPowerData();
    tmp = (int)pdata->getValue();
    avg_power += tmp;
  }
  avg_power /= num;

  memset(ret, 0, BUFLEN);
  *dlen = 0;
  p = ret;

  VAR_TO_MEM_1BYTE_BIG_ENDIAN(avg_temp, p);
  *dlen += 1;
  VAR_TO_MEM_1BYTE_BIG_ENDIAN(avg_humid, p);
  *dlen += 1;
  VAR_TO_MEM_2BYTES_BIG_ENDIAN(avg_power, p);
  *dlen += 2;

  // 3) the maximum daily temperature (1 byte), the maximum daily humidity (1 byte), the maximum power data (2 bytes)
//   int tmp, max_humid, max_temp, max_power, month;
//   max_temp = (int) tdata->getMax();

//   max_humid = (int) hdata->getMax();

//   max_power = 0;
//   for (int i=0; i<num; i++)
//   {
//     house = ds->getHouseData(i);
//     pdata = house->getPowerData();
//     tmp = (int)pdata->getValue();

//     if (tmp > max_power)
//     max_power = tmp;
//   }

//   memset(ret, 0, BUFLEN);
//   *dlen = 0;
//   p = ret;

//   VAR_TO_MEM_1BYTE_BIG_ENDIAN(max_temp, p);
//   *dlen += 1;
//   VAR_TO_MEM_1BYTE_BIG_ENDIAN(max_humid, p);
//   *dlen += 1;
//   VAR_TO_MEM_2BYTES_BIG_ENDIAN(max_power, p);
//   *dlen += 2;

  return ret;
}
