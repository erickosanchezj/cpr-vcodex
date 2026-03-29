#pragma once

#include <vector>

#include "AchievementsStore.h"
#include "../Activity.h"
#include "util/ButtonNavigator.h"

class AchievementsActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  int selectedIndex = 0;
  std::vector<AchievementView> achievements;

  void refreshEntries();

 public:
  explicit AchievementsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Achievements", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
