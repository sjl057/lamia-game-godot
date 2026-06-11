#pragma once

#include "core/object/object.h"
#include "core/variant/binder_common.h"

#define BIND(m_method_name, m_method) ClassDB::bind_method(m_method_name, m_method);
#define BIND_STA( m_method_name, m_method) ClassDB::bind_static_method(get_class_static(), m_method_name, m_method);
#define BIND_D(m_method_name, m_method, ...) ClassDB::bind_method(m_method_name, m_method, __VA_ARGS__);
#define BIND_STA_D(m_method_name, m_method, ...) ClassDB::bind_static_method(get_class_static(), m_method_name, m_method, __VA_ARGS__);

// #define CONNECT_SIGNAL(m_inst, m_signal, m_to, m_method, m_flags) auto callable = callable_mp(m_to, m_method); m_inst->connect(SNAME(m_signal), callable); return callable;
// #define CONNECT_SIGNAL_F(m_inst, m_signal, m_to, m_method, m_flags) m_inst->connect(SNAME(m_signal), callable_mp(m_to, m_method), m_flags)

enum LGTPropertyHint
{
    LGT_PROPERTY_HINT_DATABASE_ID_SELECT = PROPERTY_HINT_MAX + 9000,
    LGT_PROPERTY_HINT_MAX
};

enum EVResult
{
    EV_INVALID,
    EV_RUNNING,
    EV_NEXT_IN_TREE,
    EV_NEXT_SIBLING,
    EV_ABORT
};

VARIANT_ENUM_CAST(LGTPropertyHint);
VARIANT_ENUM_CAST(EVResult)