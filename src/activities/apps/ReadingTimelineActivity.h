#pragma once

#include <vector>

#include "util/ButtonNavigator.h"

#include "../Activity.h"
#include "util/ReadingStatsAnalytics.h"

class ReadingTimelineActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  int selectedIndex = 0;
  std::vector<ReadingStatsAnalytics::TimelineDayEntry> entries;
  bool waitForConfirmRelease = false;

  void refreshEntries();
  void openSelectedDay();

 public:
  explicit ReadingTimelineActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ReadingTimeline", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
