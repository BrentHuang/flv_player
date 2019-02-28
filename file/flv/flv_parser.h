#ifndef FLV_PARSER_H
#define FLV_PARSER_H

#include <memory>
#include <QString>
#include "flv_base.h"

//一个FLV文件，每种类型的tag都属于一个流，也就是一个flv文件最多只有一个音频流，一个视频流，
//不存在多个独立的音视频流在一个文件的情况。另外，FLV文件格式所用的是大端序。
//body部分由一个个Tag组成，每个Tag的下面有一块4bytes的空间，用来记录这个tag的长度

namespace flv
{
class FlvParser
{
public:
    FlvParser();
    ~FlvParser();

    void OnFileOpen(const QString& file_path);

private:
    int Parse(int& used_len, const unsigned char* buf, int buf_size);
    std::shared_ptr<Tag> CreateTag(const unsigned char* buf, int left_len);

private:
    std::unique_ptr<FlvHead> flv_head_;
    int tag_idx_;
};
}

#endif // FLV_PARSER_H
