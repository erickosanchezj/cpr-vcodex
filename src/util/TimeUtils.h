#pragma once

#include <cstdint>

namespace TimeUtils {

void configureTimezone();
void stopNtp();
bool syncTimeWithNtp(uint32_t timeoutMs = 5000);
bool isClockValid();
bool isClockValid(uint32_t epochSeconds);
uint32_t getAuthoritativeTimestamp();
uint32_t getCurrentValidTimestamp();
uint32_t getLocalDayOrdinal(uint32_t epochSeconds);
uint32_t getDayOrdinalForDate(int year, unsigned month, unsigned day);
bool getDateFromDayOrdinal(uint32_t dayOrdinal, int& year, unsigned& month, unsigned& day);
bool wasTimeSyncedThisBoot();
const char* getCurrentTimeZoneLabel();

}  // namespace TimeUtils
