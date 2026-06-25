#pragma once
#include <M5Cardputer.h>
#include <SD.h>

String encryptPassword(const String& pass);
String decryptPassword(const String& enc);
String toHex(const String& in);
String fromHex(const String& in);