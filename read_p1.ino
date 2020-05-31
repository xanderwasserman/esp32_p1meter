unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
{
  for (int pos = 0; pos < len; pos++) {
    crc ^= (unsigned int)buf[pos];

    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }

  return crc;
}

bool isNumber(char *res, int len)
{
  for (int i = 0; i < len; i++) {
    if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != 0)) {
      return false;
    }
  }

  return true;
}

int FindCharInArrayRev(char array[], char c, int len)
{
  for (int i = len - 1; i >= 0; i--) {
    if (array[i] == c) {
      return i;
    }
  }

  return -1;
}

long getValue(char *buffer, int maxlen, char startchar, char endchar)
{
  int s = FindCharInArrayRev(buffer, startchar, maxlen - 2);
  int l = FindCharInArrayRev(buffer, endchar, maxlen - 2) - s - 1;

  char res[16];
  memset(res, 0, sizeof(res));

  if (strncpy(res, buffer + s + 1, l))
  {
    if (endchar == '*')
    {
      if (isNumber(res, l))
        return (1000 * atof(res));
    }
    else if (endchar == ')')
    {
      if (isNumber(res, l))
        return atof(res);
    }
  }
  
  return 0;
}

void getValueWithDqCheck(long &prev_value, long &value, int len, char startChar, char endChar)
{
    prev_value = value;
    value = getValue(telegram, len, startChar, endChar);
    
    // Force the current value to never be smaller than the previous value
    if (value < prev_value)
    {
      value = prev_value;
    }
}

bool decode_telegram(int len)
{
  int startChar = FindCharInArrayRev(telegram, '/', len);
  int endChar = FindCharInArrayRev(telegram, '!', len);
  bool validCRCFound = false;

  if (startChar >= 0)
  {
    currentCRC = CRC16(0x0000, (unsigned char *) telegram + startChar, len - startChar);
  }
  else if (endChar >= 0)
  {
    currentCRC = CRC16(currentCRC, (unsigned char*)telegram + endChar, 1);

    char messageCRC[5];
    strncpy(messageCRC, telegram + endChar + 1, 4);

    messageCRC[4] = 0;
    validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);

    currentCRC = 0;
  }
  else
  {
    currentCRC = CRC16(currentCRC, (unsigned char*) telegram, len);
  }

  if (strncmp(telegram, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0)
  {
    getValueWithDqCheck(CONSUMPTION_LOW_TARIF_PREV, CONSUMPTION_LOW_TARIF, len, '(', '*');
  }

  if (strncmp(telegram, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0)
  {
    getValueWithDqCheck(CONSUMPTION_HIGH_TARIF_PREV, CONSUMPTION_HIGH_TARIF, len, '(', '*');
  }

  if (strncmp(telegram, "1-0:2.8.1", strlen("1-0:2.8.1")) == 0)
  {
    getValueWithDqCheck(DELIVERED_LOW_TARIF_PREV, DELIVERED_LOW_TARIF, len, '(', '*');
  }

  if (strncmp(telegram, "1-0:2.8.2", strlen("1-0:2.8.2")) == 0)
  {
    getValueWithDqCheck(DELIVERED_HIGH_TARIF_PREV, DELIVERED_HIGH_TARIF, len, '(', '*');
  }

  if (strncmp(telegram, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0)
  {
    getValueWithDqCheck(GAS_METER_M3_PREV, GAS_METER_M3, len, '(', '*');
  }

  if (strncmp(telegram, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0)
  {
    ACTUAL_CONSUMPTION = getValue(telegram, len, '(', '*');
  }

  if (strncmp(telegram, "1-0:21.7.0", strlen("1-0:21.7.0")) == 0)
  {
    INSTANT_POWER_USAGE = getValue(telegram, len, '(', '*');
  }

  if (strncmp(telegram, "1-0:31.7.0", strlen("1-0:31.7.0")) == 0)
  {
    INSTANT_POWER_CURRENT = getValue(telegram, len, '(', '*');
  }

  if (strncmp(telegram, "0-0:96.14.0", strlen("0-0:96.14.0")) == 0)
  {
    ACTUAL_TARIF = getValue(telegram, len, '(', ')');
  }

  if (strncmp(telegram, "0-0:96.7.21", strlen("0-0:96.7.21")) == 0)
  {
    SHORT_POWER_OUTAGES = getValue(telegram, len, '(', ')');
  }

  if (strncmp(telegram, "0-0:96.7.9", strlen("0-0:96.7.9")) == 0)
  {
    LONG_POWER_OUTAGES = getValue(telegram, len, '(', ')');
  }

  if (strncmp(telegram, "1-0:32.32.0", strlen("1-0:32.32.0")) == 0)
  {
    SHORT_POWER_DROPS = getValue(telegram, len, '(', ')');
  }

  if (strncmp(telegram, "1-0:32.36.0", strlen("1-0:32.36.0")) == 0)
  {
    SHORT_POWER_PEAKS = getValue(telegram, len, '(', ')');
  }

  return validCRCFound;
}

void read_p1_serial()
{
  if (Serial.available())
  {
    memset(telegram, 0, sizeof(telegram));

    while (Serial.available())
    {
      ESP.wdtDisable();
      int len = Serial.readBytesUntil('\n', telegram, P1_MAXLINELENGTH);
      ESP.wdtEnable(1);

      telegram[len] = '\n';
      telegram[len + 1] = 0;
      yield();
      
      bool result = decode_telegram(len + 1);

      if (useCRC) {
        if (result) {
          send_data_to_broker();
        }
      } else {
        send_data_to_broker();
      }
    }
  }
}
