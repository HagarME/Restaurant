#pragma once

template <typename T>
class priNode
{
private:
	T item;                   // البيانات → ممكن تكون Order أو Order*
    int pri;                   // الأولوية → كل ما الرقم أكبر = الأولوية أعلى (Higher number = Higher priority)
    priNode<T>* next;          

public:
    // Constructor 
    priNode(const T& r_Item, int priority)
        : item(r_Item), pri(priority), next(nullptr)
    {
        // nullptr عشان ده آخر node في البداية
    }

    // Set Item + Priority معاً (لو عايز تغير الاتنين مرة واحدة)
    void setItem(const T& r_Item, int priority)
    {
        item = r_Item;
        pri = priority;
    }

   
    void setNext(priNode<T>* nextNodePtr)
    {
        next = nextNodePtr;
    }

    
    T getItem() const { return item; }

    
    int getPriority() const { return pri; }

    
    priNode<T>* getNext() const { return next; }
};