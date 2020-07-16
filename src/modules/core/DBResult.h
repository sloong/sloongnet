/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-17 15:55:13
 * @Description: file content
 */
#ifndef SLOONGNET_DBRESULT_H
#define SLOONGNET_DBRESULT_H


#include "core.h"
namespace Sloong
{
    typedef vector<string> DBLine;
    #define TRUE_STRING "true"
    #define FALSE_STRING "false"
    class DBResult
    {
    public:
        DBResult();
        ~DBResult();

        // Get Columns number
        int GetColumnsNum();
        // Get lines number
        int GetLinesNum();

        string GetColumnName( int index );
        string GetData( int lineIndex, const string& columeName );
        string GetData( int lineIndex, int columnIndex );

        inline string GetString(const string& columeName , string def, int lineIndex = 0){
            if( GetLinesNum() == 0 ) return def;
            return GetData(lineIndex,columeName);
        }
        inline int GetInt( const string& columeName, int def, int lineIndex = 0 ){
            if( GetLinesNum() == 0 ) return def;
            return stoi(GetData(lineIndex,columeName));
        }
        inline bool GetBoolean( const string& columeName, bool def, int lineIndex = 0){
            if( GetLinesNum() == 0 ) return def;
            auto res = GetData(lineIndex,columeName);
            if (res.compare(TRUE_STRING) == 0)
                return true;
            else if (res.compare(FALSE_STRING) == 0)
                return false;
        }

        int AppendColumn( string columnName );
        void SetItemData( int lineIndex, const string& columnName, const string& data );
        void SetItemData( int lineIndex, int columnIndex, const string& data );
        void SetBoolean( int lineIndex, const string& columnName, bool data ){
            if( data == true )
                SetItemData( lineIndex, columnName, TRUE_STRING );
            else
                SetItemData( lineIndex, columnName, FALSE_STRING );
        }
        int AppendLine();
    protected:
        int GetColumnIndex(const string& name);
    protected:
        DBLine m_oColumns;
        vector<shared_ptr<DBLine>> m_oDatas;
    };

    typedef shared_ptr<DBResult> EasyResult;
}


#endif
