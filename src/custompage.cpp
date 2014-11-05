#include <QWebPage>
#include <QString>


class WebPage;


// Ensure we have at least head and body.
#define BLANK_HTML                      "<html><head></head><body></body></html>"

/**
  * @class CustomPage
  */
class CustomPage: public QWebPage
{
    Q_OBJECT

public:
    CustomPage(WebPage *parent = 0)
        : QWebPage(parent)
        , m_webPage(parent)
    {
        m_userAgent = QWebPage::userAgentForUrl(QUrl());
        setForwardUnsupportedContent(true);
    }

protected:
    QWebPage *createWindow (WebWindowType type) {
        Q_UNUSED(type);
        WebPage *newPage;

        // Return the new QWebPage to the QWebKit backend
        return newPage->m_customWebPage;
    }

private:
    WebPage *m_webPage;
    friend class WebPage;
};
