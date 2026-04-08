#include "banner.h"
#include <iostream>
#include <string>
#include <vector>

void handleBanner() {
  // clang-format off
    std::string bannerText{"  :::.   :::.    :::.  .,-:::::   ::   .:      ...    :::::::..   \n  ;;`;;  `;;;;,  `;;;,;;;'````'  ,;;   ;;,  .;;;;;;;. ;;;;``;;;; \n ,[[ '[[,  [[[[[. '[[[[[        ,[[[,,,[[[ ,[[     \[[,[[[,/[[['  \nc$$$cc$$$c $$$ \"Y$c$$$$$        \"$$$\"\"\"$$$ $$$,     $$$$$$$$$c    \n 888   888,888    Y88`88bo,__,o, 888   \"88o\"888,_ _,88P888b \"88bo,\n YMM   \"\"` MMM     YM  \"YUMMMMMP\"MMM    YMM  \"YMMMMMP\" MMMM   \"W\" "};
  // clang-format on

  std::string bannerEnhanced{};

  std::vector<std::string> charColors = {
      "\033[38;5;213m", // bright pink-magenta (primary)
      "\033[38;5;205m", // vivid magenta accent
      "\033[38;5;177m", // soft lavender-magenta (highlight)
      "\033[38;5;141m", // light purple (contrast)n
      "\033[38;5;205m"};

  std::string colorReset{"\033[0m"};

  size_t pos{0};

  while (charColors.size()) {
    for (auto text : bannerText) {
      if (pos == charColors.size()) {
        pos = 0;
      }
      bannerEnhanced += charColors[pos] + text + colorReset;
      ++pos;
    }
    break;
  }
  std::cout << bannerEnhanced << "\n\n";
}
