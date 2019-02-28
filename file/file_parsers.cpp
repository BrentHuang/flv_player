#include "file_parsers.h"

FileParsers::FileParsers() : flv_parser_()
{

}

FileParsers::~FileParsers()
{

}

void FileParsers::OnFlvFileOpen(const QString& file_path)
{
    // TODO 判断文件类型，选择文件解析器
    flv_parser_.OnFileOpen(file_path);
}
