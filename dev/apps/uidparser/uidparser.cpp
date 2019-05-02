// uidparser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "uidfile.h"

/*****************************************************************************/

CString g_strFile;
tagLanguage		g_lang = lCSharp;

void Usage()
{
    printf("\n");
    printf("USAGE: UDAParts parser for interface definition file\n");
	printf("\n");
	printf("SAMPLE: uidparser -Fc:\\SomeDirectory\\somefile.uid -L1\n");
    printf("\n");
    printf("-F<File Name> A fullpath uid file name\n");
    printf("-L<Language>  Development Language\n");
	printf("              0 or default = C#\n");
    printf("              1 = C++\n");
    printf("              2 = VB.NET\n");
//	printf("              3 = C++/CLI\n");
	printf("\n");
	printf("NOTE: A valid uid file must be specified!\n");
}

int _tmain(int argc, _TCHAR* argv[])
{
	CString str;
	int i;
	int iOption;
	char *pszOption;
	bool bSuc;
	
	if(argc <= 1)
	{
		Usage();
		exit(1);
	}

	for(i = 1; i < argc; i++) 
    {
        if(argv[i][0] == '/') argv[i][0] = '-';

        if(argv[i][0] != '-') 
        {
            printf("**** Invalid argument \"%s\"\n", argv[i]);
            Usage();
        }

        iOption = argv[i][1];
        pszOption = &argv[i][2];

        switch(iOption) 
        {
		case '?':
		case 'H':
		case 'h':
			{
				Usage();
				exit(1);
			}
			break;
        case 'F':
            g_strFile = pszOption;
            break;
        case 'L':
			g_lang = (tagLanguage)atoi(pszOption);
			if(g_lang > lVBNet)
			{
				g_lang = lCSharp;
			}
			break;
        default:
			break;
        }
    }
	
	CString strExt = g_strFile.Right(4);
	strExt.MakeLower();
	if(strExt != ".uid")
	{
		printf("No valid uid file given!");
		exit(1);
	}
	
	CUidFile	UidFile(g_lang);
	bSuc = UidFile.SetInputFile(g_strFile);
	if(bSuc)
	{
		bSuc = UidFile.GenerateFiles();
		printf(" *****  Successfully parsed! ***** \n");
	}
	else
	{
		printf("\n");
		printf(UidFile.GetErrorMessage());
		printf("!\n");
	}

	return 0;
}

