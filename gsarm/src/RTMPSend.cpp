#include "RTMPSend.h"
#include <stdio.h>
#include <time.h>
//#include "rtmp_sys.h"  //jyc20170323 rtmpdump nouse 
#include "librtmp/rtmp_sys.h"  //jyc20170323 modify copy rtmp_sys.h to /usr/local/include/librtmp
#include "common.h"
#include "RunCode.h"
#include "faac.h"
#include "faaccfg.h"
//#include "libavcodec/flv.h"  //jyc20170323 modify copy flv.h to /usr/local/include/libavcodec
//#include "MemoryContainer.h"  //jyc20170323 remove
//#include <objbase.h>  //jyc20170323 remove just for win

#define defUseRTMFP

#if defined(defUseRTMFP)

//#include "NetStream.h"  //jyc20170323 remove
#pragma comment(lib,"librtmfp.lib")
#pragma comment(lib,"libevent.lib")
#pragma comment(lib,"libeay32.lib")

#endif

//#define defRtmpSendAudio_AAC 1	// rtmp ������Ƶ

#define defUse_TSStartTime // ʱ�����ȡ���ڿ�ʼʱ��

#define MAX_VIDEO_PACKET_SIZE (2*1024*1024)//(512*1024)
#define MINNEW_VIDEO_PACKET_SIZE (32*1024)
#define KEYNEWOVER_VIDEO_PACKET_SIZE (32*1024)


// ���ж����ʱ
char *g_flv_code_morepak( char *enc, char *pend, const char *pBuf_src, const uint32_t bufLen_src )
{
#if 1
	const char *pBuf = pBuf_src;
	uint32_t bufLen = bufLen_src;
	int prefix = 0;
	uint32_t offset = 0;
	while( bufLen>0 
		&& pBuf + 3 < pBuf_src+bufLen_src )
	{
		uint32_t cplen = bufLen;
		offset = g_GetH264PreFix( (unsigned char*)pBuf, bufLen, prefix, 256 );
		if( offset > 0 )
		{
			cplen = offset-prefix;
		}
		else
		{
			cplen = bufLen;
		}

		//enc = AMF_EncodeInt32( enc, pend, cplen );  //jyc20170323 remove
		memcpy( enc, pBuf, cplen );
		enc += cplen;

		pBuf += (cplen + prefix);
		bufLen -= (cplen + prefix);
	}

	return enc; // return new size

#else

	const char *pBuf = pBuf_src;
	uint32_t bufLen = bufLen_src;
	uint32_t prefix = 0;
	uint32_t offset = 0;
	while( bufLen>0 
		&& pBuf + 3 < pBuf_src+bufLen_src )
	{
		uint32_t cplen = bufLen;
		offset = g_h264_find_next_start_code_ex( (const uint8_t*)pBuf, bufLen, prefix );
		if( offset > 0 )
		{
			cplen = offset;
		}
		else
		{
			cplen = bufLen;
		}

		enc = AMF_EncodeInt32( enc, pend, cplen );
		memcpy( enc, pBuf, cplen );
		enc += cplen;

		pBuf += (cplen + prefix);
		bufLen -= (cplen + prefix);
	}

	return enc; // return new size

#endif
}

bool g_rtmp_sendnal( RTMPSend *rtmp_send, RTMPPacket &rtmppkt, RTMPPacket &rtmppkt_sendnal, char *szBodyBuffer, char *enc, char *pend, uint32_t nTimeStamp )
{
	int nal_size = rtmp_send->getNalSize();
	if(nal_size>1){	
		enc = szBodyBuffer;	
		rtmppkt.m_headerType = RTMP_PACKET_SIZE_LARGE;
		rtmppkt.m_nTimeStamp = nTimeStamp;//0;//(uint32_t)packet->timeStamp;
		rtmppkt.m_packetType = RTMP_PACKET_TYPE_VIDEO;

		x264_nal_t *nal_ptr = rtmp_send->getNal();

		int sps_size = nal_ptr[0].i_payload;
		uint8_t *sps = nal_ptr[0].p_payload;		
		int pps_size = nal_ptr[1].i_payload;
		uint8_t *pps = nal_ptr[1].p_payload;

		//*enc++= 7 | FLV_FRAME_KEY; //jyc20170323 remove
		*enc++= 0; // AVC sequence header
		//enc = AMF_EncodeInt24(enc,pend,0); // composition time

		*enc++= 0x01;  // version 
		*enc++= sps[1]; // profile 
		*enc++= sps[2]; // profile compat
		*enc++= sps[3];
		*enc++= (uint8_t)0xFF;
		*enc++= (uint8_t)0xE1;

		//enc = AMF_EncodeInt16(enc,pend, sps_size);
		memcpy(enc, sps, sps_size);
		enc+= sps_size;
		*enc++= 1;  //pps number
		//enc = AMF_EncodeInt16(enc,pend, pps_size);
		memcpy(enc, pps, pps_size);
		enc+= pps_size;
		rtmppkt.m_nBodySize=enc - szBodyBuffer;

		char buf[256] = {0};
		snprintf( buf, sizeof(buf), "name=%s, RTMP_SendPacket NAL size=%d", rtmp_send->getName().c_str(), rtmppkt.m_nBodySize );
		g_PrintfByte( (unsigned char*)rtmppkt.m_body, rtmppkt.m_nBodySize>64?64:rtmppkt.m_nBodySize, buf );

		//RTMPPacket_Copy( &rtmppkt_sendnal, &rtmppkt );
	}
	else
	{
		//LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, getNalSize failed Nal!\r\n", rtmp_send->getName().c_str() );
		return false;
	}

	return true;
}

// audio sequence header������д���Ϣʱ����
bool g_rtmp_sendAudioInfo( RTMPSend *rtmp_send, RTMPPacket &rtmppkt, RTMPPacket &rtmppkt_sendAudioInfo, char *szBodyBuffer, char *enc, char *pend, uint32_t nTimeStamp, uint8_t &AudioHead )
{
	/* //jyc20170323 remove
	if( defAudioSource_Null == rtmp_send->GetAudioCfg().get_Audio_Source() )
	{
		return false;
	}

	AudioHead = rtmp_send->GetAudioCfg().get_FlvAudioHead();

	if( defAudioFmtType_AAC != rtmp_send->GetAudioCfg().get_Audio_FmtType() )
	{
		return false;
	}

	if( defAudioSource_File == rtmp_send->GetAudioCfg().get_Audio_Source()
		&& rtmp_send->GetIPlayBack() )
	{
		return false;
	}

	const unsigned int aacObjectType = defAACObjectType_Default;

	rtmppkt.m_packetType = RTMP_PACKET_TYPE_AUDIO;
	szBodyBuffer[0] = AudioHead;
	szBodyBuffer[1] = 0x00; // 0: AAC sequence header

#if 1
	unsigned char aac_cfg1 = 0;
	unsigned char aac_cfg2 = 0;
	rtmp_send->GetAudioCfg().get_AAC_AudioSpecificConfig( aac_cfg1, aac_cfg2, aacObjectType );
	szBodyBuffer[2] = aac_cfg1;
	szBodyBuffer[3] = aac_cfg2;

	rtmppkt.m_nBodySize = 4;

	LOGMSGEX( defLOGNAME, defLOG_INFO, "name=%s, g_rtmp_sendAudioInfo AAC cfg %02X %02X. BodySize=%d\r\n", rtmp_send->getName().c_str(), aac_cfg1, aac_cfg2, rtmppkt.m_nBodySize );

#else
	//aacObjectType
	//1 0x0a, 0x10
	//2 0x12, 0x10
	if(aacObjectType==1){
		szBodyBuffer[2] = 0x0A;
		szBodyBuffer[3] = 0X10;
	}else if(aacObjectType==2){
		szBodyBuffer[2] = 0x12;
		szBodyBuffer[3] = 0X10;
	}
	//suffix
	szBodyBuffer[4] = 0x06;    
	rtmppkt.m_nBodySize = 5;
#endif

	RTMPPacket_Copy( &rtmppkt_sendAudioInfo, &rtmppkt );
	*/
	return true;
	
}

unsigned __stdcall RTMPDataExService(LPVOID lpPara)
{
	RTMPSend *rtmp_send = (RTMPSend *)lpPara;
	const uint32_t runningkey = rtmp_send->get_runningkey();
	/*jyc20170323 remove
	while( rtmp_send->IsRunning() && runningkey == rtmp_send->get_runningkey() )
	{
		if( defAudioSource_File == rtmp_send->GetAudioCfg().get_Audio_Source()
			&& !rtmp_send->GetIPlayBack()
			)
		{
			AudioCap_File &AFS = rtmp_send->GetAFS();
			int audio_size = 0;
			unsigned char *paudio_buf = AFS.GetFrame(audio_size);
			if( paudio_buf && audio_size>0 )
			{
				//LOGMSG( "aacfile ts=%d, ensize=%d", timeGetTime(), audio_size );
				rtmp_send->PushVideo( false, (char*)paudio_buf, audio_size, timeGetTime(), true );
			}
			else
			{
				timeBeginPeriod(1);
				DWORD start = timeGetTime();
				Sleep(1);
				DWORD end = timeGetTime();
				timeEndPeriod(1);
			}
		}
		else
		{
			break;
		}
	}*/

	return 0;
}
/* jyc20170323 remove
#if defined(defUseRTMFP)
unsigned __stdcall RTMFPThread( LPVOID lpPara )
{   
	GS_RTMFP::NetConnection *nc = (GS_RTMFP::NetConnection *)lpPara;
	nc->Start();

	return 0;
}

void RTMFPNetStateHandler( void *userData, NetStatusInfo *info, uint8_t *peerid )
{
	RTMPSend *rtmp_send = (RTMPSend*)userData;

	std::string strPeerID = g_BufferToString( peerid, 32, false, false );

	std::string strinfo = " ";
	if( strcmp( info->code, "NetConnection.Connect.Success" ) == 0 )
	{
		rtmp_send->setPeerID( strPeerID );
		strinfo = " add ";
	}
	else if( strcmp( info->code, "NetConnection.Connect.Closed" ) == 0 )
	{
		if( strPeerID == rtmp_send->getPeerID() )
		{
			rtmp_send->setPeerID( "" );
			strinfo = " del ";
		}
	}

	LOGMSG( "name=%s,%speerid: %s", rtmp_send->getName().c_str(), strinfo.c_str(), strPeerID.c_str() );
	LOGMSG( "name=%s, streamid: %s", rtmp_send->getName().c_str(), rtmp_send->getStreamID().c_str() );
	LOGMSG( "name=%s, level:%s, code:%s, description:%s\r\n", rtmp_send->getName().c_str(), info->level, info->code, info->description );
}
#endif
*/

/*
unsigned __stdcall RTMPPushThread(LPVOID lpPara)
{
	RTMPSend *rtmp_send = (RTMPSend *)lpPara;
	const uint32_t runningkey = rtmp_send->get_runningkey();

	uint32_t RTMFPSession_ts = timeGetTime()-60*1000;
	const std::string RTMFPSession_strjid = defRTMFPSession_strjid;

	std::string useUrl;

	defGSReturn threadret = defGSReturn_Err;
	RTMP *r = NULL;
	RTMPPacket rtmppkt = {0};

	RTMPPacket rtmppkt_metahead = {0};
	RTMPPacket_Reset( &rtmppkt_metahead );

	RTMPPacket rtmppkt_sendnal = {0};
	RTMPPacket_Reset( &rtmppkt_sendnal );

	RTMPPacket rtmppkt_sendAudioInfo = {0};
	RTMPPacket_Reset( &rtmppkt_sendAudioInfo );

	rtmp_send->popRTMPHandle( &useUrl, false );
	const bool New_isRTMFP = g_IsRTMFP_url( useUrl );

#if defined(defUseRTMFP)
	const bool doConnect_RTMFP = IsRUNCODEEnable(defCodeIndex_SYS_Enable_RTMFP) ? New_isRTMFP : false;

	GS_RTMFP::NetConnection nc;
	GS_RTMFP::NetStream ns( &nc );

	if( doConnect_RTMFP )
	{
		rtmp_send->OnPublishUpdateSession( RTMFPSession_strjid );

		HANDLE   hth1;
		unsigned  uiThread1ID;

		ns.Publish( rtmp_send->getStreamID().c_str() );

		nc.AddStateEventHandler( RTMFPNetStateHandler, rtmp_send );
		nc.Connect( useUrl.c_str() );

		LOGMSG( "name=%s, RTMFP Connect url=\"%s\"\r\n", rtmp_send->getName().c_str(), useUrl.c_str() );

		hth1 = (HANDLE)_beginthreadex( NULL, 0, RTMFPThread, &nc, 0, &uiThread1ID );
		CloseHandle( hth1 );
	}
#endif

	if( defAudioSource_File == rtmp_send->GetAudioCfg().get_Audio_Source()
		&& !rtmp_send->GetIPlayBack()
		)
	{
		AudioCap_File &AFS = rtmp_send->GetAFS();
		if( AFS.LoadFiles( rtmp_send->GetAudioCfg() ) )
		{
			if( AFS.GetPCM() )
			{
				if( defAudioParamDef_Analyse == rtmp_send->GetAudioCfg().get_Audio_ParamDef() )
				{
					CAudioCfg::struAudioParam ap;
					ap.Audio_FmtType = defAudioFmtType_AAC;
					ap.Audio_Channels = AFS.GetPCM()->channels;
					ap.Audio_bitSize = AFS.GetPCM()->samplebytes*8;
					ap.Audio_SampleRate = AFS.GetPCM()->samplerate;
					ap.Audio_ByteRate = 0;
					rtmp_send->GetAudioCfg().set_AudioParam_Analyse( ap );
				}
			}

			LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, AudioCap_File Start success.\r\n", rtmp_send->getName().c_str() );
		}
	}

	const int c_PlayBack_spanfix = RUNCODE_Get(defCodeIndex_SYS_PlayBack_spanfix);
	if( rtmp_send->GetIPlayBack() )
	{
		if( IsRUNCODEEnable(defCodeIndex_SYS_PlayBackSetSpeed) )
		{
			int netspeed = 8192; // �ٶȵ�λ��kbps // 8192,16384
			const int ret = rtmp_send->GetIPlayBack()->PlayBackControl( GSPlayBackCode_SETSPEED, &netspeed, sizeof(int) );
			LOGMSG( "name=%s, set playback netspeed=%d kbps, ret=%d\r\n", rtmp_send->getName().c_str(), netspeed, ret );
		}

		//int hassize = rtmp_send->HasVideoPacket();
		//if( hassize > 270 )
		//{
		//	rtmp_send->GetIPlayBack()->PlayBackControl( GSPlayBackCode_PLAYPAUSE );
		//}
		if( rtmp_send->HasVideoPacket() < 70 || !rtmp_send->IsReady() )
		{
			rtmp_send->GetIPlayBack()->PlayBackControl( GSPlayBackCode_PLAYRESTART );
		}

		LOGMSG( "RTMPPushThread PlayBack_spanfix=%d", c_PlayBack_spanfix );
	}

	DWORD dwStartWait = ::timeGetTime();
	while( !rtmp_send->IsReady() || !(rtmp_send->getNalSize()>0) )
	{
		if( !rtmp_send->IsRunning() || runningkey != rtmp_send->get_runningkey() )
		{
			goto label_RTMPPushThread_End;
		}

		if( ::timeGetTime()-dwStartWait>30000 )
		{
			threadret = defGSReturn_NoData;
			LOGMSG( "name=%s, wait ready and nal timeout %dms!!!\r\n", rtmp_send->getName().c_str(), ::timeGetTime()-dwStartWait );
			goto label_RTMPPushThread_End;
		}

		timeBeginPeriod(1);
		DWORD start = timeGetTime();
		uleep(1000);
		DWORD end = timeGetTime();
		timeEndPeriod(1);
	}

	RTMPPacket_Reset( &rtmppkt );
#if 1
	//r = (RTMP*)RTMPSend::CreateRTMPInstance( vecurl, useUrl, rtmp_send->getName().c_str() );

	//r = (RTMP*)rtmp_send->popRTMPHandle();
	//if( !r )
	//{
	//	threadret = defGSReturn_ConnectSvrErr;
	//	LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, rtmp failed! RTMPHandle=null\r\n", rtmp_send->getName().c_str() );
	//	goto label_RTMPPushThread_End;
	//}
#else
	r = RTMP_Alloc();
	RTMP_Init(r);
	RTMP_SetupURL(r, (char*)url.c_str());
    RTMP_EnableWrite(r);

	if(!RTMP_Connect(r,NULL)){

		LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, RTMP_Connect failed! url=%s\r\n", rtmp_send->getName().c_str(), url.c_str() );
		goto label_RTMPPushThread_End;
	}

	if(!RTMP_ConnectStream(r,0)){

		LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, RTMP_ConnectStream failed!\r\n", rtmp_send->getName().c_str() );
		goto label_RTMPPushThread_End;
	}
#endif
	//rtmp_send->OnPublishStart();

	char *fullbuf_video_cache_buffer = rtmp_send->get_fullbuf_video_cache_buffer();//char fullbuf_video_cache_buffer[MAX_VIDEO_PACKET_SIZE+RTMP_MAX_HEADER_SIZE];
	char *video_cache_buffer = fullbuf_video_cache_buffer+RTMP_MAX_HEADER_SIZE; // ��ݻ���ǰ��Ԥ����RTMPͷ����
	char *pend = video_cache_buffer+MAX_VIDEO_PACKET_SIZE;

	char *szBodyBuffer = video_cache_buffer;
	char *enc = szBodyBuffer;

	rtmppkt.m_nChannel = 0x15;
	rtmppkt.m_headerType = RTMP_PACKET_SIZE_LARGE;//RTMP_PACKET_SIZE_MEDIUM;//RTMP_PACKET_SIZE_LARGE;
	rtmppkt.m_nTimeStamp = 0;
	rtmppkt.m_nInfoField2 = -1; // r->m_stream_id;
	if( r ) rtmppkt.m_nInfoField2 = r->m_stream_id;
	rtmppkt.m_hasAbsTimestamp = 0;
    rtmppkt.m_packetType = RTMP_PACKET_TYPE_INFO;    
	rtmppkt.m_body = szBodyBuffer;

	AVal av;
	//AVal av_val;
	
	STR2AVAL(av, "@setDataFrame");
	enc=AMF_EncodeString(enc,pend, &av);
	
    STR2AVAL(av, "onMetaData");
	enc=AMF_EncodeString(enc,pend, &av);

	*enc ++= AMF_OBJECT;
	//*enc ++= AMF_ECMA_ARRAY;
	//enc = AMF_EncodeInt32(enc, pend, 0);

	//-- AMF_OBJECT START

	STR2AVAL(av, "hasMetadata");
	enc = AMF_EncodeNamedBoolean( enc, pend, &av, 1);

	STR2AVAL(av, "hasVideo");
	enc = AMF_EncodeNamedBoolean( enc, pend, &av, 1);

	STR2AVAL(av, "hasKeyframes");
	enc = AMF_EncodeNamedBoolean( enc, pend, &av, 1);

	//STR2AVAL(av, "author");
	//STR2AVAL(av_val, "");
	//enc=AMF_EncodeNamedString( enc, pend, &av, &av_val);

	//STR2AVAL(av, "copyright");
	//STR2AVAL(av_val, "");
	//enc=AMF_EncodeNamedString( enc, pend, &av, &av_val);

	//STR2AVAL(av, "duration");    
	//enc = AMF_EncodeNamedNumber(enc, pend,  &av, 0.0);

	STR2AVAL(av, "width");
	enc = AMF_EncodeNamedNumber( enc, pend, &av, rtmp_send->getVideoWidth());

	STR2AVAL(av, "height");
	enc = AMF_EncodeNamedNumber( enc, pend, &av, rtmp_send->getVideoHeight());

	STR2AVAL(av, "videocodecid");
	enc=AMF_EncodeNamedNumber( enc, pend, &av, 7);
	//STR2AVAL(av_val, "avc1");
	//enc=AMF_EncodeNamedString( enc, pend, &av, &av_val);

	//STR2AVAL(av, "videodatarate");
	//enc=AMF_EncodeNamedNumber( enc, pend, &av, 0);        

	//STR2AVAL(av, "framerate");
	//enc=AMF_EncodeNamedNumber( enc, pend, &av, rtmp_send->getVideoFps());    

	//STR2AVAL(av, "filesize");
	//enc=AMF_EncodeNamedNumber( enc, pend, &av, 0);	

	if( defAudioSource_Null != rtmp_send->GetAudioCfg().get_Audio_Source()
		&& !rtmp_send->GetIPlayBack()
		)
	{
		int Audio_Channels = 0;
		int Audio_bitSize = 0;
		int Audio_SampleRate = 0;
		int Audio_ByteRate = 0;
		rtmp_send->GetAudioCfg().get_param_use( Audio_Channels, Audio_bitSize, Audio_SampleRate, Audio_ByteRate );

		STR2AVAL(av, "hasAudio");
		enc = AMF_EncodeNamedBoolean( enc, pend, &av, 1);

		STR2AVAL(av, "audiocodecid");
		enc=AMF_EncodeNamedNumber( enc, pend, &av, rtmp_send->GetAudioCfg().get_Audio_FmtType() );

		STR2AVAL(av, "audiosamplerate");
		enc=AMF_EncodeNamedNumber( enc, pend, &av, Audio_SampleRate );

		STR2AVAL(av, "audiochannels");
		enc=AMF_EncodeNamedNumber( enc, pend, &av, Audio_Channels );
	}

	//-- AMF_OBJECT END

  //  STR2AVAL(av, "");
    enc = AMF_EncodeInt16(enc, pend, 0); //enc= AMF_EncodeString(enc, pend,&av);
    *enc++ = AMF_OBJECT_END;

    rtmppkt.m_nBodySize = enc - szBodyBuffer;

	RTMPPacket_Copy( &rtmppkt_metahead, &rtmppkt );

#if 1
	if( !g_rtmp_sendnal( rtmp_send, rtmppkt, rtmppkt_sendnal, szBodyBuffer, enc, pend, 0 ) )
	{
		goto label_RTMPPushThread_End;
	}

#if defined(defUseRTMFP)
	if( doConnect_RTMFP )
	{
		ns.SetMediaHead( GS_RTMFP::VIDEO, (uint8_t*)rtmppkt_sendnal.m_body, rtmppkt_sendnal.m_nBodySize );
	}
#endif

#endif

	uint8_t AudioHead = 0;
	if( g_rtmp_sendAudioInfo( rtmp_send, rtmppkt, rtmppkt_sendAudioInfo, szBodyBuffer, enc, pend, 0, AudioHead ) )
	{
		//AudioHead = FLV_CODECID_PCM_MULAW | FLV_MONO | FLV_SAMPLERATE_SPECIAL | FLV_SAMPLESSIZE_16BIT;

#if defined(defUseRTMFP)
		if( doConnect_RTMFP )
		{
			ns.SetMediaHead( GS_RTMFP::AUDIO, (uint8_t*)rtmppkt_sendAudioInfo.m_body, rtmppkt_sendAudioInfo.m_nBodySize );
		}
#endif
	}

	if( defAudioSource_File == rtmp_send->GetAudioCfg().get_Audio_Source()
		&& !rtmp_send->GetIPlayBack()
		)
	{
		AudioCap_File &AFS = rtmp_send->GetAFS();
		AFS.Start( rtmp_send->GetStartTime(), true );

		HANDLE   hth1;
		unsigned  uiThread1ID;
		hth1 = (HANDLE)_beginthreadex( NULL, 0, RTMPDataExService, rtmp_send, 0, &uiThread1ID );
		CloseHandle(hth1);
	}

	if( !rtmp_send->GetIPlayBack() )
	{
		rtmp_send->ReKeyListVideoPacket();
	}

	rtmppkt.m_headerType = RTMP_PACKET_SIZE_LARGE;
	bool sendnal = false;
	DWORD first_tick = 0;
	DWORD prev_tick = 0;
	DWORD curtick = ::timeGetTime();
	DWORD basetick = ::timeGetTime();
	uint32_t prev_TimeStamp = 0;
	const DWORD this_start_send_tick = ::timeGetTime();
	while( rtmp_send->IsRunning() && runningkey == rtmp_send->get_runningkey() )
	{
		curtick = ::timeGetTime();
		if( (curtick-basetick) > 100 )LOGMSGEX( defLOGNAME, defLOG_WORN, "name=%s, RTMPSend s0 tick=%u\r\n", rtmp_send->getName().c_str(), curtick-basetick );
		basetick = ::timeGetTime();

		if( !r )
		{
			r = (RTMP*)rtmp_send->popRTMPHandle();

			// PreSendRTMP
			if( r )
			{
				rtmppkt.m_nInfoField2 = r->m_stream_id;
				rtmppkt_metahead.m_nInfoField2 = rtmppkt.m_nInfoField2;
				rtmppkt_sendnal.m_nInfoField2 = rtmppkt.m_nInfoField2;
				rtmppkt_sendAudioInfo.m_nInfoField2 = rtmppkt.m_nInfoField2;

				if( !RTMP_SendPacket( r, &rtmppkt_metahead, 0 ) )
				{
					LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, RTMP_SendPacket failed RTMP_PACKET_TYPE_INFO!\r\n", rtmp_send->getName().c_str() );
					goto label_RTMPPushThread_End;
				}

				int outChunkSize = RUNCODE_Get( defCodeIndex_RTMPSend_SetChunkSize );//32*1024;//4096; //32000
				LOGMSGEX( defLOGNAME, defLOG_INFO, "name=%s, RTMP_SetChunkSize=%d\r\n", rtmp_send->getName().c_str(), outChunkSize );
				if( !RTMP_SetChunkSize( r, outChunkSize ) )
				{
					LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, RTMP_SetChunkSize=%d error!\r\n", rtmp_send->getName().c_str(), outChunkSize );
					goto label_RTMPPushThread_End;
				}

				if( !RTMP_SendPacket( r, &rtmppkt_sendnal, 0 ) ){
					LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, RTMP_SendPacket failed Nal!\r\n", rtmp_send->getName().c_str() );
					goto label_RTMPPushThread_End;
				}

				if( rtmppkt_sendAudioInfo.m_body )
				{
					if( !RTMP_SendPacket( r, &rtmppkt_sendAudioInfo, 0 ) ){
						LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, RTMP_SendPacket failed audio info!\r\n", rtmp_send->getName().c_str() );
						//goto label_RTMPPushThread_End;
					}
				}
			}
		}

#if defined(defUseRTMFP)
		if( doConnect_RTMFP )
		{
			if( timeGetTime() - RTMFPSession_ts > 10000 )
			{
				const uint32_t RTMFPSessionCount = ns.HasSession();
				rtmp_send->setRTMFPSessionCount( RTMFPSessionCount );
				//if( RTMFPSessionCount>0 && !rtmp_send->getPeerID().empty() )// ��������ѶϿ����Ͽ�P2P����
				if( RTMFPSessionCount>0 )// ��������ѶϿ�������P2P����
				{
					rtmp_send->OnPublishUpdateSession( RTMFPSession_strjid );
				}

				RTMFPSession_ts = timeGetTime();
			}
		}
#endif

		if( rtmp_send->GetIPlayBack() )
		{
			if( rtmp_send->HasVideoPacket() < 70 )
			{
				rtmp_send->GetIPlayBack()->PlayBackControl( GSPlayBackCode_PLAYRESTART );
			}
		}

		H264VideoPackge *packet = rtmp_send->getVideoPacket();
		if( packet )
		{
#endif
			
			//LOGMSG( "name=%s, RTMPSend getVideoPacket ts=%u, prev_ts=%u, audio=%d, packetSize=%u\r\n", rtmp_send->getName().c_str(), packet->timeStamp, prev_tick, packet->isAudio, packet->size );

			curtick = ::timeGetTime();
			if( (curtick-basetick) > 100 )LOGMSGEX( defLOGNAME, defLOG_WORN, "name=%s, RTMPSend s1 getVideoPacket tick=%u, ts=%u, packetSize=%u\r\n", rtmp_send->getName().c_str(), curtick-basetick, packet->timeStamp, packet->size );
			basetick = ::timeGetTime();
			
			if( r )
			{
				if(!RTMP_IsConnected(r) || RTMP_IsTimedout(r)){
					LOGMSGEX( defLOGNAME, defLOG_ERROR,"name=%s, connect error\r\n", rtmp_send->getName().c_str());
					rtmp_send->ReleaseVideoPacket(packet);
					break;
				}
			}

			if(packet->size > MAX_VIDEO_PACKET_SIZE-9){
				LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, packet too large, packetSize=%u\r\n", rtmp_send->getName().c_str(), packet->size );
				rtmp_send->ReleaseVideoPacket(packet);
				continue;
			}

	        enc = video_cache_buffer;

			if( packet->isAudio )
			{
				rtmppkt.m_packetType = RTMP_PACKET_TYPE_AUDIO;

				*enc++= AudioHead;

				if( defAudioFmtType_AAC == rtmp_send->GetAudioCfg().get_Audio_FmtType() )
				{
					*enc++= 0x01; // 1: AAC raw
				}

				memcpy(enc, packet->buf, packet->size);
				enc+= packet->size;
			}
			else
			{
				rtmppkt.m_packetType = RTMP_PACKET_TYPE_VIDEO;
				//*enc++= (packet->buf[0] & ((1 << 5) - 1))== 7 ?  FLV_FRAME_KEY : FLV_FRAME_INTER;
				*enc++= 7 | (packet->keyframe ?  FLV_FRAME_KEY : FLV_FRAME_INTER);
				*enc++= 1;
				enc = AMF_EncodeInt24(enc,pend,0);

#if 1
				// ������
				enc = g_flv_code_morepak( enc, pend, packet->buf, packet->size );
#else
				enc = AMF_EncodeInt32(enc,pend, packet->size);
				memcpy(enc, packet->buf, packet->size);
				enc+= packet->size;
#endif
			}

			rtmppkt.m_nTimeStamp = (uint32_t)packet->timeStamp;
			
			//delete(packet->buf);
			//delete(packet);

#if 1
			curtick = ::timeGetTime();
			//if( (curtick-basetick) > 10 ) 
			//  LOGMSGEX( defLOGNAME, defLOG_WORN, "name=%s, 
			//  RTMPSend s2 AMF_Encode tick=%u, ts=%u, packetSize=%u\r\n",
			//  rtmp_send->getName().c_str(), curtick-basetick, rtmppkt.m_nTimeStamp, packet->size );
#endif
			if( rtmp_send->GetIPlayBack() )
			{
				int hassize = rtmp_send->HasVideoPacket();
				int tick_span = packet->timeStamp-prev_tick;
#if 1
				if( prev_tick && tick_span>c_PlayBack_spanfix )
				{
					tick_span -= c_PlayBack_spanfix;
					if( tick_span < 1 )
					{
						tick_span = 1;
					}
					else if( tick_span > 120 )
					{
						if( 0!=rtmp_send->GetPlayBackCtrl_speedlevel()
							&& (GSPlayBackCode_PLAYFAST==rtmp_send->GetPlayBackCtrl_Code() || GSPlayBackCode_PLAYSLOW==rtmp_send->GetPlayBackCtrl_Code()) )
						{
							if( tick_span > 3000 )
							{
								tick_span = 3000;
							}
						}
						else
						{
							tick_span = 120;
						}
					}

					timeBeginPeriod(1);
					DWORD start = timeGetTime();
					uleep(tick_span*1000);
					DWORD end = timeGetTime();
					timeEndPeriod(1);
				}
#else
				if( prev_tick && tick_span>9 )
				{
					tick_span -= 9;
					if( tick_span < 1 )
					{
						tick_span = 1;
					}
					else if( tick_span > 100 )
					{
						tick_span = 100;
					}
					Sleep(tick_span);
					//printf( "tick_span=%d\r\n", tick_span );
				}
#endif
				prev_tick = packet->timeStamp;

				if( !rtmp_send->IsRunning() )
				{
					rtmp_send->ReleaseVideoPacket(packet);
					break;
				}

				if( hassize < 70 )
				{
					rtmp_send->GetIPlayBack()->PlayBackControl( GSPlayBackCode_PLAYRESTART );
				}
			}

			rtmppkt.m_body = video_cache_buffer;
			rtmppkt.m_nBodySize = enc - video_cache_buffer;

			bool doSendPacket = false;
			if( packet->isAudio )
			{
				if( defAudioSource_Null != rtmp_send->GetAudioCfg().get_Audio_Source() )
				{
					if( rtmp_send->GetIPlayBack() )
					{
						if( rtmp_send->get_playback_sound() )
						{
							doSendPacket = true;
						}
					}
					else
					{
						doSendPacket = true;
					}
				}
			}
			else
			{
				doSendPacket = true;
			}

			if( doSendPacket )
			{
				if( rtmppkt.m_nTimeStamp <= prev_TimeStamp )
				{
					rtmppkt.m_nTimeStamp = prev_TimeStamp + 2;
				}
				prev_TimeStamp = rtmppkt.m_nTimeStamp;

#if defined(defUseRTMFP)
				if( doConnect_RTMFP )
				{
					if( RTMP_PACKET_TYPE_AUDIO == rtmppkt.m_packetType )
					{
						ns.PushAudio( rtmppkt.m_nTimeStamp, (uint8_t*)rtmppkt.m_body, rtmppkt.m_nBodySize );
					}
					else
					{
						ns.PushVideo( packet->keyframe, rtmppkt.m_nTimeStamp, (uint8_t*)rtmppkt.m_body, rtmppkt.m_nBodySize );
					}
				}
#endif

				if( r )
				{
					if(RTMP_SendPacket(r,&rtmppkt,0) ==0){
						curtick = ::timeGetTime();
						LOGMSGEX( defLOGNAME, defLOG_ERROR, "name=%s, RTMP_SendPacket error, tick=%u, ts=%u, packetSize=%u\r\n", rtmp_send->getName().c_str(), curtick-basetick, rtmppkt.m_nTimeStamp, packet->size );
						rtmp_send->ReleaseVideoPacket(packet);
						break;
					}
				}
			}

			curtick = ::timeGetTime();
			if( (curtick-basetick) > (DWORD)RUNCODE_Get(defCodeIndex_RTMPSend_NetSend_WarnTime) )
			{

				if( 0!=rtmp_send->GetPlayBackCtrl_speedlevel()
					&& (GSPlayBackCode_PLAYFAST==rtmp_send->GetPlayBackCtrl_Code() || GSPlayBackCode_PLAYSLOW==rtmp_send->GetPlayBackCtrl_Code()) )
				{
					// playback speed!=normal
				}
				else
				{
					LOGMSGEX( defLOGNAME, defLOG_WORN, "name=%s, RTMPSend s9 tick=%u, ts=%u, BodySize=%u\r\n", rtmp_send->getName().c_str(), curtick-basetick, rtmppkt.m_nTimeStamp, rtmppkt.m_nBodySize );
				}
			}
			basetick = ::timeGetTime();

			rtmp_send->ReleaseVideoPacket(packet);

			rtmppkt.m_headerType = RTMP_PACKET_SIZE_MEDIUM; 
		}else{
			timeBeginPeriod(1);
			DWORD start = timeGetTime();
			Sleep(1);
			DWORD end = timeGetTime();
			timeEndPeriod(1);
		}
	}

	threadret = defGSReturn_Success;

label_RTMPPushThread_End:

	RTMPPacket_Free( &rtmppkt_metahead );
	RTMPPacket_Free( &rtmppkt_sendnal );
	RTMPPacket_Free( &rtmppkt_sendAudioInfo );

	if( r )
	{
		RTMP_Close(r);
		RTMP_Free(r);
	}

#if defined(defUseRTMFP)
	if( doConnect_RTMFP )
	{
		nc.Close();
	}
#endif

	rtmp_send->OnSendThreadExit( threadret );
	return 0;
}
*/

std::string RTMPSend::s_RTMPSendglobalID = "";

void RTMPSend::Init( const std::string &globalID )
{
	//getMacAddress( RTMPSend::s_RTMPSendglobalID );
	RTMPSend::s_RTMPSendglobalID = globalID;
}

RTMPSend::RTMPSend(IPublishHandler *handler, const std::string& name)
	:m_handler(handler), m_playback(NULL), m_playback_sound(1), m_name(name), m_prev_tick_PrintFrame(0), m_pushindex(0)//,m_queue_lock(true)
{
	m_packet_index = 0;
	m_runningkey = 0;
	m_lastGetVideoPacketTime = timeGetTime();
	startTime = 0;
	m_lasttsVideoPackageList = 0;
	m_startTime_tick = 0;
	isRunning = false;
	m_isThreadExit = true;
	nalSize = 0;
	videoNal = NULL;//new x264_nal_t[2];
	videoWidth = 0;
	videoHeight = 0;
	videoFps = 0;
	reConnectCount = 0;

	unsigned char thisid[32] ={0};
	uint16_t thisid_num = 0;
	/* jyc20170323 remove
	GUID guid;
	if( S_OK == ::CoCreateGuid( &guid ) )
	{
		thisid_num = sizeof( guid );
		memcpy( &thisid, &guid, thisid_num );
	}
	else
	{
		const uint32_t nt = (uint32_t)this;
		thisid_num = sizeof( nt );
		memcpy( thisid, &nt, thisid_num );
	}*/

	m_StreamID =
		IsRUNCODEEnable( defCodeIndex_RTMFP_UseSpecStreamID )
		?
		m_StreamID = RUNCODE_GetStr( defCodeIndex_RTMFP_UseSpecStreamID )
		:
		m_StreamID = g_BufferToString( thisid, thisid_num, false, false );

	m_RTMFPSessionCount = 0;
	
	m_fullbuf_video_cache_buffer = NULL;

	ResetPlayBackCtrlFlag();
	m_lastThrowIndex = 0;

	m_RTMPHandle = NULL;
}

RTMPSend::~RTMPSend(void)
{
	this->Close();

	if( m_fullbuf_video_cache_buffer )
	{
		//jyc20170323 remove
		//g_GetMemoryContainer()->ReleaseMemory( m_fullbuf_video_cache_buffer, MAX_VIDEO_PACKET_SIZE+RTMP_MAX_HEADER_SIZE );
		m_fullbuf_video_cache_buffer = NULL;
	}

	delNal();

	m_queue_mutex.lock();

	if( m_RTMPHandle )
	{
		deleteRTMPHandle( m_RTMPHandle );
	}

	m_RTMPHandle = NULL;
	m_RTMPHandle_useUrl = "";

	m_queue_mutex.unlock();

}

char* RTMPSend::get_fullbuf_video_cache_buffer()
{
	if( !m_fullbuf_video_cache_buffer )
	{
		//jyc20170323 remove
		//m_fullbuf_video_cache_buffer = g_GetMemoryContainer()->GetMemory( MAX_VIDEO_PACKET_SIZE+RTMP_MAX_HEADER_SIZE );
	}

	return m_fullbuf_video_cache_buffer;
}


void RTMPSend::deleteRTMPHandle( defRTMPConnectHandle handle )
{
	if( !handle )
		return;

	RTMP *r = (RTMP*)handle;
	if( r )
	{
		//RTMP_Close(r); //jyc20170323 remove
		//RTMP_Free(r);
	}
}

defRTMPConnectHandle RTMPSend::CreateRTMPInstance( const std::vector<std::string> &vecurl, std::string &useUrl, const char *pname )
{
	if( vecurl.empty() )
	{
		//LOGMSGEX( defLOGNAME, defLOG_ERROR, "CreateRTMPInstance(%s) failed! url is null!\r\n", pname?pname:"" );
		return NULL;
	}

	// ��������
	const uint32_t SYS_RTMPUrlNumMax = RUNCODE_Get(defCodeIndex_SYS_RTMPUrlNumMax);

	LOGMSG( "CreateRTMPInstance SYS_RTMPUrlNumMax=%u, curNum=%u", SYS_RTMPUrlNumMax, vecurl.size() );

	std::vector<std::string> vecurltemp = vecurl;

	if( IsRUNCODEEnable(defCodeIndex_TEST_UseSpecRTMPUrlList) )
	{
		uint32_t urlsno = RUNCODE_Get(defCodeIndex_TEST_UseSpecRTMPUrlList,defRunCodeValIndex_2);
		
		if( urlsno > 0 && urlsno < vecurl.size() ) // 0ʱ�������
		{
			vecurltemp.insert( vecurltemp.begin(), vecurl[urlsno] );

			LOGMSG( "CreateRTMPInstance UseSpecRTMPUrlList vecurl[%d] first. temp size=%d, 0url=%s", urlsno, vecurltemp.size(), vecurltemp[0].c_str() );
		}
	}

	RTMP *r = NULL;
	//jyc20170323 remove
	//r = RTMP_Alloc();
	//RTMP_Init(r);
	bool issuccess = false;
	/* jyc20170323 remove
	for( int i=0; i<vecurltemp.size() && i<SYS_RTMPUrlNumMax; ++i )
	{
		RTMP_SetupURL(r, (char*)vecurltemp[i].c_str() );
		RTMP_EnableWrite(r);

		if(!RTMP_Connect(r,NULL))
		{
			//LOGMSGEX( defLOGNAME, defLOG_ERROR, "CreateRTMPInstance(%s), RTMP_Connect failed! url=%s", pname?pname:"", vecurltemp[i].c_str() );
			continue;
		}

		if(!RTMP_ConnectStream(r,0))
		{
			//LOGMSGEX( defLOGNAME, defLOG_ERROR, "CreateRTMPInstance(%s), RTMP_ConnectStream failed! url=%s", pname?pname:"", vecurltemp[i].c_str() );
			continue;
		}

		issuccess = true;
		useUrl = vecurltemp[i].c_str();
		break;
	}*/

	if( !issuccess )
	{
		//LOGMSGEX( defLOGNAME, defLOG_ERROR, "CreateRTMPInstance(%s) all url failed!\r\n", pname?pname:"" );
		goto label_CreateRTMPConnectInstance_End;
	}

	LOGMSG( "CreateRTMPInstance(%s) success. url=%s\r\n", pname?pname:"", useUrl.c_str() );
	return r;

label_CreateRTMPConnectInstance_End:

	if( r )
	{
		//RTMP_Close(r); //jyc20170406 remove
		//RTMP_Free(r);
	}

	return NULL;
}

void RTMPSend::pushRTMPHandle( defRTMPConnectHandle handle, const std::string &useUrl )
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	if( m_RTMPHandle )
	{
		deleteRTMPHandle( m_RTMPHandle );
	}

	m_RTMPHandle = handle;
	m_RTMPHandle_useUrl = useUrl;
}

defRTMPConnectHandle RTMPSend::popRTMPHandle( std::string *useUrl, const bool dopop )
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	defRTMPConnectHandle handle = this->m_RTMPHandle;
	if( useUrl ) *useUrl = this->m_RTMPHandle_useUrl;

	if( dopop )
	{
		this->m_RTMPHandle = NULL;
		this->m_RTMPHandle_useUrl = "";
	}

	return handle;
}

void RTMPSend::Close()
{
	isRunning = false;

	if( !m_isThreadExit )
	{
		DWORD dwStart = ::timeGetTime();
		while( !m_isThreadExit && ::timeGetTime()-dwStart < 30*1000 )
		{
			usleep(1000);
		}
		LOGMSG( "RTMPSend ThreadExit wait usetime=%dms\r\n", ::timeGetTime()-dwStart );
	}

	m_queue_mutex.lock();
	
	m_RTMFPSessionCount = 0;

	if( !videoPackage.empty() )
	{
		while(videoPackage.size()>0){
			H264VideoPackge *packet =  videoPackage.front();
			FinalDeleteVideoPacket( packet );
			videoPackage.pop_front();
		}
		videoPackage.clear();
	}

	if( !videoPackreuse.empty() )
	{
		while(videoPackreuse.size()>0){
			H264VideoPackge *packet =  videoPackreuse.front();
			FinalDeleteVideoPacket( packet );
			videoPackreuse.pop_front();
		}
		videoPackreuse.clear();
	}
	m_queue_mutex.unlock();

	m_pushindex = 0;
}

int RTMPSend::Connect(const std::string& url)
{
	this->setUrl( url );
	return 1;
}
int RTMPSend::SetVideoMetaData(int width,int height,int fps)
{
	this->videoWidth = width;
	this->videoHeight = height;
	this->videoFps = fps;
	return 1;
}
int RTMPSend::Run()
{
	/* jyc20170323 remove
	HANDLE   hth1;
    unsigned  uiThread1ID;
	if(!this->isRunning){
	   this->Close();//videoPackage.empty();
	   //startTime = timeGetTime();
	   this->isRunning = true;
	   this->m_runningkey++;
	   this->m_lastGetVideoPacketTime = timeGetTime();
	   m_isThreadExit = false;
	   hth1 = (HANDLE)_beginthreadex(NULL, 0, RTMPPushThread, this, 0, &uiThread1ID);
	   CloseHandle(hth1);
	   this->m_handler->OnPublishStart();
	}
	return this->isRunning;
	*/
}

void RTMPSend::SetVideoNal(x264_nal_t *nal,int i_nal)
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	Delete_x264_nal_t( &videoNal, nalSize );

	Copy_x264_nal_t( &this->videoNal, this->nalSize, nal, i_nal );
}

x264_nal_t* RTMPSend::getNal()
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	char chbuf[256] = {0};
	for( int i=0; i<nalSize; ++i )
	{
		snprintf( chbuf, sizeof(chbuf), "RTMPSend(%s)::getNal(%d/%d)", this->m_name.c_str(), i+1, nalSize );

		g_PrintfByte( videoNal[i].p_payload, videoNal[i].i_payload>64?64:videoNal[i].i_payload, chbuf );
	}

	return this->videoNal;
}

int RTMPSend::getNalSize()
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	return this->nalSize;
}

void RTMPSend::delNal()
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	Delete_x264_nal_t( &videoNal, nalSize );
}

void RTMPSend::PushVideo( const bool keyframe, char *buffer, int size, uint32_t timestamp, const bool isAudio )
{
	if( isAudio )
	{
		if( !this->get_playback_sound() )
		{
			return;
		}

		if( this->GetIPlayBack() )
		{
			if( !IsRUNCODEEnable(defCodeIndex_SYS_PlayBackSound) ) // Not PlayBack Sound
			{
				return;
			}
		}
	}

	m_pushindex++;
	//LOGMSG( "RTMPSend src(%s)::PushVideo(index=%d) ts=%u, key=%d, audio=%d, size=%u\r\n", m_name.c_str(), m_pushindex, (uint32_t)timestamp, keyframe, isAudio, size );

	if(wait_frist_key_frame)
	{
		if(keyframe)
		{
			m_pushindex = 0;
			LOGMSG( "RTMPSend(%s)::PushVideo(index=%u) set key=true, set wait=false, ts=%u, size=%d", this->m_name.c_str(), m_pushindex, timestamp, size );

			startTime = timestamp;
			m_lasttsVideoPackageList = 0;
			m_startTime_tick = ::timeGetTime();
			wait_frist_key_frame = false;
			m_prev_tick_PrintFrame = 0;
		}
		else
		{
			return;
		}
	}
#if 1
	if( !isAudio )
	{
		// h264 no head err
		int prefixNum = 0;
		int preindex = g_GetH264PreFix( (unsigned char*)buffer, size>16?16:size, prefixNum );

		if( 0==prefixNum || 0 == preindex  )
		{
			char chbuf[256] = {0};
			snprintf( chbuf, sizeof(chbuf), "RTMPSend(%s)::PushVideo(index=%u) prefixNum=0 err! size=%d", this->m_name.c_str(), m_pushindex, size );

			g_PrintfByte( (unsigned char*)buffer, size>32?32:size, chbuf );
		}

		buffer += prefixNum;
		size -= prefixNum;
	}
#endif

	if( size > MAX_VIDEO_PACKET_SIZE )
	{
		//LOGMSGEX( defLOGNAME, defLOG_ERROR, "RTMPSend(%s)::PushVideo packet too large, packetSize=%u\r\n", m_name.c_str(), size );
		return;
	}

	uint32_t quesize = 0;
	H264VideoPackge *packet = newVideoPacket( size, keyframe );

	packet->isAudio = isAudio;
	packet->keyframe = keyframe;
	packet->size = size;
	memcpy(packet->buf,buffer,size);
	packet->timeStamp = timestamp;
	//packet->size = g_h264_remove_all_start_code( (uint8_t*)packet->buf, packet->size );

	m_queue_mutex.lock();

	packet->index = ++m_packet_index;
	this->videoPackage.push_back(packet);
	quesize = this->videoPackage.size();

	// ������ڴ�ֵʱ��ʼ�����ͷţ��ͷŵ�һ����
	if( videoPackage.size()>GetQueMaxSize()  )
	{
		//LOGMSGEX( defLOGNAME, defLOG_WORN, "RTMPSend(%s)::PushVideo videoPackage full, release packet, beforeCount=%u\r\n", m_name.c_str(), videoPackage.size() );

		while( videoPackage.size()>100 )
		{
			H264VideoPackge *packet =  videoPackage.front();
			ReleaseVideoPacket_nolock( packet );
			videoPackage.pop_front();
		}

		//LOGMSGEX( defLOGNAME, defLOG_WORN, "RTMPSend(%s)::PushVideo videoPackage full, release packet, AfterCount=%u\r\n", m_name.c_str(), videoPackage.size() );
	}

	m_queue_mutex.unlock();

	bool PrintQueInfo = false;

	if( PrintQueInfo )
	{
		if( quesize<5 ) // �Ѿ��ڴ�ӡʱ�����ٵ�С�ڴ�ֵ�Ų���ӡ
		PrintQueInfo = false;
	}

	if( quesize>9 && !this->GetIPlayBack() && 0==(quesize%10) ) // ���ڴ�ֵ�ſ�ʼ��ӡ
		PrintQueInfo = true;

	if( quesize>200 && 0==(quesize%200) ) // ���ڴ�ֵ�ſ�ʼ��ӡ
		PrintQueInfo = true;

	if( IsRUNCODEEnable(defCodeIndex_RTMPSend_PrintFrame) ) // if( !isAudio && IsRUNCODEEnable(defCodeIndex_RTMPSend_PrintFrame) )
	{
		if( 0 == RUNCODE_Get(defCodeIndex_RTMPSend_PrintFrame,defRunCodeValIndex_2) ) // ���Ǵ�ӡ
		{
			PrintQueInfo = true;
		}
		else if( keyframe && RUNCODE_Get(defCodeIndex_RTMPSend_PrintFrame,defRunCodeValIndex_3) )
		{
			PrintQueInfo = true;
		}
		else if( ::timeGetTime()-m_prev_tick_PrintFrame >= (DWORD)RUNCODE_Get(defCodeIndex_RTMPSend_PrintFrame,defRunCodeValIndex_2) )
		{
			PrintQueInfo = true;
		}
	}

	if( PrintQueInfo )
	{
		LOGMSG( "RTMPSend(%s)::PushVideo(index=%u) ts=%u, key=%d, audio=%d, quesize=%u, size=%u\r\n", m_name.c_str(), m_pushindex, (uint32_t)timestamp, keyframe, isAudio, quesize, size );
		m_prev_tick_PrintFrame = ::timeGetTime();
	}
}

void RTMPSend::PushVideo( const bool keyframe, char *buffer, int size, const bool isAudio )
{

	uint32_t timestamp = timeGetTime();

	PushVideo(keyframe, buffer, size, timestamp, isAudio );
}

void RTMPSend::OnSendThreadExit(defGSReturn code)
{
	LOGMSG( "name=%s, OnSendThreadExit(%d:%s)%s\r\n", this->getName().c_str(), code, g_Trans_GSReturn(code).c_str(), this->GetIPlayBack()?" Playback":"" );

	m_isThreadExit = true;
	if(isRunning){
	   isRunning = false;
	   this->m_handler->OnPublishStop(code);
	}
}

H264VideoPackge* RTMPSend::getVideoPacket()
{
	H264VideoPackge* packet = NULL;

	m_queue_mutex.lock();

	this->m_lastGetVideoPacketTime = timeGetTime();

	if( this->GetIPlayBack() )
	{
		if( GSPlayBackCode_PLAYPAUSE == m_PlayBackCtrl_Code )
		{
			m_queue_mutex.unlock();
			return packet;
		}
	}

	if( !this->videoPackage.empty() )
	{
		packet = this->videoPackage.front();
		this->videoPackage.pop_front();
	}

	// ts
	if( packet )
	{
		if( this->GetIPlayBack() )
		{
			switch( m_PlayBackCtrl_Code )
			{
			case GSPlayBackCode_PLAYFAST:
			case GSPlayBackCode_PLAYSLOW:
				{
					if( m_PlayBackCtrl_speedlevel >=2 )
					{
						packet->timeStamp -= ( packet->timeStamp - m_PlayBackCtrl_ts )*4/5;
					}
					else if( m_PlayBackCtrl_speedlevel >=1 )
					{
						packet->timeStamp -= ( packet->timeStamp - m_PlayBackCtrl_ts )/2;
					}
					else if( m_PlayBackCtrl_speedlevel <= -2 )
					{
						packet->timeStamp += ( packet->timeStamp - m_PlayBackCtrl_ts );
					}
					else if( m_PlayBackCtrl_speedlevel <= -1 )
					{
						packet->timeStamp += ( packet->timeStamp - m_PlayBackCtrl_ts )/2;
					}
					else
					{
						ResetPlayBackCtrlFlag();
					}
				}
				break;

			case GSPlayBackCode_PLAYNORMAL:
				{
					ResetPlayBackCtrlFlag();
				}
				break;

			case GSPlayBackCode_SkipTime: // ��Ƶ��ת
				{
					const uint32_t SkipTime_time = m_doSkipTime_timems>=10000 ? (m_doSkipTime_timems-3000) : m_doSkipTime_timems;
					const uint32_t curFrameTSSpan = packet->timeStamp - m_PlayBackCtrl_ts;
					if( curFrameTSSpan < SkipTime_time )
					{
						ReleaseVideoPacket_nolock( packet );
						packet = NULL;
						m_queue_mutex.unlock();
						return NULL;
					}
					else
					{
						ResetPlayBackCtrlFlag();
						
						if( !packet->keyframe )
						{
							RePreFirstKeyListVideoPacket_nolock();
						}
					}
				}
				break;
			}
		}


#if defined(defUse_TSStartTime)
		packet->timeStamp -= startTime;
#else
#endif

		if( !this->GetIPlayBack() )
		{
			//packet->timeStamp = timeGetTime()-startTime; // use lock time
		}
		else
		{
			if( packet->timeStamp - m_lasttsVideoPackageList > 2000
				//&& !( m_PlayBackCtrl_ThrowFrame && GSPlayBackCode_PLAYFAST==m_PlayBackCtrl_Code && 1==m_PlayBackCtrl_speedlevel ) // �����֡ģʽ��ʱ�����������
				)
			{
				LOGMSG( "RTMPSend(%s)::getVideoPacket reset startTime, oldstartTime=%u, curts=%u!!!", this->m_name.c_str(), startTime, packet->timeStamp );
				startTime += (packet->timeStamp - m_lasttsVideoPackageList);
				packet->timeStamp = m_lasttsVideoPackageList;
			}
		}

		if( packet->timeStamp < m_lasttsVideoPackageList )
		{
			packet->timeStamp = m_lasttsVideoPackageList;
		}

		if( this->GetIPlayBack() )
		{
			switch( m_PlayBackCtrl_Code )
			{
			case GSPlayBackCode_PLAYNORMAL:
			case GSPlayBackCode_PLAYFAST:
				{
					bool doThrowFrame = packet->isAudio;

					if( !doThrowFrame && !packet->keyframe && m_PlayBackCtrl_speedlevel >=1 ) // is fastplay
					{
						if( m_PlayBackCtrl_ThrowFrame )
						{
							doThrowFrame = true;
						}
					}

					if( doThrowFrame )
					{
						ReleaseVideoPacket_nolock( packet );
						packet = NULL;
						m_queue_mutex.unlock();
						return NULL;
					}
				}
				break;
			}
		}

		m_lasttsVideoPackageList = packet->timeStamp;
	}
	// ts

	m_queue_mutex.unlock();

	return packet;
}

void RTMPSend::ResetPlayBackCtrlFlag( bool resetall )
{
	if( resetall )
	{
		m_PlayBackCtrl_Code = GSPlayBackCode_NULL;
	}

	m_PlayBackCtrl_ts = 0;
	m_doSkipTime_timems = 0;
	m_PlayBackCtrl_speedlevel = 0;
	m_PlayBackCtrl_ThrowFrame = 0;
	m_PlayBackCtrl_pause_oldcode = GSPlayBackCode_NULL;
}

int RTMPSend::PlayBackControl( GSPlayBackCode_ ControlCode, uint32_t InValue )
{
	if( !this->GetIPlayBack() )
		return false;

	switch( ControlCode )
	{
	case GSPlayBackCode_SkipTime:
		{
			if( 0==InValue )
				return false;
		}
		break;

	default:
		break;
	}

	m_queue_mutex.lock();

	const GSPlayBackCode_ oldcode = m_PlayBackCtrl_Code;
	m_PlayBackCtrl_Code = ControlCode;
	m_PlayBackCtrl_ts = startTime + m_lasttsVideoPackageList;

	switch( m_PlayBackCtrl_Code )
	{
	case GSPlayBackCode_SkipTime:
		{
			m_doSkipTime_timems = InValue;
			m_PlayBackCtrl_pause_oldcode = oldcode;
		}
		break;

	case GSPlayBackCode_PLAYFAST:
	case GSPlayBackCode_PLAYSLOW:
		{
			m_PlayBackCtrl_speedlevel += (GSPlayBackCode_PLAYFAST==m_PlayBackCtrl_Code) ? 1 : -1;
			
			if( 0==m_PlayBackCtrl_speedlevel )
			{
				ReKeyListVideoPacket_nolock();
				ResetPlayBackCtrlFlag();
			}
			else
			{
				if( m_PlayBackCtrl_speedlevel > 2 )
				{
					m_PlayBackCtrl_speedlevel = 2;
				}
				else if( m_PlayBackCtrl_speedlevel < -2 )
				{
					m_PlayBackCtrl_speedlevel = -2;
				}

				if( m_PlayBackCtrl_speedlevel > 0 )
				{
					m_PlayBackCtrl_Code = GSPlayBackCode_PLAYFAST;
				}
				else if( m_PlayBackCtrl_speedlevel < 0 )
				{
					m_PlayBackCtrl_Code = GSPlayBackCode_PLAYSLOW;
				}

				if( m_PlayBackCtrl_ThrowFrame && !InValue )
				{
					RePreFirstKeyListVideoPacket_nolock();
				}

				if( GSPlayBackCode_PLAYFAST == m_PlayBackCtrl_Code )
				{
					m_PlayBackCtrl_ThrowFrame = InValue;
				}
			}

			LOGMSG( "RTMPSend(%s):PlayBackControl speedlevel=%d, ThrowFrame=%d\r\n", this->getName().c_str(), m_PlayBackCtrl_speedlevel, m_PlayBackCtrl_ThrowFrame );
		}
		break;

	case GSPlayBackCode_PLAYNORMAL:
		{
			if( m_PlayBackCtrl_ThrowFrame )
			{
				RePreFirstKeyListVideoPacket_nolock();
			}

			ResetPlayBackCtrlFlag();
		}
		break;

	case GSPlayBackCode_PLAYPAUSE:
		{
			if( GSPlayBackCode_PLAYPAUSE != oldcode )
			{
				m_PlayBackCtrl_pause_oldcode = oldcode;
			}
		}
		break;

	case GSPlayBackCode_PLAYRESTART:
		{
			if( GSPlayBackCode_PLAYPAUSE == oldcode )
			{
				m_PlayBackCtrl_Code = m_PlayBackCtrl_pause_oldcode;
				m_PlayBackCtrl_pause_oldcode = GSPlayBackCode_NULL;
			}
		}
		break;

	default:
		break;
	}

	m_queue_mutex.unlock();

	return true;
}

int RTMPSend::HasVideoPacket()
{
	uint32_t quesize = 0;

	m_queue_mutex.lock();
	quesize = this->videoPackage.size();
	m_queue_mutex.unlock();

	return quesize;
}

H264VideoPackge* RTMPSend::newVideoPacket( int needsize, bool keyframe )
{
	H264VideoPackge *packet = NULL;

	m_queue_mutex.lock();
	if( !videoPackreuse.empty() )
	{
		if( videoPackreuse.size()>GetReuseQueMaxSize() )
		{
			//LOGMSGEX( defLOGNAME, defLOG_WORN, "newVideoPacket(%s) videoPackreuse full, release packet, beforeCount=%u\r\n", m_name.c_str(), videoPackreuse.size() );
			const int minQueSize = GetReuseQueMaxSize()/2;
			while( videoPackreuse.size()>minQueSize )
			{
				H264VideoPackge *packet =  videoPackreuse.front();
				FinalDeleteVideoPacket( packet );
			}

			//LOGMSGEX( defLOGNAME, defLOG_WORN, "newVideoPacket(%s) videoPackreuse full, release packet, AfterCount=%u\r\n", m_name.c_str(), videoPackreuse.size() );
		}

		std::deque<H264VideoPackge*>::iterator it = videoPackreuse.begin();
		for( ; it!=videoPackreuse.end(); ++it )
		{
			if( (*it)->bufLimit > needsize )
			{
				packet = (*it);
				videoPackreuse.erase(it);
				break;
			}
		}
	}
	m_queue_mutex.unlock();

	if( !packet )
	{
#if 1 // defuseMemoryContainer  //jyc20170323 remove
		//int c_bufLimit = needsize>CRM_MEMORY_MIN_LEN ? needsize:CRM_MEMORY_MIN_LEN;
		
		//char *c_pbuf = g_GetMemoryContainer()->GetMemory( c_bufLimit + sizeof(H264VideoPackge) ); // ���ṹ�ͻ��������ͬһ�����ڴ�����

		//packet = (H264VideoPackge*)c_pbuf;
		//packet->bufLimit = c_bufLimit;
		//packet->buf = c_pbuf + sizeof(H264VideoPackge);
#else
		packet = new H264VideoPackge();
		packet->bufLimit = needsize>MINNEW_VIDEO_PACKET_SIZE ? needsize:MINNEW_VIDEO_PACKET_SIZE;
		if( keyframe ) packet->bufLimit += KEYNEWOVER_VIDEO_PACKET_SIZE;
		packet->buf = new char[packet->bufLimit];
#endif
	}

	return packet;
}

void RTMPSend::ClearListVideoPacket()
{
	m_queue_mutex.lock();

	LOGMSG( "RTMPSend::ClearListVideoPacket(%s) size=%d\r\n", m_name.c_str(), videoPackage.size() );

	while( !videoPackage.empty() )
	{
		H264VideoPackge *packet =  videoPackage.front();
		ReleaseVideoPacket_nolock( packet );
		videoPackage.pop_front();
	}

	m_queue_mutex.unlock();
}

void RTMPSend::RePreFirstKeyListVideoPacket()
{
	m_queue_mutex.lock();

	RePreFirstKeyListVideoPacket_nolock();

	m_queue_mutex.unlock();
}

void RTMPSend::RePreFirstKeyListVideoPacket_nolock()
{
	if( videoPackage.empty() )
	{
		LOGMSG( "RTMPSend::RePreFirstKeyList(%s) list is empty\r\n", m_name.c_str() );
	}
	else
	{
		const uint32_t sizebefore = videoPackage.size();

		// ��һ��keyframe֮ǰ��ȫ�ӵ���keyframe���?֮�������֡ȫ����
		while( !videoPackage.empty() )
		{
			H264VideoPackge *packet =  videoPackage.front();
			if( packet->keyframe )
			{
				break;
			}

			ReleaseVideoPacket_nolock( packet );
			videoPackage.pop_front();
		}

		const uint32_t sizeafter = videoPackage.size();

		LOGMSG( "RTMPSend::RePreFirstKeyList(%s) after do, sizebefore=%d, sizeafter=%d.\r\n", m_name.c_str(), sizebefore, sizeafter );
	}
}

void RTMPSend::ReKeyListVideoPacket()
{
	m_queue_mutex.lock();

	ReKeyListVideoPacket_nolock();

	m_queue_mutex.unlock();
}

void RTMPSend::ReKeyListVideoPacket_nolock()
{
	if( videoPackage.empty() )
	{
		LOGMSG( "RTMPSend::ReKeyList(%s) list is empty\r\n", m_name.c_str() );
	}
	else
	{
		std::deque<H264VideoPackge*>::iterator it = videoPackage.begin();
		std::deque<H264VideoPackge*>::iterator itEnd = videoPackage.end();

		const bool first_is_key = (*it)->keyframe;
		const uint32_t sizebefore = videoPackage.size();

		uint32_t keycount = 0;
		for( ; it != itEnd; ++it )
		{
			//LOGMSG( "RTMPSend::ReKeyList i=%d \r\n", (*it)->keyframe, (*it)->keyframe?"!!!!!!!!!!!!!!!!!!!!!":"" );
			if( (*it)->keyframe )
			{
				keycount++;
			}
		}

		if( !first_is_key || keycount > 1 )
		{
			LOGMSG( "RTMPSend::ReKeyList(%s) keycount=%d, first_is_key=%d, do relist!\r\n", m_name.c_str(), keycount, first_is_key );

			// ֻ����һ��keyframe��ͷ
			while( !videoPackage.empty() )
			{
				H264VideoPackge *packet =  videoPackage.front();
				if( packet->keyframe )
				{
					if( keycount<=1 )
					{
						break;
					}

					keycount--;
				}

				ReleaseVideoPacket_nolock( packet );
				videoPackage.pop_front();
			}

			const uint32_t sizeafter = videoPackage.size();

			LOGMSG( "RTMPSend::ReKeyList(%s) after do, sizebefore=%d, sizeafter=%d.\r\n", m_name.c_str(), sizebefore, sizeafter );
		}
		else
		{
			LOGMSG( "RTMPSend::ReKeyList(%s) keycount=%d, sizebefore=%d, not relist.\r\n", m_name.c_str(), keycount, sizebefore );
		}
	}
}

// ֻ�������һ��key�����key֮ǰ��֮��������ȫ�����
uint32_t RTMPSend::ReListOfOneKey()
{
	uint32_t keyts = 0;

	m_queue_mutex.lock();

	if( videoPackage.empty() )
	{
		LOGMSG( "RTMPSend::ReListOfOneKey(%s) list is empty\r\n", m_name.c_str() );
	}
	else
	{
		const uint32_t sizebefore = videoPackage.size();
		std::deque<H264VideoPackge*>::iterator it = videoPackage.begin();
		std::deque<H264VideoPackge*>::iterator itEnd = videoPackage.end();

		// �޳��key��
		for( ; it != itEnd; ++it )
		{
			H264VideoPackge *packet = (*it);

			if( packet->keyframe )
				continue;

			ReleaseVideoPacket_nolock( packet );
			videoPackage.erase(it);
			it = videoPackage.begin();
			itEnd = videoPackage.end();
		}

		// ֻ����һ��key
		while( videoPackage.size()>1 )
		{
			H264VideoPackge *packet =  videoPackage.front();
			ReleaseVideoPacket_nolock( packet );
			videoPackage.pop_front();
		}

		if( !videoPackage.empty() )
		{
			H264VideoPackge *packet =  videoPackage.front();
			keyts = packet->timeStamp;
		}

		const uint32_t sizeafter = videoPackage.size();
		LOGMSG( "RTMPSend::ReListOfOneKey(%s) after do, sizebefore=%d, sizeafter=%d, keyts=%u\r\n", m_name.c_str(), sizebefore, sizeafter, keyts );
	}

	m_queue_mutex.unlock();

	return keyts;
}

// �ͷ� ��ʵ���������
void RTMPSend::ReleaseVideoPacket( H264VideoPackge *p )
{
	m_queue_mutex.lock();
	ReleaseVideoPacket_nolock( p );
	m_queue_mutex.unlock();
}

// �ͷ� ��ʵ���������
void RTMPSend::ReleaseVideoPacket_nolock( H264VideoPackge *p )
{
#if 1 // defuseMemoryContainer
	FinalDeleteVideoPacket(p);
#else
	this->videoPackreuse.push_back(p);
#endif
	//LOGMSG( "videoPackreuse %d\r\n", videoPackreuse.size() );
}

// ����ɾ���ʵ������
void RTMPSend::FinalDeleteVideoPacket( H264VideoPackge *p )
{
#if 1 // defuseMemoryContainer  //jyc20170323 remove
	//g_GetMemoryContainer()->ReleaseMemory( (char*)p, p->bufLimit+sizeof(H264VideoPackge) ); // �ṹ�ͻ��������ͬһ�����ڴ�����
#else
	delete [](p->buf);
	delete p;
#endif
}

bool RTMPSend::IsReady()
{
	if( wait_frist_key_frame )
		return false;

	if( HasVideoPacket()<1 )
		return false;

	return true;
}

bool RTMPSend::IsRtmpLive()
{
	m_queue_mutex.lock();

	const bool iscurlive = ( (timeGetTime()-m_lastGetVideoPacketTime) < 60000 );

	m_queue_mutex.unlock();

	return iscurlive;
}

uint32_t RTMPSend::GetQueMaxSize() const
{
	if( GetIPlayBack() )
	{
		return 2000;
	}

	return 200;
}

uint32_t RTMPSend::GetReuseQueMaxSize() const
{
#if 1 // defuseMemoryContainer
	if( GetIPlayBack() )
	{
		return 100;
	}

	return 100;
#else
	if( GetIPlayBack() )
	{
		return 900;
	}

	return 500;
#endif
}

std::string RTMPSend::getUrl()
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	return this->rtmpUrl;
}

void RTMPSend::setUrl( const std::string& url )
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	this->rtmpUrl = url;

	if( this->rtmpUrl.empty() )
	{
		m_PeerID = "";
	}

	if( this->m_PeerID.empty() )
	{
		this->m_P2PUrl = "";
	}
	else
	{
		if( g_IsRTMFP_url( this->rtmpUrl ) )
		{
			this->m_P2PUrl = this->rtmpUrl;
		}
	}
}

std::string RTMPSend::getStreamID()
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	return this->m_StreamID;
}

void RTMPSend::setStreamID( const std::string& StreamID )
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	this->m_StreamID = StreamID;
}

std::string RTMPSend::getP2PUrl()
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	return this->m_P2PUrl;
}

std::string RTMPSend::getPeerID()
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	return this->m_PeerID;
}

void RTMPSend::setPeerID( const std::string& PeerID )
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	this->m_PeerID = PeerID;
	
	if( this->m_PeerID.empty() )
	{
		this->m_P2PUrl = "";

#if 1
		// ��������ѶϿ�������P2P����
#else
		// ��������ѶϿ����Ͽ�P2P����
		if( g_IsRTMFP_url( this->rtmpUrl ) )
		{
			this->rtmpUrl = "";
		}
#endif
	}
	else
	{
		if( g_IsRTMFP_url(this->rtmpUrl) )
		{
			this->m_P2PUrl = this->rtmpUrl;
		}
	}
}

uint32_t RTMPSend::getRTMFPSessionCount()
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	return this->m_RTMFPSessionCount;
}

void RTMPSend::setRTMFPSessionCount( const uint32_t RTMFPSessionCount )
{
	gloox::util::MutexGuard mutexguard( m_queue_mutex );

	this->m_RTMFPSessionCount = RTMFPSessionCount;
}
