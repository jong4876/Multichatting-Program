#define _CRT_SECURE_NO_WARNINGS         // �ֽ� VC++ ������ �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "resource.h"
#include <ws2tcpip.h>
#include <time.h>
#define REMOTEPORT  9000
#define BUFSIZE     512

//#define SERVERIP   "127.0.0.1"
//#define SERVERPORT 9000

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc1(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DlgProc2(HWND, UINT, WPARAM, LPARAM);
// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...);
// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);
// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags);
// ���� ��� ������ �Լ�
DWORD WINAPI ClientMain(LPVOID arg);
char MULTICASTIP[100] = "235.7.8.1";
short SERVERPORT = 9000;
int flag = 0;

SOCKET sock; // ����
char buf[BUFSIZE + 1]; // ������ �ۼ��� ����
HANDLE hReadEvent, hWriteEvent; // �̺�Ʈ
HWND hSendButton; // ������ ��ư
HWND hEdit1, hEdit2, IPEdit; // ���� ��Ʈ��

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI Receiver(LPVOID arg)
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// SO_REUSEADDR �ɼ� ����
	BOOL optval = TRUE;
	retval = setsockopt(sock, SOL_SOCKET,
		SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// bind()
	SOCKADDR_IN localaddr;
	ZeroMemory(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(REMOTEPORT);
	retval = bind(sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// ��Ƽĳ��Ʈ �׷� ����
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICASTIP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));

	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// ������ ��ſ� ����� ����
	SOCKADDR_IN peeraddr;
	int addrlen;
	char name[10];
	// ��Ƽĳ��Ʈ ������ �ޱ�
	
	while (1) {
		// ������ �ޱ�
		addrlen = sizeof(peeraddr);

		retval = recvfrom(sock, name, 10, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		name[retval] = '\0';
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		if (strlen(buf) == 0)
			continue;

		// ���� ������ ���
		buf[retval] = '\0';
		DisplayText("\n[UDP/%s:%d] %s : %s\n", inet_ntoa(peeraddr.sin_addr),
			ntohs(peeraddr.sin_port),name ,buf);

		printf("\n[UDP/%s:%d] %s\n", inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port), buf);
	}

	// ��Ƽĳ��Ʈ �׷� Ż��
	retval = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// closesocket()
	closesocket(sock);
	DisplayText("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\r\n",
		inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

	// ���� ����
	WSACleanup();
	return 0;
}
BOOL isClassD(char *SERVERIP) {

	char *ptr;
	char tmp[4][100];
	int cnt;
	int flag = 0;
	ptr = strtok(SERVERIP, ".");
	cnt = 0;
	while (ptr != NULL) {
		printf("%d, %s\n", cnt, ptr);
		strcpy(tmp[cnt], ptr);
		ptr = strtok(NULL, ".");
		cnt++;
	}
	int Itmp;
	Itmp = atoi(tmp[0]);
	if (Itmp >= 224 && Itmp <= 239) {
		printf("flag1\n");
		Itmp = atoi(tmp[1]);
		if (Itmp >= 0 && Itmp <= 255) {
			printf("flag2\n");
			Itmp = atoi(tmp[2]);
			if (Itmp >= 0 && Itmp <= 255) {
				printf("flag3\n");
				Itmp = atoi(tmp[3]);
				if (Itmp >= 0 && Itmp <= 255) { // class D
					printf("flag4\n");
					flag = 1;
					return true;
				}
			}
		}
	}
	return false;

}
BOOL isRightPort(char *port) {
	for (int i = 0;i < strlen(port);i++) {
		if ('0' > port[i] || '9' < port[i])
			return false;
	}


	return true;
}


//��ü���� ���α׷� ����
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
	// ��ȭ���� ����
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), NULL, DlgProc1);

	/*
	// �̺�Ʈ ����
    hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (hReadEvent == NULL) return 1;
    hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hWriteEvent == NULL) return 1;
	*/


    // ���� ��� ������ ����
    CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
	// ��ȭ���� ����

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc2);

    // �̺�Ʈ ����
    CloseHandle(hReadEvent);
    CloseHandle(hWriteEvent);

    // closesocket()
    closesocket(sock);

    // ���� ����
    WSACleanup();
    return 0;
}

// IP���� �ޱ� ��ȭ����
BOOL CALLBACK DlgProc1(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char portTmp[100];
	char ipTmp[100];
	switch (uMsg) {
	case WM_INITDIALOG:
		IPEdit = GetDlgItem(hDlg, IDC_EDIT1);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT1, ipTmp, 101);
			GetDlgItemText(hDlg, IDC_EDIT2, portTmp, 101);
			strcpy(MULTICASTIP,ipTmp);
			printf("\nmulIP : ", MULTICASTIP);
			
			if (isRightPort(portTmp)) // ��Ʈ �˻� 
				SERVERPORT = atoi(portTmp);

			else
				MessageBox(hDlg, ("������ ��Ʈ��ȣ�� �ƴմϴ�."), ("���"), MB_ICONWARNING);
			
			if (isClassD(ipTmp)&& isRightPort(portTmp)) // class D IP �˻�
				EndDialog(hDlg, IDOK);

			else
				MessageBox(hDlg,("Class D�ּҰ� �ƴմϴ�."),("���"),MB_ICONWARNING);

			return TRUE;
		case IDCANCEL:

			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc2(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
        hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
        hSendButton = GetDlgItem(hDlg, IDOK);
        SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EnableWindow(hSendButton, FALSE); // ������ ��ư ��Ȱ��ȭ
            WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���
            GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
            SetEvent(hWriteEvent); // ���� �Ϸ� �˸���
            SetFocus(hEdit1);
			flag = 1;
            SendMessage(hEdit1, EM_SETSEL, 0, -1);
            return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);

    char cbuf[BUFSIZE + 256];
    vsprintf(cbuf, fmt, arg);

    int nLength = GetWindowTextLength(hEdit2);
    SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
    SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

    va_end(arg);
}

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    DisplayText("[%s] %s", msg, (char *)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags)
{
    int received;
    char *ptr = buf;
    int left = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;
        ptr += received;
    }

    return (len - left);
}

// TCP Ŭ���̾�Ʈ ���� �κ�
DWORD WINAPI ClientMain(LPVOID arg)
{
    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	printf("%s", buf);

	// ��Ƽĳ��Ʈ TTL ����
	int ttl = 2;
	retval = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *)&ttl, sizeof(ttl));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// ���� �ּ� ����ü �ʱ�ȭ
	SOCKADDR_IN remoteaddr;
	ZeroMemory(&remoteaddr, sizeof(remoteaddr));
	remoteaddr.sin_family = AF_INET;;
	remoteaddr.sin_addr.s_addr = inet_addr(MULTICASTIP);
	remoteaddr.sin_port = htons(REMOTEPORT);


	// ������ ��ſ� ����� ����
	int len;
	HANDLE hThread;

	//���ù� ������ ����
	hThread = CreateThread(NULL, 0, Receiver,
		(LPVOID)sock, 0, NULL);
	
	if (hThread == NULL) { closesocket(sock); }
	else { CloseHandle(hThread); }


 
	// ��Ƽĳ��Ʈ ������ ������
    while (1) {
		WaitForSingleObject(hWriteEvent, INFINITE); // ���� �Ϸ� ��ٸ���

		if (flag == 0)
			continue;

        // ���ڿ� ���̰� 0�̸� ������ ����
        if (strlen(buf) == 0) {
            EnableWindow(hSendButton, TRUE); // ������ ��ư Ȱ��ȭ
            SetEvent(hReadEvent); // �б� �Ϸ� �˸���
            continue;
        }

		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';

		retval = sendto(sock, "������", strlen("������"), 0,
			(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));

		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}	
		
		retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}
       // DisplayText("[UDP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\r\n", retval);

        EnableWindow(hSendButton, TRUE); // ������ ��ư Ȱ��ȭ
        SetEvent(hReadEvent); // �б� �Ϸ� �˸���
		flag = 0;
    }
	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();

    return 0;
}
