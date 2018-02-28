#include "stdafx.h"

ChattingServer::ChattingServer()
{
	InitServer();

	// accept ������ ����
	thread acceptThread{ &ChattingServer::AcceptThread, this };
	acceptThread.join();
}


ChattingServer::~ChattingServer()
{
}


// ���� �ʱ�ȭ
void ChattingServer::InitServer()
{
	//m_b_server_shut_down = false;
	//mClients.reserve(MAX_USER);

	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	m_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	if (NULL == m_hiocp) { err_quit("Init_Server"); }
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
	printf(" [%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void ChattingServer::err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, // �ɼ� ����
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // �����쿡 �̸� Define�� ��� ������ �����޼����� ������ش�.
		(LPTSTR)&lpMsgBuf // �����޼����� �����ּ�
		, 0
		, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

void ChattingServer::AcceptThread()
{
	int retval;

	//socket()
	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
									NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listen_sock) { err_quit("socket()"); };

	//bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = ::bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	//listen() 
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

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

		printf("\n[����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		if (INVALID_SOCKET == client_sock)
		{
			int error = WSAGetLastError();
			err_display("Accept::WSAAccept error\n", error);
			while (true);
		}

		// ���� �� ���� : 10��
		clientId += 1;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), m_hiocp, clientId, 0);
		//mClients.emplace_back(new User(client_sock, true, clientId));
		mClients[clientId].SetUserInfo(client_sock, true, clientId);
		
		retval = mClients[clientId].WsaRecv();
		if (SOCKET_ERROR == retval)
		{

		}
		//int ret = WSARecv(client_sock, &mClients[clientId].recv_over.wsabuf, 1, NULL,
		//	&flags, &mClients[clientId].recv_over.overlap, NULL);
		//if (0 != ret)
		//{
		//	int error = WSAGetLastError();
		//	if (ERROR_IO_PENDING != error)
		//		err_display("AcceptThread::WSARecv error!", error);
		//	//while (true);
		//}
	}
}