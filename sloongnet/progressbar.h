/**
 * linux terminal progress bar (no thread safe).
 *  @package progress.h.
 *
 * @author chenxin <chenxin619315@gmail.com>
 */
#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H
 
#include <stdio.h>
 

namespace Sloong
{
    typedef enum _PROGRESS_STYLE
    {
        Number,
        Chr,
        BackgroundColor,
    }PROGRESS_STYLE;

    class CProgressBar
    {
    public:
        CProgressBar(char *, int, PROGRESS_STYLE);
        ~CProgressBar();
        void Update( float );

    protected:
        PROGRESS_STYLE m_emStyle;
        int m_nMax;
        char* m_szTitle;
        char m_cChr;
        float m_fOffset;
        char* m_szPro;
    };
}

#endif  /*ifndef*/
