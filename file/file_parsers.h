#ifndef FILE_PARSERS_H
#define FILE_PARSERS_H

#include <QObject>
#include "flv/flv_parser.h"

class FileParsers : public QObject
{
    Q_OBJECT

public:
    FileParsers();
    virtual ~FileParsers();

public slots:
    void OnFlvFileOpen(const QString& file_path);

private:
    flv::FlvParser flv_parser_;
};

#endif // FILE_PARSERS_H
