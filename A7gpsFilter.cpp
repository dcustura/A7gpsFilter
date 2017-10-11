#include "A7gpsFilter.h"

#define CR '\015'
#define LF '\012'

static const char gpsRdSeq[7] = { '+', 'G', 'P', 'S', 'R', 'D', ':' };

size_t A7GPSFilter::write(uint8_t character) {
  return serial.write(character);
}

static String toStr(char character) {
  return character >= ' ' ? String(character) : "\\" + String(character, 8);
}

void A7GPSFilter::pullGPSData() {
  for (;;) {
    if (serial.available() == 0) {
      return;
    }
    char character = serial.peek();
    //Serial.print(String("{") + recvState + "," +  toStr(character) + "}");
    switch (recvState) {
      case BOL:
        if (CR == character) {
          recvState = BOL_CR;
          serial.read();
          break;
        }
        recvState = LINE_CONTENT;
        return;
      case BOL_CR:
        if (LF == character) {
          recvState = GPS_HEAD;
          serial.read();
          headIdx = 0;
          break;
        }
        recvState = CR_LINE_CONTENT;
        return;
      case GPS_HEAD:
        if (gpsRdSeq[headIdx] == character) {
          headIdx++;
          serial.read();
          if (sizeof(gpsRdSeq) == headIdx) {
            recvState = GPS_BOL;
          }
          break;
        }
        tailIdx = 0;
        recvState = GPS_HEAD_PUSHBACK_CRLF;
        return;
      case GPS_BOL:
        recvState = ('$' == character) ? GPS_LINE_CONTENT : BOL;
        break;
      case GPS_LINE_CONTENT:
        character = serial.read();
        callback(character);
        if (CR == character) {
          recvState = GPS_LINE_END_CR;
        }
        break;
      case GPS_LINE_END_CR:
        character = serial.read();
        callback(character);
        recvState = (LF == character) ? GPS_BOL : GPS_LINE_CONTENT;
        break;
      default:
        return;
    }
  }
}

int A7GPSFilter::read() {
  int character;
  pullGPSData();
  switch (recvState) {
    case LINE_CONTENT:
      character = serial.read();
      if (CR == character) {
        recvState = LINE_END_CR;
      }
      return character;
    case CR_LINE_CONTENT:
      recvState = LINE_CONTENT;
      return CR;
    case LINE_END_CR:
      character = serial.read();
      recvState = (LF == character) ? BOL : LINE_CONTENT;
      return character;
    case GPS_HEAD_PUSHBACK_CRLF:
      recvState = GPS_HEAD_PUSHBACK_LF;
      return CR;
    case GPS_HEAD_PUSHBACK_LF:
      recvState = (headIdx == 0) ? BOL : GPS_HEAD_PUSHBACK;
      return LF;
    case GPS_HEAD_PUSHBACK:
      character = gpsRdSeq[tailIdx++];
      if (tailIdx == headIdx) {
        recvState = LINE_CONTENT;
      }
      return character;
    default:
      //Serial.println("\nState: " + String(recvState));
      return -1;
  }
}

int A7GPSFilter::peek() {
  pullGPSData();
  switch (recvState) {
    case LINE_CONTENT:
      // intentional fallback
    case LINE_END_CR:
      return serial.peek();
    case CR_LINE_CONTENT:
      // intentional fallback
    case GPS_HEAD_PUSHBACK_CRLF:
      return CR;
    case GPS_HEAD_PUSHBACK_LF:
      return LF;
    case GPS_HEAD_PUSHBACK:
      return gpsRdSeq[tailIdx];
    default:
      return -1;
  }
}

int A7GPSFilter::available() {
  return A7GPSFilter::peek() != -1;
}
