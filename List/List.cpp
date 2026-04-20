#include "List.hpp"
//------------------------------------------------
//	Деструктор класса
//------------------------------------------------
List::~List() {
	clear();
}
//------------------------------------------------
//	Метод очистки списка
//------------------------------------------------
void List::clear() {

	ListNode* current = head;
	while (current != nullptr)
	{
		ListNode* next = current->next;
		delete current;
		current = next;
	}
	head = nullptr;
	tail = nullptr;
	count = 0;
}
//------------------------------------------------
//	Метод получения указателя на начало
//------------------------------------------------
ListNode* List::getHead() const {
	return head;
}
//------------------------------------------------
//	Метод получения указателя на конец
//------------------------------------------------
ListNode* List::getTail() const {
	return tail;
}
//------------------------------------------------
//	Метод получения количества элементов в списке
//------------------------------------------------
std::size_t List::getCount() const {
	return count;
}
//------------------------------------------------
//	Метод добавления элемента в конец списка
//------------------------------------------------
ListNode* List::pushBack(const std::string& data) {
	
	//---Создаём новый узел
	ListNode* newNode = new ListNode();
	newNode->data = data;
	newNode->next = nullptr;
	newNode->prev = tail;
	newNode->rand = nullptr;

	if (tail != nullptr)
	{
		//---Добавляем новый узел в конец списка
		tail->next = newNode;
	}
	else
	{
		//---Список был пуст, новый узел становится началом списка
		head = newNode;
	}
	//---Новый узел становится концом списка
	tail = newNode;
	
	//---Увеличиваем количество элементов в списке
	++count;

	return newNode;
}
//------------------------------------------------
//	Метод добавления элемента в начало списка
//------------------------------------------------
ListNode* List::pushBegin(const std::string& data) {

	//---Создаём новый узел
	ListNode* newNode = new ListNode();

	newNode->data = data;
	newNode->next = head;
	newNode->prev = nullptr;
	newNode->rand = nullptr;

	if (head ==nullptr)
	{
		//---Список был пуст, новый узел становится концом списка
		tail = newNode;
	}
	else
	{
		//---Добавляем новый узел в начало списка
		head->prev = newNode;
	}
	
	//---Новый узел становится началом списка
	head = newNode;

	//---Увеличиваем количество элементов в списке
	++count;

	return newNode;
}
//------------------------------------------------
//	Метод вставки элемента после указанного узла
//------------------------------------------------
ListNode* List::insertAfter(ListNode* node, const std::string& data) {

	//---Создаём новый узел
	ListNode* newNode = new ListNode();
	newNode->data = data;
	newNode->rand = nullptr;
	
	//---Если node не равен nullptr, вставляем новый узел после node
	if (node != nullptr)
	{
		//---Вставка после node
		newNode->prev = node;
		newNode->next = node->next;

		if (node->next != nullptr)
		{
			node->next->prev = newNode;
		}
		else
		{
			tail = newNode;
		}
		node->next = newNode;
	}
	//---Если node равен nullptr, вставляем новый узел в начало списка
	else
	{
		//---Вставка в начало списка
		newNode->prev = nullptr;
		newNode->next = head;

		if (head != nullptr)
		{
			head->prev = newNode;
		}
		else
		{
			tail = newNode;
		}
		head = newNode;
	}
	++count;
	return newNode;
}
//------------------------------------------------
//	Метод получения указателя на элемент по индексу
//------------------------------------------------
ListNode* getNodeAt(std::size_t index) const {
	
	//---Проверяем, что индекс не превышает количество элементов в списке
	if (index >= count) {
		return nullptr;
	}

	//---Ищем узел по индексу, начиная с головы списка
	ListNode* current = head;
	for (std::size_t i = 0; i < index && current != nullptr; ++i)
	{
		current = current->next;
	}
	return current;
}