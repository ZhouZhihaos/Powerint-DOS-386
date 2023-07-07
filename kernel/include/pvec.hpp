#ifndef P_VECTOR_H
#define P_VECTOR_H
template<typename T>
class PVector {
private:
    T* arr;            // 存储元素的数组指针
    size_t size;       // 当前元素数量
    size_t capacity;   // 数组容量

public:
    // 构造函数
    PVector();
    
    // 析构函数
    ~PVector();
    
    // 获取当前元素数量
    size_t get_size() const;
    
    // 添加元素到P_Vector末尾
    void push_back(const T& value);
    
    // 移除P_Vector末尾的元素
    void pop_back();
    
    // 获取P_Vector指定索引处的元素
    T& operator[](size_t index);
    
    // 清空P_Vector中的所有元素
    void clear();
};

template<typename T>
PVector<T>::PVector() : arr(nullptr), size(0), capacity(0) {}

// 析构函数的实现
template<typename T>
PVector<T>::~PVector() {
    clear();
}

// 获取当前元素数量的实现
template<typename T>
size_t PVector<T>::get_size() const {
    return size;
}

// 添加元素到P_Vector末尾的实现
template<typename T>
void PVector<T>::push_back(const T& value) {
    if (size == capacity) {
        // 容量不足时，进行扩容
        size_t new_capacity = (capacity == 0) ? 1 : capacity * 2;
        T* new_arr = (T*)realloc(arr, new_capacity * sizeof(T));
        if (new_arr == nullptr) {
            return;
        }
        arr = new_arr;
        capacity = new_capacity;
    }
    arr[size++] = value;
}

// 移除P_Vector末尾的元素的实现
template<typename T>
void PVector<T>::pop_back() {
    if (size > 0) {
        --size;
    }
}

// 获取P_Vector指定索引处的元素的实现
template<typename T>
T& PVector<T>::operator[](size_t index) {
    return arr[index];
}

// 清空P_Vector中的所有元素的实现
template<typename T>
void PVector<T>::clear() {
    free(arr);
    arr = nullptr;
    size = 0;
    capacity = 0;
}
#endif // P_VECTOR_H