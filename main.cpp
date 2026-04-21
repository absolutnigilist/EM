#include "ListNode.hpp"
#include "List.hpp"
#include "ListSerializer.hpp"
#include <iostream>
#include <filesystem>
#include <glog/logging.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

//------------------------------------------------
//    Получить папку, где лежит exe
//------------------------------------------------
std::filesystem::path getExecutableDir()
{
#ifdef _WIN32
	std::wstring buffer(MAX_PATH, L'\0');
	DWORD len = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

	if (len == 0)
	{
		throw std::runtime_error("GetModuleFileNameW failed");
	}

	buffer.resize(len);
	return std::filesystem::path(buffer).parent_path();
#else
	char buffer[PATH_MAX]{};
	ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);

	if (len == -1)
	{
		throw std::runtime_error("readlink(/proc/self/exe) failed");
	}

	buffer[len] = '\0';
	return std::filesystem::path(buffer).parent_path();
#endif
}
//------------------------------------------------
//	Инициализация логирования в папке log рядом с exe
//------------------------------------------------
inline void initLogging(const std::filesystem::path& logDir) {

	std::error_code ec;
	std::filesystem::create_directories(logDir, ec);

	//---ВАЖНО: строки должны жить после выхода из функции (на всякий случай)
	static std::string info, warning, error, fatal;

	info = (logDir / "info").string();
	warning = (logDir / "warning").string();
	error = (logDir / "error").string();
	fatal = (logDir / "fatal").string();

	google::SetLogFilenameExtension(".txt");
	google::SetLogDestination(google::GLOG_INFO, info.c_str());
	google::SetLogDestination(google::GLOG_WARNING, warning.c_str());
	google::SetLogDestination(google::GLOG_ERROR, error.c_str());
	google::SetLogDestination(google::GLOG_FATAL, fatal.c_str());

	google::InitGoogleLogging(APP_NAME);
}

int main(int argc, char** argv) {

	(void)argc;
	(void)argv;

	//---Определяем пути
	const std::filesystem::path exeDir = getExecutableDir();
	const std::filesystem::path logDir = exeDir / "log";
	
	//---Инициализация логирования
	initLogging(logDir);

	try
	{
		//---Создаём пустой список, загружаем список из текстового файла "inlet.in" и сериализуем список в бинарный файл "output.out"
		List list;	
		ListSerializer::loadFromFile(list, "inlet.in");
		ListSerializer::serializeBinary(list, "output.out");

		//---Создаём новый пустой список и десериализуем его из бинарного файла
		List restored;
		ListSerializer::deserializeBinary(restored, "output.out");
		
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}