#include<windows.h>
#include<iostream>
#include<cstdio>
#include<direct.h>
using namespace std;
//int FileCopy(char *src, char *dst) {
//	printf("%s %s\n", src, dst);
//	FILE *fp1, *fp2;
//	int c;
//	fp1 = fopen(src, "rb");
//	fp2 = fopen(dst, "wb");//打开文件  
//	if (fp1 == NULL || fp2 == NULL)//错误处理  
//	{
//		printf("open file failed\n");
//		if (fp1) fclose(fp1);
//		if (fp2) fclose(fp2);
//	}
//
//	while ((c = fgetc(fp1)) != EOF)//读字节  
//		fputc(c, fp2); //写数据  
//	fclose(fp1);//关闭文件  
//	fclose(fp2);
//	return 0;
//}
//不调用系统api复制
int _GetFileTime(char *szFileName,char *szOutFile) {	//读取源文件信息修改目标文件的信息
	printf("getfiletime\n");
	SYSTEMTIME st_systemTime;
	FILETIME ft_localTime;
	FILETIME lpCreationTime;
	FILETIME lpLastAccessTime;
	FILETIME lpLastWriteTime;
	long retval;
	TCHAR temp[1000],temp2[1000];
	MultiByteToWideChar(CP_UTF8, 0, szFileName, 1000, temp, MAX_PATH);
	MultiByteToWideChar(CP_UTF8, 0, szOutFile, 1000, temp2, MAX_PATH);
	HANDLE hSource = CreateFile(temp, GENERIC_READ,//只读方式
		FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,//打开现存目录
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);
	HANDLE hOut = CreateFile(temp2, GENERIC_READ|GENERIC_WRITE,//读写方式打开
		FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,//打开现存目录
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);
	if (hSource == INVALID_HANDLE_VALUE || hOut == INVALID_HANDLE_VALUE) {
		printf("can not open\n");
	}
	retval = GetFileTime(hSource, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);//获取文件时间
	if (retval) {
		FileTimeToLocalFileTime(&lpCreationTime, &ft_localTime);
		FileTimeToSystemTime(&ft_localTime, &st_systemTime);
		retval = SetFileTime(hOut, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);//设置文件时间
	}
	return 0;
}
int FileCopy(char *source, char *out) {
	WIN32_FIND_DATA wfhd;
	TCHAR temp1[1000];
	TCHAR temp2[1000];
	MultiByteToWideChar(CP_UTF8, 0, source, 1000, temp1, MAX_PATH);
	HANDLE hSourceFile = CreateFile(temp1, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//打开源文件
	MultiByteToWideChar(CP_UTF8, 0, out, 1000, temp2, MAX_PATH);
	printf("%s %s\n", source, out);
	HANDLE hOutFile = CreateFile(temp2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);//打开目标文件
	FindFirstFile(temp1, &wfhd);
	SetFileTime(hOutFile, &(wfhd.ftCreationTime), &(wfhd.ftLastAccessTime), &(wfhd.ftLastWriteTime));//设置复制的目标文件时间
	if (hSourceFile == INVALID_HANDLE_VALUE || hOutFile == INVALID_HANDLE_VALUE) {
		DWORD dwRemainSize = GetFileSize(temp1, NULL);
		printf("Can't copy %s this file to %s\n", source,out);
		return 0;
	}
	printf("%d %d %d %d\n", strlen(source), sizeof(source), strlen(out), sizeof(out));
	LARGE_INTEGER psi;
	GetFileSizeEx(hSourceFile, &psi);//读取源文件大小
	char buffer[1024];
	LONGLONG tempsize = psi.QuadPart;
	while (tempsize > 0) {//按kb拷贝数据
		DWORD dwActualRead = 0;
		ReadFile(hSourceFile, buffer, 1024, &dwActualRead, NULL);
		tempsize -= dwActualRead;
		DWORD dwActualWrote = 0;
		DWORD dwOnceWrote = 0;
		WriteFile(hOutFile, buffer, dwActualRead, &dwOnceWrote, NULL);
//		printf("%u %u\n", dwActualRead, tempsize);
	}
	CloseHandle(hSourceFile);
	CloseHandle(hOutFile);
}
int makedir(char *path,char *File) {//建立文件夹
	int result = 0;
	WIN32_FIND_DATA wfd;
	DWORD last_error = 0;
	char tt[1000];
	memset(tt, 0, sizeof(tt));
	sprintf(tt,"%s\\*.*", path);
	TCHAR Temp[1000],Temp2[1000];
	MultiByteToWideChar(CP_UTF8, 0,File, 1000, Temp, 256);
	MultiByteToWideChar(CP_UTF8, 0, tt, sizeof(tt), Temp2, 256);
	result = _mkdir(File);
	return result;
}
bool Search(char *Path, char *File, WIN32_FIND_DATA *info) {
	HANDLE hFind,hCopy;
	WIN32_FIND_DATA wfd,wfd2;
	ZeroMemory(&wfd, sizeof(wfd));
	char PathTemp[1000],PathTemp2[1000];
	memset(PathTemp, 0, sizeof(PathTemp));
	memset(PathTemp2, 0, sizeof(PathTemp2));
	sprintf(PathTemp, "%s\\*.*", Path);
	sprintf(PathTemp2, "%s\\*.*", File);
	TCHAR Temp[1000],Temp2[1000],Temp3[1000];
	MultiByteToWideChar(CP_UTF8, 0, PathTemp, sizeof(PathTemp), Temp, 256);
	MultiByteToWideChar(CP_UTF8, 0, PathTemp2, sizeof(PathTemp2), Temp2, 256);
	MultiByteToWideChar(CP_UTF8, 0, Path, 1000, Temp3, MAX_PATH);
	if ( (hFind = CreateFile(Temp3, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) )!= INVALID_HANDLE_VALUE) {
		CloseHandle(hFind);
		FileCopy(Path, File);
		return 0;
	}
	CloseHandle(hFind);
	hFind = FindFirstFile(Temp, &wfd);//读取源文件夹内第一个文件
	hCopy = FindFirstFile(Temp2, &wfd2);//读取目标文件夹的第一个文件
	if (INVALID_HANDLE_VALUE == hFind) {
		printf("Wrong file position\n");
		return 0;
	}
	if (INVALID_HANDLE_VALUE == hCopy) {
		printf("create dir %s \n", File);
		makedir(Path,File);
	}
	while (TRUE) {
		if (wfd.cFileName[0] != '.') {
			printf("now is %s\\%ws\n", Path, wfd.cFileName);
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {//轮询文件目录
				memset(PathTemp, 0, sizeof(PathTemp));
				strcpy(PathTemp, Path);
				strcat(PathTemp, "\\");
				char temp[1000];
				sprintf(temp, "%ws", wfd.cFileName);
				strcat(PathTemp, temp);
				char makedirpath[1000];
				memset(makedirpath, 0, sizeof(makedirpath));
				strcpy(makedirpath, File);
				strcat(makedirpath, "\\");
				sprintf(temp, "%ws", wfd.cFileName);
				strcat(makedirpath, temp);
				Search(PathTemp, makedirpath, &wfd);//递归复制文件夹
				_GetFileTime(Path, File);
			}
			else {
				memset(PathTemp, 0, sizeof(PathTemp));
				strcpy(PathTemp, Path);
				strcat(PathTemp, "\\");
				char temp[1000];
				sprintf(temp, "%ws", wfd.cFileName);
				strcat(PathTemp, temp);
				char makedirpath[1000];
				memset(makedirpath, 0, sizeof(makedirpath));
				strcpy(makedirpath, File);
				strcat(makedirpath, "\\");
				sprintf(temp, "%ws", wfd.cFileName);
				strcat(makedirpath, temp);
				FileCopy(PathTemp, makedirpath);//复制文件
			}
		}
		_GetFileTime(Path, File);//复制完成后写入文件信息
		BOOL flag = FindNextFile(hFind, &wfd);//指向下个文件
		if (!flag) {
			printf("Find next file failed error code: %d\n", GetLastError());
			break;
		}
	}
	FindClose(hFind);
}

int main(int argc, char ** argv) {
	char Sourcefile[1000];
	char Outfile[1000];
	strcpy(Sourcefile, argv[1]);
	strcpy(Outfile, argv[2]);
	Search(Sourcefile, Outfile, NULL);
	system("pause");
}