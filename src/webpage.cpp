#include "webpage.h"

#include <math.h>

#include <QApplication>
#include <QPainter>
#include <QPrinter>
#include <QWebFrame>
#include <QWebPage>
#include <QImageWriter>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QBuffer>

#ifdef Q_OS_WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include <system.h>
#include <callback.h>

#define BLANK_HTML  "<html><head></head><body></body></html>"

#define STDOUT_FILENAME "/dev/stdout"
#define STDERR_FILENAME "/dev/stderr"

WebPage::WebPage(const QUrl &baseUrl)
{
    setObjectName("WebPage");
    m_customWebPage = new QWebPage(this);
    m_mainFrame = m_customWebPage->mainFrame();
    m_currentFrame = m_mainFrame;
    m_mainFrame->setHtml(BLANK_HTML, baseUrl);

    // Start with transparent background.
    QPalette palette = m_customWebPage->palette();
    palette.setBrush(QPalette::Base, Qt::transparent);
    m_customWebPage->setPalette(palette);

    // Page size does not need to take scrollbars into account.
    m_mainFrame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    m_mainFrame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    m_customWebPage->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    m_customWebPage->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
    m_customWebPage->settings()->setAttribute(QWebSettings::FrameFlatteningEnabled, true);

    m_customWebPage->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    m_customWebPage->settings()->setLocalStoragePath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));

    m_customWebPage->settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    m_customWebPage->settings()->setAttribute(QWebSettings::PluginsEnabled, false);
    m_customWebPage->settings()->setAttribute(QWebSettings::AutoLoadImages, true);
    m_customWebPage->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);

    m_customWebPage->setViewportSize(QSize(400, 300));
}

WebPage::~WebPage()
{
    emit closing(this);
}

QWebFrame *WebPage::mainFrame()
{
    return m_mainFrame;
}

QString WebPage::content() const
{
    return m_mainFrame->toHtml();
}

QString WebPage::frameContent() const
{
    return m_currentFrame->toHtml();
}

void WebPage::setContent(const QString &content)
{
    m_mainFrame->setHtml(content);
}

void WebPage::setContent(const QString &content, const QString &baseUrl)
{
    if (baseUrl == "about:blank") {
        m_mainFrame->setHtml(BLANK_HTML);
    } else {
        m_mainFrame->setHtml(content, QUrl(baseUrl));
    }
}

void WebPage::setFrameContent(const QString &content)
{
    m_currentFrame->setHtml(content);
}

QString WebPage::title() const
{
    return m_mainFrame->title();
}

QString WebPage::frameTitle() const
{
    return m_currentFrame->title();
}

QString WebPage::plainText() const
{
    return m_mainFrame->toPlainText();
}

QString WebPage::framePlainText() const
{
    return m_currentFrame->toPlainText();
}

void WebPage::setViewportSize(const QVariantMap &size)
{
    int w = size.value("width").toInt();
    int h = size.value("height").toInt();
    if (w > 0 && h > 0)
        m_customWebPage->setViewportSize(QSize(w, h));
}

QVariantMap WebPage::viewportSize() const
{
    QVariantMap result;
    QSize size = m_customWebPage->viewportSize();
    result["width"] = size.width();
    result["height"] = size.height();
    return result;
}

void WebPage::setClipRect(const QVariantMap &size)
{
    int w = size.value("width").toInt();
    int h = size.value("height").toInt();
    int top = size.value("top").toInt();
    int left = size.value("left").toInt();

    if (w >= 0 && h >= 0)
        m_clipRect = QRect(left, top, w, h);
}

QVariantMap WebPage::clipRect() const
{
    QVariantMap result;
    result["width"] = m_clipRect.width();
    result["height"] = m_clipRect.height();
    result["top"] = m_clipRect.top();
    result["left"] = m_clipRect.left();
    return result;
}

void WebPage::setOutputPath(const QString &path)
{
    output_path = path;
}

QString WebPage::getOutputPath() const
{
    return output_path;
}

void WebPage::setPaperSize(const QVariantMap &size)
{
    m_paperSize = size;
}

QVariantMap WebPage::paperSize() const
{
    return m_paperSize;
}

void WebPage::release()
{
    close();
}

void WebPage::close() {
    deleteLater();
}

bool WebPage::render(const QString &fileName, const QVariantMap &option)
{
    if (m_mainFrame->contentsSize().isEmpty())
        return false;

    QString outFileName = fileName;
    QString tempFileName = "";
    QString format = "";
    int quality = -1; // QImage#sae default

    if( fileName == STDOUT_FILENAME || fileName == STDERR_FILENAME ){
        if( !QFile::exists(fileName) ){
            // create temporary file for OS that have no /dev/stdout or /dev/stderr. (ex. windows)
            tempFileName = QDir::tempPath() + "/frendertemp" + QUuid::createUuid().toString();
            outFileName = tempFileName;
        }

        format = "png"; // default format for stdout and stderr
    }
    else{
        QFileInfo fileInfo(outFileName);
        QDir dir;
        dir.mkpath(fileInfo.absolutePath());
    }

    if( option.contains("format") ){
        format = option.value("format").toString();
    }
    else if (fileName.endsWith(".pdf", Qt::CaseInsensitive) ){
        format = "pdf";
    }

    if( option.contains("quality") ){
        quality = option.value("quality").toInt();
    }

    bool retval = true;
    if ( format == "pdf" ){
        retval = renderPdf(outFileName);
    }
    else{
        QImage rawPageRendering = renderImage();

        const char *f = 0; // 0 is QImage#save default
        if( format != "" ){
            f = format.toUtf8().constData();
        }

        retval = rawPageRendering.save(outFileName, f, quality);
    }

    if( tempFileName != "" ){
        // cleanup temporary file and render to stdout or stderr
        QFile i(tempFileName);
        i.open(QIODevice::ReadOnly);

        QByteArray ba = i.readAll();

        System *system = new System(this);
        if( fileName == STDOUT_FILENAME ){
#ifdef Q_OS_WIN32
            _setmode(_fileno(stdout), O_BINARY);
#endif

            ((File *)system->_stdout())->write(QString::fromLatin1(ba.constData(), ba.size()));

#ifdef Q_OS_WIN32
            _setmode(_fileno(stdout), O_TEXT);
#endif
        }
        else if( fileName == STDERR_FILENAME ){
#ifdef Q_OS_WIN32
            _setmode(_fileno(stderr), O_BINARY);
#endif

            ((File *)system->_stderr())->write(QString::fromLatin1(ba.constData(), ba.size()));

#ifdef Q_OS_WIN32
            _setmode(_fileno(stderr), O_TEXT);
#endif
        }

        i.close();

        QFile::remove(tempFileName);
    }

    return retval;
}

QString WebPage::renderBase64(const QByteArray &format)
{
    QByteArray nformat = format.toLower();

    // Check if the given format is supported
    if (QImageWriter::supportedImageFormats().contains(nformat)) {
        QImage rawPageRendering = renderImage();

        // Prepare buffer for writing
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);

        // Writing image to the buffer, using PNG encoding
        rawPageRendering.save(&buffer, nformat);

        return bytes.toBase64();
    }

    // Return an empty string in case an unsupported format was provided
    return "";
}

QImage WebPage::renderImage()
{
    QSize contentsSize = m_mainFrame->contentsSize();
    contentsSize -= QSize(m_scrollPosition.x(), m_scrollPosition.y());
    QRect frameRect = QRect(QPoint(0, 0), contentsSize);
    if (!m_clipRect.isNull())
        frameRect = m_clipRect;

    QSize viewportSize = m_customWebPage->viewportSize();
    m_customWebPage->setViewportSize(contentsSize);

#ifdef Q_OS_WIN32
    QImage::Format format = QImage::Format_ARGB32_Premultiplied;
#else
    QImage::Format format = QImage::Format_ARGB32;
#endif

    QImage buffer(frameRect.size(), format);
    buffer.fill(Qt::transparent);

    QPainter painter;

    // We use tiling approach to work-around Qt software rasterizer bug
    // when dealing with very large paint device.
    // See http://code.google.com/p/phantomjs/issues/detail?id=54.
    const int tileSize = 4096;
    int htiles = (buffer.width() + tileSize - 1) / tileSize;
    int vtiles = (buffer.height() + tileSize - 1) / tileSize;
    for (int x = 0; x < htiles; ++x) {
        for (int y = 0; y < vtiles; ++y) {

            QImage tileBuffer(tileSize, tileSize, format);
            tileBuffer.fill(Qt::transparent);

            // Render the web page onto the small tile first
            painter.begin(&tileBuffer);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter.translate(-frameRect.left(), -frameRect.top());
            painter.translate(-x * tileSize, -y * tileSize);
            m_mainFrame->render(&painter, QRegion(frameRect));
            painter.end();

            // Copy the tile to the main buffermatch
            painter.begin(&buffer);
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.drawImage(x * tileSize, y * tileSize, tileBuffer);
            painter.end();
        }
    }

    m_customWebPage->setViewportSize(viewportSize);
    return buffer;
}

#define FRENDER_PDF_DPI 72            // Different defaults. OSX: 72, X11: 75(?), Windows: 96

qreal stringToPointSize(const QString &string)
{
    static const struct {
        QString unit;
        qreal factor;
    } units[] = {
        { "mm", 72 / 25.4 },
        { "cm", 72 / 2.54 },
        { "in", 72 },
        { "px", 72.0 / FRENDER_PDF_DPI },
        { "", 72.0 / FRENDER_PDF_DPI }
    };
    for (uint i = 0; i < sizeof(units) / sizeof(units[0]); ++i) {
        if (string.endsWith(units[i].unit)) {
            QString value = string;
            value.chop(units[i].unit.length());
            return value.toDouble() * units[i].factor;
        }
    }
    return 0;
}

qreal printMargin(const QVariantMap &map, const QString &key)
{
    const QVariant margin = map.value(key);
    if (margin.isValid() && margin.canConvert(QVariant::String)) {
        return stringToPointSize(margin.toString());
    } else {
        return 0;
    }
}

bool WebPage::renderPdf(const QString &fileName)
{
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setResolution(FRENDER_PDF_DPI);
    QVariantMap paperSize = m_paperSize;

    if (paperSize.isEmpty()) {
        const QSize pageSize = m_mainFrame->contentsSize();
        paperSize.insert("width", QString::number(pageSize.width()) + "px");
        paperSize.insert("height", QString::number(pageSize.height()) + "px");
        paperSize.insert("margin", "0px");
    }

    if (paperSize.contains("width") && paperSize.contains("height")) {
        const QSizeF sizePt(ceil(stringToPointSize(paperSize.value("width").toString())),
                            ceil(stringToPointSize(paperSize.value("height").toString())));
        printer.setPaperSize(sizePt, QPrinter::Point);
    } else if (paperSize.contains("format")) {
        const QPrinter::Orientation orientation = paperSize.contains("orientation")
                && paperSize.value("orientation").toString().compare("landscape", Qt::CaseInsensitive) == 0 ?
                    QPrinter::Landscape : QPrinter::Portrait;
        printer.setOrientation(orientation);
        static const struct {
            QString format;
            QPrinter::PaperSize paperSize;
        } formats[] = {
            { "A0", QPrinter::A0 },
            { "A1", QPrinter::A1 },
            { "A2", QPrinter::A2 },
            { "A3", QPrinter::A3 },
            { "A4", QPrinter::A4 },
            { "A5", QPrinter::A5 },
            { "A6", QPrinter::A6 },
            { "A7", QPrinter::A7 },
            { "A8", QPrinter::A8 },
            { "A9", QPrinter::A9 },
            { "B0", QPrinter::B0 },
            { "B1", QPrinter::B1 },
            { "B2", QPrinter::B2 },
            { "B3", QPrinter::B3 },
            { "B4", QPrinter::B4 },
            { "B5", QPrinter::B5 },
            { "B6", QPrinter::B6 },
            { "B7", QPrinter::B7 },
            { "B8", QPrinter::B8 },
            { "B9", QPrinter::B9 },
            { "B10", QPrinter::B10 },
            { "C5E", QPrinter::C5E },
            { "Comm10E", QPrinter::Comm10E },
            { "DLE", QPrinter::DLE },
            { "Executive", QPrinter::Executive },
            { "Folio", QPrinter::Folio },
            { "Ledger", QPrinter::Ledger },
            { "Legal", QPrinter::Legal },
            { "Letter", QPrinter::Letter },
            { "Tabloid", QPrinter::Tabloid }
        };
        printer.setPaperSize(QPrinter::A4); // Fallback
        for (uint i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
            if (paperSize.value("format").toString().compare(formats[i].format, Qt::CaseInsensitive) == 0) {
                printer.setPaperSize(formats[i].paperSize);
                break;
            }
        }
    } else {
        return false;
    }

    if (paperSize.contains("border") && !paperSize.contains("margin")) {
        // backwards compatibility
        paperSize["margin"] = paperSize["border"];
    }

    qreal marginLeft = 0;
    qreal marginTop = 0;
    qreal marginRight = 0;
    qreal marginBottom = 0;

    if (paperSize.contains("margin")) {
        const QVariant margins = paperSize["margin"];
        if (margins.canConvert(QVariant::Map)) {
            const QVariantMap map = margins.toMap();
            marginLeft = printMargin(map, "left");
            marginTop = printMargin(map, "top");
            marginRight = printMargin(map, "right");
            marginBottom = printMargin(map, "bottom");
        } else if (margins.canConvert(QVariant::String)) {
            const qreal margin = stringToPointSize(margins.toString());
            marginLeft = margin;
            marginTop = margin;
            marginRight = margin;
            marginBottom = margin;
        }
    }

    printer.setPageMargins(marginLeft, marginTop, marginRight, marginBottom, QPrinter::Point);

    m_mainFrame->print(&printer, this);
    return true;
}

qreal getHeight(const QVariantMap &map, const QString &key)
{
    QVariant footer = map.value(key);
    if (!footer.canConvert(QVariant::Map)) {
        return 0;
    }
    QVariant height = footer.toMap().value("height");
    if (!height.canConvert(QVariant::String)) {
        return 0;
    }
    return stringToPointSize(height.toString());
}

qreal WebPage::footerHeight() const
{
    return getHeight(m_paperSize, "footer");
}

qreal WebPage::headerHeight() const
{
    return getHeight(m_paperSize, "header");
}

QString getHeaderFooter(const QVariantMap &map, const QString &key, int page, int numPages)
{
    QVariant header = map.value(key);
    if (!header.canConvert(QVariant::Map)) {
        return QString();
    }
    QVariant callback = header.toMap().value("contents");
    if (callback.canConvert<QObject*>()) {
        Callback* caller = qobject_cast<Callback*>(callback.value<QObject*>());
        if (caller) {
            QVariant ret = caller->call(QVariantList() << page << numPages);
            if (ret.canConvert(QVariant::String)) {
                return ret.toString();
            }
        }
    }
    return QString();
}

QString WebPage::header(int page, int numPages)
{
    return getHeaderFooter(m_paperSize, "header", page, numPages);
}

QString WebPage::footer(int page, int numPages)
{
    return getHeaderFooter(m_paperSize, "footer", page, numPages);
}

void WebPage::setZoomFactor(qreal zoom)
{
    m_mainFrame->setZoomFactor(zoom);
}

qreal WebPage::zoomFactor() const
{
    return m_mainFrame->zoomFactor();
}
