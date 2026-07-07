#ifndef QREGEXP_H
#define QREGEXP_H

#include <QRegularExpression>

class QRegExp
{
public:
    QRegExp();
    QRegExp(const QString &pattern);
    int indexIn(const QString &str, int offset = 0);
    int matchedLength();

    QString cap( int index );

    void setCaseSensitivity( int sensitivity );
    void setMinimal( bool set );
private:
    QRegularExpression expr;
    QRegularExpressionMatch lastMatch;

    QString pattern;
    int mLength;

    bool caseSensitive;
    bool useMinimal;
};

#endif // QREGEXP_H
