/**
 * @file xml_tools.hpp
 * @brief XML工具
 * @note 基于boost的XML解析写入工具
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <filesystem>

#include <boost/program_options/detail/convert.hpp>
#include <boost/program_options/detail/utf8_codecvt_facet.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace ytlib {

/**
 * @brief 加载xml文件
 * @param[in] filepath xml文件路径，可以使用相对路径
 * @param[out] pt xml结构root
 */
inline void ReadXmlFile(const std::filesystem::path& filepath, boost::property_tree::ptree& pt) {
  const std::filesystem::path& p = std::filesystem::absolute(filepath);
  boost::property_tree::read_xml(p.string(), pt,
                                 boost::property_tree::xml_parser::trim_whitespace);
}

/**
 * @brief 写入xml文件
 * @param[in] filepath xml文件路径，可以使用相对路径
 * @param[out] pt xml结构root
 */
inline void WriteXmlFile(const std::filesystem::path& filepath, const boost::property_tree::ptree& pt) {
  const std::filesystem::path& p = std::filesystem::absolute(filepath);
  const auto& parent_path = p.parent_path();
  if (std::filesystem::status(parent_path).type() != std::filesystem::file_type::directory)
    std::filesystem::create_directories(parent_path);

  boost::property_tree::write_xml(p.string(), pt,
                                  std::locale(),
                                  boost::property_tree::xml_writer_settings<std::string>(' ', 2));
}

/**
 * @brief 读取kv结构到map<string,string>中
 * @note 读取以下结构到map<string,string>中：
 * @code
<Settings>
  <setting key="key1" value="value1" />
  <setting key="key2" value="value2" />
  <setting key="key3" value="value3" />
</Settings>
 * @endcode
 * @param[in] pt xml结构root
 * @param[out] output_map 输出的map结构
 * @param[in] settings_node 数组节点名称
 * @param[in] setting_node kv节点名称
 * @return 是否写入成功
 */
inline bool ReadXmlSettings(const boost::property_tree::ptree& pt, std::map<std::string, std::string>& output_map,
                            const std::string& settings_node = "Settings",
                            const std::string& setting_node = "setting") {
  using ptreetype = boost::property_tree::ptree;
  boost::optional<const ptreetype&> pt_settings = pt.get_child_optional(settings_node);
  if (!pt_settings) return false;

  for (ptreetype::const_iterator itr = pt_settings->begin(); itr != pt_settings->end(); ++itr) {
    if (itr->first != setting_node) continue;
    output_map.emplace(itr->second.get<std::string>("<xmlattr>.key"), itr->second.get<std::string>("<xmlattr>.value"));
  }
  return true;
}

/**
 * @brief 添加map<string,string>到xml的kv结构中
 * @note 添加map<string,string>到xml的kv结构中：
 * @code
<Settings>
  <setting key="key1" value="value1" />
  <setting key="key2" value="value2" />
  <setting key="key3" value="value3" />
</Settings>
 * @endcode
 * @param[in] input_map map结构
 * @param[out] pt xml结构root
 * @param[in] settings_node 数组节点名称
 * @param[in] setting_node kv节点名称
 */
inline void AddXmlSettings(const std::map<std::string, std::string>& input_map, boost::property_tree::ptree& pt,
                           const std::string& settings_node = "Settings",
                           const std::string& setting_node = "setting") {
  if (input_map.empty()) return;

  using ptreetype = boost::property_tree::ptree;
  ptreetype pt_settings;
  for (const auto& itr : input_map) {
    ptreetype pt_setting;
    pt_setting.put("<xmlattr>.key", itr.first);
    pt_setting.put("<xmlattr>.value", itr.second);
    pt_settings.add_child(setting_node, std::move(pt_setting));
  }
  pt.put_child(settings_node, std::move(pt_settings));
}

}  // namespace ytlib
