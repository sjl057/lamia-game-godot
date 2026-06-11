#pragma once

#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/templates/a_hash_map.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

class EventerWhiteboard : public RefCounted
{
    GDCLASS(EventerWhiteboard, RefCounted);

private:
    AHashMap<StringName, Variant> variables;

protected:
    static void _bind_methods();

public:
    void assign_from_dictionary(const Dictionary &p_dictionary);
    void set_variable(const StringName &p_name, const Variant &p_value);
    bool has_variable(const StringName &p_name) const;
    Variant get_variable(const StringName &p_name, const Variant &p_default = Variant()) const;
    void erase_variable(const StringName &p_name);
    Dictionary get_variables() const;
};