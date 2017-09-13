#include "t_file.h"

namespace ytlib
{

	bool test_KeyValueFile() {

		KeyValueFile f1;
		f1.NewFile(T_TEXT("t_file/testfile3.txt"));
		std::shared_ptr<std::map<std::string, std::string> > o1 = f1.m_fileobj;
		(*o1)["aaa"] = "bbb";
		(*o1)["ccc"] = "ddd";
		(*o1)["测试1"] = "测试2";
		f1.SaveFile();


		KeyValueFile f2;
		f2.OpenFile(T_TEXT("t_file/testfile3.txt"));
		std::shared_ptr<std::map<std::string, std::string> > o2 = f2.m_fileobj;
		
		for (std::map<std::string, std::string>::const_iterator itr = o2->begin();
		itr != o2->end(); ++itr) {
			std::cout << itr->first << " = " << itr->second << std::endl;
		}
		(*o2)["测试1"] = "测试3";
		f2.SaveFile(T_TEXT("t_file/testfile4.txt"));
		

		return true;
	}

	class SFTestObj {
		T_CLASS_SERIALIZE(&s1&s2&i1&i2)
	public:
		std::string s1;
		std::string s2;
		int32_t i1;
		int32_t i2;
	};
	typedef SerializeFile<SFTestObj> SFTestFile;

	bool test_SerializeFile() {
		SFTestFile f1;
		f1.NewFile(T_TEXT("t_file/testfile.txt"));
		std::shared_ptr<SFTestObj> o = f1.m_fileobj;
		o->s1 = "sssadafasf";
		o->s2 = "测试";
		o->i1 = 1067;
		o->i2 = 164;
		f1.SaveFile();
		tcout << f1.GetFileName() << std::endl;
		tcout << f1.GetFileParentPath() << std::endl;

		SFTestFile f2;
		f2.OpenFile(T_TEXT("t_file/testfile.txt"));
		f2.SaveFile(T_TEXT("t_file/testfile2.txt"));
		
		return true;
	}
	bool test_XMLFile() {
		XMLFile f1;
		f1.NewFile();

		f1.SaveFile(T_TEXT("t_file/TrunkCenter2.xml"));
		return true;
	}
	bool test_PrjBase() {

		PrjFile f1;
		f1.NewFile();
		f1.setPrjName(T_TEXT("testprj2"));
		f1.SaveFile(T_TEXT("t_file/testprj2.prj"));

		return true;
	}
}