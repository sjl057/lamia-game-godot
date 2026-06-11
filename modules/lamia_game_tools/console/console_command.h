#pragma once

#include "core/object/gdvirtual.gen.inc"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/typedefs.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"

class ConsoleCommandResult : public RefCounted
{
    GDCLASS(ConsoleCommandResult, RefCounted);

private:
    Variant output;
    bool error = false;
    bool clear_console = false;
    bool close_console = false;

protected:
    static void _bind_methods();

public:
    _FORCE_INLINE_ void set_output(const Variant &p_output) { output = p_output; }
    _FORCE_INLINE_ Variant get_output() const { return output; }
    _FORCE_INLINE_ void set_error(const bool &p_error) { error = p_error; }
    _FORCE_INLINE_ bool is_error() const { return error; }
    void set_error_output(const String &p_output);
    _FORCE_INLINE_ void set_clear_console(const bool &p_clear_console) { clear_console = p_clear_console; }
    _FORCE_INLINE_ bool should_clear_console() const { return clear_console; }
    _FORCE_INLINE_ void set_close_console(const bool &p_close_console) { close_console = p_close_console; }
    _FORCE_INLINE_ bool should_close_console() const { return close_console; }
};

class ConsoleCommand : public RefCounted
{
    GDCLASS(ConsoleCommand, RefCounted);

protected:
    static void _bind_methods();

    GDVIRTUAL2R_REQUIRED(Ref<ConsoleCommandResult>, _execute, const PackedStringArray&, const int&);
    GDVIRTUAL2R(TypedArray<Dictionary>, _get_argument_hints, const PackedStringArray&, const int&);
    GDVIRTUAL0R(String, _get_help_text);

public:
    virtual Ref<ConsoleCommandResult> execute(const PackedStringArray &p_args, const int &p_argcount);
    virtual TypedArray<Dictionary> get_argument_hints(const PackedStringArray &p_args, const int &p_argcount);
    virtual String get_help_text();
};

class ConsoleCommandHelp : public ConsoleCommand
{
    GDCLASS(ConsoleCommandHelp, ConsoleCommand);

protected:
    static void _bind_methods() {}

public:
    virtual Ref<ConsoleCommandResult> execute(const PackedStringArray &p_args, const int &p_argcount) override;
    virtual TypedArray<Dictionary> get_argument_hints(const PackedStringArray &p_args, const int &p_argcount) override;
    virtual String get_help_text() override;
};

class ConsoleCommandClear : public ConsoleCommand
{
    GDCLASS(ConsoleCommandClear, ConsoleCommand)

protected:
    static void _bind_methods() {}

public:
    virtual Ref<ConsoleCommandResult> execute(const PackedStringArray &p_args, const int &p_argcount) override;
    virtual String get_help_text() override; 
};

class ConsoleCommandClearHistory : public ConsoleCommand
{
    GDCLASS(ConsoleCommandClearHistory, ConsoleCommand)

protected:
    static void _bind_methods() {}

public:
    // virtual Ref<ConsoleCommandResult> execute(const PackedStringArray &p_args, const int &p_argcount) override;
    // virtual String get_help_text() override;
};

class ConsoleCommandListCommands : public ConsoleCommand
{
    GDCLASS(ConsoleCommandListCommands, ConsoleCommand);

protected:
    static void _bind_methods() {}

public:
    // virtual Ref<ConsoleCommandResult> execute(const PackedStringArray &p_args, const int &p_argcount) override;
    // virtual String get_help_text() override;
};
