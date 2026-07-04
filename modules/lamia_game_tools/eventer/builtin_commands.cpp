#include "builtin_commands.h"
#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/string/print_string.h"
#include "core/string/string_name.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/general/defs.h"
#include "modules/lamia_game_tools/general/utility.h"


void EVConditional::_bind_methods()
{
    BIND(D_METHOD("_set_else_branch_enabled", "enabled"), &EVConditional::set_else_branch_enabled);
    BIND(D_METHOD("_get_else_branch_enabled"), &EVConditional::get_else_branch_enabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "else_branch"), "_set_else_branch_enabled", "_get_else_branch_enabled");

    BIND(D_METHOD("_set_invert_enabled", "enabled"), &EVConditional::set_invert_enabled);
    BIND(D_METHOD("_get_invert_enabled"), &EVConditional::get_invert_enabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "invert"), "_set_invert_enabled", "_get_invert_enabled");

    GDVIRTUAL_BIND(_check);
}

void EVConditional::set_else_branch_enabled(const bool &p_enabled)
{
    else_branch = p_enabled;
    emit_changed();
}

void EVConditional::set_invert_enabled(const bool &p_enabled)
{
    invert = p_enabled;
    emit_changed();
}

bool EVConditional::check()
{
    bool ret = false;
    GDVIRTUAL_CALL(_check, ret);
    return invert? not ret : ret;
}

EVResult EVConditional::update(const float &p_delta)
{
    EVResult result = check()? EV_NEXT_IN_TREE : EV_NEXT_SIBLING;
    return result;
}



void EVConsolePrint::_bind_methods()
{
    BIND(D_METHOD("set_message", "message"), &EVConsolePrint::set_message);
    BIND(D_METHOD("get_message"), &EVConsolePrint::get_message);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "message", PROPERTY_HINT_MULTILINE_TEXT), "set_message", "get_message");
}

String EVConsolePrint::get_command_suffix()
{
    return message;
}



bool EVVarCheck::_set(const StringName &p_name, const Variant &p_value)
{
    if (p_name == "variable")
    {
        set_variable(p_value);
        return true;
    }
    else if (p_name == "value")
    {
        set_value(p_value);
        return true;
    }
    else if (p_name == "check_type")
    {
        set_check_type(p_value);
        return true;
    }
    else if (p_name == "from_whiteboard")
    {
        set_from_whiteboard(p_value);
        if (p_value)
        {
            set_value(StringName());
        }
        else
        {
            set_value(Variant());
        }
        notify_property_list_changed();
        return true;
    }

    return false;
}

bool EVVarCheck::_get(const StringName &p_name, Variant &r_ret) const
{
    if (p_name == "variable")
    {
        r_ret = get_variable();
        return true;
    }
    else if (p_name == "value")
    {
        r_ret = get_value();
        return true;
    }
    else if (p_name == "check_type")
    {
        r_ret = get_check_type();
        return true;
    }
    else if (p_name == "from_whiteboard")
    {
        r_ret = get_from_whiteboard();
        return true;
    }

    return false;
}

void EVVarCheck::_get_property_list(List<PropertyInfo> *p_list) const
{
    p_list->push_back(PropertyInfo(Variant::STRING_NAME, "variable"));

    if (get_from_whiteboard())
    {
        p_list->push_back(PropertyInfo(Variant::STRING_NAME, "value"));
    }
    else
    {
        p_list->push_back(PropertyInfo(Variant::NIL, "value", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NIL_IS_VARIANT | PROPERTY_USAGE_DEFAULT));
    }

    p_list->push_back(PropertyInfo(Variant::INT, "check_type", PROPERTY_HINT_ENUM, "Equals,Not Equal,Greater Than,Greater Than or Equal,Less Than,Less Than or Equal"));
    p_list->push_back(PropertyInfo(Variant::BOOL, "from_whiteboard"));
}

bool EVVarCheck::check()
{
    auto wb = get_whiteboard();

    Variant compared_value = value;
    if (get_from_whiteboard() and (compared_value.is_string() and not compared_value.is_null()))
    {
        compared_value = wb->get_variable(value);
    }

    if (wb->has_variable(variable))
    {
        return LGTUtility::get_singleton()->perform_check(get_check_type(), wb->get_variable(variable), compared_value);
	}
    else
    {
        ERR_PRINT(vformat("Variable \"%s\" not found", variable));
        return false;
    }
}

void EVVarErase::_bind_methods()
{
    BIND(D_METHOD("set_variable", "variable"), &EVVarErase::set_variable);
    BIND(D_METHOD("get_variable"), &EVVarErase::get_variable);
    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "variable"), "set_variable", "get_variable");
}

void EVVarErase::start()
{
    get_whiteboard()->erase_variable(get_variable());
}

void EVVarExists::_bind_methods()
{
    BIND(D_METHOD("set_variable", "variable"), &EVVarExists::set_variable);
    BIND(D_METHOD("get_variable"), &EVVarExists::get_variable);
    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "variable"), "set_variable", "get_variable");
}

bool EVVarExists::check()
{
    return get_whiteboard()->has_variable(get_variable());
}


bool EVVarSet::_set(const StringName &p_name, const Variant &p_value)
{
    if (p_name == "variable")
    {
        set_variable(p_value);
        return true;
    }
    else if (p_name == "value")
    {
        set_value(p_value);
        return true;
    }
    else if (p_name == "operation")
    {
        set_operation(p_value);
        return true;
    }
    else if (p_name == "from_whiteboard")
    {
        set_from_whiteboard(p_value);
        if (p_value)
        {
            set_value(StringName());
        }
        else
        {
            set_value(Variant());
        }
        notify_property_list_changed();
        return true;
    }

    return false;
}

bool EVVarSet::_get(const StringName &p_name, Variant &r_ret) const
{
    if (p_name == "variable")
    {
        r_ret = get_variable();
        return true;
    }
    else if (p_name == "value")
    {
        r_ret = get_value();
        return true;
    }
    else if (p_name == "operation")
    {
        r_ret = get_operation();
        return true;
    }
    else if (p_name == "from_whiteboard")
    {
        r_ret = get_from_whiteboard();
        return true;
    }

    return false;
}

bool EVVarSet::_property_can_revert(const StringName &p_name) const
{
    if (p_name == "value")
    {
        return get_value().is_null();
    }

    return false;
}

bool EVVarSet::_property_get_revert(const StringName &p_name, Variant &r_property) const
{
    if (p_name == "value")
    {
        if (get_from_whiteboard())
        {
            r_property = StringName();
        }
        else
        {
            r_property = Variant();
        }
        return true;
    }

    return false;
}

void EVVarSet::_get_property_list(List<PropertyInfo> *p_list) const
{
    p_list->push_back(PropertyInfo(Variant::STRING_NAME, "variable"));

    if (get_from_whiteboard())
    {
        p_list->push_back(PropertyInfo(Variant::STRING_NAME, "value"));
    }
    else
    {
        p_list->push_back(PropertyInfo(Variant::NIL, "value", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NIL_IS_VARIANT | PROPERTY_USAGE_DEFAULT));
    }

    p_list->push_back(PropertyInfo(Variant::INT, "operation", PROPERTY_HINT_ENUM, "Set,Add,Subtract,Multiply,Divide,Power,Module,Min,Max,Bit Shift Left,Bit Shift Right,Bit And,Bit Or,Bit Xor"));
    p_list->push_back(PropertyInfo(Variant::BOOL, "from_whiteboard"));
}

void EVVarSet::start()
{
    auto wb = get_whiteboard();

    Variant new_value = value;
    if (from_whiteboard and (new_value.is_string() and not new_value.is_null()))
    {
        new_value = wb->get_variable(value);
    }

    Variant old_value = wb->get_variable(variable);

    Variant result = LGTUtility::get_singleton()->perform_operation(get_operation(), old_value, new_value);
    wb->set_variable(variable, result);
}
