/*
 * @Author: WCB
 * @Date: 2020-04-21 19:41:30
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-28 20:23:39
 * @Description: file content
 */
#include <string>
using namespace std;
#include <sqlite_orm.h>
using namespace sqlite_orm;

namespace Sloong
{
    const static string TEMPLATE_TBL_NAME = "template_list";

    struct TemplateInfo
    {
        int id;
        int replicas;
        string configuation;
        string name;
        string note;
        string reference;
    };

    inline auto InitStorage(const string& path) {
        using namespace sqlite_orm;
        return make_storage(path,
            make_table(TEMPLATE_TBL_NAME,
                make_column("id", &TemplateInfo::id, autoincrement(), primary_key()),
                make_column("replicas", &TemplateInfo::replicas),
                make_column("configuation", &TemplateInfo::configuation),
                make_column("name", &TemplateInfo::name),
                make_column("note", &TemplateInfo::note),
                make_column("reference", &TemplateInfo::reference))
        );
    }

    using Storage = decltype(InitStorage(""));
}