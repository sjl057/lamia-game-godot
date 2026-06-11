#pragma once

#include "core/object/object.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "scene/gui/tree.h"

class FilterTree : public Tree
{
    GDCLASS(FilterTree, Tree)

private:
    bool _update_filter(String p_filter, TreeItem *p_parent = nullptr);

protected:
    static void _bind_methods();

public:
    bool item_matches_terms(TreeItem *p_item, PackedStringArray terms);
    void set_filter(String p_filter);

};