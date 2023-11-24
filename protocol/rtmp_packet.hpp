#ifndef RS_RTMP_PACKET_HPP
#define RS_RTMP_PACKET_HPP

#include <common/core.hpp>
#include <protocol/rtmp_amf0.hpp>

namespace rtmp
{

enum class PeerBandwidthType
{
    HARD = 0,
    SOTF = 1,
    DYNAMIC = 2
};


class Packet
{
public:
    Packet();
    virtual ~Packet();

public:
    virtual int GetPreferCID();
    virtual int GetMessageType();
    virtual int Encode(int &size, char *&payload);
    virtual int Decode(BufferManager *manager);

protected:
    virtual int GetSize();
    virtual int EncodePacket(BufferManager *manager);
};

class SetChunkSizePacket: public Packet
{
public:
    SetChunkSizePacket();
    virtual ~SetChunkSizePacket();
public:
    //Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    //Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;

public:
    int32_t chunk_size;
};

class ConnectAppPacket : public Packet
{
public:
    ConnectAppPacket();
    virtual ~ConnectAppPacket();

public:
    // Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    //Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;
public:
    std::string command_name;
    double transaction_id;
    AMF0Object *args;
    AMF0Object *command_object;
};

class ConnectAppResPacket : public Packet
{
public:
    ConnectAppResPacket();
    ~ConnectAppResPacket();

public:
    //Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    // Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;

public:
    std::string command_name;
    double transaction_id;
    AMF0Object *props;
    AMF0Object *info;
};

class SetWindowAckSizePacket : public Packet
{
public:
    SetWindowAckSizePacket();
    virtual ~SetWindowAckSizePacket();

public:
    //Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    //Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;
public:
    int32_t ackowledgement_window_size;
};

class SetPeerBandwidthPacket : public Packet
{
public:
    SetPeerBandwidthPacket();
    virtual ~SetPeerBandwidthPacket();
public:
    //Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
protected:
    //Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;
public:
    int32_t bandwidth;
    int8_t type;
};

class FMLEStartPacket : public Packet
{
public:
    FMLEStartPacket();
    virtual ~FMLEStartPacket();

public:
    // Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    //Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;
public:
    std::string command_name;
    double transaction_id;
    std::string stream_name;
    AMF0Any *command_object;
};

class FMLEStartResPacket : public Packet
{
public:
    FMLEStartResPacket(double trans_id);
    virtual ~FMLEStartResPacket();

public:
    // Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    //Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;
public:
    std::string command_name;
    double transaction_id;
    std::string stream_name;
    AMF0Any *command_object;
    AMF0Any *args;
};

class CreateStreamPacket : public Packet
{
public:
    CreateStreamPacket();
    virtual ~CreateStreamPacket();

public:
    // Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    // Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;

public:
    std::string command_name;
    double transaction_id;
    AMF0Any *command_object;
};

class CreateStreamResPacket : public Packet
{
public:
    CreateStreamResPacket(double trans_id, int sid);
    virtual ~CreateStreamResPacket();

public:
    // Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    // Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;

public:
    std::string command_name;
    double transaction_id;
    AMF0Any *command_object;
    double stream_id;

};

class PublishPacket : public Packet
{
public:
    PublishPacket();
    virtual ~PublishPacket();

public:
    // Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    // Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;
public:
    std::string command_name;
    double transaction_id;
    AMF0Any *command_object;
    std::string stream_name;
    std::string type;
};

class OnStatusCallPacket : public Packet
{
public:
    OnStatusCallPacket();
    virtual ~OnStatusCallPacket();

public:
    // Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    // Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;

public:
    std::string command_name;
    double transaction_id;
    AMF0Any *args;
    AMF0Object *data;

};

class OnMetadataPacket : public Packet
{
public:
    OnMetadataPacket();
    virtual ~OnMetadataPacket();

public:
    // Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    // Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;

public:
    std::string name;
    AMF0Object *metadata;
};

class PlayPacket : public Packet
{
public:
    PlayPacket();
    virtual ~PlayPacket();

public:
    // Packet
    virtual int GetPreferCID() override;
    virtual int GetMessageType() override;
    virtual int Decode(BufferManager *manager) override;

protected:
    // Packet
    virtual int GetSize() override;
    virtual int EncodePacket(BufferManager *manager) override;

public:
    std::string command_name;
    double  transaction_id;
    AMF0Any *command_obj;
    std::string stream_name;
    double start;
    double duration;
    bool reset;
};


} //namespace rtmp

#endif
