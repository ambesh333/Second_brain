#include "lru_k.h"
#include <stdexcept>
#include <algorithm>

namespace lru_k {

    LRUK::LRUK(size_t num_frames , size_t k):replacer_size_(num_frames),k_(k){}

    auto LRUK::Evict() -> std::optional<frame_id_t>{
            if(curr_size_ == 0){
                return std::nullopt;
            }

            frame_id_t victim_id = -1;
            bool found_victim = false;
            bool current_victim_is_inf = false;
            size_t current_victim_timestamp = 0;

            for(auto &pair : node_store_){
                auto &node = pair.second;
                // donot evict latched frames
                if(!node.is_evictable_){
                    continue;
                }

                bool is_inf = node.history_.size() < k_; // check if node is infinite( newbie)
                size_t node_timestamp = node.history_.front(); // get the timestamp of the node

                if(!found_victim){ // first victim assignment
                    victim_id = node.fid_;
                    current_victim_is_inf = is_inf;
                    current_victim_timestamp = node_timestamp;
                    found_victim = true;
                    continue;
                }

                if(is_inf && !current_victim_is_inf){ // if current node is infinite (newbie) and victim is not infinite (veteran)
                    victim_id = node.fid_;
                    current_victim_is_inf = true;
                    current_victim_timestamp = node_timestamp;
                    
                }else if(is_inf && current_victim_is_inf){ // if current node is infinite (newbie) and victim is infinite (newbie)
                    if(node_timestamp < current_victim_timestamp){ // we want the earliest timestamped node
                        victim_id = node.fid_;
                        current_victim_timestamp = node_timestamp;
                    }
                }else if(!is_inf && !current_victim_is_inf){ // if current node is not infinite (veteran) and victim is not infinite (veteran)
                    if(node_timestamp < current_victim_timestamp){ // we want the earliest timestamped node
                        victim_id = node.fid_;
                        current_victim_timestamp = node_timestamp;
                    }
                }


            }

            if(found_victim){
                curr_size_--;
                node_store_.erase(victim_id);
                return victim_id;
            }

            return std::nullopt;


    }

    void LRUK::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type){
        std::scoped_lock<std::mutex> lock(latch_); // for thread safety
        
        if(frame_id > static_cast<frame_id_t>(replacer_size_)){ // check if frame id is valid by comparing it with replacer size
            throw std::runtime_error("RecordAccess: Frame ID invalid");
        }

        current_timestamp_++; // instead of using timestamp we can use clock ticks

        if(node_store_.find(frame_id) == node_store_.end()){ // if node is not present in the node store
            LRUKNode new_node;
            new_node.fid_ = frame_id;
            new_node.k_ = k_;
            node_store_[frame_id] = new_node;
        }

        auto &node = node_store_[frame_id];
        node.history_.push_back(current_timestamp_);

         if(node.history_.size() > k_){
            node.history_.pop_front();
         }

    }

    void LRUK::SetEvictable(frame_id_t frame_id , bool set_evictable){
        std::scoped_lock<std::mutex> lock(latch_);

        if(node_store_.find(frame_id) == node_store_.end()){
            throw std::runtime_error("SetEvictable: Frame ID invalid");
            return;
        }

        auto &node = node_store_[frame_id]; // get the node

        if(node.is_evictable_ && !set_evictable){ // if node is evictable and we want to make it non-evictable
            curr_size_--; // tracks the number of evictable nodes not currently in the replacer
        } else if (!node.is_evictable_ && set_evictable){ // if node is not evictable and we want to make it evictable
            curr_size_++; // increase the size of the replacer
        }

        node.is_evictable_ = set_evictable;
    }

    void LRUK::Remove(frame_id_t frame_id){
        std:: scoped_lock<std::mutex> lock(latch_);
        
        if(node_store_.find(frame_id) == node_store_.end()){
            return;
        }

        auto &node = node_store_[frame_id];
        if(!node.is_evictable_){
            throw std::runtime_error("Remove: Frame ID invalid");
        }

        node_store_.erase(frame_id);
        curr_size_--;
    }

    auto LRUK::Size() -> size_t{
        std::scoped_lock<std::mutex> lock(latch_);
        return curr_size_;
    }

}

