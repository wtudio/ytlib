#pragma once
#include <ytlib/NetTools/NetAdapterBase.h>
#include <ytlib/SupportTools/Serialize.h>
#include <ytlib/NetTools/TcpConnectionPool.h>
#include <ytlib/Common/FileSystem.h>
#include <boost/asio.hpp>
#include <future>

namespace ytlib {

	/*
	使用boost.asio的一个简易网络适配器

	使用boost序列化的方式直接将类序列化到发送流
	直接将接收流反序列化到类


	除了类成员外，还支持：
	1、将数据拷贝进vec_datas中
	2、一些创建在堆中、确保生存期并且只读的数据，可以取其智能指针放于vec_datasPtr中
	3、文件发送。需要确立发送、接收目录，否则直接放到可执行文件目录下

	考虑跨平台性，牵涉到网络传输的数据一律使用:
	char
	标准int（不能用int、long、size_t这些）


	实际使用时对DataPackage再加一层模板（不要用继承）
	-------------------------------------------------------
	
	所有数据都使用shared_buf的形式存储。提供一些重载的函数将一些常见数据形式转变成shared_buf。（还可以拓展自行添加）
	提供两种存入待发送数据的方式：
	1、safe方式：将数据实际内容拷贝到一个shared_buf

	2、unsafe方式：只支持传入shared_buf。用户需要保证数据不被其主动删除或修改
	
	(实际上这里只是做一个示例，仅一些改动不大的情况会直接使用这样的方式。具体情况还是得具体写数据结构来达到最高效率)

	----------------------------------------------------
	传输使用以下格式：
	先传一个报头：（8 byte）
	head: 2 byte

	tag: 2 byte
	c+0:对象
	d+255:所有数据的tip，以\n分割
	d+i:第i个数据
	f+255:所有文件的tip+文件名（只有文件名，没有路径），格式：tip1=filename1\ntip2=filename2\n...
	f+i:第i个文件的数据
	o+v:结束符

	size: 4 byte ：默认windows，即小端
	num = byte1+byte2*256+byte3*65536+byte4*2^24
	byte1=num%256,byte2=(num/256) ...

	然后传输size个byte的数据

	*/

	struct shared_buf {
		boost::shared_array<char> buf;
		uint32_t buf_size;
	};

	//T需要能boost序列化
	template<class T>
	class DataPackage {
	public:
		T obj;//可序列化的类
		//tip-data形式
		std::map<std::string, shared_buf> map_datas;//数据,最大支持255个
		std::map<std::string, std::string> map_files;//文件,最大支持255个
	};

	template<class T>
	class TcpNetAdapter : public NetAdapterBase<TcpEp, std::shared_ptr<DataPackage<T>>> {
	protected:
#define TCPHEAD1 'Y'
#define TCPHEAD2 'T'
#define CLASSHEAD 'C'
#define DATAHEAD 'D'
#define FILEHEAD 'F'
#define TCPEND1 'O'
#define TCPEND2 'V'

#define HEAD_SIZE 8
		static void set_buf_from_num(char* p, uint32_t n) {
#ifdef _MSC_VER
			memcpy(p, &n, 4);
#else
			p[0] = char(n % 256); n /= 256;	p[1] = char(n % 256); n /= 256;
			p[2] = char(n % 256); n /= 256;	p[3] = char(n % 256);
#endif // _MSC_VER
		}
		static uint32_t get_num_from_buf(char* p) {
#ifdef _MSC_VER
			uint32_t n;	memcpy(&n, p, 4); return n;
#else
			return (p[0] + p[1] * 256 + p[2] * 65536 + p[3] * 256 * 65536);
#endif // _MSC_VER
		}
	

		typedef std::shared_ptr<DataPackage<T>> dataPtr;

		//子类：连接------------------------------------------------------------------------
		class TcpConnection :public TcpConnectionBase {
			typedef std::shared_ptr<boost::asio::streambuf> buff_Ptr;
		public:
			TcpConnection(boost::asio::io_service& io_, 
				std::function<void(const TcpEp &)> errcb_,
				tpath* p_RecvPath_,
				std::function<void(dataPtr &)> cb_):
				TcpConnectionBase(io_, errcb_), 
				m_recv_callback(cb_), 
				p_RecvPath(p_RecvPath_){

			}
			virtual ~TcpConnection() {}

			//同步发送，加锁
			bool write(const std::shared_ptr<std::vector<boost::asio::const_buffer>> & data_) {
				boost::system::error_code err;
				write_mutex.lock();
				sock.write_some(*data_, err);
				write_mutex.unlock();
				if (err) {
					printf_s("write failed : %s\n", err.message().c_str());
					err_CallBack(remote_ep);
					return false;
				}
				return true;
			}
			
			void start() {
				boost::asio::async_read(sock, boost::asio::buffer(header), boost::asio::transfer_exactly(HEAD_SIZE),
					std::bind(&TcpConnection::on_read_head, this, std::placeholders::_1, std::placeholders::_2));
			}

			bool read_get_err(const boost::system::error_code & err) {
				if (err) {
					printf_s("read failed : %s\n", err.message().c_str());
					err_CallBack(remote_ep);
					return true;
				}
				return false;
			}
			//接收完成后先开启下一个异步接收再最后调用回调
			void on_read_head(const boost::system::error_code & err, size_t read_bytes) {
				if (read_get_err(err)) return;
				//解析报头
				if (header[0] == TCPHEAD1 && header[1] == TCPHEAD2 && read_bytes == HEAD_SIZE) {
					uint32_t pack_size = get_num_from_buf(&header[4]);
					if (header[2] == CLASSHEAD) {
						if (RDataPtr) {
							printf_s("read failed : incomplete package\n");
							err_CallBack(remote_ep);
							return;
						}
						RDataPtr = dataPtr(new DataPackage<T>());
						buff_Ptr buff_ = buff_Ptr(new boost::asio::streambuf());
						boost::asio::async_read(sock, *buff_, boost::asio::transfer_exactly(pack_size),
							std::bind(&TcpConnection::on_read_obj, this, buff_, std::placeholders::_1, std::placeholders::_2));
						return;
					}
					if (header[2] == DATAHEAD) {
						if (header[3] == static_cast<char>(uint8_t(255))) {

						}
						else {

						}
						return;
					}
					if (header[2] == FILEHEAD) {
						if (header[3] == static_cast<char>(uint8_t(255))) {

						}
						else {

						}
						return;
					}
					if (header[2] == TCPEND1 && header[3] == TCPEND2) {
						dataPtr re = RDataPtr;
						RDataPtr.reset();
						boost::asio::async_read(sock, boost::asio::buffer(header), boost::asio::transfer_exactly(HEAD_SIZE),
							std::bind(&TcpConnection::on_read_head, this, std::placeholders::_1, std::placeholders::_2));
						//需要等待所有操作完成
						while (!re.unique());
						m_recv_callback(std::move(re));
						return;
					}
				}

				printf_s("read failed : recv an invalid header\n");
				err_CallBack(remote_ep);
				return;
			}

			void on_read_obj(buff_Ptr& buff_,const boost::system::error_code & err, size_t read_bytes) {
				if (read_get_err(err)) return;
				boost::asio::async_read(sock, boost::asio::buffer(header), boost::asio::transfer_exactly(HEAD_SIZE),
					std::bind(&TcpConnection::on_read_head, this, std::placeholders::_1, std::placeholders::_2));
				boost::archive::binary_iarchive iar(*buff_);
				iar >> RDataPtr->obj;
			}
			void on_read_data_tips(const boost::system::error_code & err, size_t read_bytes) {

			}
			void on_read_data(const boost::system::error_code & err, size_t read_bytes) {

			}
			void on_read_file_tips(const boost::system::error_code & err, size_t read_bytes) {

			}
			void on_read_file(const boost::system::error_code & err, size_t read_bytes) {

			}


		protected:
			std::mutex write_mutex;
			std::function<void(dataPtr &)> m_recv_callback;
			tpath* p_RecvPath;//接收文件路径

			//接收缓存
			dataPtr RDataPtr;
			char header[HEAD_SIZE];

		};
		//子类：连接池---------------------------------------------------------------------
		class TcpConnectionPool : public TcpConnectionPoolBase<TcpConnection> {
		public:
			TcpConnectionPool(uint16_t port_, 
				tpath* p_RecvPath_,
				std::function<void(dataPtr &)> cb_) :
				TcpConnectionPoolBase(port_), 
				p_RecvPath(p_RecvPath_),
				m_recv_callback(cb_){

			}
			virtual ~TcpConnectionPool() {}

		protected:
			//工厂函数
			TcpConnectionPtr getNewTcpConnectionPtr() {
				return TcpConnectionPtr(new TcpConnection(service, 
					std::bind(&TcpConnectionPoolBase<TcpConnection>::on_err, this, std::placeholders::_1),
					p_RecvPath,
					m_recv_callback
					));
			}
			std::function<void(dataPtr &)> m_recv_callback;
			tpath* p_RecvPath;//接收文件路径
		};
		//类本体-------------------------------------------------------------------------
	public:
		TcpNetAdapter(uint32_t myid_,
			const TcpEp & hostInfo_,
			std::function<void(std::shared_ptr<DataPackage<T>> &)> recvcb_,
			const tstring& rp = T_TEXT(""),	const tstring& sp = T_TEXT("")) :
			NetAdapterBase(myid_, hostInfo_, recvcb_) ,
			m_RecvPath(tGetAbsolutePath(rp)), m_SendPath(tGetAbsolutePath(sp)),
			m_TcpConnectionPool(hostInfo_.port(),&m_RecvPath, m_receiveCallBack){
			boost::filesystem::create_directories(m_RecvPath);
			boost::filesystem::create_directories(m_SendPath);
			m_TcpConnectionPool.start();
		}
		virtual ~TcpNetAdapter(){}


	protected:
		virtual bool _sendfun(const dataPtr & Tdata_, const std::vector<TcpEp>& dst_) {
			//将Tdata_先转换为std::vector<boost::asio::const_buffer>再一次性发送
			std::shared_ptr<std::vector<boost::asio::const_buffer>> buffersPtr = 
				std::shared_ptr<std::vector<boost::asio::const_buffer>>(new std::vector<boost::asio::const_buffer>());
			//第一步：发送对象
			char c_head_buff[HEAD_SIZE]{ TCPHEAD1 ,TCPHEAD2,CLASSHEAD,0 };
			boost::asio::streambuf objbuff;
			boost::archive::binary_oarchive oar(objbuff);
			oar << Tdata_->obj;
			set_buf_from_num(&c_head_buff[4], static_cast<uint32_t>(objbuff.size()));
			buffersPtr->push_back(std::move(boost::asio::const_buffer(c_head_buff, HEAD_SIZE)));
			buffersPtr->push_back(std::move(objbuff.data()));
			//第二步，发送数据
			//先发送tips数据包
			std::map<std::string, shared_buf>& map_datas = Tdata_->map_datas;
			//缓冲区不能放在内层scope里
			std::string data_tips;
			char d0_head_buff[HEAD_SIZE]{ TCPHEAD1 ,TCPHEAD2,DATAHEAD,static_cast<char>(uint8_t(255)) };
			boost::shared_array<char> d_head_buff = boost::shared_array<char>(new char[map_datas.size() * HEAD_SIZE]);
			if (map_datas.size() > 0) {
				for (std::map<std::string, shared_buf>::const_iterator itr = map_datas.begin(); itr != map_datas.end(); ++itr) {
					data_tips += itr->first;
					data_tips += '\n';
				}
				set_buf_from_num(&d0_head_buff[4], static_cast<uint32_t>(data_tips.size()));
				buffersPtr->push_back(std::move(boost::asio::const_buffer(d0_head_buff, HEAD_SIZE)));
				buffersPtr->push_back(std::move(boost::asio::buffer(data_tips)));
				//再发送每个数据包。size为0则不发送
				uint8_t ii = 0;
				for (std::map<std::string, shared_buf>::const_iterator itr = map_datas.begin(); itr != map_datas.end(); ++itr) {
					if (itr->second.buf_size > 0) {
						size_t cur_offerset = ii * HEAD_SIZE;
						memcpy(&d_head_buff[cur_offerset], d0_head_buff, 3);
						d_head_buff[cur_offerset + 3] = static_cast<char>(ii);
						set_buf_from_num(&d_head_buff[cur_offerset + 4], itr->second.buf_size);
						buffersPtr->push_back(std::move(boost::asio::const_buffer(&d_head_buff[cur_offerset], HEAD_SIZE)));
						buffersPtr->push_back(std::move(boost::asio::const_buffer(itr->second.buf.get(), itr->second.buf_size)));
					}
					++ii;
				}
			}
			
			//第三步，发送文件
			//先发送文件名信息
			std::map<std::string, std::string>& map_files = Tdata_->map_files;
			std::string file_tips;
			char f0_head_buff[HEAD_SIZE]{ TCPHEAD1 ,TCPHEAD2,FILEHEAD,static_cast<char>(uint8_t(255)) };
			boost::shared_array<char> f_head_buff = boost::shared_array<char>(new char[map_files.size() * HEAD_SIZE]);
			std::vector<boost::shared_array<char>> vec_file_buf;
			if (map_files.size() > 0) {
				for (std::map<std::string, std::string>::const_iterator itr = map_files.begin(); itr != map_files.end(); ++itr) {
					file_tips += itr->first; file_tips += '=';//tip
					file_tips += boost::filesystem::path(itr->second).filename().string(); file_tips += '\n';
				}
				set_buf_from_num(&f0_head_buff[4], static_cast<uint32_t>(file_tips.size()));
				buffersPtr->push_back(std::move(boost::asio::const_buffer(f0_head_buff, HEAD_SIZE)));
				buffersPtr->push_back(std::move(boost::asio::buffer(file_tips)));
				//再发送每个文件。二进制方式。文件不存在的话就跳过该文件
				uint8_t ii = 0;
				for (std::map<std::string, std::string>::const_iterator itr = map_files.begin(); itr != map_files.end(); ++itr) {
					tpath file_path(T_STRING_TO_TSTRING(itr->second));
					if (!file_path.is_absolute()) {
						file_path = m_SendPath / file_path;
					}
					std::ifstream f(file_path.string<tstring>(), ios::in | ios::binary);
					if (f) {
						size_t cur_offerset = ii * HEAD_SIZE;
						memcpy(&f_head_buff[cur_offerset], f0_head_buff, 3);
						f_head_buff[cur_offerset + 3] = static_cast<char>(ii);
						f.seekg(0, f.end);
						uint32_t length = static_cast<uint32_t>(f.tellg());
						set_buf_from_num(&f_head_buff[cur_offerset + 4], length);
						f.seekg(0, f.beg);
						boost::shared_array<char> file_buf = boost::shared_array<char>(new char[length]);
						f.read(file_buf.get(), length);
						f.close();
						buffersPtr->push_back(std::move(boost::asio::const_buffer(&f_head_buff[cur_offerset], HEAD_SIZE)));
						buffersPtr->push_back(std::move(boost::asio::const_buffer(file_buf.get(), length)));
						vec_file_buf.push_back(std::move(file_buf));
					}
					++ii;
				}
			}
			//第四步：发送结束符
			char end_head_buff[HEAD_SIZE]{ TCPHEAD1 ,TCPHEAD2,TCPEND1,TCPEND2 };
			buffersPtr->push_back(std::move(boost::asio::const_buffer(end_head_buff, HEAD_SIZE)));

			//单地址优化
			if (dst_.size() == 1) {
				return _send_one(buffersPtr, dst_[0]);
			}
			//否则使用多线程并行群发
			std::vector<std::future<bool>> v;
			size_t len = dst_.size();
			for (size_t ii = 0; ii < len; ii++) {
				v.push_back(std::async(std::launch::async, &TcpNetAdapter::_send_one, this, buffersPtr, dst_[ii]));
			}
			bool result = true;
			for (size_t ii = 0; ii < len; ii++) {
				result = result&&(v[ii].get());
			}
			return result;
		}

		virtual bool _send_one(const std::shared_ptr<std::vector<boost::asio::const_buffer>> & Tdata_, const TcpEp & ep) {
			std::shared_ptr<TcpConnection> pc = m_TcpConnectionPool.getTcpConnectionPtr(ep);
			if (!pc) return false;
			return pc->write(Tdata_);
		}

		//接收数据

		TcpConnectionPool m_TcpConnectionPool;
		tpath m_RecvPath;//接收、发送文件路径
		tpath m_SendPath;

	};



}