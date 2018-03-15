#include "stdafx.h"

IMPLEMENT_SINGLETON(ChattingServer);

//User ChattingServer::mClients[MAX_USER] = { 0,0,0 };
ChattingServer::ChattingServer()
{
	//std::thread acceptThread{ &ChattingServer::AcceptThread , this };
	
}

ChattingServer::~ChattingServer()
{
}


User* ChattingServer::GetUserInfo(int clientId)
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (mClients[i].GetUserId() == clientId)
			return &mClients[i];
	}

	return nullptr;
}

// 서버 초기화
void ChattingServer::InitServer()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	m_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	if (NULL == m_hiocp) { err_display("InitServer() : ", WSAGetLastError()); }

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int m_cpu_core = static_cast<int>(si.dwNumberOfProcessors) / 2;

	// accept 스레드 생성
	std::thread acceptThread{ &ChattingServer::AcceptThread , this };

	// worker 스레드 생성
	std::vector<std::thread *> workerThreads;
	for (int i = 0; i < m_cpu_core * 2; ++i)
	{
		workerThreads.emplace_back(new std::thread{ &ChattingServer::WorkerThread, this });
	}

	for (auto thread : workerThreads) {
		thread->join();
		delete thread;
	}

	acceptThread.join();

	
}

void ChattingServer::ReleaseServer()
{

}

void ChattingServer::err_display(char *msg, int err_no)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	printf("%s\n", msg, (LPCTSTR)lpMsgBuf);
	printf("err_no : %d\n", err_no);
	LocalFree(lpMsgBuf);
}

void ChattingServer::AcceptThread()
{
	int retval;

	//socket()
	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
									NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listen_sock) { err_display("WSASocket() : ", WSAGetLastError()); };

	//bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = ::bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_display("bind() : ", WSAGetLastError());

	//listen() 
	retval = ::listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_display("listen() : ", WSAGetLastError());

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;

	while (1)
	{
		//accept()
		addrlen = sizeof(clientaddr);
		SOCKET client_sock = WSAAccept(listen_sock, reinterpret_cast<sockaddr *>(&clientaddr), &addrlen, NULL, NULL);
		if (client_sock == INVALID_SOCKET) {
			err_display("WSAAccept() : ", WSAGetLastError());
			break;
		}

		printf("\n[서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		if (INVALID_SOCKET == client_sock)
		{
			int error = WSAGetLastError();
			err_display("Accept::WSAAccept Error\n", error);
			while (true);
		}

		// 유저 수 제한 : 10명
		clientId += 1;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), m_hiocp, clientId, 0);
		//mClients.emplace_back(new User(client_sock, true, clientId));
		mClients[clientId].SetUserInfo(client_sock, true, clientId);

		srand(time(NULL));
		// 접속한 클라이언트에게 랜덤한 채널 인덱스 부여 = 서버 접속과 동시에 채널 입장
		Enter_Channel enter_channel_packet;
		enter_channel_packet.size = sizeof(enter_channel_packet);
		enter_channel_packet.id = clientId;
		enter_channel_packet.channelIndex = rand() % 5 + 1;		// 채널 인덱스 범위 1~5
		mClients[clientId].SetChannelIndex(enter_channel_packet.channelIndex);

		// 새로운 채널 추가
		Channel channel(enter_channel_packet.channelIndex);
		channel.channelManager()->addChannel(&channel);
				
		int result = mClients[clientId].SendPacket
						(reinterpret_cast<unsigned char*>(&enter_channel_packet));
		if (SOCKET_ERROR == result) {
			if (ERROR_IO_PENDING != WSAGetLastError()) {
				err_display("Accept::WSASend Error! : ", WSAGetLastError());
			}
		} 
				
		retval = mClients[clientId].WsaRecv();
		
		if (SOCKET_ERROR == retval)
		{
			if (ERROR_IO_PENDING != WSAGetLastError()) {
				err_display("Accept::WSARecv Error! : ", WSAGetLastError());
			}
		}
	}
}

void ChattingServer::WorkerThread()
{
	while (false == m_b_server_shut_down)
	{
		unsigned long long id = { 0 };
		DWORD io_size = { 0 };
		Overlap *ovlp = { nullptr };

		BOOL result = GetQueuedCompletionStatus(m_hiocp, &io_size, (PULONG_PTR)&id,
			reinterpret_cast<LPOVERLAPPED*>(&ovlp), INFINITE);
		if (FALSE == result) {
			int error = WSAGetLastError();
			if (WSA_IO_PENDING != error)
				err_display("GetQueuedCompletionStatus error!\n", error);
		}

		if (0 == io_size) {
			// 접속 종료한 클라 disconnect
			mClients[id].CloseSocket();
			continue;
		}
		if (OV_RECV == ovlp->event_type) {
			int retval = mClients[id].PacketRessembly(id, io_size);
			if (SOCKET_ERROR == retval) {
				int err_no = WSAGetLastError();
				if (ERROR_IO_PENDING != err_no) { err_display("WorkerThread::WSARecv", err_no); }
			}
			continue;
		}
		else if (OV_SEND == ovlp->event_type) {
			delete ovlp;
			continue;
		}
		else {
			printf("Unknown IOCP event !!\n");
			exit(-1);
		}
	}
}