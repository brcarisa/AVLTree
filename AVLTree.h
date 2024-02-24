#ifndef TREE_H
#define TREE_H

#include <iostream>
#include <limits>

template <typename Key, typename Value>
class AVLTree {
 protected:
  struct node;

 public:
  class Iterator;
  class ConstIterator;

  using key_type = Key;
  using value_type = Value;
  using reference = value_type&;
  using const_reference = const value_type&;
  using iterator = Iterator;
  using const_iterator = ConstIterator;
  using size_type = size_t;

  class Iterator {
   public:
    Iterator() : it_node(nullptr), it_prev_node(nullptr) {}
    explicit Iterator(node* Node, node* node_past = nullptr)
        : it_node(Node), it_prev_node(node_past) {}

    iterator& operator++() {
      if (it_node == nullptr) {
        return *this;
      }

      if (it_node->right_ != nullptr) {
        it_node = GetMinNode(it_node->right_);
      } else {
        node* prev = it_node;
        it_node = it_node->parent_;
        while (it_node != nullptr && prev == it_node->right_) {
          prev = it_node;
          it_node = it_node->parent_;
        }
      }
      return *this;
    }

    iterator operator++(int) {
      Iterator tmp = *this;
      operator++();
      return tmp;
    }

    iterator& operator--() {
      if (it_node == nullptr) {
        // Если узел равен nullptr, просто возвращаем итератор на nullptr
        return *this;
      }

      if (it_node->left_ != nullptr) {
        // Если у узла есть левый потомок, идем к максимальному узлу в левом
        // поддереве
        it_node = GetMaxNode(it_node->left_);
      } else {
        // Иначе идем к родителю, пока текущий узел не будет правым потомком
        node* prev = it_node;
        it_node = it_node->parent_;
        while (it_node != nullptr && prev == it_node->left_) {
          prev = it_node;
          it_node = it_node->parent_;
        }
      }
      return *this;
    }

    iterator operator--(int) {
      Iterator tmp = *this;
      operator--();
      return tmp;
    }

    reference operator*() {
      if (it_node == nullptr) {
        throw std::out_of_range("Trying to dereference end() iterator");
      }
      return it_node->key_;
    }

    bool operator==(const Iterator& other) const noexcept {
      return it_node == other.it_node;
    }

    bool operator!=(const Iterator& other) const noexcept {
      return it_node != other.it_node;
    }

    friend class AVLTree<Key, Value>;

    node* get_node() const { return it_node; }

   protected:
    node* it_node;
    node* it_prev_node;
    node* MoveForward(AVLTree::node* Node) {
      if (Node->right_ != nullptr) return GetMinNode(Node->right_);
      auto parent = Node->parent_;
      for (; parent != nullptr && Node == parent->left_;) {
        Node = parent;
        parent = Node->parent_;
      }
      return parent;
    }
    node* MoveBack(node* Node) {
      if (Node->left_ != nullptr) {
        return GetMinNode(Node->left_);
      }
      return nullptr;
    }
  };

  class ConstIterator : public Iterator {
    ConstIterator() : Iterator() {}
    const_reference operator*() const { return Iterator::operator*(); }
  };

  // ConstructorsDestructors
  AVLTree() : root(nullptr) {}

  AVLTree(const AVLTree& other) { root = CopyTree(other.root, nullptr); }

  AVLTree(AVLTree&& other) noexcept {
    root = other.root;
    other.root = nullptr;
  }

  ~AVLTree() { clean(); }

  AVLTree& operator=(AVLTree&& other) noexcept {
    if (this != &other) {
      root = other.root;
      other.root = nullptr;
    }
    return *this;
  }

  AVLTree& operator=(const AVLTree& other) {
    if (this != &other) {
      AVLTree temp(other);
      clean();
      *this = std::move(temp);
    }
    return *this;
  }

  iterator begin() noexcept { return AVLTree::Iterator(GetMinNode(root)); }

  const_iterator begin() const noexcept {
    return AVLTree::Iterator(GetMinNode(root));
  }

  iterator end() noexcept {
    if (root == nullptr) return begin();
    node* lst_node = GetMaxNode(root);
    Iterator it(nullptr, lst_node);
    return it;
  }

  const_iterator end() const noexcept {
    if (root == nullptr) return begin();
    node* lst_node = GetMaxNode(root);
    Iterator it(nullptr, lst_node);
    return it;
  }

  [[nodiscard]] bool empty() const noexcept { return root == nullptr; }

  [[nodiscard]] size_t size() { return RecursiveSize(root); }

  [[nodiscard]] size_type size_max() const noexcept {
    return ((std::numeric_limits<size_type>::max() / 2) - sizeof(Key) -
            sizeof(node)) /
           sizeof(node);
  }

  void clean() {
    if (root != nullptr) FreeNode(root);
    root = nullptr;
  }

  std::pair<iterator, bool> insert(const key_type& key,
                                   const value_type& value = value_type()) {
    std::pair<Iterator, bool> return_value;
    if (root == nullptr) {
      root = new node(key, value);
      return_value.first = Iterator(root);
      return_value.second = true;
    } else {
      bool status_insert = RecursiveInsertion(root, key, value);
      return_value.first = Search(key);
      return_value.second = status_insert;
    }
    return return_value;
  }

  void erase(iterator pos) {
    if (root == nullptr || pos.it_node == nullptr) return;
    root = RecursiveDeleting(root, *pos);
  }

  void swap(AVLTree& other) { std::swap(root, other.root); }

  void merge(AVLTree& other) {
    AVLTree tree(other);
    Iterator it = tree.begin();
    for (; it != tree.end(); ++it) {
      std::pair<Iterator, bool> res = insert(*it);
      if (res.second) other.erase(res.first);
    }
  }

  bool contains(const key_type& key) {
    return RecursiveSearch(root, key) != nullptr;
  }

  const_iterator find(const key_type& key) const { return Search(key); }
  iterator find(const key_type& key) { return Search(key); }

 protected:
  iterator Search(const key_type& key) {
    node* desired_node = RecursiveSearch(root, key);
    return Iterator(desired_node);
  }
  struct node {
    node(key_type key, value_type value) : value_(value), key_(key) {}
    node(key_type key, value_type value, node* parent)
        : value_(value), key_(key), parent_(parent) {}
    int height_ = 0;
    value_type value_;
    key_type key_;
    node* parent_ = nullptr;
    node* left_ = nullptr;
    node* right_ = nullptr;
    friend class AVLTree<Key, Value>;
  };

  node* root;

  void FreeNode(node* Node) {
    if (Node == nullptr) return;
    FreeNode(Node->left_);
    FreeNode(Node->right_);
    delete Node;
  }

  node* CopyTree(node* Node, node* parent) {
    if (Node == nullptr) return nullptr;
    node* new_node = new node(Node->key_, Node->value_, parent);
    new_node->left_ = CopyTree(Node->left_, new_node);
    new_node->right_ = CopyTree(Node->right_, new_node);
    return new_node;
  }

  void SwapValue(node* x, node* y) {
    std::swap(x->key_, y->key_);
    std::swap(x->value_, y->value_);
  }

  void RightRotation(node* Node) {
    node* new_left = Node->left_->left_;
    node* new_right = Node->right_;
    node* new_left_right = Node->left_->right_;
    SwapValue(Node, Node->left_);
    Node->right_ = Node->left_;

    Node->left_ = new_left;
    if (Node->left_) {
      Node->left_->parent_ = Node;
    }

    Node->right_->left_ = new_left_right;
    if (Node->right_->left_) {
      Node->right_->left_->parent_ = Node->right_;
    }

    Node->right_->right_ = new_right;
    if (Node->right_->right_) {
      Node->right_->right_->parent_ = Node->right_;
    }

    SetHeight(Node->right_);
    SetHeight(Node);
  }

  void LeftRotation(node* Node) {
    node* new_left = Node->left_;
    node* new_right = Node->right_->right_;
    node* new_right_left = Node->right_->left_;
    SwapValue(Node, Node->right_);
    Node->left_ = Node->right_;

    Node->right_ = new_right;
    if (Node->right_) {
      Node->right_->parent_ = Node;
    }

    Node->left_->right_ = new_right_left;
    if (Node->left_->right_) {
      Node->left_->right_->parent_ = Node->left_;
    }

    Node->left_->left_ = new_left;
    if (Node->left_->left_) {
      Node->left_->left_->parent_ = Node->left_;
    }

    SetHeight(Node->left_);
    SetHeight(Node);
  }

  void Balancing(node* Node) {
    int bal = GetBalanceNum(Node);
    if (bal == -2) {
      if (GetBalanceNum(Node->left_) == 1) LeftRotation(Node->left_);
      RightRotation(Node);
    } else if (bal == 2) {
      if (GetBalanceNum(Node->right_) == -1) RightRotation(Node->right_);
      LeftRotation(Node);
    }
  }

  int GetBalanceNum(node* Node) const {
    if (Node != nullptr) {
      return GetHeightNum(Node->right_) - GetHeightNum(Node->left_);
    }
    return 0;
  }

  int GetHeightNum(node* Node) const {
    return Node == nullptr ? -1 : Node->height_;
  }

  void SetHeight(node* Node) {
    if (Node != nullptr) {
      Node->height_ =
          std::max(GetHeightNum(Node->left_), GetHeightNum(Node->right_)) + 1;
    }
  }

  static node* GetMinNode(node* Node) {
    if (Node == nullptr) return nullptr;
    if (Node->left_ == nullptr) return Node;
    return GetMinNode(Node->left_);
  }

  static node* GetMaxNode(node* Node) {
    if (Node == nullptr) return nullptr;
    if (Node->right_ == nullptr) return Node;
    return GetMaxNode(Node->right_);
  }

  // SupFunctions
  bool RecursiveInsertion(node* Node, const Key& key, Value value) {
    if (Node == nullptr) return false;
    bool status = false;
    if (Node->key_ > key) {
      if (Node->left_ == nullptr) {
        Node->left_ = new node(key, value, Node);
        status = true;
      } else {
        status = RecursiveInsertion(Node->left_, key, value);
      }
    } else if (Node->key_ < key) {
      if (Node->right_ == nullptr) {
        Node->right_ = new node(key, value, Node);
        status = true;
      } else {
        status = RecursiveInsertion(Node->right_, key, value);
      }
    }
    SetHeight(Node);
    Balancing(Node);
    return status;
  }

  node* RecursiveDeleting(node* Node, Key key) {
    if (Node == nullptr) return nullptr;
    if (key < Node->key_) {
      Node->left_ = RecursiveDeleting(Node->left_, key);
    } else if (key > Node->key_) {
      Node->right_ = RecursiveDeleting(Node->right_, key);
    } else {
      if (Node->left_ == nullptr || Node->right_ == nullptr) {
        node* node_right = Node->right_;
        node* node_left = Node->left_;
        node* node_parent = Node->parent_;
        delete Node;
        if (node_left == nullptr) {
          Node = node_right;
        } else {
          Node = node_left;
        }
        if (Node != nullptr) Node->parent_ = node_parent;
      } else {
        node* minimum_key_node_in_right = GetMinNode(Node->right_);
        Node->key_ = minimum_key_node_in_right->key_;
        Node->value_ = minimum_key_node_in_right->value_;
        Node->right_ =
            RecursiveDeleting(Node->right_, minimum_key_node_in_right->key_);
      }
    }
    if (Node != nullptr) {
      SetHeight(Node);
      Balancing(Node);
    }
    return Node;
  }

  size_t RecursiveSize(node* Node) {
    if (Node == nullptr) return 0;
    size_t left_tree = RecursiveSize(Node->left_);
    size_t right_tree = RecursiveSize(Node->right_);
    return left_tree + right_tree + 1;
  }

  node* RecursiveSearch(node* Node, const Key& key) {
    if (Node == nullptr || key == Node->key_) return Node;
    if (Node->key_ > key) {
      return RecursiveSearch(Node->left_, key);
    } else {
      return RecursiveSearch(Node->right_, key);
    }
  }
};

#endif  // TREE_H
