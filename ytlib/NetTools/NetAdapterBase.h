#pragma once

#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <shared_mutex>
#include <mutex> 

namespace ytlib {

	/*
	网络适配器：
	使用时需要建立一个主机id与其网络信息的map，然后发送时只需传递数据和目的地id即可

	*/

	//网络适配器模板
	template<class _HostInfo,//主机信息格式
		class _TransData,
		class _RecvData = _TransData> //默认接收数据格式与发送数据格式相同
	class NetAdapterBase {
	public:

		/*参数：
		myid_：自身id
		hostInfo_：自身主机信息
		recvcb_：接收数据回调，应当是非阻塞、立即返回的
		这里的通道初始化可以改？
		*/
		NetAdapterBase(uint32_t myid_, const _HostInfo & hostInfo_,
			std::function<void(_RecvData &)> recvcb_):
			m_myid(myid_),
			m_receiveCallBack(recvcb_)
		{
			m_mapHostInfo[m_myid] = hostInfo_;
		};
		virtual ~NetAdapterBase() {

		}

		//提供给发送方的发送接口
		virtual bool Send(const _TransData & Tdata_, uint32_t dst_) {
			if (dst_ == m_myid) return false;
			std::vector<_HostInfo> vec_hostinfo;
			m_hostInfoMutex.lock_shared();
			std::map<uint32_t, _HostInfo>::const_iterator itr = m_mapHostInfo.find(dst_);
			if (itr == m_mapHostInfo.end()) return false;
			vec_hostinfo.push_back(itr->second);
			m_hostInfoMutex.unlock_shared();
			return _sendfun(Tdata_, vec_hostinfo);
		}

		virtual bool Send(const _TransData & Tdata_,const std::vector<uint32_t>& dst_) {
			std::vector<_HostInfo> vec_hostinfo;
			size_t size = dst_.size();
			if (size == 0) return false;
			m_hostInfoMutex.lock_shared();
			for (size_t ii = 0; ii < size; ++ii) {
				if (dst_[ii] == m_myid) return false;
				std::map<uint32_t, _HostInfo>::const_iterator itr = m_mapHostInfo.find(dst_[ii]);
				if (itr == m_mapHostInfo.end()) return false;
				vec_hostinfo.push_back(itr->second);
			}
			m_hostInfoMutex.unlock_shared();
			return _sendfun(Tdata_, vec_hostinfo);
		}
		
		//hostinfo操作
		inline _HostInfo GetMyHostInfo() {
			std::shared_lock<std::shared_mutex> lck(m_hostInfoMutex);
			return m_mapHostInfo[m_myid];
		}
		inline _HostInfo GetHostInfo(uint32_t hostid_) {
			std::shared_lock<std::shared_mutex> lck(m_hostInfoMutex);
			std::map<uint32_t, _HostInfo>::const_iterator itr = m_mapHostInfo.find(hostid_);
			if (itr != m_mapHostInfo.end()) return itr->second;
			else return _HostInfo();//用异常？
		}
		//设置主机info，有则覆盖，无责添加
		bool SetHost(uint32_t hostid_, const _HostInfo & hostInfo_) {
			std::unique_lock<std::shared_mutex> lck(m_hostInfoMutex);
			m_mapHostInfo[hostid_] = hostInfo_;
			return true;
		}
		//移除主机info。不可移除本机info
		bool RemoveHost(uint32_t hostid_) {
			if (hostid_ == m_myid) return false;
			std::unique_lock<std::shared_mutex> lck(m_hostInfoMutex);
			std::map<uint32_t, _HostInfo>::iterator itr = m_mapHostInfo.find(hostid_);
			if (itr == m_mapHostInfo.end()) return false;
			m_mapHostInfo.erase(itr);
			return true;
		}


	protected:
		//发送函数
		virtual bool _sendfun(const _TransData & Tdata_, const std::vector<_HostInfo>& dst_) = 0;

		const uint32_t m_myid;//自身id，构造之后无法修改
		std::shared_mutex m_hostInfoMutex;//主机列表的读写锁
		std::map<uint32_t, _HostInfo> m_mapHostInfo;//主机列表：id-info
		std::function<void(_RecvData &)> m_receiveCallBack;//回调，直接供子类调用

	};
}