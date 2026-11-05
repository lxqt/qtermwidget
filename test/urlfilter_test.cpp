#include "Filter.h"

#include <cstdlib>
#include <iostream>

using Konsole::Filter;
using Konsole::RegExpFilter;
using Konsole::UrlFilter;

static void check(const QString &text, const QString &expected)
{
    QList<int> linePositions{0};
    UrlFilter filter;
    filter.setBuffer(&text, &linePositions);
    filter.process();

    const QList<Filter::HotSpot *> spots = filter.hotSpots();
    if (spots.size() != 1)
    {
        std::cerr << "expected one URL in: " << text.toStdString() << '\n';
        std::exit(1);
    }

    const auto *spot = static_cast<RegExpFilter::HotSpot *>(spots.constFirst());
    if (spot->capturedTexts().constFirst() != expected
        || spot->endColumn() - spot->startColumn() != expected.size())
    {
        std::cerr << "expected " << expected.toStdString() << ", got "
                  << spot->capturedTexts().constFirst().toStdString() << '\n';
        std::exit(1);
    }
}

int main()
{
    check(QStringLiteral("Codex issue #45091 (https://github.com/openai/codex/issues/45091): another"),
          QStringLiteral("https://github.com/openai/codex/issues/45091"));
    check(QStringLiteral("[*C*](https://en.wikipedia.org/wiki/C_(programming_language))"),
          QStringLiteral("https://en.wikipedia.org/wiki/C_(programming_language)"));
    check(QStringLiteral("https://example.com/a,b?x=1#part"),
          QStringLiteral("https://example.com/a,b?x=1#part"));
    check(QStringLiteral("https://example.com..."), QStringLiteral("https://example.com"));
    check(QStringLiteral("HTTPS://EXAMPLE.COM/Path"), QStringLiteral("HTTPS://EXAMPLE.COM/Path"));
}
