#pragma once

#include "core/object/object.h"
#include "core/object/script_language.h"
#include "core/variant/binder_common.h"
#include "core/variant/variant.h"

class LGTUtility : public Object
{
    GDCLASS(LGTUtility, Object);

public:
    enum Check
    {
        CHECK_NONE,
        CHECK_EQUAL,
        CHECK_NOT_EQUAL,
        CHECK_GREATER_THAN,
        CHECK_GREATER_THAN_OR_EQUAL,
        CHECK_LESS_THAN,
        CHECK_LESS_THAN_OR_EQUAL,
        CHECK_MAX
    };

    enum Operation
    {
        OPERATION_NONE,
        OPERATION_ADD,
        OPERATION_SUBTRACT,
        OPERATION_MULTIPLY,
        OPERATION_DIVIDE,
        OPERATION_POWER,
        OPERATION_MODULE,
        OPERATION_BIT_SHIFT_LEFT,
        OPERATION_BIT_SHIFT_RIGHT,
        OPERATION_BIT_AND,
        OPERATION_BIT_OR,
        OPERATION_BIT_XOR,
        OPERATION_IN,
        OPERATION_MATH_MIN,
        OPERATION_MATH_MAX,
        OPERATION_MAX
    };

protected:
    static LGTUtility *singleton;
    static void _bind_methods();

public:
    _FORCE_INLINE_ static LGTUtility *get_singleton()
    {
        return singleton;
    }

    void collect_scripts_from_dir(const String &p_path, AHashMap<StringName, Ref<Script>> *r_ret);

    bool perform_check(const Check &p_check, const Variant &p_a, const Variant &p_b);
    Variant perform_operation(const Operation &p_operation, const Variant &p_a, const Variant &p_b) const;

    LGTUtility();
    ~LGTUtility();
};

VARIANT_ENUM_CAST(LGTUtility::Check);
VARIANT_ENUM_CAST(LGTUtility::Operation);