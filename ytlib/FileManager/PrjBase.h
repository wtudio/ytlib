/**
 * @file PrjBase.h
 * @brief 项目管理型文件
 * @details 基于xml格式，制定通用的项目管理型文件
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <ytlib/FileManager/FileBase.h>
#include <ytlib/SupportTools/XMLTools.h>

#include <boost/algorithm/string.hpp>

namespace ytlib {

/*
  示例xml：
<?xml version="1.0" encoding="utf-8"?>
<Project Name="testprj1" Version="0.0.1">
  <Settings>
    <setting key="key1" value="value1" />
    <setting key="key2" value="value2" />
    <setting key="key3" value="value3" />
  </Settings>
  <ItemGroup>
    <Item itemname="file1.txt" type="type1">
      <Settings>
        <setting key="key1" value="value1" />
        <setting key="key2" value="value2" />
      </Settings>
    </Item>
    <Item itemname="file2.txt" type="type2">
      <Settings>
        <setting key="key1" value="value1" />
        <setting key="key2" value="value2" />
      </Settings>
    </Item>
  </ItemGroup>
</Project>
 */
struct ItemInfo {
  tstring type;
  std::map<tstring, tstring> settings;
};

struct PrjObj {
  tstring m_PrjName;
  tstring m_PrjVersion;
  std::map<tstring, tstring> m_PrjSettings;  //key-value
  std::map<tstring, ItemInfo> m_PrjItems;    //path-obj
};
/**
 * @brief 项目管理型文件类
 * 基于xml格式，制定通用的项目管理型文件。
 * root节点：工程名称、版本、settings节点，item节点（多个）。
 * settings节点：key-value对。
 * item节点:名称、类型，settings节点。名称即为路径。
 */
class PrjFile : public FileBase<PrjObj> {
 public:
  PrjFile() : FileBase(), m_version(T_TEXT("0.0.1")) {}
  virtual ~PrjFile() {}

  inline tstring getPrjName() const { return m_fileobj->m_PrjName; }
  bool setPrjName(const tstring& name_) {
    if (name_.empty()) return false;
    m_fileobj->m_PrjName = name_;
    return true;
  }

  //version应该是在类初始化时确定的。如果文件里读出来的version不对则需要报错
  inline tstring getPrjVersion() const { return m_version; }

  inline std::map<tstring, tstring> getPrjSettings() const { return m_fileobj->m_PrjSettings; }
  inline void setPrjSettings(const std::map<tstring, tstring>& setting_) { m_fileobj->m_PrjSettings = setting_; }

  ItemInfo getPrjItem(const tstring& itemname_) const {
    std::map<tstring, ItemInfo>::const_iterator itr = m_fileobj->m_PrjItems.find(itemname_);
    if (itr != m_fileobj->m_PrjItems.end())
      return itr->second;
    else
      return ItemInfo();
  }
  //检查文件目录是否在工程目录下。不判断文件是否存在。只可使用相对路径，如果有重名的就覆盖掉
  bool setPrjItem(const tstring& itemname_, const ItemInfo& obj_) {
    if (itemname_.empty()) return false;
    if (tpath(itemname_).is_absolute()) return false;
    m_fileobj->m_PrjItems[itemname_] = obj_;
    return true;
  }
  bool delPrjItem(const tstring& itemname_) {
    std::map<tstring, ItemInfo>::const_iterator itr = m_fileobj->m_PrjItems.find(itemname_);
    if (itr == m_fileobj->m_PrjItems.end()) return false;
    m_fileobj->m_PrjItems.erase(itr);
    return true;
  }
  //获取一种类型的子项目
  std::vector<tstring> getTypeItem(const tstring& type_) const {
    std::vector<tstring> re;
    for (std::map<tstring, ItemInfo>::const_iterator itr = m_fileobj->m_PrjItems.begin();
         itr != m_fileobj->m_PrjItems.end(); ++itr) {
      if (itr->second.type == type_) re.push_back(itr->first);
    }
    return re;
  }

  //获取工程路径，供items计算绝对路径
  inline tstring getPrjPath() const { return GetFileParentPath(); }
  //获取绝对路径
  inline tstring getItemAbsolutePath(const tstring& path_) {
    return (tpath(GetFileParentPath()) / tpath(path_)).string<tstring>();
  }

 protected:
  const tstring m_version;
  /*virtual std::shared_ptr<PrjObj> m_fileobj const{
      return FileBase::m_fileobj;
    }*/
  virtual bool CheckFileName(const tstring& filename) const {
    tstring Suffix1 = T_TEXT("xml");
    tstring Suffix2 = T_TEXT("prj");
    if ((boost::to_lower_copy(filename.substr(filename.length() - Suffix1.length(), Suffix1.length())) != Suffix1) && (boost::to_lower_copy(filename.substr(filename.length() - Suffix2.length(), Suffix2.length())) != Suffix2)) {
      return false;
    }
    return true;
  }
  virtual bool CreateFileObj() {
    m_fileobj = std::make_shared<PrjObj>();
    if (m_filepath.empty()) {
      m_fileobj->m_PrjName = T_TEXT("NewProject");
    } else {
      m_fileobj->m_PrjName = GetFileName();
    }
    //在此处确定version。子类继承时也在此
    m_fileobj->m_PrjVersion = m_version;
    return true;
  }
  virtual bool GetFileObj() {
    if (!CreateFileObj()) return false;
    tpath path = tGetAbsolutePath(m_filepath);

    tptree ptRoot;
    if (!realXml(path.string<tstring>(), ptRoot)) return false;
    //解析,同时还要检查

    try {
      tptree ptprj = ptRoot.get_child(T_TEXT("Project"));
      //判断version是否符合
      if (m_version != ptprj.get<tstring>(T_TEXT("<xmlattr>.Version"))) {
        tcout << T_TEXT("Version not matched : ") << m_version << T_TEXT(" VS ") << ptprj.get<tstring>(T_TEXT("<xmlattr>.Version")) << std::endl;
        return false;
      }
      m_fileobj->m_PrjName = ptprj.get<tstring>(T_TEXT("<xmlattr>.Name"));
      if (m_fileobj->m_PrjName.empty()) {
        tcout << T_TEXT("Invalid Project Name!") << std::endl;
        return false;
      }
      m_fileobj->m_PrjVersion = m_version;
      //settings 可选
      if (!readSettings(ptprj, m_fileobj->m_PrjSettings)) return false;
      //ItemGroup 可选
      boost::optional<tptree&> ptItemGroup = ptprj.get_child_optional(T_TEXT("ItemGroup"));
      if (ptItemGroup) {
        for (tptree::iterator itrptig = ptItemGroup->begin(); itrptig != ptItemGroup->end(); ++itrptig) {
          tptree& Itempt = itrptig->second;
          tstring itemname(Itempt.get<tstring>(T_TEXT("<xmlattr>.itemname")));
          if (itemname.empty()) {
            tcout << T_TEXT("Invalid Item Name!") << std::endl;
            return false;
          }
          ItemInfo tmpiteminfo;
          tmpiteminfo.type = Itempt.get<tstring>(T_TEXT("<xmlattr>.type"));
          if (!readSettings(Itempt, tmpiteminfo.settings)) return false;
          m_fileobj->m_PrjItems[itemname] = std::move(tmpiteminfo);
        }
      }
    } catch (const std::exception& e) {
      std::cout << "load Project file failed : " << e.what() << std::endl;
      return false;
    }

    return true;
  }
  virtual bool SaveFileObj() {
    tptree ptroot;
    //默认之前检查有效。不检查直接写入
    tptree ptprj;
    ptprj.put(T_TEXT("<xmlattr>.Name"), m_fileobj->m_PrjName);
    ptprj.put(T_TEXT("<xmlattr>.Version"), m_fileobj->m_PrjVersion);
    if (!writeSettings(m_fileobj->m_PrjSettings, ptprj)) return false;
    std::map<tstring, ItemInfo>& tmpiteminfo = m_fileobj->m_PrjItems;
    if (tmpiteminfo.size() > 0) {
      tptree ptItemGroup;
      for (std::map<tstring, ItemInfo>::const_iterator itr = tmpiteminfo.begin();
           itr != tmpiteminfo.end(); ++itr) {
        tptree ptitem;
        ptitem.put(T_TEXT("<xmlattr>.itemname"), itr->first);
        ptitem.put(T_TEXT("<xmlattr>.type"), itr->second.type);
        if (!writeSettings(itr->second.settings, ptitem)) return false;
        ptItemGroup.add_child(T_TEXT("Item"), ptitem);
      }
      ptprj.put_child(T_TEXT("ItemGroup"), ptItemGroup);
    }
    ptroot.add_child(T_TEXT("Project"), ptprj);
    return writeXml(m_filepath, ptroot);
  }
};

}  // namespace ytlib