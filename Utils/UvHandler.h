#pragma once

#include "Header.h"
#include "IOBuffer.h"

namespace Utils {

// �ܵ����࣬����libuvʵ�֣��ο����ӣ�
// https://zhuanlan.zhihu.com/p/139127919
// http://docs.libuv.org/en/v1.x/
class UvHandler : private Utils::CIOBuffer::Allocator {
public:
	virtual ~UvHandler();

protected:
	UvHandler();
	// д����ǰ������Ҫ�����ڴ���Э��ͷ
	void PreWrite(Utils::CIOBuffer * buffer, WORD wCmd);
	// �ܵ��첽д����
	int Write(uv_pipe_t * pipe, WORD wCmd, const BYTE * data, size_t len);
	// �ܵ����ӳɹ���ص�
	virtual void OnConnected(uv_pipe_t * pipe, bool status);
	// �ܵ��Ͽ����Ӻ�ص�
	virtual void OnDisconnected(uv_pipe_t * pipe);

	// �ⲿ�ֿ�����д��ʵ��Э��ָ�
	void ReadCompleted(uv_pipe_t * pipe, Utils::CIOBuffer * buffer);
	size_t GetMinimumMessageSize() const;
	size_t GetMessageSize(const Utils::CIOBuffer * buffer) const;
	// Э�������������������д
	virtual void ProcessCommand(uv_pipe_t * pipe, const Utils::CIOBuffer * buffer) = 0;

protected:
	// �¼�ѭ��
	int EventPoll(uv_run_mode mode = UV_RUN_DEFAULT);
	// ֹͣ�¼�ѭ��
	void EventStop();
	// �¼�ѭ��ֹͣ���ͷ���Դ
	void EventStopped();
	// ��ʼ���¼�ѭ���������ܵ�����˼���
	void EventInitServer();
	// ��ʼ���¼�ѭ���������ܵ��ͻ�������
	void EventInitClient();
	// �ܵ��ͻ�������
	void StartConnectPipe();

	static uv_pipe_t * CreatePipe(uv_loop_t * loop);
	static void ClosePipe(uv_pipe_t * handle);

private:
	// �������йܵ���ͳһ�رգ�һ����������ʱ�ͷ���Դʹ��
	static void UvWalkCb(uv_handle_t* handle, void* arg);
	// �رչܵ��ص�
	static void UvCloseCb(uv_handle_t* handle);
	// ����˹ܵ������ص������пͻ��˹ܵ���������ʱ�ص�
	static void UvConnectionCb(uv_stream_t* server, int status);
	// �ͻ��˹ܵ����ӷ��ػص���statusΪ0��ɹ�������ʧ��
	static void UvConnectCb(uv_connect_t* req, int status);
	// �ܵ���ȡ����ǰ�����仺�����ص�������������ڴ棬���ڶ�����
	static void UvAllocCb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	// �ܵ���ȡ���ݻص���nread��ʾ���������ݳ��ȣ����<0���ʾ��ȡ������ʱ��رչܵ�
	static void UvReadCb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
	// �ܵ�����������ɻص������status<0���ʾ����ʧ�ܣ���ʱ��رչܵ�
	static void UvWriteCb(uv_write_t* req, int status);

protected:
	// �¼�ѭ��
	uv_loop_t event_loop_;
	// �첽֪ͨ���̰߳�ȫ
	uv_async_t event_notify_;
};

}
