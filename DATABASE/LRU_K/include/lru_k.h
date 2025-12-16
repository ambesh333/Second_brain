#pragma once 

#include<list>
#include<mutex>
#include<optional>
#include<unordered_map>
#include<limits>
#include<iostream>

namespace lru_k{

    using frame_id_t = int;

    enum class AccessType{
      Unknown = 0,
      Lookup,
      Scan,
      Index 
    };

    class LRUK {
        public:
            explicit LRUK(size_t num_frames , size_t k);

        // Deleted copy constructor and assignment operator
        LRUK(const LRUK &)= delete;
        LRUK &operator =(const LRUK &)= delete;

        auto Evict() -> std::optional<frame_id_t>;
        void RecordAccess(frame_id_t frame_id, AccessType access_type = AccessType::Unknown);
        void SetEvictable(frame_id_t frame_id, bool is_evictable);
        void Remove(frame_id_t frame_id);
        auto Size() -> size_t;        

        private:
            struct LRUKNode {
                frame_id_t fid_;
                std::list<size_t> history_;
                bool is_evictable_{false};
                size_t k_;

                auto GetKDistance(size_t current_timestamp) const -> uint64_t {
                    if(history_.size() < k_){
                        return std::numeric_limits<uint64_t>::max();
                    }
                    return current_timestamp - history_.front();
                }
            };

            std::unordered_map<frame_id_t,LRUKNode> node_store_;
            size_t current_timestamp_{0};
            size_t curr_size_{0};
            size_t replacer_size_;
            size_t k_;
            std::mutex latch_;
    };

}