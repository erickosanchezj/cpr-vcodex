#include "AppsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "BookmarksAppActivity.h"
#include "ReadingHeatmapActivity.h"
#include "ReadingStatsActivity.h"
#include "ReadingTimelineActivity.h"
#include "SleepAppActivity.h"
#include "SyncDayActivity.h"
#include "components/UITheme.h"
#include "util/HeaderDateUtils.h"

void AppsActivity::onEnter() {
  Activity::onEnter();
  selectedIndex = 0;
  requestUpdate();
}

void AppsActivity::loop() {
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    onGoHome();
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    openSelectedApp();
    return;
  }

  buttonNavigator.onNext([this] {
    selectedIndex = ButtonNavigator::nextIndex(selectedIndex, APP_COUNT);
    requestUpdate();
  });

  buttonNavigator.onPrevious([this] {
    selectedIndex = ButtonNavigator::previousIndex(selectedIndex, APP_COUNT);
    requestUpdate();
  });
}

void AppsActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  HeaderDateUtils::drawHeaderWithDate(renderer, tr(STR_APPS));

  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int contentHeight = pageHeight - contentTop - metrics.buttonHintsHeight - metrics.verticalSpacing * 2;

  GUI.drawList(renderer, Rect{0, contentTop, pageWidth, contentHeight}, APP_COUNT, selectedIndex,
               [](const int index) {
                 if (index == 0) return std::string(tr(STR_SETTINGS_TITLE));
                 if (index == 1) return std::string(tr(STR_READING_STATS));
                 if (index == 2) return std::string(tr(STR_READING_HEATMAP));
                 if (index == 3) return std::string(tr(STR_READING_TIMELINE));
                 if (index == 4) return std::string(tr(STR_MENU_RECENT_BOOKS));
                 if (index == 5) return std::string(tr(STR_BOOKMARKS));
                 if (index == 6) return std::string(tr(STR_SYNC_DAY));
                 if (index == 7) return std::string(tr(STR_FILE_TRANSFER));
                 return std::string(tr(STR_SLEEP));
               },
               [](const int index) {
                 if (index == 0) return std::string(tr(STR_SETTINGS_APP_DESC));
                 if (index == 1) return std::string(tr(STR_READING_STATS_DESC));
                 if (index == 2) return std::string(tr(STR_READING_HEATMAP_DESC));
                 if (index == 3) return std::string(tr(STR_READING_TIMELINE_DESC));
                 if (index == 4) return std::string(tr(STR_RECENT_BOOKS_APP_DESC));
                 if (index == 5) return std::string(tr(STR_BOOKMARKS_APP_DESC));
                 if (index == 6) return std::string(tr(STR_SYNC_DAY_DESC));
                 if (index == 7) return std::string(tr(STR_FILE_TRANSFER_APP_DESC));
                 return std::string(tr(STR_SLEEP_APP_DESC));
               },
               [](const int index) {
                 if (index == 0) return UIIcon::Settings;
                 if (index == 1) return UIIcon::Book;
                 if (index == 2) return UIIcon::Library;
                 if (index == 3) return UIIcon::Recent;
                 if (index == 4) return UIIcon::Recent;
                 if (index == 5) return UIIcon::Book;
                 if (index == 6) return UIIcon::Wifi;
                 if (index == 7) return UIIcon::Transfer;
                 return UIIcon::Folder;
               });

  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

void AppsActivity::openSelectedApp() {
  std::unique_ptr<Activity> activity;
  if (selectedIndex == 0) {
    activityManager.goToSettings();
    return;
  } else if (selectedIndex == 1) {
    activity = std::make_unique<ReadingStatsActivity>(renderer, mappedInput);
  } else if (selectedIndex == 2) {
    activity = std::make_unique<ReadingHeatmapActivity>(renderer, mappedInput);
  } else if (selectedIndex == 3) {
    activity = std::make_unique<ReadingTimelineActivity>(renderer, mappedInput);
  } else if (selectedIndex == 4) {
    activityManager.goToRecentBooks();
    return;
  } else if (selectedIndex == 5) {
    activity = std::make_unique<BookmarksAppActivity>(renderer, mappedInput);
  } else if (selectedIndex == 6) {
    activity = std::make_unique<SyncDayActivity>(renderer, mappedInput);
  } else if (selectedIndex == 7) {
    activityManager.goToFileTransfer();
    return;
  } else {
    activity = std::make_unique<SleepAppActivity>(renderer, mappedInput);
  }

  startActivityForResult(std::move(activity), [this](const ActivityResult&) { requestUpdate(); });
}
