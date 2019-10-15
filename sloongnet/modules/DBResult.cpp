#include "DBResult.h"

Sloong::CDBResult::CDBResult()
{
}

Sloong::CDBResult::~CDBResult()
{
}

int Sloong::CDBResult::GetColumnsNum()
{
    m_oColumns.size();
}

int Sloong::CDBResult::GetLinesNum()
{
    m_oDatas.size();
}

int Sloong::CDBResult::GetColumnIndex(const string& name)
{
    for( size_t i = 0; i< m_oColumns.size(); i++)
    {
        if( name.compare(m_oColumns[i]) == 0 )
            return i;
    }
    return -1;
}

string Sloong::CDBResult::GetColumnName(int index)
{
    return m_oColumns[index];
}


string Sloong::CDBResult::GetData( int lineIndex, const string& columnName )
{
    int columnIndex = GetColumnIndex(columnName);
    return GetData(lineIndex, columnIndex);
}


string Sloong::CDBResult::GetData( int lineIndex, int columnIndex )
{
    return m_oDatas[lineIndex]->at(columnIndex);
}

int Sloong::CDBResult::AppendColumn( string columnName )
{
    m_oColumns.push_back(columnName);
    return m_oColumns.size()-1;
}


void Sloong::CDBResult::SetItemData( int lineIndex, const string& columnName, const string& data )
{
    int columnIndex = GetColumnIndex(columnName);
    return SetItemData(lineIndex, columnIndex, data);
}


void Sloong::CDBResult::SetItemData( int lineIndex, int columnIndex, const string& data )
{
    m_oDatas[lineIndex]->at(columnIndex) = data;
}


int Sloong::CDBResult::AppendLine()
{
    auto newLine = make_shared<DBLine>();
    for( size_t i = 0 ; i < m_oColumns.size(); i++ )
        newLine->push_back("");
    m_oDatas.push_back(newLine);
    return m_oDatas.size()-1;
}