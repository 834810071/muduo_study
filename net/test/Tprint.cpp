#include <iostream>
#include <memory>

template<typename T>
class SmartPointer {
private:
    T* _ptr;
    size_t* _count;

public:
    SmartPointer(T* ptr = nullptr) :
            _ptr(ptr) {
        if (_ptr) {
            _count = new size_t(1);
        } else {
            _count = new size_t(0);
        }
    }

    SmartPointer(const SmartPointer& ptr) {
        if (this != &ptr) {
            this->_ptr = ptr._ptr;
            this->_count = ptr._count;
            (*this->_count)++;
        }
    }

    SmartPointer& operator=(const SmartPointer& ptr) {
        if (this->_ptr == ptr._ptr) {
            return *this;
        }

        if (this->_ptr) {
            (*this->_count)--;
            if (this->_count == 0) {
                delete this->_ptr;
                delete this->_count;
            }
        }

        this->_ptr = ptr._ptr;
        this->_count = ptr._count;
        (*this->_count)++;
        return *this;
    }

    T& operator*() {
        assert(this->_ptr == nullptr);
        return *(this->_ptr);

    }

    T* operator->() {
        assert(this->_ptr == nullptr);
        return this->_ptr;
    }

    ~SmartPointer() {
        (*this->_count)--;
        if (*this->_count == 0) {
            delete this->_ptr;
            delete this->_count;
        }
    }

    size_t use_count(){
        return *this->_count;
    }
};

int main() {
    {
        SmartPointer<int> sp(new int(10));  // 1
        SmartPointer<int> sp2(sp);  // 2
        SmartPointer<int> sp3(new int(20)); // 1
        sp2 = sp3;
        std::cout << sp.use_count() << std::endl;   // 1
        std::cout << sp3.use_count() << std::endl;  // 2
    }
    //delete operator
}