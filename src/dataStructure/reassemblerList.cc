#include "reassemblerList.hh"
#include "debug.hh"

bool ReassembleList::insert(uint64_t firstIdx, std::string&& str, uint64_t firstUnPopIdx) {
    // 1. 裁剪超出范围部分
    if (firstIdx + str.size() <= firstUnPopIdx) return false;
    if (firstIdx >= firstUnPopIdx + capacity_) return false;
    if (firstIdx < firstUnPopIdx) {
        str.erase(0, firstUnPopIdx - firstIdx);
        firstIdx = firstUnPopIdx;
    }
    if (firstIdx + str.size() > firstUnPopIdx + capacity_) {
        str.resize(firstUnPopIdx + capacity_ - firstIdx);
    }

    Seg seg(firstIdx, std::move(str));
    if (que.find(seg) != que.end()) return true;

    // 2. 查找可合并的区间
    auto it = que.lower_bound(seg);
    if (it != que.begin()) {
        auto prev = std::prev(it);
        if (prev->getLastIdx() >= seg.getLastIdx()) return true;
        if (prev->getLastIdx() + 1 >= seg.firstIdx_) {
            // overlap or adjacent
            uint64_t overlap = prev->getLastIdx() + 1 - seg.firstIdx_;
            if (overlap < seg.str_.size())
                seg.str_ = std::move(prev->str_ + seg.str_.substr(overlap));
            else
                seg.str_ = std::move(prev->str_);
            seg.firstIdx_ = prev->firstIdx_;
            que.erase(prev);
        }
    }

    while (it != que.end() && seg.getLastIdx() + 1 >= it->firstIdx_) {
        uint64_t overlap = seg.getLastIdx() + 1 - it->firstIdx_;
        if (overlap < it->str_.size())
            seg.str_.append(it->str_.substr(overlap));
        que.erase(it++);
    }

    que.insert(std::move(seg));
    return true;
}

bool ReassembleList::isEmpty() {
    return que.empty();
}

std::string ReassembleList::pop() {
    std::string retStr = std::move(que.begin()->str_);
    que.erase(que.begin());
    // debug("ReassembleList::pop {} {}", retStr.size(), retStr);
    return retStr;
}

int ReassembleList::getFirstIdx() {
    if (que.empty()) return - 1;
    return que.begin()->firstIdx_;
}

uint64_t ReassembleList::getFirstSegSize() const {
    return que.begin()->str_.size();
}

const std::set<ReassembleList::Seg> ReassembleList::getQue() const {
    return que;
}