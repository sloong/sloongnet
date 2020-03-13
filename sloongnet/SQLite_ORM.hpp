#include <string>
using namespace std;
#include "sqlite_orm/sqlite_orm.h"
using namespace sqlite_orm;

namespace Sloong
{
    const static string SERVER_TBL_NAME = "server_list";
    const static string TEMPLATE_TBL_NAME = "template_list";
    const static string CONFIGS_TBL_NAME = "configuations";

    struct ModuleInfo
    {
        string uuid;
        int template_id;
        int configuation_id;
        string name;
        string note;
    };
    struct TemplateInfo
    {
        int id;
        int configuation_id;
        string name;
        string note;
    };
    struct ConfigInfo
    {
        int id;
        string configuation;
    };

    inline auto InitStorage(const string& path) {
        using namespace sqlite_orm;
        return make_storage(path,
            make_table(SERVER_TBL_NAME,
                make_column("uuid", &ModuleInfo::uuid, primary_key()),
                make_column("template_id", &ModuleInfo::template_id, default_value(-1) ),
                make_column("configuation_id", &ModuleInfo::configuation_id, default_value(-1)),
                make_column("name", &ModuleInfo::name),
                make_column("note", &ModuleInfo::note)),
            make_table(TEMPLATE_TBL_NAME,
                make_column("id", &TemplateInfo::id, autoincrement(), primary_key()),
                make_column("configuation_id", &TemplateInfo::configuation_id),
                make_column("name", &TemplateInfo::name),
                make_column("note", &TemplateInfo::note)),
            make_table(CONFIGS_TBL_NAME,
                make_column("id", &ConfigInfo::id, autoincrement(), primary_key()),
                make_column("configuation", &ConfigInfo::configuation)));
    }

    using Storage = decltype(InitStorage(""));
}