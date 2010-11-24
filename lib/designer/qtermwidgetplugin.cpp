
#include "qtermwidgetplugin.h"

#include <QtPlugin>

#include "qtermwidget.h"


QTermWidgetPlugin::QTermWidgetPlugin(QObject *parent)
    : QObject(parent), initialized(false)
{
    Q_INIT_RESOURCE(qtermwidgetplugin);
}


QTermWidgetPlugin::~QTermWidgetPlugin()
{
}


void QTermWidgetPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    initialized = true;
}


bool QTermWidgetPlugin::isInitialized() const
{
    return initialized;
}


QWidget *QTermWidgetPlugin::createWidget(QWidget *parent)
{
    return new QTermWidget(0, parent);
}


QString QTermWidgetPlugin::name() const
{
    return "QTermWidget";
}


QString QTermWidgetPlugin::group() const
{
    return "Input Widgets";
}


QIcon QTermWidgetPlugin::icon() const
{
    return QIcon(":qtermwidget.png");
}


QString QTermWidgetPlugin::toolTip() const
{
    return "QTermWidget component/widget";
}


QString QTermWidgetPlugin::whatsThis() const
{
    return "Qt based terminal emulator";
}


bool QTermWidgetPlugin::isContainer() const
{
    return false;
}


QString QTermWidgetPlugin::domXml() const
{
    return "<widget class=\"QTermWidget\" name=\"termWidget\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>400</width>\n"
        "   <height>200</height>\n"
        "  </rect>\n"
        " </property>\n"
        " <property name=\"toolTip\" >\n"
        "  <string></string>\n"
        " </property>\n"
        " <property name=\"whatsThis\" >\n"
        "  <string></string>\n"
        " </property>\n"
        "</widget>\n";
}


QString QTermWidgetPlugin::includeFile() const
{
    return "qtermwidget.h";
}


Q_EXPORT_PLUGIN2(QTermWidgetPlugin, QTermWidgetPlugin)
