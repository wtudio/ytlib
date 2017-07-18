#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/program_options/detail/convert.hpp>
#include <boost/program_options/detail/utf8_codecvt_facet.hpp>
#include <ytlib/Common/TString.h>
#include <ytlib/Common/FileSystem.h>

namespace wtlib
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
				boost::property_tree::xml_writer_settings<tstring>(WT_TEXT('\t'), 1));
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
	static bool readSettings(const tptree& pt, std::map<tstring, tstring>& inputmap_) {
		try {
			boost::optional<const tptree&> ptSettings = pt.get_child_optional(WT_TEXT("Settings"));
			if (ptSettings) {
				for (tptree::const_iterator itrptsetting = ptSettings->begin(); itrptsetting != ptSettings->end(); ++itrptsetting) {
					inputmap_[itrptsetting->second.get<tstring>(WT_TEXT("<xmlattr>.key"))] = itrptsetting->second.get<tstring>(WT_TEXT("<xmlattr>.value"));
				}
			}
		}
		catch (const std::exception& e) {
			std::cout << "read settings failed : " << e.what() << std::endl;
			return false;
		}
		return true;
	}
	//添加settings节点
	static bool writeSettings(const std::map<tstring, tstring>& inputmap_, tptree& pt ) {
		try {
			if (inputmap_.size() == 0) return true;
			tptree ptSettings;
			for (std::map<tstring, tstring>::const_iterator itr = inputmap_.begin(); itr != inputmap_.end(); ++itr) {
				tptree ptsetting;
				ptsetting.put(WT_TEXT("<xmlattr>.key"), itr->first);
				ptsetting.put(WT_TEXT("<xmlattr>.value"), itr->second);
				ptSettings.add_child(WT_TEXT("setting"), ptsetting);
			}
			pt.put_child(WT_TEXT("Settings"), ptSettings);
		}
		catch (const std::exception& e) {
			std::cout << "write settings failed : " << e.what() << std::endl;
			return false;
		}
		return true;
	}
}