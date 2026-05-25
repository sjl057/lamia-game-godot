#include "filter_tree.h"

void FilterTree::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_filter", "filter"), &FilterTree::set_filter, DEFVAL(String()));
}

void FilterTree::set_filter(String p_filter)
{
    _update_filter(p_filter);
    if (!p_filter.is_empty())
    {
        if (get_selected())
        {
            call_deferred("scroll_to_item", get_selected());
        }
    }
}

bool FilterTree::_update_filter(String p_filter, TreeItem *p_parent)
{
    if (not p_parent) { p_parent = get_root(); };
    if (not p_parent) { return false; };

    PackedStringArray terms = p_filter.to_lower().split(" ", false);
    bool keep = item_matches_terms(p_parent, terms);
    bool selectable = keep;

    if (true)
    {
        TreeItem *next = get_root()->get_first_child();
        while (next)
        {
            if (next->has_meta("original_parent"))
            {
                next->get_parent()->remove_child(next);
                Object::cast_to<TreeItem>(next->get_meta("original_parent"))->add_child(next);
                next->remove_meta("original_parent");
            }
            next = next->get_next_in_tree();
        }
    } 

    bool keep_for_children = false;
    TreeItem *next_child = p_parent->get_first_child();
    while (next_child)
    {
        keep_for_children = _update_filter(p_filter, next_child) or keep_for_children;
        next_child = next_child->get_next();
    }
    p_parent->set_visible(keep_for_children or selectable);

    if (selectable)
    {
        p_parent->clear_custom_color(0);
        if (p_parent->get_metadata(0))
        {
            p_parent->set_selectable(0, true);
        }
    }
    else if (keep_for_children)
    {
        if (not p_parent->is_visible())
        {
            TreeItem *filtered_parent = p_parent->get_parent();
            while (filtered_parent)
            {
                if (filtered_parent == get_root() or filtered_parent->is_visible())
                {
                    break;
                }
                filtered_parent = filtered_parent->get_parent();
            }

            if (filtered_parent)
            {
                for (int i = 0; i < p_parent->get_child_count(); i++)
                {
                    TreeItem *child = p_parent->get_child(i);
                    bool is_selected = child->is_selected(0);
                    p_parent->remove_child(child);
                    filtered_parent->add_child(child);
                    TreeItem *prev_child = p_parent->get_prev();
                    if (prev_child)
                    {
                        child->move_after(prev_child);
                    }
                    if (is_selected)
                    {
                        child->select(0);
                    }
                }
                return false;
            }
        }
        else
        {
            p_parent->set_custom_color(0, Color(0.6627451, 0.6627451, 0.6627451, 1));
            p_parent->set_selectable(0, false);
        }
    }

    return p_parent->is_visible();
}

bool FilterTree::item_matches_terms(TreeItem *p_item, PackedStringArray terms)
{
    if (terms.is_empty()) { return true; }
    for (int i = 0; i < terms.size(); i++)
    {
        String term = terms[i];
        for (int j = 0; j < get_columns(); j++)
        {
            if (p_item->get_text(j).to_lower().contains(term))
            {
                return true;
            }
        }
    }
    return false;
}