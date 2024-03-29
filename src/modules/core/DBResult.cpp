/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2019-11-05 08:59:19
 * @LastEditTime: 2021-09-02 17:26:15
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/DBResult.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
/*** 
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

#include "DBResult.h"

Sloong::DBResult::DBResult()
{
}

Sloong::DBResult::~DBResult()
{
}

int Sloong::DBResult::GetColumnsNum()
{
    return (int)m_oColumns.size();
}

int Sloong::DBResult::GetLinesNum()
{
    return (int)m_oDatas.size();
}

int Sloong::DBResult::GetColumnIndex(const string &name)
{
    for (size_t i = 0; i < m_oColumns.size(); i++)
    {
        if (name.compare(m_oColumns[i]) == 0)
            return (int)i;
    }
    return -1;
}

string Sloong::DBResult::GetColumnName(int index)
{
    return m_oColumns[index];
}

string Sloong::DBResult::GetData(int lineIndex, const string &columnName)
{
    int columnIndex = GetColumnIndex(columnName);
    return GetData(lineIndex, columnIndex);
}

string Sloong::DBResult::GetData(int lineIndex, int columnIndex)
{
    return m_oDatas[lineIndex]->at(columnIndex);
}

void Sloong::DBResult::SetItemData(int lineIndex, const string &columnName, const string &data)
{
    int columnIndex = GetColumnIndex(columnName);
    return SetItemData(lineIndex, columnIndex, data);
}

void Sloong::DBResult::SetItemData(int lineIndex, int columnIndex, const string &data)
{
    m_oDatas[lineIndex]->at(columnIndex) = data;
}

int Sloong::DBResult::AppendColumn(string columnName)
{
    m_oColumns.emplace_back(columnName);
    return (int)m_oColumns.size() - 1;
}

int Sloong::DBResult::AppendLine()
{
    auto newLine = make_shared<DBLine>();
    for (size_t i = 0; i < m_oColumns.size(); i++)
        newLine->push_back("");
    m_oDatas.emplace_back(newLine);
    return (int)m_oDatas.size() - 1;
}