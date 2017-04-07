################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/APlayer.o \
../src/AddressManager.o \
../src/AudioCap.o \
../src/AudioCapBase.o \
../src/AudioCap_AAC.o \
../src/AudioCap_File.o \
../src/AudioCap_G711.o \
../src/AudioCfg.o \
../src/Audio_WAV2AAC.o \
../src/AutoCallEvent.o \
../src/AutoControlEvent.o \
../src/AutoEventthing.o \
../src/AutoNoticeEvent.o \
../src/AutoSendSMSEvent.o \
../src/CANDeviceControl.o \
../src/CameraControl.o \
../src/CameraFocal.o \
../src/CameraPTZ.o \
../src/Column.o \
../src/CommLink.o \
../src/CommLinkManager.o \
../src/ControlEvent.o \
../src/ControlMessage.o \
../src/DBUpdate.o \
../src/DataStoreMgr.o \
../src/Database.o \
../src/DeviceAddress.o \
../src/DeviceConnection.o \
../src/DeviceManager.o \
../src/EditAttrMgr.o \
../src/GSIOTClient.o \
../src/GSIOTConfig.o \
../src/GSIOTControl.o \
../src/GSIOTDevice.o \
../src/GSIOTDeviceInfo.o \
../src/GSIOTEvent.o \
../src/GSIOTHeartbeat.o \
../src/GSIOTInfo.o \
../src/GSIOTUser.o \
../src/GSIOTUserMgr.o \
../src/GSRemoteTalkMgr.o \
../src/H264MediaFile.o \
../src/H264NalReader.o \
../src/HeartbeatMon.o \
../src/HttpRequest.o \
../src/IGSMessageHandler.o \
../src/IPCamConnection.o \
../src/IPCameraBase.o \
../src/IPCameraManager.o \
../src/MediaControl.o \
../src/MediaStream.o \
../src/ModbusProc.o \
../src/NetBaseClient.o \
../src/NetMan.o \
../src/NetModule.o \
../src/NetTcpClient.o \
../src/NetUdpClient.o \
../src/PresetManager.o \
../src/RFDevice.o \
../src/RFDeviceControl.o \
../src/RFRemoteControl.o \
../src/RFSignalDefine.o \
../src/RS485DevControl.o \
../src/RTMPSend.o \
../src/RTMPSession.o \
../src/RTSPConnection.o \
../src/RTSPManager.o \
../src/RemoteButtonClass.o \
../src/RemoteButtonManager.o \
../src/RunCode.o \
../src/SQLiteHelper.o \
../src/SerialDataBuffer.o \
../src/SerialMessage.o \
../src/Statement.o \
../src/TimerManager.o \
../src/Transaction.o \
../src/TriggerControl.o \
../src/VObjClass.o \
../src/XmppGSAuth.o \
../src/XmppGSChange.o \
../src/XmppGSEvent.o \
../src/XmppGSManager.o \
../src/XmppGSMessage.o \
../src/XmppGSPlayback.o \
../src/XmppGSPreset.o \
../src/XmppGSRelation.o \
../src/XmppGSReport.o \
../src/XmppGSResult.o \
../src/XmppGSState.o \
../src/XmppGSTalk.o \
../src/XmppGSUpdate.o \
../src/XmppGSVObj.o \
../src/XmppRegister.o \
../src/common.o \
../src/gsarm.o \
../src/rs232.o 

CPP_SRCS += \
../src/APlayer.cpp \
../src/AddressManager.cpp \
../src/AudioCap.cpp \
../src/AudioCapBase.cpp \
../src/AudioCap_AAC.cpp \
../src/AudioCap_File.cpp \
../src/AudioCap_G711.cpp \
../src/AudioCfg.cpp \
../src/Audio_WAV2AAC.cpp \
../src/AutoCallEvent.cpp \
../src/AutoControlEvent.cpp \
../src/AutoEventthing.cpp \
../src/AutoNoticeEvent.cpp \
../src/AutoSendSMSEvent.cpp \
../src/CANDeviceControl.cpp \
../src/CameraControl.cpp \
../src/CameraFocal.cpp \
../src/CameraPTZ.cpp \
../src/Column.cpp \
../src/CommLink.cpp \
../src/CommLinkManager.cpp \
../src/ControlEvent.cpp \
../src/ControlMessage.cpp \
../src/DBUpdate.cpp \
../src/DataStoreMgr.cpp \
../src/Database.cpp \
../src/DeviceAddress.cpp \
../src/DeviceConnection.cpp \
../src/DeviceManager.cpp \
../src/EditAttrMgr.cpp \
../src/GSIOTClient.cpp \
../src/GSIOTConfig.cpp \
../src/GSIOTControl.cpp \
../src/GSIOTDevice.cpp \
../src/GSIOTDeviceInfo.cpp \
../src/GSIOTEvent.cpp \
../src/GSIOTHeartbeat.cpp \
../src/GSIOTInfo.cpp \
../src/GSIOTUser.cpp \
../src/GSIOTUserMgr.cpp \
../src/GSRemoteTalkMgr.cpp \
../src/H264MediaFile.cpp \
../src/H264NalReader.cpp \
../src/HeartbeatMon.cpp \
../src/HttpRequest.cpp \
../src/IGSMessageHandler.cpp \
../src/IPCamConnection.cpp \
../src/IPCameraBase.cpp \
../src/IPCameraManager.cpp \
../src/MediaControl.cpp \
../src/MediaStream.cpp \
../src/ModbusProc.cpp \
../src/NetBaseClient.cpp \
../src/NetMan.cpp \
../src/NetModule.cpp \
../src/NetTcpClient.cpp \
../src/NetUdpClient.cpp \
../src/PresetManager.cpp \
../src/RFDevice.cpp \
../src/RFDeviceControl.cpp \
../src/RFRemoteControl.cpp \
../src/RFSignalDefine.cpp \
../src/RS485DevControl.cpp \
../src/RTMPSend.cpp \
../src/RTMPSession.cpp \
../src/RTSPConnection.cpp \
../src/RTSPManager.cpp \
../src/RemoteButtonClass.cpp \
../src/RemoteButtonManager.cpp \
../src/RunCode.cpp \
../src/SQLiteHelper.cpp \
../src/SerialDataBuffer.cpp \
../src/SerialMessage.cpp \
../src/Statement.cpp \
../src/TimerManager.cpp \
../src/Transaction.cpp \
../src/TriggerControl.cpp \
../src/VObjClass.cpp \
../src/XmppGSAuth.cpp \
../src/XmppGSChange.cpp \
../src/XmppGSEvent.cpp \
../src/XmppGSManager.cpp \
../src/XmppGSMessage.cpp \
../src/XmppGSPlayback.cpp \
../src/XmppGSPreset.cpp \
../src/XmppGSRelation.cpp \
../src/XmppGSReport.cpp \
../src/XmppGSResult.cpp \
../src/XmppGSState.cpp \
../src/XmppGSTalk.cpp \
../src/XmppGSUpdate.cpp \
../src/XmppGSVObj.cpp \
../src/XmppRegister.cpp \
../src/common.cpp \
../src/rs232.cpp 

CC_SRCS += \
../src/gsarm.cc 

OBJS += \
./src/APlayer.o \
./src/AddressManager.o \
./src/AudioCap.o \
./src/AudioCapBase.o \
./src/AudioCap_AAC.o \
./src/AudioCap_File.o \
./src/AudioCap_G711.o \
./src/AudioCfg.o \
./src/Audio_WAV2AAC.o \
./src/AutoCallEvent.o \
./src/AutoControlEvent.o \
./src/AutoEventthing.o \
./src/AutoNoticeEvent.o \
./src/AutoSendSMSEvent.o \
./src/CANDeviceControl.o \
./src/CameraControl.o \
./src/CameraFocal.o \
./src/CameraPTZ.o \
./src/Column.o \
./src/CommLink.o \
./src/CommLinkManager.o \
./src/ControlEvent.o \
./src/ControlMessage.o \
./src/DBUpdate.o \
./src/DataStoreMgr.o \
./src/Database.o \
./src/DeviceAddress.o \
./src/DeviceConnection.o \
./src/DeviceManager.o \
./src/EditAttrMgr.o \
./src/GSIOTClient.o \
./src/GSIOTConfig.o \
./src/GSIOTControl.o \
./src/GSIOTDevice.o \
./src/GSIOTDeviceInfo.o \
./src/GSIOTEvent.o \
./src/GSIOTHeartbeat.o \
./src/GSIOTInfo.o \
./src/GSIOTUser.o \
./src/GSIOTUserMgr.o \
./src/GSRemoteTalkMgr.o \
./src/H264MediaFile.o \
./src/H264NalReader.o \
./src/HeartbeatMon.o \
./src/HttpRequest.o \
./src/IGSMessageHandler.o \
./src/IPCamConnection.o \
./src/IPCameraBase.o \
./src/IPCameraManager.o \
./src/MediaControl.o \
./src/MediaStream.o \
./src/ModbusProc.o \
./src/NetBaseClient.o \
./src/NetMan.o \
./src/NetModule.o \
./src/NetTcpClient.o \
./src/NetUdpClient.o \
./src/PresetManager.o \
./src/RFDevice.o \
./src/RFDeviceControl.o \
./src/RFRemoteControl.o \
./src/RFSignalDefine.o \
./src/RS485DevControl.o \
./src/RTMPSend.o \
./src/RTMPSession.o \
./src/RTSPConnection.o \
./src/RTSPManager.o \
./src/RemoteButtonClass.o \
./src/RemoteButtonManager.o \
./src/RunCode.o \
./src/SQLiteHelper.o \
./src/SerialDataBuffer.o \
./src/SerialMessage.o \
./src/Statement.o \
./src/TimerManager.o \
./src/Transaction.o \
./src/TriggerControl.o \
./src/VObjClass.o \
./src/XmppGSAuth.o \
./src/XmppGSChange.o \
./src/XmppGSEvent.o \
./src/XmppGSManager.o \
./src/XmppGSMessage.o \
./src/XmppGSPlayback.o \
./src/XmppGSPreset.o \
./src/XmppGSRelation.o \
./src/XmppGSReport.o \
./src/XmppGSResult.o \
./src/XmppGSState.o \
./src/XmppGSTalk.o \
./src/XmppGSUpdate.o \
./src/XmppGSVObj.o \
./src/XmppRegister.o \
./src/common.o \
./src/gsarm.o \
./src/rs232.o 

CC_DEPS += \
./src/gsarm.d 

CPP_DEPS += \
./src/APlayer.d \
./src/AddressManager.d \
./src/AudioCap.d \
./src/AudioCapBase.d \
./src/AudioCap_AAC.d \
./src/AudioCap_File.d \
./src/AudioCap_G711.d \
./src/AudioCfg.d \
./src/Audio_WAV2AAC.d \
./src/AutoCallEvent.d \
./src/AutoControlEvent.d \
./src/AutoEventthing.d \
./src/AutoNoticeEvent.d \
./src/AutoSendSMSEvent.d \
./src/CANDeviceControl.d \
./src/CameraControl.d \
./src/CameraFocal.d \
./src/CameraPTZ.d \
./src/Column.d \
./src/CommLink.d \
./src/CommLinkManager.d \
./src/ControlEvent.d \
./src/ControlMessage.d \
./src/DBUpdate.d \
./src/DataStoreMgr.d \
./src/Database.d \
./src/DeviceAddress.d \
./src/DeviceConnection.d \
./src/DeviceManager.d \
./src/EditAttrMgr.d \
./src/GSIOTClient.d \
./src/GSIOTConfig.d \
./src/GSIOTControl.d \
./src/GSIOTDevice.d \
./src/GSIOTDeviceInfo.d \
./src/GSIOTEvent.d \
./src/GSIOTHeartbeat.d \
./src/GSIOTInfo.d \
./src/GSIOTUser.d \
./src/GSIOTUserMgr.d \
./src/GSRemoteTalkMgr.d \
./src/H264MediaFile.d \
./src/H264NalReader.d \
./src/HeartbeatMon.d \
./src/HttpRequest.d \
./src/IGSMessageHandler.d \
./src/IPCamConnection.d \
./src/IPCameraBase.d \
./src/IPCameraManager.d \
./src/MediaControl.d \
./src/MediaStream.d \
./src/ModbusProc.d \
./src/NetBaseClient.d \
./src/NetMan.d \
./src/NetModule.d \
./src/NetTcpClient.d \
./src/NetUdpClient.d \
./src/PresetManager.d \
./src/RFDevice.d \
./src/RFDeviceControl.d \
./src/RFRemoteControl.d \
./src/RFSignalDefine.d \
./src/RS485DevControl.d \
./src/RTMPSend.d \
./src/RTMPSession.d \
./src/RTSPConnection.d \
./src/RTSPManager.d \
./src/RemoteButtonClass.d \
./src/RemoteButtonManager.d \
./src/RunCode.d \
./src/SQLiteHelper.d \
./src/SerialDataBuffer.d \
./src/SerialMessage.d \
./src/Statement.d \
./src/TimerManager.d \
./src/Transaction.d \
./src/TriggerControl.d \
./src/VObjClass.d \
./src/XmppGSAuth.d \
./src/XmppGSChange.d \
./src/XmppGSEvent.d \
./src/XmppGSManager.d \
./src/XmppGSMessage.d \
./src/XmppGSPlayback.d \
./src/XmppGSPreset.d \
./src/XmppGSRelation.d \
./src/XmppGSReport.d \
./src/XmppGSResult.d \
./src/XmppGSState.d \
./src/XmppGSTalk.d \
./src/XmppGSUpdate.d \
./src/XmppGSVObj.d \
./src/XmppRegister.d \
./src/common.d \
./src/rs232.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

