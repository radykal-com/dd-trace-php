#include <tea/testing/catch2.hpp>

extern "C" {
#include <hook/hook.h>
#include <hook/table.h>
#include <value/value.h>
#include <tea/extension.h>
#if PHP_VERSION_ID >= 70000
#include <interceptor/php7/interceptor.h>
#else
#include <interceptor/php5/interceptor.h>
#endif

    static PHP_MINIT_FUNCTION(ddtrace_testing_hook) {
        zai_hook_minit();
        return SUCCESS;
    }

    static PHP_RINIT_FUNCTION(ddtrace_testing_hook) {
        zai_hook_rinit();
        zai_hook_activate();
        // test ZEND_DECLARE_*_DELAYED opcodes for opcache
        CG(compiler_options) |= ZEND_COMPILE_DELAYED_BINDING;
        zai_interceptor_rinit(ZAI_TSRMLS_C);
        return SUCCESS;
    }

    static PHP_RSHUTDOWN_FUNCTION(ddtrace_testing_hook) {
        zai_interceptor_rshutdown();
        zai_hook_rshutdown();
        return SUCCESS;
    }

    static PHP_MSHUTDOWN_FUNCTION(ddtrace_testing_hook) {
        zai_hook_mshutdown();
        return SUCCESS;
    }

    static int ddtrace_testing_startup() {
        zai_interceptor_startup(tea_extension_dummy());
        return SUCCESS;
    }

    static void init_interceptor_test() {
        tea_extension_op_array_ctor(zai_interceptor_op_array_ctor);
#if PHP_VERSION_ID >= 70000
        tea_extension_op_array_handler(zai_interceptor_op_array_pass_two);
#endif
        tea_extension_startup(ddtrace_testing_startup);
        tea_extension_minit(PHP_MINIT(ddtrace_testing_hook));
        tea_extension_rinit(PHP_RINIT(ddtrace_testing_hook));
        tea_extension_mshutdown(PHP_MSHUTDOWN(ddtrace_testing_hook));
        tea_extension_rshutdown(PHP_RSHUTDOWN(ddtrace_testing_hook));
    }
}

static bool zai_hook_test_begin(zend_ulong invocation, zend_execute_data *ex, void *fixed, void *dynamic TEA_TSRMLS_DC) {
    REQUIRE_FALSE(1);
    return true;
}

static void zai_hook_test_end(zend_ulong invocation, zend_execute_data *ex, zval *rv, void *fixed, void *dynamic TEA_TSRMLS_DC) {
    REQUIRE_FALSE(1);
}

#define INTERCEPTOR_TEST_CASE(description, ...) \
    TEA_TEST_CASE_WITH_STUB_WITH_PROLOGUE(     \
        "interceptor", description,     \
        "./stubs/unresolved.php",                                  \
        init_interceptor_test();,                    \
        __VA_ARGS__)

#define INSTALL_HOOK(fn) INSTALL_CLASS_HOOK("", fn)
#define INSTALL_CLASS_HOOK(class, fn) REQUIRE(zai_hook_install( \
    ZAI_STRL_VIEW(class), \
    ZAI_STRL_VIEW(fn), \
    zai_hook_test_begin, \
    zai_hook_test_end, \
    ZAI_HOOK_AUX(NULL, NULL), \
    0 TEA_TSRMLS_CC) != -1)
#define CALL_FN(fn, ...) do { \
    zval *result; \
    ZAI_VALUE_INIT(result); \
    zai_string_view _fn_name = ZAI_STRL_VIEW(fn);               \
    REQUIRE(zai_symbol_call(ZAI_SYMBOL_SCOPE_GLOBAL, NULL, ZAI_SYMBOL_FUNCTION_NAMED, &_fn_name, &result TEA_TSRMLS_CC, 0)); \
    __VA_ARGS__               \
    ZAI_VALUE_DTOR(result);                          \
} while (0)

TEA_TEST_CASE_WITH_PROLOGUE("interceptor", "runtime top-level resolving", init_interceptor_test();, {
    INSTALL_HOOK("doAlias");
    INSTALL_CLASS_HOOK("TopLevel", "foo");

    volatile bool stub = true;
    zend_try {
        stub = tea_execute_script("./stubs/unresolved.php" TEA_TSRMLS_CC);
    } zend_catch {
        stub = false;
    } zend_end_try();
    REQUIRE(stub);

    {
        zend_function *fn = zai_symbol_lookup_function_literal(ZEND_STRL("doAlias") ZAI_TSRMLS_CC);
        CHECK(zai_hook_installed_func(fn));
    }
    {
        zend_class_entry *ce = zai_symbol_lookup_class_literal(ZEND_STRL("TopLevel") ZAI_TSRMLS_CC);
        zai_string_view name = ZAI_STRL_VIEW("foo");
        zend_function *classfn = zai_symbol_lookup_function(ZAI_SYMBOL_SCOPE_CLASS, ce, &name ZAI_TSRMLS_CC);
        CHECK(zai_hook_installed_func(classfn));
    }
});

INTERCEPTOR_TEST_CASE("runtime eval resolving", {
    INSTALL_HOOK("dynamicFunction");
    CALL_FN("doEval");
    zend_function *fn = zai_symbol_lookup_function_literal(ZEND_STRL("dynamicFunction") ZAI_TSRMLS_CC);
    REQUIRE(zai_hook_installed_func(fn));
});

INTERCEPTOR_TEST_CASE("runtime function resolving", {
    INSTALL_HOOK("aFunction");
    CALL_FN("defineFunc");
    zend_function *fn = zai_symbol_lookup_function_literal(ZEND_STRL("aFunction") ZAI_TSRMLS_CC);
    REQUIRE(zai_hook_installed_func(fn));
});

INTERCEPTOR_TEST_CASE("runtime simple class resolving", {
    INSTALL_CLASS_HOOK("Normal", "foo");
    CALL_FN("defineNormal");
    zend_class_entry *ce = zai_symbol_lookup_class_literal(ZEND_STRL("Normal") ZAI_TSRMLS_CC);
    zai_string_view name = ZAI_STRL_VIEW("foo");
    zend_function *fn = zai_symbol_lookup_function(ZAI_SYMBOL_SCOPE_CLASS, ce, &name ZAI_TSRMLS_CC);
    REQUIRE(zai_hook_installed_func(fn));
});

INTERCEPTOR_TEST_CASE("runtime inherited class resolving", {
    INSTALL_CLASS_HOOK("Inherited", "bar");
    CALL_FN("defineInherited");
    zend_class_entry *ce = zai_symbol_lookup_class_literal(ZEND_STRL("Inherited") ZAI_TSRMLS_CC);
    zai_string_view name = ZAI_STRL_VIEW("bar");
    zend_function *fn = zai_symbol_lookup_function(ZAI_SYMBOL_SCOPE_CLASS, ce, &name ZAI_TSRMLS_CC);
    REQUIRE(zai_hook_installed_func(fn));
});

INTERCEPTOR_TEST_CASE("runtime inherited delayed class resolving", {
    INSTALL_CLASS_HOOK("Inherited", "bar");
    CALL_FN("defineNormal");
    CALL_FN("defineDelayedInherited");
    zend_class_entry *ce = zai_symbol_lookup_class_literal(ZEND_STRL("Inherited") ZAI_TSRMLS_CC);
    zai_string_view name = ZAI_STRL_VIEW("bar");
    zend_function *fn = zai_symbol_lookup_function(ZAI_SYMBOL_SCOPE_CLASS, ce, &name ZAI_TSRMLS_CC);
    REQUIRE(zai_hook_installed_func(fn));
});

INTERCEPTOR_TEST_CASE("runtime trait using class resolving", {
    INSTALL_CLASS_HOOK("TraitImport", "bar");
    CALL_FN("defineTraitUser");
    zend_class_entry *ce = zai_symbol_lookup_class_literal(ZEND_STRL("TraitImport") ZAI_TSRMLS_CC);
    zai_string_view name = ZAI_STRL_VIEW("bar");
    zend_function *fn = zai_symbol_lookup_function(ZAI_SYMBOL_SCOPE_CLASS, ce, &name ZAI_TSRMLS_CC);
    REQUIRE(zai_hook_installed_func(fn));
});

INTERCEPTOR_TEST_CASE("runtime class_alias resolving", {
    INSTALL_CLASS_HOOK("Aliased", "foo");
    CALL_FN("doAlias");
    zend_class_entry *ce = zai_symbol_lookup_class_literal(ZEND_STRL("Aliased") ZAI_TSRMLS_CC);
    zai_string_view name = ZAI_STRL_VIEW("foo");
    zend_function *fn = zai_symbol_lookup_function(ZAI_SYMBOL_SCOPE_CLASS, ce, &name ZAI_TSRMLS_CC);
    REQUIRE(zai_hook_installed_func(fn));
});

#if PHP_VERSION_ID >= 70000  // not a scenario which can happen on PHP 5
INTERCEPTOR_TEST_CASE("ensure runtime post-declare resolving does not impact error", {
    INSTALL_CLASS_HOOK("Inherited", "bar");
    CALL_FN("failDeclare", REQUIRE(zval_is_true(result)););
    REQUIRE(zend_hash_num_elements(&zai_hook_resolved) == 0);
});
#endif
