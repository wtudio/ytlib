#pragma once

#include <ytlib/FileManager/FileBase.h>
#include <ytlib/FileManager/KeyValueFile.h>
#include <ytlib/FileManager/SerializeFile.h>
#include <ytlib/FileManager/XMLFile.h>
#include <ytlib/FileManager/PrjBase.h>

namespace ytlib
{
	bool test_KeyValueFile();
	bool test_SerializeFile();
	bool test_XMLFile();
	bool test_PrjBase();
}