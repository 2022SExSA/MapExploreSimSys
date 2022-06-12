TEMPLATE = subdirs

SUBDIRS += \
    Demos \
    MESSMain \
    MESSWidgets

Demos.depends = MESSWidgets
MESSMain.depends = MESSWidgets
