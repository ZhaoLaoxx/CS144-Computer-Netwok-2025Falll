#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::popList(uint64_t firstIdx, bool isLastSubString, std::string& str) {
  // debug("Reassembler::popList firstUnPopIdx: {} list.getFirstIdx: {} str: {}", firstUnPopIdx, list.getFirstIdx(), str);
  // 魔法数，todo 优化
  if (list.getFirstIdx() != - 1 && firstUnPopIdx >= list.getFirstIdx()) {
    // 当满足条件，可以 pop 最前面的 seg 到 writer 中
    int listFirstIdx = list.getFirstIdx();
    string popStr = list.pop();
    if (output_.writer().available_capacity() == 0) {
      // writer 中没有一点可用空间，直接丢弃
      return;
    }
    // 裁剪 str 到合适长度
    uint64_t offerSet = firstUnPopIdx - listFirstIdx;
    if (offerSet >= popStr.size()) return;
    popStr = popStr.substr(offerSet, min(offerSet + output_.writer().available_capacity(), popStr.size()));
    output_.writer().push(popStr);
    firstUnPopIdx += popStr.size();
  }
  if (isLastSubString) {
    finalIdx = firstIdx + str.size();
  }
  if (firstUnPopIdx == finalIdx) {
    output_.writer().close();
  }
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // debug("Reassembler::insert insert({}, {}, {}) called", first_index, data, is_last_substring);
  if (output_.writer().is_closed()) return;

  // 检查重拍器最前面的一段seg是否满足条件可以pop，如果可以将数据写入到输出流中
  popList(first_index, is_last_substring, data);

  // 插入seg
  list.insert(first_index, std::move(data), firstUnPopIdx);

  popList(first_index, false, data);
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  std::priority_queue<ReassembleList::Seg> que = list.getQue();
  std::set<int> idxSet;
  while(!que.empty()) {
    ReassembleList::Seg seg = que.top();
    que.pop();
    for (int i = seg.getFirstIdx(); i <= seg.getLastIdx(); i ++) {
      idxSet.insert(i);
    }
  }
  // debug("Reassembler::count_bytes_pending() {}", idxSet.size());
  return idxSet.size();
}
