symbian {
  TARGET.UID3 = 0x200343E5

  #DEPLOYMENT.installer_header=0xA000D7CE

  vendorinfo = \
  "%{\"Dmitriev K. S.\"}" \
  ":\"Dmitriev K. S.\""

  my_deployment.pkg_prerules = vendorinfo
  DEPLOYMENT += my_deployment
}

SOURCES += main.cxx \
    machine.cxx \
    parser.cxx \
    lexer.cxx \
    centralwindow.cxx \
    tapeview.cxx \
    tracetablemodel.cxx \
    stateslistmodel.cxx \
    symbolslistmodel.cxx \
    settingsdlg.cxx \
    options.cxx \
    widgetgroup.cxx \
    messages.cxx \
    tapelistmodel.cxx \
    cmcbdelegate.cxx \
    hmdlistmodel.cxx \
    prdedtblmodel.cxx \
    minibrowser.cxx
HEADERS += machine.h \
    parser.h \
    lexer.h \
    centralwindow.h \
    tapeview.h \
    tracetablemodel.h \
    stateslistmodel.h \
    symbolslistmodel.h \
    settingsdlg.h \
    options.h \
    widgetgroup.h \
    messages.h \
    tapelistmodel.h \
    cmcbdelegate.h \
    hmdlistmodel.h \
    prdedtblmodel.h \
    minibrowser.h
FORMS += centralwindow.ui \
    settingsdlg.ui \
    about.ui
RESOURCES += resources/rsrc.qrc

VERSION = 1.0

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()





