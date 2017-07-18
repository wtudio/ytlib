#pragma once
#include <ytlib/ProcessManager/ProcessBase.h>
#include <ytlib/SupportTools/ChannelBase.h>
#include <ytlib/Common/TString.h>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <mutex>

namespace wtlib {

	//网络适配器模板，异步发送
	template<class _HostInfo,//主机信息格式
	class _RecvData,
	class _TransData>
	class NetAdapterBase : public ProcessBase {
	public:

		typedef std::pair<_TransData, std::vector<tstring>> _TransData_dst;//数据及其目的地列表。支持群发、重复发送

		/*参数：
		myid_：自身id
		hostInfo_：自身主机信息
		recvcb_：接收数据回调
		*/
		NetAdapterBase(const tstring& myid_, const _HostInfo & hostInfo_, 
			std::function<void(_RecvData &)> recvcb_):
			ProcessBase(),
			m_myid(myid_),
			m_bStopFlag(false),
			m_TChannel(std::bind(&NetAdapterBase::_sendfun,this, std::placeholders::_1),1,1000),
			m_RChannel(recvcb_,1,1000)
		{
			m_mapHostInfo[m_myid] = hostInfo_;
		};
		virtual ~NetAdapterBase() {
			stop();
		}

		//提供给发送方的发送接口
		inline bool Send(const _TransData & Tdata_,const std::vector<tstring>& dst_) {
			return is_running && m_TChannel.Add(std::move(std::make_pair<_TransData, std::vector<tstring>>(Tdata_, dst_)));
		}
		

		//hostinfo操作
		inline _HostInfo GetMyHostInfo() {
			std::lock_guard<std::mutex> lck(m_hostInfoMutex);
			return m_mapHostInfo[m_myid];
		}
		inline _HostInfo GetHostInfo(const tstring & hostid_) {
			std::lock_guard<std::mutex> lck(m_hostInfoMutex);
			std::map<tstring, _HostInfo>::iterator itr = m_mapHostInfo.find(hostid_);
			if (itr != m_mapHostInfo.end()) return itr->second;
			else return _HostInfo();//用异常？
		}
		//设置主机info，有则覆盖，无责添加
		bool SetHost(const tstring & hostid_, const _HostInfo & hostInfo_) {
			std::lock_guard<std::mutex> lck(m_hostInfoMutex);
			m_mapHostInfo[hostid_] = hostInfo_;
		}
		//移除主机info。不可移除本机info
		bool RemoveHost(const tstring & hostid_) {
			if (hostid_ == m_myid) return false;
			std::lock_guard<std::mutex> lck(m_hostInfoMutex);
			std::map<tstring, _HostInfo>::iterator itr = m_mapHostInfo.find(hostid_);
			if (itr == m_mapHostInfo.end()) return false;
			m_mapHostInfo.erase(itr);
			return true;
		}

	protected:
		//提供给接收线程的接收数据接口。供子类调用
		inline bool Recv(const _RecvData & Rdata_) {
			return is_running && m_RChannel.Add(Rdata_);
		}
		//发送函数
		void _sendfun(const _TransData_dst& Tdata_) {
			std::vector<tstring> &dstVec = Tdata_.second;
			size_t len = dstVec.size();
			for (size_t ii = 0; ii < len; ii++) {
				_finalsend(std::move(Tdata_.first), GetHostInfo(dstVec[ii]));
			}
		}
		virtual void _finalsend(const _TransData& data_, const _HostInfo& hostinfo_) = 0;//最终发送函数

		const tstring m_myid;//自身id，构造之后无法修改

		std::mutex m_hostInfoMutex;
		std::map<tstring, _HostInfo> m_mapHostInfo;//主机列表：id-info

		std::function<void(_RecvData &)> m_receiveCallBack;//回调
		std::atomic_bool m_bStopFlag;

		ChannelBase<_TransData_dst> m_TChannel;//发送通道
		ChannelBase<_RecvData> m_RChannel;//接收通道
		

	};
}