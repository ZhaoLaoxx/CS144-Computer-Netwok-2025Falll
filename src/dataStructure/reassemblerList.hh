# include <string>
# include <queue>
# include <set>
# include <cstdint>

class ReassembleList {
public:
    bool insert(uint64_t firstIdx, std::string&& str, uint64_t firstUnPopIdx);
    std::string pop();
    bool isEmpty();
    int getFirstIdx();
    uint64_t getFirstSegSize() const;
    ReassembleList(uint64_t capacity) : firstIdxSet(), que(), capacity_(capacity) {};
    class Seg {
        public:
            Seg() : firstIdx_(0), str_() {};
            Seg(int firstIdx, std::string&& str) : firstIdx_(firstIdx), str_(std::move(str)) {};
            int getLastIdx() { return firstIdx_ + str_.size() - 1; };
            int getFirstIdx() { return firstIdx_; };
            bool operator < (const Seg& seg) const {
                if (firstIdx_ != seg.firstIdx_) {
                    return firstIdx_ > seg.firstIdx_;
                }
                return str_.size() < seg.str_.size();
            }
        private:
            int firstIdx_;
            std::string str_;
            friend ReassembleList;
    };
    const std::priority_queue<ReassembleList::Seg> getQue() const;
private:
    std::set<int> firstIdxSet;
    std::priority_queue<Seg> que;
    uint64_t capacity_;
    friend Seg;
};