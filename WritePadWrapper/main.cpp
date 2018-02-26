/************************************************************************
* Copyright (C) 2018  Toni Stachewicz, Niklas Hoffmann
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
************************************************************************/

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string.h>

#include "include/json/json.h"

extern "C" {

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#ifdef TRUE
#undef TRUE
#endif
#define TRUE  1


#include "include/RecognizerWrapper.h"

RECOGNIZER_PTR _recognizer;

enum LANGUAGE {
    GERMAN, ENGLISCH
};

LANGUAGE determineLanguageToUse(const Json::Value &language) {
  LANGUAGE languageToUse;
  if (language.asString() == "de")
    languageToUse = GERMAN;
  else
    languageToUse = ENGLISCH;
  return languageToUse;
}

BOOL enableRecognizer(LANGUAGE language) {
  const char *mainDictEN = "./English.dct";
  const char *mainDictDE = "./German.dct";

  std::cout << "Initialize recognizer..." << std::endl;
  int flags = 0;

  switch (language) {
    case GERMAN:
      _recognizer = HWR_InitRecognizer(mainDictDE, NULL, NULL, NULL, LANGUAGE_GERMAN, &flags);
      break;
    case ENGLISCH:
    default:
      _recognizer = HWR_InitRecognizer(mainDictEN, NULL, NULL, NULL, LANGUAGE_ENGLISH, &flags);
  }

  if (NULL != _recognizer) {
    unsigned int update_flags = HWR_GetRecognitionFlags(_recognizer);
    if ((flags & FLAG_CORRECTOR) == 0) printf("Warning: autocorrector did not initialize.\n");
    if ((flags & FLAG_ANALYZER) == 0) printf("Warning: statistical analyzer did not initialize.\n");
    if ((flags & FLAG_USERDICT) == 0) printf("Warning: user dictionary did not initialize.\n");
    if ((flags & FLAG_MAINDICT) == 0) printf("Warning: main dictionary did not initialize.\n");

    // here, detecting multiple words at a time can be enabled - however, it does not seem to work yet
    //    update_flags |= FLAG_SINGLEWORDONLY;
    update_flags &= ~FLAG_SINGLEWORDONLY;

    HWR_SetRecognitionFlags(_recognizer, update_flags);
    printf("%s recognizer is enabled.\n", HWR_GetLanguageName(_recognizer));
  }
  return (NULL != _recognizer) ? TRUE : FALSE;
}

static const CGTracePoint createTracePoint(double x, double y, int pressure) {
  const CGPoint t1 = {static_cast<CGFloat>(x), static_cast<CGFloat>(y)};
  return {t1, pressure};
}

Json::Value parseJson(const char *jsonString) {
  std::string converted(jsonString);

  Json::Value jsonInput;
  std::istringstream iss(converted, std::istringstream::in);
  iss >> jsonInput;
  return jsonInput;
}

void extractPointsFromJson(const Json::Value &points, CGTracePoint *tracePoints) {
  for (int i = 0; i < points.size(); ++i) {
    Json::Value point = points[i];
    tracePoints[i] = createTracePoint(point["x"].asUInt64(), point["y"].asUInt64(), 150);
  }
}

Json::Value createJsonFromResults(const std::vector<wchar_t *> &result) {
  Json::Value resultList;
  for (int i = 0; i < result.size(); ++i) {
    std::wstring ws(result[i]);
    std::string str(ws.begin(), ws.end());
    resultList["suggestions"][i] = Json::Value(str);
  }
  return resultList;
}

std::wstring convertJsonToWString(const Json::Value &resultList) {
  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::string out = writeString(builder, resultList);
  std::wstring converted_result(out.begin(), out.end());
  return converted_result;
}

void checkValidResult(const UCHR *pText) {
  if (pText == NULL || *pText == 0) {
    throw std::runtime_error("Got no result!");
  }
  std::string result = std::to_string(*pText);
  if (strcmp(result.c_str(), kEmptyWord) == 0) {
    throw std::runtime_error("Got no result!");
  }
}

const wchar_t *recognizeInk(LANGUAGE languageToUse, CGTracePoint tracePoints[], size_t length) {
  enableRecognizer(languageToUse);

  CGStroke aStrokes = tracePoints;
  const UCHR *pText = NULL;
  HWR_RecognizerAddStroke(_recognizer, aStrokes, length);

  if (HWR_Recognize(_recognizer)) {
    pText = HWR_GetResult(_recognizer);
    checkValidResult(pText);

    int pTextLength = 0;
    for (; pTextLength < std::numeric_limits<int>::max() && pText[pTextLength] != 0; pTextLength++);

    std::wstring fullWText;
    for (int i = 0; i < pTextLength; ++i) {
      fullWText += wchar_t(pText[i]);
    }
    std::wcout << "Result: " << fullWText << std::endl;

    wchar_t *real_result = new wchar_t[fullWText.length() + 1];
    wcscpy(real_result, fullWText.c_str());
    return real_result;
  }
  return nullptr;
}

const std::vector<wchar_t *> recognizeInkSuggestionList(LANGUAGE languageToUse, CGTracePoint tracePoints[], size_t length) {
  enableRecognizer(languageToUse);

  CGStroke aStrokes = tracePoints;
  HWR_RecognizerAddStroke(_recognizer, aStrokes, length);

  if (HWR_Recognize(_recognizer)) {
    int wordCount = HWR_GetResultWordCount(_recognizer);
    std::cout << "Word Count: " << wordCount << std::endl;
    std::vector<wchar_t *> suggestions;

    for (int i = 0; i < wordCount; i++) {
      int alternativesCount = HWR_GetResultAlternativeCount(_recognizer, i);
      std::cout << "Alternatives count: " << alternativesCount << std::endl;
      for (int j = 0; j < alternativesCount; j++) {
        const UCHR *chrWord = HWR_GetResultWord(_recognizer, i, j);
        int pTextLength = 0;
        for (; pTextLength < std::numeric_limits<int>::max() && chrWord[pTextLength] != 0; pTextLength++);

        std::wstring alternative;
        for (int k = 0; k < pTextLength; ++k) {
          alternative += wchar_t(chrWord[k]);
        }
        std::wcout << "Alternative: " << alternative << std::endl;

        wchar_t *real_result = new wchar_t[alternative.length() + 1];
        wcscpy(real_result, alternative.c_str());
        suggestions.push_back(real_result);
      }
    }
    return suggestions;
  }
  return std::vector<wchar_t *>();
}

const wchar_t *recognizeSingleSuggestion(const char *jsonString) {
  try {
    Json::Value jsonInput = parseJson(jsonString);
    Json::Value points = jsonInput["points"];
    Json::Value language = jsonInput["language"];
    LANGUAGE languageToUse = determineLanguageToUse(language);
    CGTracePoint tracePoints[points.size()];
    extractPointsFromJson(points, tracePoints);

    return recognizeInk(languageToUse, tracePoints, sizeof(tracePoints) / sizeof(tracePoints[0]));
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    std::wstring error = L"Error";
    return error.c_str();
  }
}

const wchar_t *recognizeMultipleSuggestions(const char *jsonString) {
  try {
    Json::Value jsonInput = parseJson(jsonString);
    Json::Value points = jsonInput["points"];
    Json::Value language = jsonInput["language"];
    LANGUAGE languageToUse = determineLanguageToUse(language);
    CGTracePoint tracePoints[points.size()];
    extractPointsFromJson(points, tracePoints);

    std::vector<wchar_t *> result = recognizeInkSuggestionList(languageToUse, tracePoints, sizeof(tracePoints) / sizeof(tracePoints[0]));

    Json::Value resultList = createJsonFromResults(result);
    std::wstring converted_result = convertJsonToWString(resultList);

    // Putting the result to heap is required to access from python server
    wchar_t *real_result = new wchar_t[converted_result.length() + 1];
    wcscpy(real_result, converted_result.c_str());
    std::wcout << "Return result: " << real_result << std::endl;
    return real_result;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    std::wstring error = L"Error";
    return error.c_str();
  }
}
}
