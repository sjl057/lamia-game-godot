#pragma once

#include "core/object/object.h"
#include "core/variant/binder_common.h"

enum LGUPropertyHint
{
    LGU_PROPERTY_HINT_DATABASE_ID_SELECT = PROPERTY_HINT_MAX + 9000,
    LGU_PROPERTY_HINT_DATABASE_SELECT,
    LGU_PROPERTY_HINT_MAX
};

VARIANT_ENUM_CAST(LGUPropertyHint);