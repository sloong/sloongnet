#ifndef SLOONGNET_DBRESULT_H
#define SLOONGNET_DBRESULT_H


#include "main.h"
namespace Sloong
{
    typedef vector<string> DBLine;
    class CDBResult
    {
    public:
        CDBResult();
        ~CDBResult();

        // Get Columns number
        int GetColumnsNum();
        // Get lines number
        int GetLinesNum();

        string GetColumnName( int index );
        string GetData( int lineIndex, const string& columnName );
        string GetData( int lineIndex, int columnIndex );
        int AppendColumn( string columnName );
        void SetItemData( int lineIndex, const string& columnName, const string& data );
        void SetItemData( int lineIndex, int columnIndex, const string& data );
        int AppendLine();
    protected:
        int GetColumnIndex(const string& name);
    protected:
        DBLine m_oColumns;
        vector<shared_ptr<DBLine>> m_oDatas;
    };

    typedef shared_ptr<CDBResult> EasyResult;
}


#endif
