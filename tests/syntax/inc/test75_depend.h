// A header test75_depend includes twice behind a guard.

#ifndef TEST75_DEPEND_H
#define TEST75_DEPEND_H

#include "test75_depend_nested.h"
#include "test75_depend_nested.h"

#define TEST75_HEADER 60

#endif
