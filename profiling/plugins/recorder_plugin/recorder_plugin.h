#ifndef DATADOG_PHP_RECORDER_PLUGIN_H
#define DATADOG_PHP_RECORDER_PLUGIN_H

#include "datadog-profiling.h"

#include <ddprof/ffi.h>
#include <ddtrace_attributes.h>
#include <stack-collector/stack-collector.h>

/* The recorder has two high level responsibilities:
 *  1. Aggregate samples.
 *  2. Export profiles once the configured period has elapsed.
 */

typedef struct datadog_php_record_values {
  uint64_t count;    // usually 0 or 1
  int64_t wall_time; // wall time in ns since last sample, may be 0
  int64_t cpu_time;  // cpu time in ns since last sample, may be 0
} datadog_php_record_values;

DDTRACE_HOT __attribute__((nonnull)) bool
datadog_php_recorder_plugin_record(datadog_php_record_values record_values,
                                   int64_t tid,
                                   const datadog_php_stack_sample *sample);

void datadog_php_recorder_plugin_first_activate(bool profiling_enabled);
DDTRACE_COLD void
datadog_php_recorder_plugin_shutdown(zend_extension *extension);
bool datadog_php_recorder_plugin_is_enabled(void);
bool datadog_php_recorder_plugin_cpu_time_is_enabled(void);

DDTRACE_COLD void datadog_php_recorder_plugin_diagnose(void);

#endif // DATADOG_PHP_RECORDER_PLUGIN_H
