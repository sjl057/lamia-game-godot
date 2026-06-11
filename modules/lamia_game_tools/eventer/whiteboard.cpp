#include "whiteboard.h"
#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/string/string_name.h"
#include "core/templates/pair.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/general/defs.h"

void EventerWhiteboard::_bind_methods()
{
    BIND(D_METHOD("assign_from_dictionary", "dictionary"), &EventerWhiteboard::assign_from_dictionary);
    BIND(D_METHOD("set_variable", "name", "value"), &EventerWhiteboard::set_variable);
    BIND(D_METHOD("has_variable", "name"), &EventerWhiteboard::has_variable);
    BIND(D_METHOD("get_variable", "name", "default"), &EventerWhiteboard::get_variable);
    BIND(D_METHOD("erase_variable", "name"), &EventerWhiteboard::erase_variable);
    BIND(D_METHOD("get_variables"), &EventerWhiteboard::get_variables);
}

void EventerWhiteboard::assign_from_dictionary(const Dictionary &p_dictionary)
{
    ERR_FAIL_COND(not (p_dictionary.get_typed_key_builtin() == Variant::STRING_NAME and p_dictionary.get_typed_value_builtin() == Variant::VARIANT_MAX));
    variables.clear();
    for (const KeyValue<Variant, Variant> &E : p_dictionary)
    {
        variables.insert(E.key, E.value);
    }
}

void EventerWhiteboard::set_variable(const StringName &p_name, const Variant &p_value)
{
    variables.insert(p_name, p_value);
}

bool EventerWhiteboard::has_variable(const StringName &p_name) const
{
    return variables.has(p_name);
}

Variant EventerWhiteboard::get_variable(const StringName &p_name, const Variant &p_default) const
{
    if (variables.has(p_name))
    {
        return variables.get(p_name);
    }
    return p_default;
}

void EventerWhiteboard::erase_variable(const StringName &p_name)
{
    variables.erase(p_name);
}

Dictionary EventerWhiteboard::get_variables() const
{
    Dictionary ret;
    for (const KeyValue<StringName, Variant> &E : variables)
    {
        ret.set(E.key, E.value);
    }
    return ret;
}
