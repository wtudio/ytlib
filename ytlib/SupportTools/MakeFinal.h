#pragma once

namespace ytlib
{
	/*
	制作一个无法被继承的类：
	class FinalClass2 : virtual public MakeFinal<FinalClass2>{
	public :
		FinalClass2() {}
		~FinalClass2() {}

	};
	*/
	template <typename T>
	class MakeFinal {
		friend T;
	private:
		MakeFinal() {}
		~MakeFinal() {}
	};


}


