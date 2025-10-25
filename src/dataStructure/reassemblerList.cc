#include "reassemblerList.hh"
#include "debug.hh"

bool ReassembleList::insert(uint64_t firstIdx, std::string&& str, uint64_t firstUnPopIdx) {
    if (firstIdx >= firstUnPopIdx + capacity_) return false;   // 如果整个字符串都在范围外，直接丢弃
    // 裁剪字符串 str
    int len = firstUnPopIdx + capacity_ - firstIdx;
    str = str.substr(0, len);
    // debug("ReassembleList::insert: {} str: {} str.size(): {}", firstIdx, str, str.size());
    que.emplace(firstIdx, std::move(str));
    auto mergeStr = [&](ReassembleList::Seg& firstSeg, ReassembleList::Seg& secondSeg) -> bool {
        if (firstSeg.getLastIdx() + 1 < secondSeg.firstIdx_) {
            return false;
        }
        if (firstSeg.getLastIdx() >= secondSeg.getLastIdx()) {
            return true;
        }
        int len_ = secondSeg.firstIdx_ - firstSeg.firstIdx_;
        firstSeg.str_ = firstSeg.str_.substr(0, len_);
        firstSeg.str_.append(secondSeg.str_);
        return true;
    };
    while(que.size() >= 2) {
        ReassembleList::Seg front = que.top();
        que.pop();
        ReassembleList::Seg seg = que.top();
        que.pop();
        if (mergeStr(front, seg)) {
            que.push(front);
        } else {
            que.push(front), que.push(seg);
            break;
        }
    }
    return true;
}

bool ReassembleList::isEmpty() {
    return que.empty();
}

std::string ReassembleList::pop() {
    std::string retStr = que.top().str_;
    que.pop();
    // debug("ReassembleList::pop {} {}", retStr.size(), retStr);
    return retStr;
}

int ReassembleList::getFirstIdx() {
    if (que.empty()) return - 1;
    return que.top().firstIdx_;
}

uint64_t ReassembleList::getFirstSegSize() const {
    return que.top().str_.size();
}

const std::priority_queue<ReassembleList::Seg> ReassembleList::getQue() const {
    return que;
}