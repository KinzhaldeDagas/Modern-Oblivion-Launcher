#pragma once

struct IniSection {
    const wchar_t* sectionName;
    const wchar_t** options;
    int optionCount;
};

struct CommonOption {
    const wchar_t* commonKey;
    const wchar_t* realSection;
    const wchar_t* realKey;
};

extern const CommonOption commonOptions[];
extern const int numCommonOptions;

extern const IniSection iniSections[];
extern const int numIniSections;
