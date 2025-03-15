unsigned int crc16(unsigned int crc, unsigned char *buf, int len)
{
    for (int pos = 0; pos < len; pos++)
    {
        crc ^= (unsigned int)buf[pos];

        for (int i = 8; i != 0; i--)
        {
            if ((crc & 0x0001) != 0)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool isNumber(char *res, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != '-' && res[i] != '+'))
        {
            return false;
        }
    }
    return true;
}

int findCharInArrayRev(char array[], char c, int len)
{
    for (int i = len - 1; i >= 0; i--)
    {
        if (array[i] == c)
        {
            return i;
        }
    }
    return -1;
}

long getValue(char *buffer, int maxlen, char startchar, char endchar)
{
    int s = -1, e = -1;
    // Search forward for startchar
    for (int i = 0; i < maxlen; i++) {
        if (buffer[i] == startchar) {
            s = i;
            break;
        }
    }
    // Search forward for endchar after s
    for (int i = s + 1; i < maxlen; i++) {
        if (buffer[i] == endchar) {
            e = i;
            break;
        }
    }
    if (s == -1 || e == -1 || (e - s - 1) <= 0 || (e - s - 1) >= 16) {
        Serial.println("Error: Invalid start or end char index in getValue");
        return 0;
    }
    int l = e - s - 1;
    char res[16] = {0};
    strncpy(res, buffer + s + 1, l);
    res[l] = '\0';
    
    Serial.print("Extracted raw value: ");
    Serial.println(res);
    
    if (endchar == '*') {
        if (isNumber(res, l)) {
            float conv = atof(res) * 1000;  // Convert kWh to Wh
            Serial.print("Converted (scaled): ");
            Serial.println(conv);
            return (long)conv;
        }
    } else if (endchar == ')') {
        if (isNumber(res, l)) {
            float conv = atof(res);
            Serial.print("Converted: ");
            Serial.println(conv);
            return (long)conv;
        }
    }
    Serial.println("Warning: Unhandled endchar type");
    return 0;
}

bool decodeTelegram(int len)
{
    int startChar = findCharInArrayRev(telegram, '/', len);
    int endChar = findCharInArrayRev(telegram, '!', len);
    bool validCRCFound = false;

#ifdef DEBUG
    Serial.println("Debug: Raw telegram content:");
    Serial.println(telegram);
#endif

    if (startChar < 0) {
        Serial.println("Error: Start character '/' not found in telegram");
        return false;
    }
    if (endChar < 0) {
        Serial.println("Error: End character '!' not found in telegram");
        return false;
    }

    // Compute CRC from '/' to '!' (include the '!' character)
    currentCRC = crc16(0x0000, (unsigned char *)telegram + startChar, endChar - startChar);
    currentCRC = crc16(currentCRC, (unsigned char *)telegram + endChar, 1);

    // Extract the CRC from the message (assumed to be 4 hex digits immediately after '!')
    if (endChar + 4 < len)
    {
        char messageCRC[5] = {0};
        strncpy(messageCRC, telegram + endChar + 1, 4);
        messageCRC[4] = '\0';  // Ensure null termination

        // Remove any trailing whitespace or carriage return characters
        for (int i = 0; i < 4; i++) {
            if (messageCRC[i] == '\r' || messageCRC[i] == ' ') {
                messageCRC[i] = '\0';
                break;
            }
        }

        unsigned int receivedCRC = strtol(messageCRC, NULL, 16);

#ifdef DEBUG
        Serial.print("Received CRC (HEX): 0x");
        Serial.println(receivedCRC, HEX);
        Serial.print("Calculated CRC (HEX): 0x");
        Serial.println(currentCRC, HEX);
#endif

        validCRCFound = (receivedCRC == currentCRC);

#ifdef DEBUG
        if (validCRCFound)
            Serial.println("CRC Valid!");
        else
            Serial.println("CRC Invalid!");
#endif

    // TODO: remove this, it is for debugging
    validCRCFound = true;

    }
    else
    {
        Serial.println("Error: CRC Extraction Out of Bounds");
        return false;
    }

    // Loop through all telegramObjects and extract values from the corresponding OBIS line.
    for (int i = 0; i < NUMBER_OF_READOUTS; i++)
    {
        Serial.print("Searching for OBIS Code: ");
        Serial.println(telegramObjects[i].code);
        char *p = strstr(telegram, telegramObjects[i].code);
        if (p != NULL)
        {
            Serial.println("Found matching OBIS Code! Extracting value...");
            long newValue = getValue(p, strlen(p), '(', telegramObjects[i].endChar);
            if (newValue != telegramObjects[i].value)
            {
                telegramObjects[i].value = newValue;
                telegramObjects[i].sendData = true;
                Serial.print("Updated Value for ");
                Serial.print(telegramObjects[i].name);
                Serial.print(": ");
                Serial.println(newValue);
            }
        }
    }

    return validCRCFound;
}

bool readP1Serial()
{
    memset(telegram, 0, sizeof(telegram));
    int pos = 0;
    bool endFound = false;

    unsigned long startTime = millis();
    while (millis() - startTime < 3000) // 3s timeout
    {
        while (Serial2.available())
        {
            char c = Serial2.read();

            // Add the character to the telegram buffer if there is space
            if (pos < P1_MAXLINELENGTH - 2) 
            {
                telegram[pos++] = c;
            }

            // If the end marker '!' is encountered, flag that a full telegram is likely available
            if (c == '!')
            {
                endFound = true;
            }

            // When a newline is encountered, normalize the telegram string by adding a null terminator.
            if (c == '\n')
            {
                telegram[pos] = '\0';
            }
        }

        // If we have detected the end marker, process the full telegram.
        if (endFound) 
        {
            Serial.println("Full telegram received:");
            Serial.println(telegram);

            // Decode the full telegram using the total number of bytes read (pos)
            bool result = decodeTelegram(pos);

            if (result)
            {
                Serial.println("Telegram decoded successfully.");
                return true;
            }
            else
            {
                Serial.println("Failed to decode full telegram!");
                return false;
            }
        }
    }

    Serial.println("Error: Timeout while reading full P1 telegram");
    return false;
}



