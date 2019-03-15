#include "file_parsers.h"

FileParsers::FileParsers() : flv_parser_()
{

}

FileParsers::~FileParsers()
{

}

void FileParsers::OnFlvFileOpen(const QString& file_path)
{
    flv_parser_.OnFileOpen(file_path);
}
