#ifndef FLV_BASE_H
#define FLV_BASE_H

#include <string>

#define FLV_HEAD_LEN 9
#define PREVIOUS_TAG_SIZE_LEN 4
#define TAG_HEAD_LEN 11

namespace flv
{
struct FlvHead
{
    int version;
    int has_video, has_audio;
    unsigned int head_size; // FLV版本1时为9，表明的是FLV头的大小，为后期的FLV版本扩展使用。包括这四个字节。

    FlvHead()
    {
        version = 0;
        has_video = 0;
        has_audio = 0;
        head_size = 0;
    }

    ~FlvHead() {}

    int Build(const unsigned char* buf, int left_len);
};

struct TagHead
{
    int type;
    int data_size; // 数据区的大小，不包括tag head
    int timestamp; //当前帧时戳，单位是毫秒。相对于FLV文件的第一个TAG时戳。第一个tag的时戳总是0。
    int timestamp_ex; // 如果时戳大于0xFFFFFF，将会使用这个字节。这个字节是时戳的高8位，上面的三个字节是低24位。
    int stream_id; // 总是0

    TagHead()
    {
        type = 0;
        data_size = 0;
        timestamp = 0;
        timestamp_ex = 0;
        stream_id = 0;
    }

    ~TagHead() {}

    int Build(const unsigned char* buf, int left_len);
};

struct Tag
{
    int tag_idx;
    unsigned int dts; // timestamp_ex + timestamp。音频pts和dts一样，视频pts=dts+cts
    TagHead tag_head;
    std::string tag_data;

    Tag() : tag_head(), tag_data()
    {
        tag_idx = 0;
        dts = 0;
    }

    virtual ~Tag() {}

    virtual int Build(int tag_idx, const TagHead& tag_head, const unsigned char* buf, int left_len);
};
}

#endif // FLV_BASE_H
