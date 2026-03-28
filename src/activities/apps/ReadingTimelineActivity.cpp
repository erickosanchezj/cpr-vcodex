#include "ReadingTimelineActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "ReadingDayDetailActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "util/HeaderDateUtils.h"

namespace {
std::string getTopBookSubtitle(const ReadingStatsAnalytics::TimelineDayEntry& entry) {
  const std::string booksReadLabel = std::to_string(entry.booksReadCount) + " " + tr(STR_BOOKS_READ);
  if (entry.topBook == nullptr) {
    return booksReadLabel;
  }

  if (!entry.topBook->author.empty()) {
    return entry.topBook->author + " | " + booksReadLabel;
  }

  const std::string title = entry.topBook->title.empty() ? entry.topBook->path : entry.topBook->title;
  return title + " | " + booksReadLabel;
}

std::string getTimelineTitle(const ReadingStatsAnalytics::TimelineDayEntry& entry) {
  return ReadingStatsAnalytics::formatDayOrdinalLabel(entry.dayOrdinal);
}
}  // namespace

void ReadingTimelineActivity::refreshEntries() {
  entries = ReadingStatsAnalytics::buildTimelineEntries();
  if (selectedIndex >= static_cast<int>(entries.size())) {
    selectedIndex = std::max(0, static_cast<int>(entries.size()) - 1);
  }
}

void ReadingTimelineActivity::openSelectedDay() {
  if (selectedIndex < 0 || selectedIndex >= static_cast<int>(entries.size())) {
    return;
  }

  startActivityForResult(std::make_unique<ReadingDayDetailActivity>(renderer, mappedInput, entries[selectedIndex].dayOrdinal),
                         [this](const ActivityResult&) {
                           refreshEntries();
                           requestUpdate();
                         });
}

void ReadingTimelineActivity::onEnter() {
  Activity::onEnter();
  refreshEntries();
  waitForConfirmRelease = mappedInput.isPressed(MappedInputManager::Button::Confirm);
  requestUpdate();
}

void ReadingTimelineActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }

  if (waitForConfirmRelease) {
    if (!mappedInput.isPressed(MappedInputManager::Button::Confirm)) {
      waitForConfirmRelease = false;
    }
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    openSelectedDay();
    return;
  }

  buttonNavigator.onNextRelease([this] {
    if (entries.empty()) {
      return;
    }
    selectedIndex = ButtonNavigator::nextIndex(selectedIndex, static_cast<int>(entries.size()));
    requestUpdate();
  });

  buttonNavigator.onPreviousRelease([this] {
    if (entries.empty()) {
      return;
    }
    selectedIndex = ButtonNavigator::previousIndex(selectedIndex, static_cast<int>(entries.size()));
    requestUpdate();
  });
}

void ReadingTimelineActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto& metrics = UITheme::getInstance().getMetrics();
  const int pageWidth = renderer.getScreenWidth();
  const int pageHeight = renderer.getScreenHeight();
  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int listHeight = pageHeight - contentTop - metrics.buttonHintsHeight - metrics.verticalSpacing;

  HeaderDateUtils::drawHeaderWithDate(renderer, tr(STR_READING_TIMELINE));

  if (entries.empty()) {
    renderer.drawText(UI_10_FONT_ID, metrics.contentSidePadding, contentTop + 20, tr(STR_NO_READING_STATS));
  } else {
    GUI.drawList(renderer, Rect{0, contentTop, pageWidth, listHeight}, static_cast<int>(entries.size()), selectedIndex,
                 [this](const int index) { return getTimelineTitle(entries[index]); },
                 [this](const int index) { return getTopBookSubtitle(entries[index]); },
                 [](const int) { return UIIcon::Recent; },
                 [this](const int index) { return ReadingStatsAnalytics::formatDurationHm(entries[index].totalReadingMs); });
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), entries.empty() ? "" : tr(STR_OPEN), tr(STR_DIR_UP),
                                            tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
