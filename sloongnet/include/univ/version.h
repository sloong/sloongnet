#ifndef VERSION_H
#define VERSION_H


// Version
#define VERSION_NUMBER						2,5,2,255
#define VERSION_FILEVERSION					L"2.5.2.255"
#define VERSION_BUILDTIME					L"2020/01/21"

#ifdef _DEBUG
#define VERSION_PRODUCTVERSION 				L"Ver.2.5 for Debug"
#define	VERSION_FILEDESCRIPTION				L"Sloong Universal Debug Libaray"
#define	VERSION_PRODUCTNAME					L"Sloong Universal Debug Libaray"
#else
#define VERSION_PRODUCTVERSION 				L"Ver.2.5")
#define	VERSION_FILEDESCRIPTION				L"Sloong Universal Libaray"
#define	VERSION_PRODUCTNAME					L"Sloong Universal Libaray"
#endif // _DEBUG
#define VERSION_INTERNALNAME				L"Universal.dll")
#define VERSION_COMPANYNAME 				L"Sloong, Inc.")
#define	VERSION_LEGALCOPYRIGHT				L"Copyright (C) 2014-2020 Sloong, Inc."


#endif // !VERSION_H
