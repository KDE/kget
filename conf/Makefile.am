noinst_LIBRARIES = libconf.a

libconf_a_SOURCES = dlgconnectionbase.ui dlgautomationbase.ui dlglimitsbase.ui \
                    dlgadvancedbase.ui dlgdirectoriesbase.ui dlgsystembase.ui \
                    dlgConnection.cpp dlgAutomation.cpp dlgLimits.cpp \
                    dlgAdvanced.cpp dlgDirectories.cpp dlgSystem.cpp \
                    settings.kcfgc

INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../views $(all_includes)

METASOURCES = AUTO

kde_kcfg_DATA = kget.kcfg
