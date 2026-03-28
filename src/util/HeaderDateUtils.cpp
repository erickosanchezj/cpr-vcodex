#include "HeaderDateUtils.h"

#include <GfxRenderer.h>
#include <HalPowerManager.h>

#include <ctime>

#include "CrossPointSettings.h"
#include "CrossPointState.h"
#include "ReadingStatsStore.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "util/TimeUtils.h"

namespace {
void drawHeaderDate(const GfxRenderer& renderer, const ThemeMetrics& metrics, const int pageWidth,
                    const std::string& dateText) {
  if (dateText.empty()) {
    return;
  }

  const bool showBatteryPercentage =
      SETTINGS.hideBatteryPercentage != CrossPointSettings::HIDE_BATTERY_PERCENTAGE::HIDE_ALWAYS;
  const int batteryX = pageWidth - 12 - metrics.batteryWidth;
  int rightEdge = batteryX - 8;

  if (showBatteryPercentage) {
    const std::string batteryText = std::to_string(powerManager.getBatteryPercentage()) + "%";
    rightEdge -= renderer.getTextWidth(SMALL_FONT_ID, batteryText.c_str()) + 4;
  }

  const int dateWidth = renderer.getTextWidth(SMALL_FONT_ID, dateText.c_str());
  const int dateX = std::max(metrics.contentSidePadding, rightEdge - dateWidth);
  renderer.drawText(SMALL_FONT_ID, dateX, metrics.topPadding + 5, dateText.c_str());
}

std::string formatHeaderDateText(const uint32_t timestamp, const bool usedFallback) {
  if (!TimeUtils::isClockValid(timestamp)) {
    return "";
  }

  tm localTime = {};
  time_t currentTime = static_cast<time_t>(timestamp);
  if (localtime_r(&currentTime, &localTime) == nullptr) {
    return "";
  }

  char buffer[24];
  snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d%s", localTime.tm_mday, localTime.tm_mon + 1,
           localTime.tm_year + 1900, usedFallback ? "!" : "");
  return buffer;
}
}  // namespace

HeaderDateUtils::DisplayDateInfo HeaderDateUtils::getDisplayDateInfo() {
  TimeUtils::configureTimezone();
  const uint32_t authoritativeTimestamp = TimeUtils::getAuthoritativeTimestamp();
  if (TimeUtils::isClockValid(authoritativeTimestamp)) {
    return {authoritativeTimestamp, false};
  }

  if (TimeUtils::isClockValid(APP_STATE.lastKnownValidTimestamp)) {
    return {APP_STATE.lastKnownValidTimestamp, true};
  }

  bool usedFallback = false;
  const uint32_t timestamp = READING_STATS.getDisplayTimestamp(&usedFallback);
  return {timestamp, usedFallback};
}

std::string HeaderDateUtils::getDisplayDateText() {
  if (!SETTINGS.displayDay) {
    return "";
  }

  const auto info = getDisplayDateInfo();
  return formatHeaderDateText(info.timestamp, info.usedFallback);
}

void HeaderDateUtils::drawHeaderWithDate(GfxRenderer& renderer, const char* title, const char* subtitle) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int pageWidth = renderer.getScreenWidth();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, title, subtitle);
  drawHeaderDate(renderer, metrics, pageWidth, getDisplayDateText());
}
