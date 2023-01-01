/**
 * @file test.cpp
 * @author Shane Menzies
 * @brief
 * @date 12/30/22
 *
 *
 */

#include "test.h"

#include "terminal/terminal.h"

void test() { active_terminal->write_s("Successful call from user-land\n"); }
