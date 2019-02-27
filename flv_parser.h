#ifndef FLV_PARSER_H
#define FLV_PARSER_H

#include <QObject>

class FlvParser : public QObject
{
    Q_OBJECT

public:
    FlvParser();
    virtual ~FlvParser();
};

#endif // FLV_PARSER_H
