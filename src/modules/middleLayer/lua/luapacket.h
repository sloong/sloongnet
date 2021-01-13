/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-30 11:27:49
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/middleLayer/lua/luapacket.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#pragma once

#include "core.h"

struct lua_State;
namespace Sloong
{
    class CLuaPacket
    {
    public:
        static const char className[];

    public:
        CLuaPacket();
        CLuaPacket(lua_State *L);
        virtual ~CLuaPacket();

        int clear(lua_State *L);
        int setdata(lua_State *L);
        int getdata(lua_State *L);

        void SetData(const string &key, const string &value);
        string GetData(const string &key, const string &def);

        /**
             * @Description: Check the map object is changed after the Initialize with ParseFromString function.
             * @Params: No
             * @Return: if no changed, return 0. else return the changed count.
             */
        int IsChanged();

        /**
             * @Description: Confirm the modify. user need call the SerializeToString function to get the data and save it.
             * @Params: No
             * @Return: No
             */
        void ConfirmChange();

        /**
             * @Description: Get changed items array. 
             * @Params: No
             * @Return: the array for changed items key.
             */
        shared_ptr<vector<string>> GetChangedItems();

    protected:
        bool Exist(const string &key);

    private:
        mutex m_oMutex;
        map<string, string> m_oDataMap;
        shared_ptr<vector<string>> m_oChangeList;
    };

} // namespace Sloong
