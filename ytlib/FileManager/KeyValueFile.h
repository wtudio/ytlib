#pragma once
#include <ytlib/FileManager/FileBase.h>
#include <map>
#include <fstream>

namespace ytlib
{
	//使用map格式。涉及到存储的都弄成string
	class KeyValueFile :public FileBase<std::map<std::string, std::string>>
	{
	public:
		KeyValueFile() :FileBase() {}
		virtual ~KeyValueFile() {}
	protected:
		//从打开的文件中解析获取文件内容结构体
		virtual bool GetFileObj() {
			if (!CreateFileObj()) return false;
			tpath path = tGetAbsolutePath(m_filepath);

			std::ifstream infile(path.string<tstring>().c_str(), std::ios::in);
			if (infile) {
				std::string buf;
				for (; getline(infile, buf);) {
					size_t pos = buf.find_first_of('=');
					if (pos != std::string::npos) {
						std::string key(buf.substr(0, pos)); boost::trim(key);
						std::string value(buf.substr(pos + 1)); boost::trim(value);
						(*m_fileobj)[key] = value;
					}
				}
				infile.close();
				return true;
			}
			return false;
		}
		//将当前的文件内容结构体保存为文件
		virtual bool SaveFileObj() {
			std::ofstream ofile(m_filepath.c_str(), std::ios::trunc);
			if (ofile) {
				for (std::map<std::string, std::string>::iterator itr = m_fileobj->begin(); itr != m_fileobj->end(); ++itr) {
					ofile << itr->first << " = " << itr->second << std::endl;
				}
				ofile.close();
				return true;
			}
			return false;
		}
	};
}