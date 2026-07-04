#include "utility.h"
#include "core/io/dir_access.h"
#include "core/io/resource_loader.h"
#include "core/object/class_db.h"
#include "core/variant/variant.h"

LGTUtility *LGTUtility::singleton = nullptr;

void LGTUtility::_bind_methods()
{
    BIND_ENUM_CONSTANT(CHECK_NONE);
    BIND_ENUM_CONSTANT(CHECK_EQUAL);
    BIND_ENUM_CONSTANT(CHECK_NOT_EQUAL);
    BIND_ENUM_CONSTANT(CHECK_GREATER_THAN);
    BIND_ENUM_CONSTANT(CHECK_GREATER_THAN_OR_EQUAL);
    BIND_ENUM_CONSTANT(CHECK_LESS_THAN);
    BIND_ENUM_CONSTANT(CHECK_LESS_THAN_OR_EQUAL);
    BIND_ENUM_CONSTANT(CHECK_MAX);

    BIND_ENUM_CONSTANT(OPERATION_NONE);
    BIND_ENUM_CONSTANT(OPERATION_ADD);
    BIND_ENUM_CONSTANT(OPERATION_SUBTRACT);
    BIND_ENUM_CONSTANT(OPERATION_MULTIPLY);
    BIND_ENUM_CONSTANT(OPERATION_DIVIDE);
    BIND_ENUM_CONSTANT(OPERATION_POWER);
    BIND_ENUM_CONSTANT(OPERATION_MODULE);
    BIND_ENUM_CONSTANT(OPERATION_MATH_MIN);
    BIND_ENUM_CONSTANT(OPERATION_MATH_MAX);
    BIND_ENUM_CONSTANT(OPERATION_BIT_SHIFT_LEFT);
    BIND_ENUM_CONSTANT(OPERATION_BIT_SHIFT_RIGHT);
    BIND_ENUM_CONSTANT(OPERATION_BIT_AND);
    BIND_ENUM_CONSTANT(OPERATION_BIT_OR);
    BIND_ENUM_CONSTANT(OPERATION_BIT_XOR);
    BIND_ENUM_CONSTANT(OPERATION_MAX);
}

void LGTUtility::collect_scripts_from_dir(const String &p_path, AHashMap<StringName, Ref<Script>> *r_ret)
{
    if (p_path.is_empty()) { return; }

    Error err;
    Ref<DirAccess> dir = DirAccess::open(p_path, &err);

    if (err != OK)
    {
        ERR_FAIL_MSG(vformat("Couldn't open directory: %s", p_path));
        return;
    }

    dir->list_dir_begin();
    String path = dir->get_next();
    while (not path.is_empty())
    {
        String full_path = p_path.path_join(path);
        if (dir->current_is_dir() && path != "." && path != ".." && path != "./")
        {
            collect_scripts_from_dir(full_path, r_ret);
        }
        else
        {
            String script_name = path.get_file().get_basename().trim_prefix("res://");

            if (r_ret->has(path))
            {
                int i = 0;
                for (const KeyValue<StringName, Ref<Script>> &KV : *r_ret)
                {
                    String included_script_name = KV.key;
                    included_script_name = included_script_name.get_file().get_basename().trim_prefix("res://");
                    if (script_name.contains(included_script_name))
                    {
                        i++;
                    }
                }
                script_name += vformat("_%d", i);
            }

            if (path.get_extension() == "gd" and not r_ret->has(script_name))
            {
                Ref<Script> scr = ResourceLoader::load(p_path.path_join(path));
                if (scr.is_valid())
                {
                    StringName base_type = scr->get_instance_base_type();
                    if (base_type == StringName())
                    {
                        scr->reload(true);
                        base_type = scr->get_instance_base_type();
                    }
                    if (base_type != StringName())
                    {
                        r_ret->insert(script_name, scr);
                    }
                }
            }
        }
        path = dir->get_next();
    }
    dir->list_dir_end();
}

bool LGTUtility::perform_check(const Check &p_check, const Variant &p_a, const Variant &p_b)
{
    switch (p_check)
    {
        case CHECK_NONE:
            return true;
		case CHECK_EQUAL:
            return Variant::evaluate(Variant::OP_EQUAL, p_a, p_b);
		case CHECK_NOT_EQUAL:
            return Variant::evaluate(Variant::OP_NOT_EQUAL, p_a, p_b);
		case CHECK_GREATER_THAN:
            return Variant::evaluate(Variant::OP_GREATER, p_a, p_b);
		case CHECK_GREATER_THAN_OR_EQUAL:
            return Variant::evaluate(Variant::OP_GREATER_EQUAL, p_a, p_b);
		case CHECK_LESS_THAN:
            return Variant::evaluate(Variant::OP_LESS, p_a, p_b);
		case CHECK_LESS_THAN_OR_EQUAL:
            return Variant::evaluate(Variant::OP_LESS_EQUAL, p_a, p_b);
		default:
            return false;
	}
}

Variant LGTUtility::perform_operation(const Operation &p_operation, const Variant &p_a, const Variant &p_b) const
{
    switch (p_operation)
    {
		case OPERATION_NONE:
            return p_b;
		case OPERATION_ADD:
            return Variant::evaluate(Variant::OP_ADD, p_a, p_b);
		case OPERATION_SUBTRACT:
            return Variant::evaluate(Variant::OP_SUBTRACT, p_a, p_b);
		case OPERATION_MULTIPLY:
            return Variant::evaluate(Variant::OP_MULTIPLY, p_a, p_b);
		case OPERATION_DIVIDE:
            return Variant::evaluate(Variant::OP_DIVIDE, p_a, p_b);
		case OPERATION_POWER:
            return Variant::evaluate(Variant::OP_POWER, p_a, p_b);
		case OPERATION_MODULE:
            return Variant::evaluate(Variant::OP_MODULE, p_a, p_b);
		case OPERATION_BIT_SHIFT_LEFT:
            return Variant::evaluate(Variant::OP_SHIFT_LEFT, p_a, p_b);
		case OPERATION_BIT_SHIFT_RIGHT:
            return Variant::evaluate(Variant::OP_SHIFT_RIGHT, p_a, p_b);
		case OPERATION_BIT_AND:
            return Variant::evaluate(Variant::OP_BIT_AND, p_a, p_b);
		case OPERATION_BIT_OR:
            return Variant::evaluate(Variant::OP_BIT_OR, p_a, p_b);
		case OPERATION_BIT_XOR:
            return Variant::evaluate(Variant::OP_BIT_XOR, p_a, p_b);
		case OPERATION_IN:
            return Variant::evaluate(Variant::OP_IN, p_a, p_b);
		// case OPERATION_MATH_MIN:
        //     return MIN(p_a, p_b);
		// case OPERATION_MATH_MAX:
        //     return MAX(p_a, p_b);
		default:
			return Variant();
	}
}

LGTUtility::LGTUtility()
{
    singleton = this;
}

LGTUtility::~LGTUtility()
{
    singleton = nullptr;
}

