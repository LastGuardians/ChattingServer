#include "stdafx.h"



PacketHandler::PacketHandler()
{
}


PacketHandler::~PacketHandler()
{
}

void PacketHandler::SendBroadcast(google::protobuf::MessageLite * msg)
{

}

void PacketHandler::PrintMessage(const protobuf::Message & message) const
{	
	std::string textFormatStr;
	protobuf::TextFormat::PrintToString(message, &textFormatStr);
	printf("%s\n", textFormatStr.c_str());
}

//void PacketHandler::PacketProcess(protobuf::io::CodedInputStream & input_stream, const PacketHandler & handler)
//{
//	MessageHeader messageHeader;
//	// ����� �о
//	while (input_stream.ReadRaw(&messageHeader, MessageHeaderSize))
//	{
//		// ���� �＼�� �Ҽ� �ִ� ���� �����Ϳ� ���� ���̸� �˾Ƴ�
//
//		const void* payload_ptr = NULL;
//		int remainSize = 0;
//		input_stream.GetDirectBufferPointer(&payload_ptr, &remainSize);
//		if (remainSize < (signed)messageHeader.size)
//			break;
//
//
//		// �޼��� ��ü�� �о�� ���� ��Ʈ���� ����
//		protobuf::io::ArrayInputStream payload_array_stream(payload_ptr, messageHeader.size);
//		protobuf::io::CodedInputStream payload_input_stream(&payload_array_stream);
//
//
//		// �޼��� ��ü ������ ��ŭ ������ ����
//		input_stream.Skip(messageHeader.size);
//
//		// �޼��� �������� ������ȭ�ؼ� ������ �޼��带 ȣ������
//		Channel_P::channel_chatting message;
//		if (false == message.ParseFromCodedStream(&payload_input_stream))
//			break;
//
//		handler.Handle(message);
//
//
//		/*switch (messageHeader.type)
//		{
//		case simple::LOGIN:
//		{
//			simple::Login message;
//			if (false == message.ParseFromCodedStream(&payload_input_stream))
//				break;
//			handler.Handle(message);
//		}
//		break;
//		case simple::CHAT:
//		{
//			simple::Chat message;
//			if (false == message.ParseFromCodedStream(&payload_input_stream))
//				break;
//			handler.Handle(message);
//		}
//		break;
//		case simple::MOVE:
//		{
//			simple::Move message;
//			if (false == message.ParseFromCodedStream(&payload_input_stream))
//				break;
//			handler.Handle(message);
//		}
//		break;
//		}*/
//	}
//}

