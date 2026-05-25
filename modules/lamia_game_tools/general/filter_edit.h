#pragma once

#include "core/object/object.h"
#include "scene/gui/line_edit.h"

class FilterEdit : public LineEdit
{
    GDCLASS(FilterEdit, LineEdit)

protected:
    static void _bind_methods() {};

public:
    FilterEdit();
};

