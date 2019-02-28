#include "flv_parser.h"
#include <fstream>
#include <iostream>
#include <QDebug>
#include "byte_util.h"
#include "metadata_tag.h"
#include "audio_tag.h"
#include "video_tag.h"

namespace flv
{
FlvParser::FlvParser() : flv_head_()
{
    tag_idx_ = 0;
}

FlvParser::~FlvParser()
{
}

void FlvParser::OnFileOpen(const QString& file_path)
{
    std::fstream fin;
    fin.open(file_path.toStdString(), std::ios_base::in | std::ios_base::binary);
    if (!fin)
    {
        qDebug() << "failed to open file: " << file_path;
        return;
    }

    const int buf_size = 128 * 1024;
    unsigned char* buf = new unsigned char[buf_size];
    if (NULL == buf)
    {
        qDebug() << "failed to alloc memory";
        fin.close();
        return;
    }

    int flv_pos = 0;

    forever
    {
        fin.read((char*) buf + flv_pos, buf_size - flv_pos);

        const int read_len = fin.gcount();
        if (0 == read_len)
        {
            qDebug() << "finished parsing file: " << file_path;
            break;
        }

        flv_pos += read_len;

        int used_len = 0;
        Parse(used_len, buf, flv_pos);

        if (flv_pos != used_len)
        {
            memmove(buf, buf + used_len, flv_pos - used_len);
        }

        flv_pos -= used_len;
    }

    delete [] buf;
    fin.close();
}

int FlvParser::Parse(int& used_len, const unsigned char* buf, int buf_size)
{
    int offset = 0;

    if (NULL == flv_head_)
    {
        if ((buf_size - offset) < FLV_HEAD_LEN)
        {
            used_len = offset;
            return 0;
        }

        flv_head_.reset(new FlvHead());
        if (nullptr == flv_head_)
        {
            qDebug() << "failed to alloc memory";
            return -1;
        }

        if (flv_head_->Build(buf + offset, buf_size - offset) != 0)
        {
            return -1;
        }

        offset += flv_head_->head_size;
    }

    forever
    {
        if ((buf_size - offset) < (PREVIOUS_TAG_SIZE_LEN + TAG_HEAD_LEN))
        {
            used_len = offset;
            return 0;
        }

        unsigned int prev_tag_size = ShowU32(buf + offset);
        (void) prev_tag_size;
        offset += 4;

        std::shared_ptr<Tag> tag = CreateTag(buf + offset, buf_size - offset);
        if (nullptr == tag)
        {
            offset -= 4;
            break;
        }

        offset += (TAG_HEAD_LEN + tag.get()->tag_head.data_size);
    }

    used_len = offset;
    return 0;
}

std::shared_ptr<Tag> FlvParser::CreateTag(const unsigned char* buf, int left_len)
{
    if (int(ShowU24(&buf[1]) + TAG_HEAD_LEN) > left_len) // 该tag的data还没有读全
    {
        return nullptr;
    }

    TagHead tag_head;

    if (tag_head.Build(buf, left_len) != 0)
    {
        return nullptr;
    }

    std::shared_ptr<Tag> tag = nullptr;

    switch (tag_head.type)
    {
        case 0x12:
        {
            tag.reset(new MetadataTag());
        }
        break;

        case 0x9:
        {
            tag.reset(new VideoTag());
        }
        break;

        case 0x8:
        {
            tag.reset(new AudioTag());
        }
        break;

        default:
        {
            qDebug() << "invalid tag type: " << tag_head.type;
            return nullptr;
        }
        break;
    }

    if (nullptr == tag)
    {
        qDebug() << "failed to alloc memory";
        return nullptr;
    }

    if (tag.get()->Build(tag_idx_++, tag_head, buf, left_len) != 0)
    {
        return nullptr;
    }

    return tag;
}
}
