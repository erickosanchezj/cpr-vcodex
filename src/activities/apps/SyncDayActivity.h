#pragma once

#include "../Activity.h"

class SyncDayActivity final : public Activity {
  bool wifiConnectedOnEnter = false;
  bool connectedInActivity = false;
  bool syncing = false;
  bool lastSyncSucceeded = false;
  bool lastSyncFailed = false;
  unsigned long lastClockRefreshMs = 0;

  void openWifiSelection();
  void syncTime();
  void updateClockTick();
  bool isWifiConnected() const;
  std::string getStatusMessage() const;

 public:
  explicit SyncDayActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("SyncDay", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool preventAutoSleep() override { return syncing; }
};
