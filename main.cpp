#include "ListNode.hpp"
#include "List.hpp"
#include "ListSerializer.hpp"
#include <iostream>


int main(int argc, char** argv) {

	try
	{
		List list;

		ListSerializer::loadFromFile(list, "inlet.in");
		ListSerializer::serializeBinary(list, "output.out");

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