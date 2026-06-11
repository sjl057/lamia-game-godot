#pragma once

#include "core/input/input_event.h"
#include "core/object/object.h"
#include "core/typedefs.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/console/console_command.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/scroll_container.h"

class Console : public Control
{
    GDCLASS(Console, Control)

private:
    String default_message;
    String output_prefix = "[color=green][b]Console:[/b][/color] ";
    String output_error_prefix = "Console: ";
    Color background_color = Color(0.0, 0.0, 0.0, 0.7);
    int completion_max_lines = 7;
    float completion_line_separation = 20.f;

    ScrollContainer *output_scroll = nullptr;
    VBoxContainer *output = nullptr;
    LineEdit *text_input = nullptr;

    Vector<Node*> output_items;

    TypedArray<Dictionary> argument_hints;

    PackedStringArray completion_options;
    int completion_option_index = -1;

    Vector<String> command_history;
    String command_history_current;
    int command_history_index = -1;

    void load_command_history();
    void push_command_history(const String &p_command_string);
    void clear_command_history();
    void apply_command_history_index(const int &p_index);

    void refresh_completion_and_argument_hints(const String &p_text);

    String append_completion_option(const String &p_text, const String &p_option) const;

    RichTextLabel *create_output_label(const Variant &p_output, const bool &p_add_arrow = true) const;

    void push_output(const Variant &p_output, const int &p_indent = 0, const bool &p_add_arrow = true, const bool &p_to_out = true);
    void push_output_command(const String &p_command_string, const Variant &p_result, const bool &p_to_out = true);
    void push_output_error(const Variant &p_output, const bool &p_to_out = true);

    void scroll_to_bottom();

    void draw_background();
    void draw_argument_hints();
    void draw_completion_options();

    void _on_input_text_changed(const String &p_new_text);

    virtual void input(const Ref<InputEvent> &p_event) override;

protected:
    void _notification(int p_what);
    static void _bind_methods();

public:
    _FORCE_INLINE_ void set_default_message(const String &p_message) { default_message = p_message; }
    _FORCE_INLINE_ String get_default_message() const { return default_message; }
    _FORCE_INLINE_ void set_output_prefix(const String &p_prefix) { output_prefix = p_prefix; }
    _FORCE_INLINE_ String get_output_prefix() const { return output_prefix; }
    _FORCE_INLINE_ void set_output_error_prefix(const String &p_prefix) { output_error_prefix = p_prefix; }
    _FORCE_INLINE_ String get_output_error_prefix() const { return output_error_prefix; }
    _FORCE_INLINE_ void set_background_color(const Color &p_color)
    { 
        background_color = p_color; 
        queue_redraw();
    }
    _FORCE_INLINE_ Color get_background_color() const { return background_color; }
    _FORCE_INLINE_ void set_completion_max_lines(const int &p_lines)
    {
        completion_max_lines = MAX(p_lines, 1);
        queue_redraw();
    }
    _FORCE_INLINE_ int get_completion_max_lines() const { return completion_max_lines; }
    _FORCE_INLINE_ void set_completion_line_separation(const int &p_separation)
    { 
        completion_line_separation = MAX(p_separation, 0.f);
        queue_redraw();
    }
    _FORCE_INLINE_ float get_completion_line_separation() const { return completion_line_separation; }

    static PackedStringArray split_command_string(const String &p_command_string);
    static int get_arg_index(const String &p_text);

    bool is_open() const;
    void execute_command(const String &p_command_string, const bool &p_save_to_history = true);

    void clear();

    Console();
};