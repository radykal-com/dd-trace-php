#ifndef ZAI_INTERCEPTOR_H
#define ZAI_INTERCEPTOR_H

#include <Zend/zend_compile.h>

void zai_interceptor_op_array_ctor(zend_op_array *op_array);

void zai_interceptor_startup(zend_module_entry *module_entry);
void zai_interceptor_rinit(TSRMLS_D);
void zai_interceptor_rshutdown(void);

#endif  // ZAI_INTERCEPTOR_H