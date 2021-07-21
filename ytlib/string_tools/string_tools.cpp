#include "string_tools.hpp"

#include <cstring>

namespace ytlib {

using std::map;
using std::set;
using std::string;
using std::vector;

using std::size_t;

map<string, string> SplitToMap(const string& source, const string& vsep, const string& msep,
                               bool trimempty) {
  map<string, string> result;
  if (source.empty()) return result;

  size_t m, n, pos = 0;
  string str = source;
  str.append(vsep);

  string sub;
  string first;
  string second;
  while ((n = str.find(vsep, pos)) != string::npos) {
    sub = str.substr(pos, n - pos);
    if (!sub.empty()) {
      if ((m = sub.find(msep)) != string::npos) {
        first = sub.substr(0, m);
        second = sub.substr(m + msep.size());
        if (trimempty) {
          result[trim(first)] = trim(second);
        } else {
          result[first] = second;
        }
      }
    }
    pos = n + vsep.size();
  }

  return result;
}

string JoinMap(const map<string, string>& kvmap, const string& vsep, const string& msep) {
  string result;
  for (auto& itr : kvmap) {
    // 若不为空，则需要增加分隔符
    if (!result.empty()) result += vsep;
    result += itr.first + msep + itr.second;
  }
  return result;
}

string GetValueFromStrKV(const string& str, const string& key, const string& vsep,
                         const string& msep, bool trimempty) {
  size_t pos = str.find(key);
  if (string::npos == pos) return "";

  pos = str.find(msep, pos);
  if (string::npos == pos) return "";
  pos += msep.length();

  size_t pos_end = str.find(vsep, pos);
  if (string::npos == pos_end) pos_end = str.length();

  string re = str.substr(pos, pos_end - pos);
  if (trimempty) trim(re);

  return re;
}

vector<string> SplitToVec(const string& source, const string& sep, bool trimempty) {
  vector<string> re;
  size_t pos1, pos2 = 0;
  const string& real_sep = trimempty ? (sep + " ") : sep;
  do {
    pos1 = source.find_first_not_of(real_sep, pos2);
    if (pos1 == string::npos) break;
    pos2 = source.find_first_of(real_sep, pos1);
    re.emplace_back(source.substr(pos1, pos2 - pos1));
  } while (pos2 != string::npos);
  return re;
}

string JoinVec(const vector<string>& vec, const string& sep) {
  string result;
  for (auto& itr : vec) {
    // 若不为空，则需要增加分隔符
    if (!result.empty()) result += sep;
    result += itr;
  }
  return result;
}

set<string> SplitToSet(const string& source, const string& sep, bool trimempty) {
  set<string> re;
  size_t pos1, pos2 = 0;
  const string& real_sep = trimempty ? (sep + " ") : sep;
  do {
    pos1 = source.find_first_not_of(real_sep, pos2);
    if (pos1 == string::npos) break;
    pos2 = source.find_first_of(real_sep, pos1);
    re.emplace(source.substr(pos1, pos2 - pos1));
  } while (pos2 != string::npos);
  return re;
}

string JoinSet(const set<string>& st, const string& sep) {
  string result;
  for (auto& itr : st) {
    // 若不为空，则需要增加分隔符
    if (!result.empty()) result += sep;
    result += itr;
  }
  return result;
}

bool CheckIfInList(const string& strlist, const string& key, char sep) {
  if (key.empty()) return false;
  if (key.find(sep) != string::npos) return false;
  size_t pos = strlist.find(key);
  while (pos != string::npos) {
    if ((pos > 0 && strlist[pos - 1] != sep) ||
        ((pos + key.length()) < strlist.length() && strlist[pos + key.length()] != sep)) {
      // 如果前后有不是分割符的情况就查下一个
      pos = strlist.find(key, pos + key.length());
    } else {
      return true;
    }
  }
  return false;
}

int CmpVersion(const string& ver1, const string& ver2) {
  const vector<string>& version1_detail = SplitToVec(ver1, ".");
  const vector<string>& version2_detail = SplitToVec(ver2, ".");

  size_t idx = 0;
  for (idx = 0; idx < version1_detail.size() && idx < version2_detail.size(); ++idx) {
    int ver1 = atoi(version1_detail[idx].c_str());
    int ver2 = atoi(version2_detail[idx].c_str());
    if (ver1 < ver2)
      return -1;
    else if (ver1 > ver2)
      return 1;
  }
  if (idx == version1_detail.size() && idx == version2_detail.size()) {
    return 0;
  }
  return version1_detail.size() > version2_detail.size() ? 1 : -1;
}

string& ReplaceString(string& str, const string& ov, const string& nv) {
  if (str.empty()) return str;
  vector<size_t> vec_pos;
  size_t pos = 0, old_len = ov.size(), new_len = nv.size();
  while (string::npos != (pos = str.find(ov, pos))) {
    vec_pos.emplace_back(pos);
    pos += old_len;
  }
  size_t& vec_len = pos = vec_pos.size();
  if (vec_len) {
    if (old_len == new_len) {
      for (size_t ii = 0; ii < vec_len; ++ii)
        memcpy(const_cast<char*>(str.c_str() + vec_pos[ii]), nv.c_str(), new_len);
    } else if (old_len > new_len) {
      char* p = const_cast<char*>(str.c_str()) + vec_pos[0];
      vec_pos.emplace_back(str.size());
      for (size_t ii = 0; ii < vec_len; ++ii) {
        memcpy(p, nv.c_str(), new_len);
        p += new_len;
        size_t cplen = vec_pos[ii + 1] - vec_pos[ii] - old_len;
        memmove(p, str.c_str() + vec_pos[ii] + old_len, cplen);
        p += cplen;
      }
      str.resize(p - str.c_str());
    } else {
      size_t diff = new_len - old_len;
      vec_pos.emplace_back(str.size());
      str.resize(str.size() + diff * vec_len);
      char* p = const_cast<char*>(str.c_str()) + str.size();
      for (size_t ii = vec_len - 1; ii < vec_len; --ii) {
        size_t cplen = vec_pos[ii + 1] - vec_pos[ii] - old_len;
        p -= cplen;
        memmove(p, str.c_str() + vec_pos[ii] + old_len, cplen);
        p -= new_len;
        memcpy(p, nv.c_str(), new_len);
      }
    }
  }
  return str;
}

bool IsAlnumStr(const string& str) {
  if (str.length() == 0) return false;
  for (auto& c : str) {
    if (!isalnum(c)) return false;
  }
  return true;
}

bool IsDigitStr(const string& str) {
  if (str.length() == 0) return false;
  for (auto& c : str) {
    if (c > '9' || c < '0') return false;
  }
  return true;
}

}  // namespace ytlib