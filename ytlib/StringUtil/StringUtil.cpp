#include "StringUtil.h"

namespace ytlib {

using std::map;
using std::set;
using std::string;
using std::vector;

string& trim(string& s) {
  if (s.empty()) return s;
  s.erase(0, s.find_first_not_of(" "));
  s.erase(s.find_last_not_of(" ") + 1);
  return s;
}

map<string, string> SplitToMap(const string& source, const string& vsep, const string& msep,
                               bool trimempty) {
  map<string, string> result;
  if (source.empty()) return result;

  size_t pos = 0;
  size_t n;
  size_t m;
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

  std::string re = str.substr(pos, pos_end - pos);
  if (trimempty) trim(re);

  return re;
}

vector<string> SplitToVec(const string& source, const string& separators, bool cleanempty,
                          bool trimempty) {
  vector<string> result;
  if (source.empty()) return result;

  string::const_iterator it1 = source.begin();
  string::const_iterator it2;
  string::const_iterator it3;
  string::const_iterator end = source.end();

  while (it1 != end) {
    if (trimempty) {
      while (it1 != end && isspace(*it1)) ++it1;
    }
    it2 = it1;
    while (it2 != end && separators.find(*it2) == string::npos) ++it2;
    it3 = it2;
    if (it3 != it1 && (trimempty)) {
      --it3;
      while (it3 != it1 && isspace(*it3)) --it3;
      if (!isspace(*it3)) ++it3;
    }
    if (cleanempty) {
      if (it3 != it1) result.push_back(string(it1, it3));
    } else {
      result.push_back(string(it1, it3));
    }
    it1 = it2;
    if (it1 != end) ++it1;
  }

  return result;
}

string JoinVec(const vector<string>& vec, const string& separators) {
  string result;
  for (auto& itr : vec) {
    // 若不为空，则需要增加分隔符
    if (!result.empty()) result += separators;
    result += itr;
  }
  return result;
}

string JoinSet(const set<string>& st, const string& separators) {
  string result;
  for (auto& itr : st) {
    // 若不为空，则需要增加分隔符
    if (!result.empty()) result += separators;
    result += itr;
  }
  return result;
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

int IosCmpVersion(const string& ver1, const string& ver2) {
  string tmp_ver1 = ver1;
  string tmp_ver2 = ver2;
  return CmpVersion(ReplaceString(tmp_ver1, "ios", ""), ReplaceString(tmp_ver2, "ios", ""));
}

bool CheckVersionInside(const string& ver, const string& start_ver, const string& end_ver) {
  string start_ver_fix = start_ver.empty() ? "0.0.0.0" : start_ver;
  string end_ver_fix = end_ver.empty() ? "999.9.9.9" : end_ver;
  return (CmpVersion(ver, start_ver_fix) >= 0 && CmpVersion(ver, end_ver_fix) <= 0);
}

string& ReplaceString(string& str, const string& ov, const string& nv) {
  if (str.empty()) return str;
  vector<size_t> vec_pos;
  size_t pos = 0, old_len = ov.size(), new_len = nv.size();
  while (string::npos != (pos = str.find(ov, pos))) {
    vec_pos.push_back(pos);
    pos += old_len;
  }
  size_t& vec_len = pos = vec_pos.size();
  if (vec_len) {
    if (old_len == new_len) {
      for (size_t ii = 0; ii < vec_len; ++ii)
        memcpy(const_cast<char*>(str.c_str() + vec_pos[ii]), nv.c_str(), new_len);
    } else if (old_len > new_len) {
      char* p = const_cast<char*>(str.c_str()) + vec_pos[0];
      vec_pos.push_back(str.size());
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
      vec_pos.push_back(str.size());
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
  const char* ps = str.c_str();
  size_t len = str.length();
  for (size_t ii = 0; ii < len; ++ii) {
    if (!isalnum(ps[ii])) return false;
  }
  return true;
}

bool CheckIfInList(const std::string& strlist, const std::string& key, char sep) {
  if (key.empty()) return false;
  if (key.find(sep) != std::string::npos) return false;
  size_t pos = strlist.find(key);
  while (pos != std::string::npos) {
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

}  // namespace ytlib