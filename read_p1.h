#ifndef READ_P1_H
#define READ_P1_H

unsigned int crc16(unsigned int crc, unsigned char *buf, int len);
bool isNumber(char *res, int len);
int findCharInArrayRev(char array[], char c, int len);
long getValue(char *buffer, int maxlen, char startchar, char endchar);
bool decodeTelegram(int len);
bool readP1Serial();

#endif
