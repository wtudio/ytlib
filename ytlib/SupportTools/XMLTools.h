#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/program_options/detail/convert.hpp>
#include <boost/program_options/detail/utf8_codecvt_facet.hpp>
#include <ytlib/Common/TString.h>
#include <ytlib/Common/FileSystem.h>

namespace ytlib
{
#if defined(UNICODE)
	typedef boost::property_tree::wptree tptree;
#else
	typedef boost::property_tree::ptree tptree;
#endif

	static bool realXml(const tstring& filepath, tptree& ptProfile){
		tpath path = tGetAbsolutePath(filepath);
#if defined(UNICODE)
		std::locale xml_locale(std::locale(""), new boost::program_options::detail::utf8_codecvt_facet());
#else
		std::locale xml_locale;
#endif
		try {
			boost::property_tree::read_xml(path.string(), ptProfile,
				boost::property_tree::xml_parser::trim_whitespace, xml_locale);
		}
		catch (const std::exception& e) {
			std::cout << ("load XML file failed : ") << e.what() << std::endl;
			return false;
		}
		return true;
	}


	static bool writeXml(const tstring& filepath,const tptree& ptProfile) {

		tpath path = tGetAbsolutePath(filepath);
#if defined(UNICODE)
		std::locale xml_locale(std::locale(""), new boost::program_options::detail::utf8_codecvt_facet());
#else
		std::locale xml_locale;
#endif
		try {
			boost::property_tree::write_xml(path.string(), ptProfile, xml_locale,
				boost::property_tree::xml_writer_settings<tstring>(T_TEXT('\t'), 1));
		}
		catch (const std::exception& e) {
			std::cout << ("write XML file failed : ") << e.what() << std::endl;
			return false;
		}
		return true;
	}

	//读取以下结构到map中：
	/*
	<Settings>
		<setting key="key1" value="value1" />
		<setting key="key2" value="value2" />
		<setting key="key3" value="value3" />
	</Settings>
	*/
	static bool readSettings(const tptree& pt, std::map<ytlib::tstring, ytlib::tstring>& inputmap_) {
		try {
			boost::optional<const tptree&> ptSettings = pt.get_child_optional(T_TEXT("Settings"));
			if (ptSettings) {
				for (tptree::const_iterator itrptsetting = ptSettings->begin(); itrptsetting != ptSettings->end(); ++itrptsetting) {
					inputmap_[itrptsetting->second.get<ytlib::tstring>(T_TEXT("<xmlattr>.key"))] = itrptsetting->second.get<ytlib::tstring>(T_TEXT("<xmlattr>.value"));
				}
			}
		}
		catch (const std::exception& e) {
			std::cout << "read settings failed : " << e.what() << std::endl;
			return false;
		}
		return true;
	}

/* 
	static bool readSettings(const tptree& pt, std::map<std::string, std::string>& inputmap_) {
		try {
			boost::optional<const tptree&> ptSettings = pt.get_child_optional(T_TEXT("Settings"));
			if (ptSettings) {
				for (tptree::const_iterator itrptsetting = ptSettings->begin(); itrptsetting != ptSettings->end(); ++itrptsetting) {
					inputmap_[T_TSTRING_TO_STRING(itrptsetting->second.get<ytlib::tstring>(T_TEXT("<xmlattr>.key")))] = 
						T_TSTRING_TO_STRING(itrptsetting->second.get<ytlib::tstring>(T_TEXT("<xmlattr>.value")));
				}
			}
		}
		catch (const std::exception& e) {
			std::cout << "read settings failed : " << e.what() << std::endl;
			return false;
		}
		return true;
	}
*/
	//添加settings节点
	static bool writeSettings(const std::map<ytlib::tstring, ytlib::tstring>& inputmap_, tptree& pt ) {
		try {
			if (inputmap_.size() == 0) return true;
			tptree ptSettings;
			for (std::map<ytlib::tstring, ytlib::tstring>::const_iterator itr = inputmap_.begin(); itr != inputmap_.end(); ++itr) {
				tptree ptsetting;
				ptsetting.put(T_TEXT("<xmlattr>.key"), itr->first);
				ptsetting.put(T_TEXT("<xmlattr>.value"), itr->second);
				ptSettings.add_child(T_TEXT("setting"), ptsetting);
			}
			pt.put_child(T_TEXT("Settings"), ptSettings);
		}
		catch (const std::exception& e) {
			std::cout << "write settings failed : " << e.what() << std::endl;
			return false;
		}
		return true;
	}

/*
	
	static bool writeSettings(const std::map<std::string, std::string>& inputmap_, tptree& pt) {
		try {
			if (inputmap_.size() == 0) return true;
			tptree ptSettings;
			for (std::map<std::string, std::string>::const_iterator itr = inputmap_.begin(); itr != inputmap_.end(); ++itr) {
				tptree ptsetting;
				ptsetting.put(T_TEXT("<xmlattr>.key"), T_STRING_TO_TSTRING(itr->first));
				ptsetting.put(T_TEXT("<xmlattr>.value"), T_STRING_TO_TSTRING(itr->second));
				ptSettings.add_child(T_TEXT("setting"), ptsetting);
			}
			pt.put_child(T_TEXT("Settings"), ptSettings);
		}
		catch (const std::exception& e) {
			std::cout << "write settings failed : " << e.what() << std::endl;
			return false;
		}
		return true;
	}
*/
}