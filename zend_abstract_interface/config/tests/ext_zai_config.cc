extern "C" {
#include "ext_zai_config.h"
#include "config/config.h"
#include "tea/extension.h"
}

#include <atomic>

static ext_zai_config_minit_fn ext_orig_minit;
void (*ext_zai_config_pre_rinit)();

static PHP_MINIT_FUNCTION(zai_config) {
    return ext_orig_minit(INIT_FUNC_ARGS_PASSTHRU);
}

static PHP_MSHUTDOWN_FUNCTION(zai_config) {
#if PHP_VERSION_ID >= 80000
    zend_result result = SUCCESS;
#else
    int result = SUCCESS;
#endif

    zend_try {
        zai_config_mshutdown();
        UNREGISTER_INI_ENTRIES();
    } zend_catch {
        result = FAILURE;
    } zend_end_try();

    return result;
}

static PHP_RINIT_FUNCTION(zai_config) {
#if PHP_VERSION_ID >= 80000
    zend_result result = SUCCESS;
#else
    int result = SUCCESS;
#endif

    zend_try {
        if (ext_zai_config_pre_rinit) {
            ext_zai_config_pre_rinit();
        }

        zai_config_activate();
    } zend_catch {
        result = FAILURE;
    } zend_end_try();

    return result;
}

static PHP_RSHUTDOWN_FUNCTION(zai_config) {
#if PHP_VERSION_ID >= 80000
    zend_result result = SUCCESS;
#else
    int result = SUCCESS;
#endif

    zend_try {
        zai_config_deactivate();
    } zend_catch {
        result = FAILURE;
    } zend_end_try();

    return result;
}

void ext_zai_config_ctor(ext_zai_config_minit_fn orig_minit) {
    ext_zai_config_pre_rinit = NULL;
    ext_orig_minit = orig_minit;

    tea_extension_minit(PHP_MINIT(zai_config));
    tea_extension_rinit(PHP_RINIT(zai_config));
    tea_extension_rshutdown(PHP_RSHUTDOWN(zai_config));
    tea_extension_mshutdown(PHP_MSHUTDOWN(zai_config));
}
