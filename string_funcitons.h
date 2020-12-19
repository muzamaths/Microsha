#ifndef MICROSHA_STRING_FUNCITONS_H
#define MICROSHA_STRING_FUNCITONS_H

#include <string>
#include <vector>
#include "error_functions.h"

/* Splits string(text) into substrings(words) divided by token */
void split_string_by_token(const std::string &text, char token, std::vector<std::string> &words);

#endif //MICROSHA_STRING_FUNCITONS_H
