#ifndef __MY_WEBSOCKET_H__
#define __MY_WEBSOCKET_H__

#pragma once
#include "CLog.h"
#include "httpserv.h"
#include "iiswebsocket.h"
#include "libwshandshake.hpp"

using Mutex = std::mutex; //std::mutex recursive_mutex

class MyWebSocket : public IHttpStoredContext
{
protected:
	DWORD BUFFERLENGTH = 1024;


public:
	CLog *p_log;

	struct REMOTE
	{
		HTTP_CONNECTION_ID connectionId;
		std::string addr;
		std::string host;
		std::string port;
		std::string user;
	} remote;

	Mutex mtx;

	/*
	IIS variable
	*/
	//IHttp- variable
	IHttpServer* m_HttpServer;
	//http context
	IHttpContext* m_HttpContext;
	// socket context
	IWebSocketContext * m_WebSocketContext;

	void * readBuffer;
	DWORD readBufferLength;
	void * writeBuffer;

	std::string data;

	//Constructor
	MyWebSocket() = delete;
	MyWebSocket(CLog *log, IHttpServer *is, IHttpContext *ic, IWebSocketContext *wsc) : p_log(log), m_HttpServer(is), m_HttpContext(ic), m_WebSocketContext(wsc)
	{
		OStringstream strLog;
		strLog << __FUNCTION__ << " "<< this;
		p_log->write(&strLog);
		readBuffer = ic->AllocateRequestMemory(BUFFERLENGTH);
		writeBuffer = ic->AllocateRequestMemory(BUFFERLENGTH);		
	};

	~MyWebSocket() {
		OStringstream strLog;
		strLog << __FUNCTION__ << " " << this << " websocket[" << remote.port << "]  closed, connectionId: " << remote.connectionId;
		p_log->write(&strLog);

		if (m_WebSocketContext) m_WebSocketContext->CancelOutstandingIO();
	}

	//move operator
//	WebSocket& operator=(WebSocket&&);

	//deleter
	void CleanupStoredContext()
	{
		OStringstream strLog;
		strLog << __FUNCTION__ << " " << this << " websocket[" << remote.port << "]  closed, connectionId: " << remote.connectionId;
		if (p_log) p_log->write(&strLog);

		remote.connectionId = 0;

		m_HttpServer = nullptr;
		m_HttpContext = nullptr;
		if (m_WebSocketContext) m_WebSocketContext->CancelOutstandingIO();
		m_WebSocketContext = nullptr;
		delete this;
	};

	void Reading();
	HRESULT Write(std::string data);
};

namespace functorWebSocket {
	//State Machine Async Function
	void WINAPI ReadAsyncCompletion(HRESULT hrError, VOID * pvCompletionContext, DWORD cbIO, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);
	void WINAPI WritAsyncCompletion(HRESULT hrError, PVOID pvCompletionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);

	//Supplementaly
	void WINAPI fWebSocketNULL(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);
}

#endif // __MY_WEBSOCKET_H__