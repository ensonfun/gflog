// test_gflog.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../gflog/include/logging.h"
#include <atlpath.h>
#include <WinBase.h>

int _tmain(int argc, _TCHAR* argv[])
{
	fprintf(stderr, "Hello, World!\n");

	wchar_t module_file_name[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, module_file_name, MAX_PATH);

	CPath log_file(module_file_name);
	log_file.RenameExtension(L".log");

	logging::LoggingSettings settings;
	settings.log_file = log_file.m_strPath;
	
	logging::InitLogging(settings);
	logging::SetLogItems(true, true, true, false);

	LOG(INFO) << "multi byte log.";
	LOG(INFO) << L"wide char log.";

	LOG_IF(INFO, true) << "LOG_IF log.";

	// 如果条件不成立，会打印日志，并且触发中断，并不是崩溃哦
	// 类似的还有CHECK等
	char *p = NULL;
	CHECK(p);
	CHECK_EQ(1, 100);
	return 0;
}

