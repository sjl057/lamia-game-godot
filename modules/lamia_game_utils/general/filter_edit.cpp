#include "filter_edit.h"
#include "core/io/resource_loader.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"

FilterEdit::FilterEdit()
{
    set_clear_button_enabled(true);
    set_placeholder("Filter...");
    set_emoji_menu_enabled(false);
    set_keep_editing_on_text_submit(true);
    Ref<Theme> theme = EditorNode::get_singleton()->get_editor_theme();
    if (theme.is_valid())
    {
        set_right_icon(theme->get_icon(SNAME("Search"), EditorStringName(EditorIcons)));
    }
}