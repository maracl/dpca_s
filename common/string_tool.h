#ifndef _STRING_TOOL_H
#define _STRING_TOOL_H

#include <iostream>
#include <stdio.h>
#include <string.h>

#include <stdlib.h> 
#include <vector>

using namespace std;

void splits(std::string& s, std::string& delim, std::vector< std::string >* ret);

char** ch_search(string data, const char *pattern, int* size);

int amend_rect(string &str, double v);

#endif