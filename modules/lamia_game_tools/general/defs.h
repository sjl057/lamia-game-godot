#pragma once

#include "core/object/object.h"
#include "core/variant/binder_common.h"

enum LGTPropertyHint
{
    LGT_PROPERTY_HINT_DATABASE_ID_SELECT = PROPERTY_HINT_MAX + 9000,
    LGT_PROPERTY_HINT_DATABASE_SELECT,
    LGT_PROPERTY_HINT_MAX
};

VARIANT_ENUM_CAST(LGTPropertyHint);