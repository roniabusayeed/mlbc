#pragma once
#include <string>

#include "util.h"


const std::string   APPLICATION_RESOURCE_DIR        = joinPaths(std::getenv("HOME"), ".mlbc/res");

const std::string   APPLICATION_THEME_DIR           = joinPaths(APPLICATION_RESOURCE_DIR, "themes");
const std::string   APPLICATION_UI_FONT_DIR         = joinPaths(APPLICATION_RESOURCE_DIR, "fonts");

const std::string   APPLICATION_THEME_FILEPATH      = joinPaths(APPLICATION_THEME_DIR, "dark-theme.json");

const std::string   APPLICATION_UI_FONT_FILEPATH    = joinPaths(APPLICATION_UI_FONT_DIR, "Roboto-Medium.ttf");
const float         APPLICATION_UI_FONT_SIZE        = 16.0f;
