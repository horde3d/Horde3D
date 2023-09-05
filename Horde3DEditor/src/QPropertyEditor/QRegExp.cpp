#include "QRegExp.h"

QRegExp::QRegExp() : caseSensitive( false ), useMinimal( false )
{
    this->mLength = -1;
}

QRegExp::QRegExp(const QString &pattern) : caseSensitive( false ), useMinimal( false )
{
    this->pattern = pattern;
    expr.setPattern( pattern );
}

int QRegExp::indexIn(const QString &str, int offset)
{
    //Найти совпадение в str со смещением позиции offset (по умолчанию 0).
    //Возвращает позицию первого совпадения или -1, если совпадений не было.
    if ( expr.pattern().isEmpty() ) expr.setPattern( str );
//    QRegularExpression q (pattern);

    int flags = 0;
    if ( caseSensitive ) flags |= QRegularExpression::CaseInsensitiveOption;
    if ( useMinimal ) flags |= QRegularExpression::InvertedGreedinessOption;

    expr.setPatternOptions( (QRegularExpression::PatternOptions) flags );

    QRegularExpressionMatch match = expr.match(str,offset);
    bool hasMatch = match.hasMatch();
    if (!hasMatch) { this->mLength = -1; return -1; }

    lastMatch = match;
    int startOffset = match.capturedStart();
    int endOffset = match.capturedEnd();
    this->mLength = endOffset - startOffset;

    return startOffset;
}

int	QRegExp::matchedLength()
{
    // returns the length of the last matched string or -1 otherwise
    return this->mLength;
}

QString QRegExp::cap(int index)
{
    if ( index < 1 && index > mLength ) return QString();
    if ( !lastMatch.isValid() ) return QString();

    return lastMatch.captured( index );
}

void QRegExp::setCaseSensitivity(int sensitivity)
{
    caseSensitive = sensitivity;
}

void QRegExp::setMinimal(bool set)
{
    useMinimal = set;
}
