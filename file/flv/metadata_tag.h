#ifndef METADATA_TAG_H
#define METADATA_TAG_H

#include <memory>
#include "flv_base.h"

//一般来说，该Tag Data结构包含两个AMF包。AMF（Action Message Format）是Adobe设计的一种通用数据封装格式。第一个AMF包封装字符串类型数据，用来装入一个“onMetaData”标志，这个标志与Adobe的一些API调用有关。
//第二个AMF包封装一个数组类型，这个数组中包含了音视频信息项的名称和值。具体说明如下：

//第一个AMF包：
//       第1个字节表示AMF包类型，一般总是0x02，表示字符串，其他值表示意义请查阅文档。
//       第2-3个字节为UI16类型值，表示字符串的长度，一般总是0x000A（“onMetaData”长度）。
//       后面字节为字符串数据，一般总为“onMetaData”。

//第二个AMF包：
//       第1个字节表示AMF包类型，一般总是0x08，表示数组。
//       第2-5个字节为UI32类型值，表示数组元素的个数。
//       后面即为各数组元素的封装，数组元素为元素名称和值组成的对。表示方法如下：
//       第1-2个字节表示元素名称的长度，假设为L。
//       后面跟着为长度为L的字符串。
//       第L+3个字节表示元素值的类型。
//       后面跟着为对应值，占用字节数取决于值的类型

namespace flv
{
struct MetadataTag : public Tag, public std::enable_shared_from_this<MetadataTag>
{
    double duration; // 时长
    double width; // 视频宽度
    double height; // 视频高度
    double videodatarate; // 视频码率（比特率）
    double framerate; // 视频帧率（fps）
    double videocodecid; // 视频编码
    double audiodatarate; // 音频码率
    double audiosamplerate; // 音频采样率
    double audiosamplesize; // 音频采样精度
    bool stereo; // 是否为立体声
    double audiocodecid; // 音频编码
    std::string major_brand;
    std::string minor_version;
    std::string compatible_brands;
    std::string encoder;
    double filesize; // 文件大小

    MetadataTag() : major_brand(), minor_version(), compatible_brands(), encoder()
    {
        duration = 0;
        width = 0;
        height = 0;
        videodatarate = 0;
        framerate = 0;
        videocodecid = 0;
        audiodatarate = 0;
        audiosamplerate = 0;
        audiosamplesize = 0;
        stereo = false;
        audiocodecid = 0;
        filesize = 0;
    }

    virtual ~MetadataTag() {}

    int Build(int tag_idx, const TagHead& tag_head, const unsigned char* buf, int left_len) override;

private:
    int ParseECMAArray(const unsigned char* metadata);
    void Print();
};
}

#endif // METADATA_TAG_H
