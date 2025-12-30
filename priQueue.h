#pragma once
#include <iostream>
using namespace std;

// Generic Priority Queue using Max Heap (Array-based)
template <typename T>
class priQueue
{
    struct Node {
        T data;
        int priority;
    };

    Node* array;
    int count;
    int capacity;

    void resize() {
        capacity *= 2;
        Node* newArray = new Node[capacity];
        for (int i = 0; i < count; i++) {
            newArray[i] = array[i];
        }
        delete[] array;
        array = newArray;
    }

    void heapifyUp(int index) {
        if (index == 0) return;
        int parent = (index - 1) / 2;
        // Max Heap: Parent should be greater than child
        if (array[index].priority > array[parent].priority) {
            Node temp = array[index];
            array[index] = array[parent];
            array[parent] = temp;
            heapifyUp(parent);
        }
    }

    void heapifyDown(int index) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;

        if (left < count && array[left].priority > array[largest].priority)
            largest = left;

        if (right < count && array[right].priority > array[largest].priority)
            largest = right;

        if (largest != index) {
            Node temp = array[index];
            array[index] = array[largest];
            array[largest] = temp;
            heapifyDown(largest);
        }
    }

public:
    priQueue() {
        capacity = 20;
        count = 0;
        array = new Node[capacity];
    }

    ~priQueue() {
        delete[] array;
    }

    bool isEmpty() const { return count == 0; }
    int getSize() const { return count; }

    void enqueue(const T& data, int priority) {
        if (count == capacity) resize();
        array[count].data = data;
        array[count].priority = priority;
        heapifyUp(count);
        count++;
    }

    bool dequeue(T& topEntry, int& priority) {
        if (isEmpty()) return false;
        topEntry = array[0].data;
        priority = array[0].priority;

        // Move last element to root
        array[0] = array[count - 1];
        count--;
        heapifyDown(0);
        return true;
    }

    bool peek(T& topEntry, int& priority) const {
        if (isEmpty()) return false;
        topEntry = array[0].data;
        priority = array[0].priority;
        return true;
    }

    void print() const {
        if (isEmpty()) {
            cout << "Empty Priority Queue\n";
            return;
        }
        cout << "Priority Queue (Heap): ";
        for (int i = 0; i < count; i++) {
            cout << array[i].data << "(P:" << array[i].priority << ") ";
        }
        cout << "\n";
    }

    // Helper for traversal if needed (e.g. for GUI iteration)
    bool getItem(int i, T& result) const {
        if (i < 0 || i >= count) return false;
        result = array[i].data;
        return true;
    }

    //get head
    T getHead() const {
        if (isEmpty()) 
            throw std::out_of_range("getHead(): queue is empty");
        return array[0].data;
	}

    //
};