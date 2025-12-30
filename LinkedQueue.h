#pragma once
#ifndef LNKDQU
#define LNKDQU
#include "Generic_DS/Node.h"// Node<T> must have: T getItem(), void setNext(Node*), Node<T>* getNext()
#include <stdexcept>
#include <iostream>

template <typename T>
class LinkedQueue
{
private:
    Node<T>* front;
    Node<T>* back;
    int count;        // Number of elements

public:

    LinkedQueue() : front(nullptr), back(nullptr), count(0) {}


    ~LinkedQueue()
    {
        clear();
    }

    // Delete copy operations (we own raw pointers)
    LinkedQueue(const LinkedQueue&) = delete;
    LinkedQueue& operator=(const LinkedQueue&) = delete;

    // Move constructor
    LinkedQueue(LinkedQueue&& other) noexcept
        : front(other.front), back(other.back), count(other.count)
    {
        other.front = nullptr;
        other.back = nullptr;
        other.count = 0;
    }

    // Move assignment
    LinkedQueue& operator=(LinkedQueue&& other) noexcept
    {
        if (this != &other)
        {
            clear();
            front = other.front;
            back = other.back;
            count = other.count;

            other.front = nullptr;
            other.back = nullptr;
            other.count = 0;
        }
        return *this;
    }

    // Add item to the back (FIFO)
    void enqueue(const T& item)
    {
        Node<T>* newNode = new Node<T>(item);
        count++;

        if (isEmpty())
            front = back = newNode;
        else
        {
            back->setNext(newNode);
            back = newNode;
        }
    }

    // Remove and return front item - throws if empty
    T dequeue()
    {
        if (isEmpty())
            throw std::out_of_range("dequeue(): queue is empty");

        T item = front->getItem();
        Node<T>* temp = front;
        front = front->getNext();

        if (front == nullptr)
            back = nullptr;

        delete temp;
        count--;
        return item;
    }

    // Return front item without removing - throws if empty
    T peek() const
    {
        if (isEmpty())
            throw std::out_of_range("peek(): queue is empty");

        return front->getItem();
    }

    // Check if queue is empty
    bool isEmpty() const
    {
        return front == nullptr;
    }

    // Get number of elements
    int size() const
    {
        return count;
    }

    // Clear all elements (fast version)
    void clear()
    {
        while (front != nullptr)
        {
            Node<T>* temp = front;
            front = front->getNext();
            delete temp;
        }
        back = nullptr;
        count = 0;
    }

    // Print function 
    void print(std::ostream& os = std::cout) const
    {
        os << "Queue[ ";
        Node<T>* current = front;
        while (current != nullptr)
        {
            os << current->getItem();
            if (current->getNext()) os << " <- ";
            current = current->getNext();
        }
        os << " ] (size: " << count << ")\n";
    }
};

// Optional: global operator<< if you really want cout << queue 
template <typename T>
std::ostream& operator<<(std::ostream& os, const LinkedQueue<T>& q)
{
    q.print(os);
    return os;
}

#endif // !LNKDQU
