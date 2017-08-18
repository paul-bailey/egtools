/* egdebug.h - handy macros for breakpoints, etc., in code */
#ifndef EGDEBUG_H
#define EGDEBUG_H

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define MARK() \
  fprintf(stderr, "\e[33m%s %d (%s)\e[0m\n", __FUNCTION__, __LINE__, errno ? strerror(errno) : "")
#define FUNCPRINTF(fmt, args...) \
  fprintf(stderr, "\e[33m%s: " fmt "\e[0m", __FUNCTION__, ## args)

#endif /* EGDEBUG_H */
