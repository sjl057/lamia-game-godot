#include "console.h"
#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/error/error_macros.h"
#include "core/input/input_event.h"
#include "core/io/file_access.h"
#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "core/math/rect2.h"
#include "core/math/vector2.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/script_language.h"
#include "core/string/print_string.h"
#include "core/string/string_name.h"
#include "core/typedefs.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/console/console_command.h"
#include "modules/lamia_game_tools/console/console_db.h"
#include "modules/lamia_game_tools/general/defs.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/scroll_container.h"
#include "scene/main/canvas_item.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "scene/theme/theme_db.h"
#include "servers/text/text_server.h"

void Console::_bind_methods()
{
    BIND(D_METHOD("is_open"), &Console::is_open);
    BIND(D_METHOD("clear"), &Console::clear);

    BIND(D_METHOD("set_default_message", "message"), &Console::set_default_message);
    BIND(D_METHOD("get_default_message"), &Console::get_default_message);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_message", PROPERTY_HINT_MULTILINE_TEXT), "set_default_message", "get_default_message");

    BIND(D_METHOD("set_output_prefix", "prefix"), &Console::set_output_prefix);
    BIND(D_METHOD("get_output_prefix"), &Console::get_output_prefix);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "output_prefix"), "set_output_prefix", "get_output_prefix");
    ADD_PROPERTY_DEFAULT("output_prefix", "[color=green][b]Console:[/b][/color] ");

    BIND(D_METHOD("set_output_error_prefix", "prefix"), &Console::set_output_error_prefix);
    BIND(D_METHOD("get_output_error_prefix"), &Console::get_output_error_prefix);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "output_error_prefix"), "set_output_error_prefix", "get_output_error_prefix");
    ADD_PROPERTY_DEFAULT("output_error_prefix", "Console: ");

    BIND(D_METHOD("set_background_color", "color"), &Console::set_background_color);
    BIND(D_METHOD("get_background_color"), &Console::get_background_color);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");
    ADD_PROPERTY_DEFAULT("background_color", Color(0.0, 0.0, 0.0, 0.7));

    BIND(D_METHOD("set_completion_max_lines", "lines"), &Console::set_completion_max_lines);
    BIND(D_METHOD("get_completion_max_lines"), &Console::get_completion_max_lines);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "completion_max_lines"), "set_completion_max_lines", "get_completion_max_lines");
    ADD_PROPERTY_DEFAULT("completion_max_lines", 7);

    BIND(D_METHOD("set_completion_line_separation", "separation"), &Console::set_completion_line_separation);
    BIND(D_METHOD("get_completion_line_separation"), &Console::get_completion_line_separation);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "completion_line_separation"), "set_completion_line_separation", "get_completion_line_separation");
    ADD_PROPERTY_DEFAULT("completion_line_separation", 20.f);
}

Console::Console()
{
    ConsoleDB::scan_script_command_paths();

    auto full_vbox = memnew(VBoxContainer);
    full_vbox->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
    add_child(full_vbox);

    output_scroll = memnew(ScrollContainer);
    output_scroll->set_v_size_flags(SIZE_EXPAND_FILL);
    full_vbox->add_child(output_scroll);

    output = memnew(VBoxContainer);
    output->set_alignment(BoxContainer::ALIGNMENT_END);
    output->set_h_size_flags(SIZE_EXPAND_FILL);
    output->set_v_size_flags(SIZE_EXPAND_FILL);
    output_scroll->add_child(output);

    auto input_hbox = memnew(HBoxContainer);
    full_vbox->add_child(input_hbox);

    auto input_label = memnew(Label);
    input_label->set_text(" >");
    input_hbox->add_child(input_label);

    text_input = memnew(LineEdit);
    text_input->set_placeholder("Enter a command...");
    text_input->set_keep_editing_on_text_submit(true);
    text_input->set_context_menu_enabled(false);
    text_input->set_emoji_menu_enabled(false);
    text_input->set_clear_button_enabled(true);
    text_input->set_flat(true);
    text_input->set_h_size_flags(SIZE_EXPAND_FILL);
    text_input->connect(SNAME("text_changed"), callable_mp(this, &Console::_on_input_text_changed));
    input_hbox->add_child(text_input);
}

void Console::_notification(int p_what)
{
    switch (p_what)
    {
        case NOTIFICATION_ENTER_TREE:
            if (not Engine::get_singleton()->is_editor_hint())
            {
                set_process_input(true);
            }
            break;
        
        case NOTIFICATION_READY:
            if (not Engine::get_singleton()->is_editor_hint() and not default_message.is_empty())
            {
                push_output(default_message, 0, true, false);
                load_command_history();
            }
            break;

        case NOTIFICATION_DRAW:
            draw_background();
            draw_argument_hints();
            draw_completion_options();
            break;

        case NOTIFICATION_VISIBILITY_CHANGED:
            if (not Engine::get_singleton()->is_editor_hint())
            {
                if (is_ready())
                {
                    if (is_visible())
                    {
                        set_process_input(true);
                        text_input->grab_focus();
                    }
                    else
                    {
                        set_process_input(false);
                        text_input->release_focus();
                    }
                }
            }
            break;
    }
}

void Console::input(const Ref<InputEvent> &p_event)
{
    Ref<InputEventKey> event = p_event;
    if (event.is_valid() and event->is_pressed())
    {
        switch (event->get_keycode())
        {
            case Key::UP:
                accept_event();
                if (not completion_options.is_empty())
                {
                    completion_option_index = Math::wrapi(completion_option_index - 1, -1, completion_options.size());
                    queue_redraw();
                }
                else
                {
                    accept_event();
                    if (not command_history.is_empty())
                    {
                        if (command_history_index <= -1)
                        {
                            command_history_index = 0;
                            command_history_current = text_input->get_text();
                        }
                        else
                        {
                            command_history_index = MIN(command_history_index + 1, command_history.size() - 1);
                        }
                        apply_command_history_index(command_history_index);
                    }
                }
                break;

            case Key::DOWN:
                accept_event();
                if (not completion_options.is_empty())
                {
                    completion_option_index = Math::wrapi(completion_option_index + 1, -1, completion_options.size());
                    queue_redraw();
                }
                else
                {
                    if (command_history_index > -1 and not command_history.is_empty())
                    {
                        command_history_index = MAX(command_history_index - 1, -1);
                        apply_command_history_index(command_history_index);
                    }
                }
                break;

            case Key::ENTER:
            case Key::KP_ENTER:
            case Key::TAB:
                if (not completion_options.is_empty() and completion_option_index > -1)
                {
                    accept_event();
                    text_input->set_text(append_completion_option(text_input->get_text(), completion_options[completion_option_index]));
                    text_input->set_caret_column(text_input->get_text().length());
                    argument_hints.clear();
                    completion_options.clear();
                    completion_option_index = -1;
                    queue_redraw();
                }
                else
                {
                    if (event->get_keycode() != Key::TAB)
                    {
                        accept_event();
                        
                        command_history_index = -1;
                        command_history_current = String();

                        if (not text_input->get_text().is_empty())
                        {
                            execute_command(text_input->get_text());
                        }
                        text_input->clear();
                    }
                }
                break;

            default:
                break;
        }
    }
}

bool Console::is_open() const
{
    return is_visible();
}

void Console::load_command_history()
{
    command_history.clear();
    command_history_current = String();
    command_history_index = -1;

    auto file = FileAccess::get_file_as_string("user://" + (String)GLOBAL_GET(SNAME("console/history/history_path")));
    if (not file.is_empty())
    {
        Vector<String> split = file.split("\n", false);
        for (const String &line : split)
        {
            command_history.append(line);
        }
    }
}

void Console::push_command_history(const String &p_command_string)
{
    int max_lines = GLOBAL_GET(SNAME("console/history/history_max_lines"));
    if (command_history.size() >= max_lines)
    {
        command_history.remove_at(command_history.size() - 1);
    }
    command_history.insert(0, p_command_string);
    auto file = FileAccess::open("user://" + (String)GLOBAL_GET(SNAME("console/history/history_path")), FileAccess::WRITE);
    if (file.is_valid())
    {
        for (const String &line : command_history)
        {
            file->store_line(line);
        }
        file->close();
    }
}

void Console::clear_command_history()
{
    command_history.clear();
    auto file = FileAccess::open("user://" + (String)GLOBAL_GET(SNAME("console/history/history_path")), FileAccess::WRITE);
    if (file.is_valid())
    {
        file->close();
    }
}

void Console::apply_command_history_index(const int &p_index)
{
    if (p_index <= -1)
    {
        text_input->set_text(command_history_current);
    }
    else
    {
        text_input->set_text(command_history[p_index]);
    }
    text_input->set_caret_column(text_input->get_text().length());

    completion_options.clear();
    completion_option_index = -1;
    argument_hints.clear();
    queue_redraw();
}

PackedStringArray Console::split_command_string(const String &p_command_string)
{
    if (p_command_string.is_empty()) { return PackedStringArray(); }

    PackedStringArray ret;

    PackedStringArray args;
    String current_arg;
    bool escaping_commas = false;
    for (int i = 0; i < p_command_string.length(); i++)
    {
        const char32_t current_char = p_command_string[i];
        if (current_char == '\"')
        {
            escaping_commas = not escaping_commas;
            current_arg += current_char;
        }
        else if (current_char == ',' and not escaping_commas)
        {
            args.append(current_arg);
            current_arg = String();
        }
        else
        {
            current_arg += current_char;
        }
    }
    args.append(current_arg);

    for (const String &arg : args)
    {
        ret.append(arg.strip_edges());
    }

    // command name should always be snake case
    ret.set(0, ret[0].to_lower());

    return ret;
}

int Console::get_arg_index(const String &p_text)
{
    if (p_text.is_empty()) { return 0; }
    int count = p_text.get_slice_count(",");
    return count - 2;
}

void Console::execute_command(const String &p_command_string, const bool &p_save_to_history)
{
    PackedStringArray args = split_command_string(p_command_string);
    String command = args[0];
    args.remove_at(0);

    Ref<ConsoleCommand> command_instance = ConsoleDB::get_command_instance(command);

    if (command_instance.is_valid())
    {
        const int argcount = args.size();
        Ref<ConsoleCommandResult> result = command_instance->execute(args, argcount);
        if (result.is_valid())
        {
            push_output_command(p_command_string, result->get_output());
            if (result->should_clear_console())
            {
                clear();
            }
            if (result->should_close_console())
            {
                hide();
            }
        }
        else
        {
            push_output(p_command_string);
        }

        command_instance.unref();
    }
    else
    {
        push_output_error(vformat("Command not found: %s", command));
    }

    if (p_save_to_history)
    {
        push_command_history(p_command_string);
    }
}

RichTextLabel *Console::create_output_label(const Variant &p_output, const bool &p_add_arrow) const
{
    auto output_label = memnew(RichTextLabel);
    output_label->set_use_bbcode(true);
    output_label->set_fit_content(true);
    output_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD);
    output_label->set_scroll_active(false);
    output_label->set_shortcut_keys_enabled(false);
    output_label->set_text(p_add_arrow ? vformat(" > %s", p_output) : (String)p_output);
    return output_label;
}

void Console::push_output(const Variant &p_output, const int &p_indent, const bool &p_add_arrow, const bool &p_to_out)
{
    if (p_to_out)
    {
        print_line_rich(vformat("%s%s", output_prefix, p_output));
    }

    auto output_label = create_output_label(p_output, p_add_arrow);
    if (p_indent > 0)
    {
        auto margin = memnew(MarginContainer);
        margin->add_theme_constant_override(SNAME("margin_left"), p_indent);
        margin->add_child(output_label);
        output->add_child(margin);
        output_items.append(margin);
    }
    else
    {
        output->add_child(output_label); 
        output_items.append(output_label);
    }

    scroll_to_bottom();
}

void Console::push_output_command(const String &p_command_string, const Variant &p_result, const bool &p_to_out)
{
    push_output(p_command_string, 0, true, p_to_out);
    push_output(p_result, 24, false, true);
}

void Console::push_output_error(const Variant &p_output, const bool &p_to_out)
{
    if (p_to_out)
    {
        ERR_PRINT(vformat("%s%s", output_error_prefix, p_output));
    }
    auto output_label = memnew(Label);
    output_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD);
    output_label->set_text(vformat(" ERROR: %s", p_output));
    output->add_child(output_label);
    output_items.append(output_label);

    scroll_to_bottom();
}

void Console::scroll_to_bottom()
{
    if (not SceneTree::get_singleton()->is_connected(SNAME("process_frame"), callable_mp(output_scroll, &ScrollContainer::set_v_scroll)))
    {
        SceneTree::get_singleton()->connect(SNAME("process_frame"), callable_mp(output_scroll, &ScrollContainer::set_v_scroll).bind(output_scroll->get_size().y), CONNECT_ONE_SHOT | CONNECT_DEFERRED);
    }
}

void Console::clear()
{
    for (Node *child : output_items)
    {
        child->queue_free();
    }
    output_items.clear();
}

String Console::append_completion_option(const String &p_text, const String &p_option) const
{
    PackedStringArray split = split_command_string(p_text);
    String new_string = p_text.trim_suffix(split[split.size() - 1]);

    if (not new_string.is_empty())
    {
        return new_string + p_option;
    }
    else
    {
        return p_option;
    }
}

void Console::refresh_completion_and_argument_hints(const String &p_text)
{
    argument_hints.clear();
    completion_options.clear();
    completion_option_index = -1;
    if (p_text.is_empty())
    {
        queue_redraw();
        return;
    }

    PackedStringArray command_input = split_command_string(p_text);
    if (command_input.size() == 1) // Command is always first
    {
        for (const String &command : ConsoleDB::get_commands())
        {
            if (command.begins_with(command_input[0]) and not command_input[0].contains(command))
            {
                completion_options.append(command);
            }
        }
        completion_options.sort();
    }
    else // Must be an argument because it can't be empty or negative
    {
        Ref<ConsoleCommand> command_instance = ConsoleDB::get_command_instance(command_input[0]);
        if (command_instance.is_valid())
        {
            argument_hints = command_instance->get_argument_hints(command_input, command_input.size() - 1);
        }

        if (not argument_hints.is_empty())
        {
            int current_arg = get_arg_index(p_text);
            Dictionary hints = argument_hints[current_arg];
            if (current_arg < argument_hints.size() and hints.has(SNAME("defaults")))
            {
                completion_options.append_array(hints.get(SNAME("defaults"), PackedStringArray()));
            }
        }
    }   

    queue_redraw();
}

void Console::draw_background()
{
    draw_rect(Rect2(Point2(), get_size()), background_color);
}

void Console::draw_argument_hints()
{
    if (argument_hints.is_empty()) { return; }

    Ref<Font> font = ThemeDB::get_singleton()->get_fallback_font();

    const String current_text = text_input->get_text();

    Size2 caret_position = font->get_string_size(current_text);
    float base_x = caret_position.x + 5.f;
    float base_y = get_size().y + 9.f;

    int current_arg = get_arg_index(current_text);
    int i = 0;
    float offset = 0.f;

    PackedStringArray arg_names;
    PackedFloat32Array arg_offsets;

    for (const Dictionary hint : argument_hints)
    {
        String argument;

        if (hint.has(SNAME("name")))
        {
            argument = hint.get(SNAME("name"), String());
        }

        if (hint.has(SNAME("type")))
        {
            Variant type = hint.get(SNAME("type"), Variant());
            if (static_cast<Variant::Type>(type))
            {
                type = Variant::get_type_name(type);
            }
            else if (type.is_array())
            {
                Array types = type;
                String union_type_string;
                for (const Variant &union_type : types)
                {
                    union_type_string += static_cast<Variant::Type>(union_type) ? Variant::get_type_name(union_type) : (String)union_type;
                    union_type_string += "|";
                }
                type = union_type_string.rstrip("|");
            }

            if (argument.is_empty())
            {
                argument += type.stringify();
            }
            else
            {
                argument += vformat(": %s", type);
            }

            if (i < argument_hints.size() - 1)
            {
                argument += ", ";
            }
        }

        if (hint.has(SNAME("optional")) and hint["optional"])
        {
            argument = vformat("<%s>", argument);
        }
        
        arg_names.append(argument);
        arg_offsets.append(offset);
        offset += font->get_string_size(argument).x + 3.f;

        i++;
    }

    if (current_arg < arg_offsets.size())
    {
        Rect2 rect = Rect2(base_x - arg_offsets[current_arg], base_y, offset, completion_line_separation);
        rect = rect.grow(3.f);
        draw_rect(rect, background_color);

        i = 0;
        for (const Dictionary hint : argument_hints)
        {
            Color color = i == current_arg ? Color(1.0, 1.0, 1.0, 1.0) : Color(0.6627451, 0.6627451, 0.6627451, 1.0);
            draw_string(font, \
            Point2(base_x + arg_offsets[i] - arg_offsets[current_arg], base_y + (completion_line_separation - 5.f)), \
            arg_names[i],\
            HORIZONTAL_ALIGNMENT_LEFT,\
            -1,\
            ThemeDB::get_singleton()->get_fallback_font_size(),\
            color);
            i++;
        }
    }
}

void Console::draw_completion_options()
{
    if (completion_options.is_empty()) { return; }

    const int completion_option_count = completion_options.size();

    Ref<Font> font = ThemeDB::get_singleton()->get_fallback_font();

    Size2 caret_position = font->get_string_size(text_input->get_text());
    float base_x = caret_position.x + 5.f;
    float base_y = not argument_hints.is_empty() ? get_size().y + 35.f : get_size().y + 9.f;

    Size2 rect_size;
    for (int i = 0; i < completion_option_count; i++)
    {
        String option = completion_options[i];
        Size2 option_size = font->get_string_size(option);
        rect_size.x = MAX(rect_size.x, option_size.x);
        rect_size.y += completion_line_separation;
    }
    rect_size.y = MIN(rect_size.y, completion_line_separation * completion_max_lines);

    Rect2 rect = Rect2(Point2(base_x, base_y), rect_size);
    rect = rect.grow(3.f);
    rect.size.y += 9.f;
    draw_rect(rect, background_color);

    const int scroll_point = Math::floor(completion_max_lines / 2.f);
    const int scroll_end = (completion_option_count - 1) - scroll_point;

    int offset = 0;
    bool down_arrow = true;
    bool up_arrow = false;

    if (completion_option_count > completion_max_lines)
    {
        if (completion_option_index >= scroll_end)
        {
            offset = scroll_end - scroll_point;
            down_arrow = false;
            up_arrow = true;
        }
        else if (completion_option_index >= scroll_point)
        {
            offset = completion_option_index - scroll_point;
            up_arrow = true;
        }
    }
    else
    {
        down_arrow = false;
    }

    for (int i = offset; i < completion_option_count; i++)
    {
        String option = completion_options[i];
        Size2 option_size = font->get_string_size(option);
        Point2 pos = Point2(base_x, base_y + ((i - offset) * 20.f) + option_size.y);
        Color color = Color(0.6627451, 0.6627451, 0.6627451, 1.0);
        if (completion_option_index > -1)
        {
            color = completion_option_index == i ? Color(1.0, 0.0, 0.0, 1.0) : Color(1.0, 1.0, 1.0, 1.0);
        }
        draw_string(font, pos, option, HORIZONTAL_ALIGNMENT_LEFT, -1, ThemeDB::get_singleton()->get_fallback_font_size(), color);
        if ((i + 1) - offset > completion_max_lines) { break; }
    }

    if (up_arrow)
    {
        Point2 pos = Point2(base_x - 14.f, base_y);
        draw_rect(Rect2(pos, Size2(11.0, 11.0)), background_color);
        draw_string(font, pos + Vector2(1.f, 10.f), "^");
    }

    if (down_arrow)
    {
        Point2 pos = Point2(base_x - 14.f, (base_y + rect_size.y) - 8.0);
        draw_rect(Rect2(pos, Size2(11.0, 11.0)), background_color);
        draw_string(font, pos + Vector2(1.f, 10.f), "v");
    }
}

void Console::_on_input_text_changed(const String &p_new_text)
{
    refresh_completion_and_argument_hints(p_new_text);
}

