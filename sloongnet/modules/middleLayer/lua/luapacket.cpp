#include "stdafx.h"

#include "Lunar.h"
#include "luapacket.h"
#include <sstream>
#include "exception.h"
#include "log.h"
#include <iostream>

#define LOCK_GUARD(m) {lock_guard<mutex> lck(m);}


extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

using namespace Sloong::Universal;

#define PAI(L,n) (long)(lua_gettop(L) >= abs(n) && lua_isnumber(L,n) ? luaL_checknumber((L),(n)) : 0) //get number param

#define PAS(L,n) (lua_gettop(L) >= abs(n) && lua_isstring(L,n) ? luaL_checkstring((L),(n)) : "") // get string param

#define PASD(L,n,def) (lua_gettop(L) >= abs(n) && lua_isstring(L,n) ? luaL_checkstring((L),(n)) : def) // get string param

#define PAD(L,n) (double)(lua_gettop(L) >= abs(n) && lua_isnumber(L,n) ? luaL_checknumber((L),(n)) : 0.0f) // get float param

const char CLuaPacket::className[] =  "LuaPacket";

CLuaPacket::CLuaPacket()
{
	m_oChangeList = make_shared<vector<string>>();
}

CLuaPacket::CLuaPacket(lua_State* L)
{
}

CLuaPacket::~CLuaPacket()
{
}

int CLuaPacket::clear(lua_State *L)
{
    m_oDataMap.clear();
    return 0;
}

int CLuaPacket::setdata(lua_State *L)
{
    int nType = lua_type(L,2);
    if( nType == LUA_TUSERDATA || nType == LUA_TTABLE )
    {
        lua_pushboolean(L,false);
        return 1;
    }
    string key, value;
    if(lua_isnumber(L,1))
    {
        key = Helper::ntos(PAI(L,1));
    }
    else
    {
        key = PAS(L,1);
    }

    value = PAS(L,2);

    SetData(key,value);
    lua_pushboolean(L,true);
    return 1;
}

void CLuaPacket::SetData(const string& key, const string& value)
{
    if( key.empty() )
    {
        return;
    }

	bool bChanged = true;
	for (auto item = m_oChangeList->begin(); item != m_oChangeList->end(); item++) {
		if (key.compare(*item) == 0)
		{
			bChanged = false;
			break;
		}
	}

	if (bChanged)
		m_oChangeList->push_back(key);

    LOCK_GUARD(m_oMutex);
    m_oDataMap[key] = value;
}

int CLuaPacket::getdata(lua_State *L)
{
    string key;
    if(lua_isnumber(L,1))
    {
        key = Helper::ntos(PAI(L,1));
    }
    else
        key = string(PAS(L,1));

    
    if( !Exist(key) ) {
        if(lua_gettop(L) < 2 ){
            lua_pushnil(L);
        }else{
            lua_pushstring(L,PAS(L,2));
        }
    }else{
        auto value = m_oDataMap[key];
        lua_pushlstring(L,value.c_str(),value.length());
    }

    return 1;
}


string CLuaPacket::GetData(const string& key, const string& def )
{
    if( true == Exist(key) ){
        return m_oDataMap[key];
    }else{
       return def;
    }
}


shared_ptr<vector<string>> Sloong::Universal::CLuaPacket::GetChangedItems()
{
	return m_oChangeList;
}

bool CLuaPacket::Exist(const string& key)
{
    if( m_oDataMap.find(key) != m_oDataMap.end() ){
        return true;
    }else{
        return false;
    }
}



int Sloong::Universal::CLuaPacket::IsChanged()
{
	return m_oChangeList->size();
}

void Sloong::Universal::CLuaPacket::ConfirmChange()
{
	m_oChangeList->clear();
}

