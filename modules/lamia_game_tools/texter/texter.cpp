#include "texter.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/general/defs.h"


void TexterDisplaySettings::_bind_methods()
{
    BIND(D_METHOD("set_font", "font"), &TexterDisplaySettings::set_font);
    BIND(D_METHOD("get_font"), &TexterDisplaySettings::get_font);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "font", PROPERTY_HINT_RESOURCE_TYPE, "Font"), "set_font", "get_font");

    BIND(D_METHOD("set_font_size", "font_size"), &TexterDisplaySettings::set_font_size);
    BIND(D_METHOD("get_font_size"), &TexterDisplaySettings::get_font_size);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "font_size"), "set_font_size", "get_font_size");
   
    BIND(D_METHOD("set_font_color", "font_color"), &TexterDisplaySettings::set_font_color);
    BIND(D_METHOD("get_font_color"), &TexterDisplaySettings::get_font_color);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "font_color"), "set_font_color", "get_font_color");
    
    BIND(D_METHOD("set_type_speed_override", "font"), &TexterDisplaySettings::set_type_speed_override);
    BIND(D_METHOD("get_type_speed_override"), &TexterDisplaySettings::get_type_speed_override);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "font_type_speed_override"), "set_type_speed_override", "get_type_speed_override");
    
    BIND(D_METHOD("set_font", "font"), &TexterDisplaySettings::set_font);
    BIND(D_METHOD("get_font"), &TexterDisplaySettings::get_font);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "font_size"), "set_font_size", "get_font_size");
    
    BIND(D_METHOD("set_font", "font"), &TexterDisplaySettings::set_font);
    BIND(D_METHOD("get_font"), &TexterDisplaySettings::get_font);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "font_size"), "set_font_size", "get_font_size");
    
    BIND(D_METHOD("set_font", "font"), &TexterDisplaySettings::set_font);
    BIND(D_METHOD("get_font"), &TexterDisplaySettings::get_font);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "font_size"), "set_font_size", "get_font_size");
    
    BIND(D_METHOD("set_font", "font"), &TexterDisplaySettings::set_font);
    BIND(D_METHOD("get_font"), &TexterDisplaySettings::get_font);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "font_size"), "set_font_size", "get_font_size");
    
}


void Texter::_bind_methods()
{
    BIND(D_METHOD("set_audio_player_path", "path"), &Texter::set_audio_player_path);
    BIND(D_METHOD("get_audio_player_path"), &Texter::get_audio_player_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "audio_player_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "AudioStreamPlayer,AudioStreamPlayer2D,AudioStreamPlayer3D"), "set_audio_player_path", "get_audio_player_path");
}
