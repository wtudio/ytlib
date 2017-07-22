#pragma once
#include <ytlib/ProcessManager/ProcessBase.h>
#include <ytlib/SupportTools/ChannelBase.h>
#include <ytlib/Common/TString.h>
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

	使用通道进行处理（不一定最优）
	*/



	//网络适配器模板，异步发送
	template<class _HostInfo,//主机信息格式
	class _TransData,
	class _RecvData = _TransData> //默认接收数据格式与发送数据格式相同
	class NetAdapterBase : public ProcessBase {
	public:

		typedef std::pair<_TransData, std::vector<uint32_t>> _TransData_dst;//数据及其目的地列表。支持群发、重复发送

		/*参数：
		myid_：自身id
		hostInfo_：自身主机信息
		recvcb_：接收数据回调
		这里的通道初始化可以改？
		*/
		NetAdapterBase(uint32_t myid_, const _HostInfo & hostInfo_,
			std::function<void(_RecvData &)> recvcb_):
			ProcessBase(),
			m_myid(myid_),
			m_bStopFlag(false),
			recvcallback(recvcb_)
		{
			m_mapHostInfo[m_myid] = hostInfo_;
		};
		virtual ~NetAdapterBase() {
			stop();
		}

		virtual bool init() {
			m_TChannelPtr = std::shared_ptr<ChannelBase<_TransData_dst>>(
				new ChannelBase<_TransData_dst>(std::bind(&NetAdapterBase::_sendfun, this, std::placeholders::_1), 1, 1000));
			m_RChannelPtr = std::shared_ptr<ChannelBase<_RecvData>>(
				new ChannelBase<_RecvData>(recvcallback, 1, 1000));
			return ProcessBase::init();
		}

		//提供给发送方的发送接口
		inline bool Send(const _TransData & Tdata_,const std::vector<uint32_t>& dst_) {
			return is_running && m_TChannel.Add(std::move(std::make_pair<_TransData, std::vector<uint32_t>>(Tdata_, dst_)));
		}
		
		//hostinfo操作
		inline _HostInfo GetMyHostInfo() {
			std::shared_lock<std::shared_mutex> lck(m_hostInfoMutex);
			return m_mapHostInfo[m_myid];
		}
		inline _HostInfo GetHostInfo(uint32_t hostid_) {
			std::shared_lock<std::shared_mutex> lck(m_hostInfoMutex);
			std::map<uint32_t, _HostInfo>::iterator itr = m_mapHostInfo.find(hostid_);
			if (itr != m_mapHostInfo.end()) return itr->second;
			else return _HostInfo();//用异常？
		}
		//设置主机info，有则覆盖，无责添加
		bool SetHost(uint32_t hostid_, const _HostInfo & hostInfo_) {
			std::unique_lock<std::shared_mutex> lck(m_hostInfoMutex);
			m_mapHostInfo[hostid_] = hostInfo_;
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
		//提供给接收线程的接收数据接口。供子类调用
		inline bool Recv(const _RecvData & Rdata_) {
			return is_running && m_RChannel.Add(Rdata_);
		}
		//发送函数
		virtual void _sendfun(const _TransData_dst& Tdata_) = 0;


	protected:
		const uint32_t m_myid;//自身id，构造之后无法修改

		std::shared_mutex m_hostInfoMutex;//主机列表的读写锁
		std::map<uint32_t, _HostInfo> m_mapHostInfo;//主机列表：id-info

		std::function<void(_RecvData &)> m_receiveCallBack;//回调
		std::atomic_bool m_bStopFlag;

		std::shared_ptr<ChannelBase<_TransData_dst>> m_TChannelPtr;//发送通道
		std::shared_ptr<ChannelBase<_RecvData>> m_RChannelPtr;//接收通道
		std::function<void(_RecvData &)> recvcallback;//接收回掉函数
	};
}