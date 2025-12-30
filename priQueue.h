#pragma once
#include "priNode.h"
#include <iostream>
using namespace std;

template <typename T>
class priQueue
{
private:
    priNode<T>* head;   
	int count;         // number of elements in the priority queue
public:
    // Default Constructor
    priQueue() : head(nullptr), count(0) {}

	// Destructor to free all nodes in the priority queue without any memory leaks
    ~priQueue()
    {
		// dequeue all elements to free memory
        while (!isEmpty())
        {
			T dummy;      // variable وهمي للبيانات
			int p;        // variable وهمي للأولوية
            dequeue(dummy, p);
        }
    }

	// insert element based on its priority into the priority queue ( sorted insertion )
    void enqueue(const T& data, int priority)
    {
        count++;  // نزود العدد
		priNode<T>* newNode = new priNode<T>(data, priority);  // create new node

		// case1 : insert at the head if the queue is empty or the new node has higher priority than the head
        if (head == nullptr || priority > head->getPriority())
        {
            newNode->setNext(head);
            head = newNode;
            return;  
        }

		// case2 : reasearch the correct position to insert the new node
        priNode<T>* curr = head;
        while (curr->getNext() != nullptr && priority <= curr->getNext()->getPriority())
        {
            curr = curr->getNext();  
        }

		// insert the new node at the correct position
        newNode->setNext(curr->getNext());
        curr->setNext(newNode);
    }

	// delete  normal element with highest priority to return vip element
    bool dequeue(T& topEntry, int& priority)
    {
        if (isEmpty())
            return false; 

        topEntry = head->getItem();         
        priority = head->getPriority();     

		priNode<T>* temp = head;   
        head = head->getNext();    
        delete temp;               
		count--;                   // less one element in the queue
        return true;
    }

	// peek at the element with highest priority without removing it
    bool peek(T& topEntry, int& priority) const
    {
        if (isEmpty())
            return false;

        topEntry = head->getItem();
        priority = head->getPriority();
        return true;
    }

    
    bool isEmpty() const
    {
        return head == nullptr;
    }

	// return the size of the priority queue
    int getSize() const
    {
        return count;
    }

	// print the priority queue for debugging
    void print() const
    {
        if (isEmpty())
        {
            cout << "Empty Priority Queue\n";
            return;
        }

        cout << "Priority Queue: ";
        priNode<T>* curr = head;
        while (curr != nullptr)
        {
            cout << curr->getItem() << "(P:" << curr->getPriority() << ")";
            if (curr->getNext())
				cout << " <- ";  // arrow to next node to appear clearly the order from heighst priority to lowest
            curr = curr->getNext();
        }
        cout << "  [Head]\n";
    }
};