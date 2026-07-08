


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

    
}