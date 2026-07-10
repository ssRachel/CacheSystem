


template <typename Key, typename Value> class LruCache;

template <typename Key, typename Value> 
class LruNode {
    Key key_;
    Value value_;
    size_t accessCount_; // 访问次数
    std::weak_ptr<LruNode<Key, Value>> prev_;
    std::shared_ptr<LruNode<Key, Value>> next_;

    LruNode(Key key, Value value)
        : key_(key)
        , value_(value)
        , accessCount_(1)
    {}

    Key getKey() const { return key_; }
    Value getValue() const { return value_; }
    void setValue(Value &value) { value_ = value; }
    size_t getAccessCount() const { return accessCount_; }
    void incrementAccessCount() { ++accessCount_; }
    
    friend class LruCache<Key, Value>;
};

template <typename Key, typename Value>
class LruCache : public KICachePolicy<Key, Value>
{
public:
    using LruNodeType = LruNode<Key, Value>;
    using NodePtr = std::shared_ptr<LruNodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;


    LruCache(int capacity)
        : capacity_(capacity)
    {
        initialzeList();
    }

    ~LruCache() override = default;

    // 添加缓存
    void put(Key key, Value value) override
    {
        if(capacity_ <= 0)
            return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodemap_.find(key);
        if(it != nodemap_.end())
        {
            updateExistingNode(it->second, value);
            return;
        }

        addNewNode(key, value);

    }

    bool get(Key key, Value &value) override
    {
        std::lock_guard<std::mutex_> lock(mutex_);
        auto it = nodemap_.find(key);
        if(it != nodemap_.end())
        {
            moveToMostRecent(it->second);
            value = it->second->getValue();
            return true;
        }
        return false;
    }

    Value get(Key key) override
    {
        Value value = {};
        get(key, value);
        return value;
    }


    // 删除指定元素
    void remove(Key key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodemap_.find(key);
        if(it != nodemap_.end())
        {
            removeNode(it->second);
            nodemap_.erase(it);
        }
    }

private:
    void initialzeList()
    {
        dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
        dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
        dummyHead_->next_ = dummyTail_;
        dummyTail_->prev_ = dummyHead_;
    }

    void updateExistingNode(NodePtr node, const Value &value)
    {
        node->setValue(value);
        moveToMostRecent(node);
    }

    void addNewNode(const Key &key, const Value &value)
    {
        if(nodemap_.size() >= capacity_)
        {
            evictLeastRecent();
        }

        NodePtr newNode = std::make_shared<LruNodeType>(key, value);
        insertNode(newNode);
        nodemap_[key] = newNode;
    }

    // 将该节点移动到最新的位置
    void moveToMostRecent(NodePtr node)
    {
        removeNode(node);
        insertNode(node);
    }

    void removeNode(NodePtr node)
    {
        if(!node.prev_.expired() && node->next_)
        {
            auto prev = node->prev_.lock();
            prev->next_ = node->next_;
            node->next_->prev_ = prev;
            node->next_ = nullptr;
        }
    }

    // 从尾部插入节点
    void insertNode(NodePtr node)
    {
        node->next_ = dummyTail_;
        node->prev_ = dummyTail_->prev_;
        dummyTail_->prev_.lock()->next_ = node;
        dummyTail_->prev_ = node; 
    }

    // 驱逐最近最少节点
    void evictLeastRecent()
    {
        NodePtr leastRecent = dummyHead_->next_;
        removeNode(leastRecent);
        nodemap_.erase(leastRecent->getKey());
    }

private:
    int         capacity_; // 缓存容量
    NodeMap     nodemap_; // key -> node
    std::mutex  mutex_;
    NodePtr     dummyHead_; // 虚拟头结点
    NodePtr     dummyTail_;
    
}

template <typename Key, typename Value>
class LruKCache : public LruCache<Key, Value>
{
public:
    LruKCache(int capacity, int history_capacity, int k)
	    : LruCache<Key, Value>(capacity)
	    , historyList(std::make_unique<LruCache<Key, size_t>>(history_capacity))
	    , k_(k)
    {}
	
private:
	int 					k_; // 进入缓存队列的评判标准
	std::unique_ptr<LruCache<Key, size_t>> 	historyList_; // 访问数据历史记录(value) 	
	std::unordered_map<Key, Value> 		historyValueMap_; // 存储未达到k_次访问的数据值
}
