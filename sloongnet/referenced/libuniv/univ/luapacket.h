#ifndef LUAPACKET_H
#define LUAPACKET_H


#include "univ.h"
#include <mutex>
#include<map>
using std::mutex;
using std::map;
struct lua_State;
namespace Sloong
{
    namespace Universal
    {
        class UNIVERSAL_API CLuaPacket
        {
        public:
            static const char className[];
        public:
            CLuaPacket();
            CLuaPacket(lua_State* L);
            virtual ~CLuaPacket();

            int clear(lua_State* L);
            int setdata(lua_State* L);
            int getdata(lua_State* L);

            void SetData(string key, string value);
            string GetData(string key, string def);

            string SerializeToString();
            void ParseFromString(string& str );

            /**
             * @Remarks: Check the map object is changed after the Initialize with ParseFromString function.
             * @Params: No
             * @Return: if no changed, return 0. else return the changed count.
             */
            int IsChanged();

            /**
             * @Remarks: Confirm the modify. user need call the SerializeToString function to get the data and save it.
             * @Params: No
             * @Return: No
             */
            void ConfirmChange();

            /**
             * @Remarks: Get changed items array. 
             * @Params: No
             * @Return: the array for changed items key.
             */
			shared_ptr<vector<string>> GetChangedItems();

        protected:
            bool Exist(string key);

        private:
            mutex	m_oMutex;
            map<string,string> m_oDataMap;
            shared_ptr<vector<string>>  m_oChangeList;
        };

    }
}

#endif // LUAPACKET_H
