#include "metadata_tag.h"
#include <QDebug>
#include "byte_util.h"

namespace flv
{
int MetadataTag::Build(int tag_idx, const TagHead& tag_head, const unsigned char* buf, int left_len)
{
    if (Tag::Build(tag_idx, tag_head, buf, left_len) != 0)
    {
        return -1;
    }

    const unsigned char* pd = (const unsigned char*) tag_data.data();

    if ((int) pd[0] != 0x2 || ShowU16(&pd[1]) != 0x000a || strncmp((const char*) &pd[3], "onMetaData", 10) != 0)
    {
        qDebug() << "invalid metadata tag";
        return -1;
    }

    const unsigned char* metadata = pd + 13; // tag data偏移13个字节。第一个字节是0x02，表示string类型，后面的2字节是字符串的长度，一般为0x00a，即'onMetaData'的长度，再后面就是字符串'onMetaData'。一共13个字节

    switch (metadata[0])
    {
        case 0x08: // 8 = ECMA array type
        {
            ParseECMAArray(metadata);
        }
        break;

        default:
        {
        }
        break;
    }

    return 0;
}

int MetadataTag::ParseECMAArray(const unsigned char* metadata)
{
    const int array_elem_count = ShowU32(&metadata[1]);
    int offset = 5;

    for (int i = 0; i < array_elem_count; ++i)
    {
        double num_value = 0;
        bool bool_value = false;
        std::string str_value = "";

        const int elem_name_len = ShowU16(&metadata[offset]);
        offset += 2;

        char elem_name[elem_name_len + 1] = "";
        memcpy(elem_name, &metadata[offset], elem_name_len);
        offset += elem_name_len;

        const int elem_value_type = metadata[offset];
        offset += 1;

        switch (elem_value_type)
        {
            case 0: // Number type，数据占sizeof(double)字节
            {
                num_value = HexStr2double(&metadata[offset], sizeof(double));
                offset += 8;
            }
            break;

            case 1: // Boolean type, 数据占1字节
            {
                if (metadata[offset++] != 0)
                {
                    bool_value = true;
                }
            }
            break;

            case 2: // String type
            {
                const int str_value_len = ShowU16(metadata + offset);
                offset += 2;
                str_value.assign((const char*) (metadata + offset), str_value_len);
                offset += str_value_len;
            }
            break;

            case 12: // Long string type
            {
                const int str_value_len = ShowU32(metadata + offset);
                offset += 4;
                str_value.assign((const char*) (metadata + offset), str_value_len);
                offset += str_value_len;
            }
            break;

            // TODO other types
            default:
            {
                qDebug() << "elem value type: " << elem_value_type;
            }
            break;
        }

        if (0 == strncmp(elem_name, "duration", 8))
        {
            duration = num_value;
        }
        else if (0 == strncmp(elem_name, "width", 5))
        {
            width = num_value;
        }
        else if (0 == strncmp(elem_name, "height", 6))
        {
            height = num_value;
        }
        else if (0 == strncmp(elem_name, "videodatarate", 13))
        {
            videodatarate = num_value;
        }
        else if (0 == strncmp(elem_name, "framerate", 9))
        {
            framerate = num_value;
        }
        else if (0 == strncmp(elem_name, "videocodecid", 12))
        {
            videocodecid = num_value;
        }
        else if (0 == strncmp(elem_name, "audiodatarate", 13))
        {
            audiodatarate = num_value;
        }
        else if (0 == strncmp(elem_name, "audiosamplerate", 15))
        {
            audiosamplerate = num_value;
        }
        else if (0 == strncmp(elem_name, "audiosamplesize", 15))
        {
            audiosamplesize = num_value;
        }
        else if (0 == strncmp(elem_name, "stereo", 6))
        {
            stereo = bool_value;
        }
        else if (0 == strncmp(elem_name, "audiocodecid", 12))
        {
            audiocodecid = num_value;
        }
        else if (0 == strncmp(elem_name, "major_brand", 11))
        {
            major_brand = str_value;
        }
        else if (0 == strncmp(elem_name, "minor_version", 13))
        {
            minor_version = str_value;
        }
        else if (0 == strncmp(elem_name, "compatible_brands", 17))
        {
            compatible_brands = str_value;
        }
        else if (0 == strncmp(elem_name, "encoder", 7))
        {
            encoder = str_value;
        }
        else if (0 == strncmp(elem_name, "filesize", 8))
        {
            filesize = num_value;
        }
        else
        {
            // TODO
            qDebug() << "elem name: " << elem_name;
        }
    }

    return 0;
}
}
