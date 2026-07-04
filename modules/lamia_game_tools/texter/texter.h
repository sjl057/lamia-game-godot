#pragma once

#include "core/io/resource.h"
#include "core/math/color.h"
#include "core/math/math_defs.h"
#include "core/math/vector2.h"
#include "core/object/object.h"
#include "core/string/node_path.h"
#include "core/templates/local_vector.h"
#include "core/typedefs.h"
#include "scene/gui/control.h"
#include "scene/theme/theme_db.h"

class TexterDisplaySettings : public Resource
{
    GDCLASS(TexterDisplaySettings, Resource);

private:
    Ref<Font> font;
    int font_size = 16;
    Color font_color;

    float type_speed_override;
    float type_speed_mult = 1.0;

    int outline_size;
    Color outline_color;

    int shadow_size;
    Color shadow_color;
    Point2i shadow_offset;

    bool shadow_over_outline;

protected:
    static void _bind_methods();

public:
    _FORCE_INLINE_ void set_font(Ref<Font> p_font) { font = p_font; emit_changed(); }
    _FORCE_INLINE_ Ref<Font> get_font() const { return font; }
    _FORCE_INLINE_ void set_font_size(const int &p_font_size) { font_size = p_font_size; emit_changed(); }
    _FORCE_INLINE_ int get_font_size() const { return font_size; }
    _FORCE_INLINE_ void set_font_color(const Color &p_font_color) { font_color = p_font_color; emit_changed(); }
    _FORCE_INLINE_ Color get_font_color() const { return font_color; }
    _FORCE_INLINE_ void set_type_speed_override(const float &p_type_speed_override) { type_speed_override = p_type_speed_override; emit_changed(); }
    _FORCE_INLINE_ float get_type_speed_override() const { return type_speed_override; }
    // _FORCE_INLINE_ void set_type_speed_mult(float p_type_speed_mult) { type_speed_mult = p_type_speed_mult; emit_changed(); }
    // _FORCE_INLINE_ float get_type_speed_mult() const { return type_speed_mult; }
    _FORCE_INLINE_ void set_outline_size(const int &p_outline_size) { outline_size = p_outline_size; emit_changed(); }
    _FORCE_INLINE_ int get_outline_size() const { return outline_size; }
    _FORCE_INLINE_ void set_outline_color(const Color &p_outline_color) { outline_color = p_outline_color; emit_changed(); }
    _FORCE_INLINE_ Color get_outline_color() const { return outline_color; }
    _FORCE_INLINE_ void set_shadow_size(const int &p_shadow_size) { shadow_size = p_shadow_size; emit_changed(); }
    _FORCE_INLINE_ int get_shadow_size() const { return shadow_size; }
    _FORCE_INLINE_ void set_shadow_color(const Color &p_shadow_color) { shadow_color = p_shadow_color; emit_changed(); }
    _FORCE_INLINE_ Color get_shadow_color() const { return shadow_color; }
    _FORCE_INLINE_ void set_shadow_offset(const Point2i &p_shadow_offset) { shadow_offset = p_shadow_offset; emit_changed(); }
    _FORCE_INLINE_ Point2i get_shadow_offset() const { return shadow_offset; }
    _FORCE_INLINE_ void set_shadow_over_outline(const bool &p_shadow_over_outline) { shadow_over_outline = p_shadow_over_outline; emit_changed(); }
    _FORCE_INLINE_ bool get_shadow_over_outline() const { return shadow_over_outline; }
};


class TexterTypeSettings : public Resource
{
    GDCLASS(TexterTypeSettings, Resource);

private:


protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	bool _property_can_revert(const StringName &p_name) const;
	bool _property_get_revert(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

    static void _bind_methods() {}
};


class Texter : public Control
{
    GDCLASS(Texter, Control);

private:
    struct Character
    {
        int index = 0;
        int line = 0;
        int line_index = 0;
        Point2 position;
    };

    struct PreprocessState
    {
        Ref<Font> font = ThemeDB::get_singleton()->get_fallback_font();
        int font_size = ThemeDB::get_singleton()->get_fallback_font_size();
        Color font_color;
        Color font_last_color;

        bool newline = false;
        bool escaping = false;
    };

    struct TypeState
    {
        String text;
        LocalVector<Character> characters;
        Point2 caret;

        TypeState(const String &p_text, const LocalVector<Character> &p_chars);
    };

    String text;
    Ref<TexterDisplaySettings> display_settings;
    Ref<TexterTypeSettings> type_settings;
    bool typewriter = true;
    HorizontalAlignment horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
    VerticalAlignment vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    bool autowrap = true;
    int line_separation = 0;
    bool always_draw = false;

    NodePath audio_player_path;

protected:
    static void _bind_methods();

public:
    void set_text(const String &p_text);
    _FORCE_INLINE_ String get_text() const { return text; }
    void set_display_settings(Ref<TexterDisplaySettings> p_display_settings);
    _FORCE_INLINE_ Ref<TexterDisplaySettings> get_display_settings() const { return display_settings; }
    void set_type_settings(Ref<TexterTypeSettings> p_type_settings);
    _FORCE_INLINE_ Ref<TexterTypeSettings> get_type_settings() const { return type_settings; }
    void set_horizontal_alignment(const HorizontalAlignment &p_alignment);
    _FORCE_INLINE_ HorizontalAlignment get_horizontal_alignment() const { return horizontal_alignment; }
    void set_vertical_alignment(const VerticalAlignment &p_alignment);
    _FORCE_INLINE_ VerticalAlignment get_vertical_alignment() const { return vertical_alignment; }
    void set_autowrap(const bool &p_autowrap);
    _FORCE_INLINE_ bool get_autowrap() const { return autowrap; }
    void set_line_separation(const int &p_separation);
    _FORCE_INLINE_ int get_line_separation() const { return line_separation; }
    void set_always_draw(const bool &p_always_draw);
    _FORCE_INLINE_ bool get_always_draw() const { return always_draw; }

    _FORCE_INLINE_ void set_audio_player_path(const NodePath &p_path) { audio_player_path = p_path; }
    _FORCE_INLINE_ NodePath get_audio_player_path() const { return audio_player_path; }
};