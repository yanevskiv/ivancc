// A header test075_depend includes twice behind a guard.

#ifndef TEST075_DEPEND_H
#define TEST075_DEPEND_H

#include "test075_depend_nested.h"
#include "test075_depend_nested.h"

#define TEST075_HEADER 60

#endif
