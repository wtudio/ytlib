#pragma once
#include <sigar/sigar.h>
#include <boost/serialization/singleton.hpp>
extern "C"
{
#include  <sigar/sigar_format.h>
}

namespace ytlib
{
	//一个封装了sigar的全局单例。注意，在连续调用GetCpuUsage或GetMemUsage时应保证一定的时间间隔
	class SysInfo {
	public:
		SysInfo() {
			sigar_open(&sigar_core);
			old = new sigar_cpu_t();
			current = new sigar_cpu_t();
			sigar_cpu_get(sigar_core, old);
		}
		~SysInfo() {
			sigar_close(sigar_core);
			delete old;
			delete current;
		}
		double GetCpuUsage() {
			sigar_cpu_get(sigar_core, current);
			sigar_cpu_perc_calculate(old, current, &perc);
			std::swap(old, current);
			return perc.combined;
		}
		double GetMemUsage() {
			sigar_mem_get(sigar_core, &currentmem);
			return  ((double)currentmem.used / (double)currentmem.total);
		}

	private:
		sigar_t *sigar_core;
		sigar_cpu_t *old;
		sigar_cpu_t *current;

		sigar_cpu_perc_t perc;
		sigar_mem_t currentmem;

	};

	typedef boost::serialization::singleton<SysInfo> SingletonSysInfo;

	static double GetCpuUsage() {
		return SingletonSysInfo::get_mutable_instance().GetCpuUsage();
	}
	static double GetMemUsage() {
		return SingletonSysInfo::get_mutable_instance().GetMemUsage();
	}
}