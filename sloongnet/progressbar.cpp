/**
 * linux terminal progress bar (no thread safe).
 *  @package progress.c
 *
 * @author chenxin <chenxin619315@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <univ/defines.h>
 
#include "progressbar.h"
using namespace Sloong;

CProgressBar::CProgressBar( const char *title, int max, PROGRESS_STYLE style )
{
    m_cChr = '#';
    m_szTitle = title;
    m_emStyle = style;
    m_nMax = max;
    m_fOffset = 100.0f / (float)max;
    m_szPro = new char[max+1];

    if ( style == BackgroundColor )
        memset(m_szPro, 0, max+1);
    else
    {
        memset(m_szPro, 32, max);
        memset(m_szPro+max, 0x00, 1);
    }
}

CProgressBar::~CProgressBar()
{
    SAFE_DELETE_ARR(m_szPro);
}
 
void CProgressBar::Update( float bit )
{
    int val = (int)(bit * m_nMax);
    switch ( m_emStyle )
    {
        case Number:
            printf("\033[?25l\033[31m\033[1m%s%d%%\033[?25h\033[0m\r",
                m_szTitle, (int)(m_fOffset * val));
            fflush(stdout);
            break;
        case Chr:
            memset(m_szPro, '#', val);
            printf("\033[?25l\033[31m\033[1m%s[%-s] %d%%\033[?25h\033[0m\r",
                m_szTitle, m_szPro, (int)(m_fOffset * val));
            fflush(stdout);
            break;
        case BackgroundColor:
            memset(m_szPro, 32, val);
            printf("\033[?25l\033[31m\033[1m%s\033[41m %d%% %s\033[?25h\033[0m\r",
                m_szTitle, (int)(m_fOffset * val), m_szPro);
            fflush(stdout);
            break;
    }
}
