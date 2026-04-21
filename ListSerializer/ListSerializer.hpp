#pragma once

class List;

class ListSerializer
{
public:

	//---Метод чтения списка из текстового файла
	static void loadFromFile(List& list, const std::string& fileName);

	//---Метод сериализует список в бинарный файл
	static void serializeBinary(const List& list, const std::string& fileName);

	//---Метод десериализует список из бинарного файла
	static void deserializeBinary(List& list, const std::string& fileName);

};