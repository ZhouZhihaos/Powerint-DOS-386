#ifndef __BOX__
#define __BOX__
#include <psheet.hpp>
class Node {
 public:
  uint32_t key;
  uint32_t value;
  bool deleted;
  Node* next;
  int index;
  Node(uint32_t k, uint32_t v) : key(k), value(v), next(nullptr) {}
};

class HashTable {
 private:
  Node** table;
  int capacity;
  int size;  // 当前哈希表中的元素个数

 public:
  HashTable(int initialSize) {
    capacity = initialSize;
    table = new Node*[capacity];
    for (int i = 0; i < capacity; i++) {
      table[i] = nullptr;
    }
    size = 0;
  }

  ~HashTable() {
    for (int i = 0; i < capacity; i++) {
      Node* curr = table[i];
      while (curr) {
        Node* temp = curr;
        curr = curr->next;
        delete temp;
      }
    }
    delete[] table;
  }

  void insert(uint32_t key, uint32_t value) {
    if (shouldResize()) {
      resizeTable();
    }

    int hashValue = hashFunction(key);
    Node* curr = table[hashValue];
    Node* newNode;
    newNode = new Node(key, value);
    int index = hashFunction(key);
    newNode->deleted = false;
    newNode->next = nullptr;
    if (!table[index]) {
      table[index] = newNode;
    } else {
      Node* node = table[index];
      while (node->next) {
        node = node->next;
      }
      node->next = newNode;
    }
    size++;
  }

  uint32_t search(uint32_t key) {
    int index = hashFunction(key);

    Node* node = table[index];
    while (node) {
      if (!node->deleted && node->key == key) {
        return node->value;
      }
      node = node->next;
    }

    return -1;  // 未找到
  }
  void remove(uint32_t key) {
    int index = hashFunction(key);

    Node* node = table[index];
    while (node) {
      if (node->key == key) {
        node->deleted = true;
        break;
      }
      node = node->next;
    }
  }

 private:
  int hashFunction(int key) { return key % capacity; }

  bool shouldResize() {
    // 当元素个数大于等于容量的一半时，需要扩容
    return size >= capacity;
  }

  void resizeTable() {
    int newCapacity = capacity * 2;
    Node** newTable = new Node*[newCapacity];
    for (int i = 0; i < newCapacity; ++i) {
      newTable[i] = nullptr;
    }

    for (int i = 0; i < capacity; ++i) {
      Node* node = table[i];
      while (node) {
        Node* next = node->next;
        if (!node->deleted) {
          int index = node->key % newCapacity;

          if (!newTable[index]) {
            newTable[index] = node;
            node->next = nullptr;
          } else {
            Node* curr = newTable[index];
            while (curr->next) {
              curr = curr->next;
            }
            curr->next = node;
            node->next = nullptr;
          }
        } else {
          delete node;
          size--;
        }

        node = next;
      }
    }

    delete[] table;
    table = newTable;
    capacity = newCapacity;
  }
};
struct Box {
  PSheetBase* ps;
  int x, y, x1, y1,flag;
  uint32_t val;
  void (*callback)(PSheetBase* ps, int x, int y, uint32_t val);
};
class WindowBox {
 public:
  WindowBox();
  void register_box(
      PSheetBase* ps,
      int x,
      int y,
      int x1,
      int y1,
      uint32_t val,
      void (*callback)(PSheetBase* ps, int x, int y, uint32_t val));
  PSheetBase* click_handle(struct SHTCTL* ctl, int x, int y);
  void box_logout(PSheetBase* ps);

 private:
  PVector<Box>* box;
  HashTable* ht;
  int num;
};

#endif