#include "console_command.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/console/console_db.h"
#include "modules/lamia_game_tools/general/defs.h"

void ConsoleCommandResult::_bind_methods()
{
    BIND(D_METHOD("set_output", "output"), &ConsoleCommandResult::set_output);
    BIND(D_METHOD("get_output"), &ConsoleCommandResult::get_output);
    ADD_PROPERTY(PropertyInfo(Variant::VARIANT_MAX, "output"), "set_output", "get_output");

    BIND(D_METHOD("set_error", "error"), &ConsoleCommandResult::set_error);
    BIND(D_METHOD("is_error"), &ConsoleCommandResult::is_error);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "error"), "set_error", "is_error");

    BIND(D_METHOD("set_clear_console", "clear_console"), &ConsoleCommandResult::set_clear_console);
    BIND(D_METHOD("should_clear_console"), &ConsoleCommandResult::should_clear_console);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clear_console"), "set_clear_console", "should_clear_console");

    BIND(D_METHOD("set_close_console", "close_console"), &ConsoleCommandResult::set_close_console);
    BIND(D_METHOD("should_close_console"), &ConsoleCommandResult::should_close_console);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "close_console"), "set_close_console", "should_close_console");

    BIND(D_METHOD("set_error_output", "output"), &ConsoleCommandResult::set_error_output);
}

void ConsoleCommandResult::set_error_output(const String &p_output)
{
    set_output(p_output);
    set_error(true);
}


void ConsoleCommand::_bind_methods()
{
    GDVIRTUAL_BIND(_execute, "args", "argcount");
    GDVIRTUAL_BIND(_get_argument_hints, "args", "argcount");
    GDVIRTUAL_BIND(_get_help_text);
}

Ref<ConsoleCommandResult> ConsoleCommand::execute(const PackedStringArray &p_args, const int &p_argcount)
{
    Ref<ConsoleCommandResult> ret;
    GDVIRTUAL_CALL(_execute, p_args, p_argcount, ret);
    return ret;
}

TypedArray<Dictionary> ConsoleCommand::get_argument_hints(const PackedStringArray &p_args, const int &p_argcount)
{
    TypedArray<Dictionary> ret;
    GDVIRTUAL_CALL(_get_argument_hints, p_args, p_argcount, ret);
    return ret;
}

String ConsoleCommand::get_help_text()
{
    String ret;
    GDVIRTUAL_CALL(_get_help_text, ret);
    return ret;
}


Ref<ConsoleCommandResult> ConsoleCommandHelp::execute(const PackedStringArray &p_args, const int &p_argcount)
{
    Ref<ConsoleCommandResult> result;
    result.instantiate();

    if (p_argcount >= 1)
    {
        String command = p_args[0];
        Ref<ConsoleCommand> command_instance = ConsoleDB::get_command_instance(command);
        if (command_instance.is_valid())
        {
            result->set_output(command_instance->get_help_text());
        }
        else
        {
            result->set_error_output(vformat("Command not found: %s", command));
        }
    }
    else
    {
        result->set_error_output("No command provided");
    }

    return result;
}

TypedArray<Dictionary> ConsoleCommandHelp::get_argument_hints(const PackedStringArray &p_args, const int &p_argcount)
{
    TypedArray<Dictionary> ret;
    Dictionary hint1;
    hint1.set("name", "command");
    hint1.set("defaults", ConsoleDB::get_commands());

    ret.append(hint1);

    return ret;
}

String ConsoleCommandHelp::get_help_text()
{
    return "Provides help for a given command. However, you already know this as you typed it in?";
}


Ref<ConsoleCommandResult> ConsoleCommandClear::execute(const PackedStringArray &p_args, const int &p_argcount)
{
    Ref<ConsoleCommandResult> result;
    result.instantiate();

    result->set_clear_console(true);

    return result;
}

String ConsoleCommandClear::get_help_text()
{
    return "Clears the console's output.";
}