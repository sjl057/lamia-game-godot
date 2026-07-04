#pragma once

#include "core/object/gdvirtual.gen.inc"
#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/typedefs.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/eventer/sequence.h"
#include "modules/lamia_game_tools/general/defs.h"
#include "modules/lamia_game_tools/general/utility.h"



class EVConditional : public EVCommand
{
    GDCLASS(EVConditional, EVCommand);

private:
    bool else_branch = false;
    bool invert = false;

protected:
    GDVIRTUAL0R_REQUIRED(bool, _check)
    static void _bind_methods();

public:
    void set_else_branch_enabled(const bool &p_enabled);
    _FORCE_INLINE_ bool get_else_branch_enabled() const { return else_branch; };
    void set_invert_enabled(const bool &p_enabled);
    _FORCE_INLINE_ bool get_invert_enabled() const { return invert; };

    virtual bool check();

    virtual bool can_add_children() override { return true; }
    virtual EVResult update(const float &p_delta) override;
};


class EVConsolePrint : public EVCommand
{
    GDCLASS(EVConsolePrint, EVCommand);

private:
    String message;

protected:
    static void _bind_methods();

public:
    _FORCE_INLINE_ void set_message(const String &p_message) { message = p_message; }
    _FORCE_INLINE_ String get_message() const { return message; }

    virtual String get_command_category() override { return "Utility"; }

    virtual void start() override
    {
        print_line_rich(message);
    };
};



class EVVarCheck : public EVConditional
{
    GDCLASS(EVVarCheck, EVConditional);

private:
    StringName variable;
    Variant value;
    LGTUtility::Check check_type;
    bool from_whiteboard;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

    static void _bind_methods() {};

public:
    _FORCE_INLINE_ void set_variable(const StringName &p_variable)
    { variable = p_variable; emit_changed(); }
    _FORCE_INLINE_ StringName get_variable() const { return variable; }
    _FORCE_INLINE_ void set_value(const Variant &p_value)
    { value = p_value; emit_changed(); }
    _FORCE_INLINE_ Variant get_value() const { return value; }
    _FORCE_INLINE_ void set_check_type(const LGTUtility::Check p_check_type)
    { check_type = p_check_type; emit_changed(); }
    _FORCE_INLINE_ LGTUtility::Check get_check_type() const { return check_type; }
    _FORCE_INLINE_ void set_from_whiteboard(const bool &p_from_whiteboard)
    { from_whiteboard = p_from_whiteboard; emit_changed(); }
    _FORCE_INLINE_ bool get_from_whiteboard() const { return from_whiteboard; }

    virtual String get_command_category() override { return "Whiteboard"; };

    virtual bool check() override;
};


class EVVarErase : public EVCommand
{
    GDCLASS(EVVarErase, EVCommand);

private:
    StringName variable;

protected:
    static void _bind_methods();

public:
    _FORCE_INLINE_ void set_variable(const StringName &p_variable)
    { variable = p_variable; emit_changed(); }
    _FORCE_INLINE_ StringName get_variable() const { return variable; }

    virtual String get_command_category() override { return "Whiteboard"; };

    virtual void start() override;
};


class EVVarExists : public EVConditional
{
    GDCLASS(EVVarExists, EVConditional);

private:
    StringName variable;

protected:
    static void _bind_methods();

public:
    _FORCE_INLINE_ void set_variable(const StringName &p_variable)
    { variable = p_variable; emit_changed(); }
    _FORCE_INLINE_ StringName get_variable() const { return variable; }

    virtual String get_command_category() override { return "Whiteboard"; };

    virtual bool check() override;
};


class EVVarSet : public EVCommand
{
    GDCLASS(EVVarSet, EVCommand);

private:
    StringName variable;
    Variant value;
    LGTUtility::Operation operation = LGTUtility::OPERATION_NONE;
    bool from_whiteboard = false;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	bool _property_can_revert(const StringName &p_name) const;
	bool _property_get_revert(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

    static void _bind_methods() {};

public:
    _FORCE_INLINE_ void set_variable(const StringName &p_variable)
    { variable = p_variable; emit_changed(); }
    _FORCE_INLINE_ StringName get_variable() const { return variable; }
    _FORCE_INLINE_ void set_value(const Variant &p_value)
    { value = p_value; emit_changed(); }
    _FORCE_INLINE_ Variant get_value() const { return value; }
    _FORCE_INLINE_ void set_operation(const LGTUtility::Operation p_operation)
    { operation = p_operation; emit_changed(); }
    _FORCE_INLINE_ LGTUtility::Operation get_operation() const { return operation; }
    _FORCE_INLINE_ void set_from_whiteboard(const bool &p_from_whiteboard)
    { from_whiteboard = p_from_whiteboard; emit_changed(); }
    _FORCE_INLINE_ bool get_from_whiteboard() const { return from_whiteboard; }

    virtual String get_command_category() override { return "Whiteboard"; };

    virtual void start() override;
};
