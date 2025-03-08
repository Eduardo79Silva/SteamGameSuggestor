#include "utils.h"
#include <cctype>
#include <codecvt>
#include <cstdio>
#include <locale>
#include <string>

std::string unixTimeToHumanReadable(long int seconds) {

  // Save the time in Human
  // readable format
  std::string ans = "";

  // Number of days in month
  // in normal year
  int daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  long int currYear, daysTillNow, extraTime, extraDays, index, date, month,
      hours, minutes, secondss, flag = 0;

  // Calculate total days unix time T
  daysTillNow = seconds / (24 * 60 * 60);
  extraTime = seconds % (24 * 60 * 60);
  currYear = 1970;

  // Calculating current year
  while (true) {
    if (currYear % 400 == 0 || (currYear % 4 == 0 && currYear % 100 != 0)) {
      if (daysTillNow < 366) {
        break;
      }
      daysTillNow -= 366;
    } else {
      if (daysTillNow < 365) {
        break;
      }
      daysTillNow -= 365;
    }
    currYear += 1;
  }
  // Updating extradays because it
  // will give days till previous day
  // and we have include current day
  extraDays = daysTillNow + 1;

  if (currYear % 400 == 0 || (currYear % 4 == 0 && currYear % 100 != 0))
    flag = 1;

  // Calculating MONTH and DATE
  month = 0, index = 0;
  if (flag == 1) {
    while (true) {

      if (index == 1) {
        if (extraDays - 29 < 0)
          break;
        month += 1;
        extraDays -= 29;
      } else {
        if (extraDays - daysOfMonth[index] < 0) {
          break;
        }
        month += 1;
        extraDays -= daysOfMonth[index];
      }
      index += 1;
    }
  } else {
    while (true) {

      if (extraDays - daysOfMonth[index] < 0) {
        break;
      }
      month += 1;
      extraDays -= daysOfMonth[index];
      index += 1;
    }
  }

  // Current Month
  if (extraDays > 0) {
    month += 1;
    date = extraDays;
  } else {
    if (month == 2 && flag == 1)
      date = 29;
    else {
      date = daysOfMonth[month - 1];
    }
  }

  // Calculating HH:MM:YYYY
  hours = extraTime / 3600;
  minutes = (extraTime % 3600) / 60;
  secondss = (extraTime % 3600) % 60;

  ans += std::to_string(date);
  ans += "/";
  ans += std::to_string(month);
  ans += "/";
  ans += std::to_string(currYear);
  ans += " ";
  ans += std::to_string(hours);
  ans += ":";
  ans += std::to_string(minutes);
  ans += ":";
  ans += std::to_string(secondss);

  // Return the time
  return ans;
}

// Function to convert UTF-8 string to wstring
std::wstring utf8ToWstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

// Function to convert wstring to UTF-8 string
std::string wstringToUtf8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// Function to remove specific characters from a UTF-8 string
std::string removeSpecialCharacters(const std::string& input) {
    std::wstring winput = utf8ToWstring(input);
    std::wstring wresult;
    for (wchar_t wc : winput) {
        if (wc != L'™') { // Add any other wide characters you want to exclude
            wresult += wc;
        }
    }
    return wstringToUtf8(wresult);
}
